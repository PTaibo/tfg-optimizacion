#include "bitmap.h"
#include "cuda_utils.h"

#include <assert.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#define RANKBLK (2) // Default number of words per rank block

// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap(bitIdx_t size, std::string fileName, bitIdx_t bitsPerRankBlk)
{
    // Initialize attributes (size, word_s, bitsPerBlk...)
    attributes d_tmp;
    _size = d_tmp.size = size;
    d_tmp.word_s = _word_s;
    d_tmp.bitsPerBlk = bitsPerRankBlk*d_tmp.word_s;
    if (!d_tmp.bitsPerBlk) {
        d_tmp.bitsPerBlk = RANKBLK*d_tmp.word_s;
    }
    d_tmp.changedBitmap = false;
    cudaMalloc(&d_att, sizeof(attributes));
    cudaMemcpy(d_att, &d_tmp, sizeof(attributes), cudaMemcpyHostToDevice);

    // Initialize bitmap
    size_t bytes = ( (_size + _word_s - 1) / d_tmp.word_s ) * sizeof(word_t); // Int ceiling division: (A + B - 1) / B
    _bits = (word_t*) malloc(bytes);
    HANDLE_ERROR(cudaMalloc(&d_bits, bytes));
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
        throw std::invalid_argument("Couldn't open bitmap file");
    file.read((char*)_bits, bytes);
    HANDLE_ERROR(cudaMemcpy(d_bits, _bits, bytes, cudaMemcpyHostToDevice));

    // Initialize rank support structures
    ulong ceiling_div = (_size + d_tmp.bitsPerBlk - 1) / d_tmp.bitsPerBlk;
    HANDLE_ERROR(cudaMalloc(&d_rankS, (ceiling_div + 1) * sizeof(uint32_t)));
    HANDLE_ERROR(cudaMemset(d_rankS, 0, (ceiling_div + 1) * sizeof(uint32_t)));
    HANDLE_ERROR(cudaMalloc(&d_lazyRank, (ceiling_div + 1) * sizeof(int32_t)));
    HANDLE_ERROR(cudaMemset(d_lazyRank, 0, (ceiling_div + 1) * sizeof(int32_t)));
}

// BitMap::BitMap(const BitMap& bitmap)
// {
//     _size = bitmap._size;
//     _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
//     _bitsPerBlk = bitmap._bitsPerBlk;
//     _rankS.assign(bitmap._rankS.begin(), bitmap._rankS.end());
//     _changedBitmap = bitmap._changedBitmap;
//     _lazyRank.assign(bitmap._lazyRank.begin(), bitmap._lazyRank.end());
// }

BitMap::~BitMap()
{
    cudaFree(d_att);
    free(_bits);
    cudaFree(d_bits);
    cudaFree(d_rankS);
    cudaFree(d_lazyRank);
}

// // SINGLE BIT OPERATIONS
BitMap::word_t BitMap::getMask(bitIdx_t idx)
{
    bitIdx_t msb = _word_s - 1;
    bitIdx_t bit = msb - (idx % _word_s);
    return (word_t)1 << bit; // Mask with 1 in pos bit
}

__device__ BitMap::word_t BitMap::d_getMask(bitIdx_t idx)
{
    bitIdx_t msb = d_att->word_s - 1;
    bitIdx_t bit = msb - (idx % d_att->word_s);
    return (word_t)1 << bit; // Mask with 1 in pos bit
}

int8_t BitMap::get(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t word = idx / _word_s;
    word_t mask = getMask(idx);
    return (_bits[word] & mask) ? 1 : 0;
}

__device__ int8_t BitMap::d_get(bitIdx_t idx)
{
    if (idx >= d_att->size)
        return -1;

    bitIdx_t word = idx / d_att->word_s;
    word_t mask = d_getMask(idx);
    return (d_bits[word] & mask) ? 1 : 0;
}

// int8_t BitMap::set(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return 0;
//
//     bitIdx_t word = idx / word_s;
//     word_t mask = getMask(idx);
//     _bits[word] = _bits[word] | mask;
//     _changedBitmap = true;
//     _lazyRank[(idx/_bitsPerBlk)+1]++;
//
//     return 1;
// }
//
// int8_t BitMap::clear(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return 0;
//
//     bitIdx_t word = idx / word_s;
//     word_t mask = ~getMask(idx);
//     _bits[word] = _bits[word] & mask;
//     _changedBitmap = true;
//     _lazyRank[(idx/_bitsPerBlk)+1]--;
//
//     return 1;
// }
//
// int8_t BitMap::toggle(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return -1;
//
//     bitIdx_t word = idx / word_s;
//     word_t mask = getMask(idx);
//     _bits[word] = _bits[word] ^ mask;
//     _changedBitmap = true;
//
//     if (_bits[word] & mask) {
//         _lazyRank[(idx/_bitsPerBlk)+1]++;
//         return 1;
//     }
//     else {
//         _lazyRank[(idx/_bitsPerBlk)+1]--;
//         return 0;
//     }
// }
//
// // BITMAP OPERATIONS
// void BitMap::updateRank()
// {
//     uint64_t acum = 0;
//     for (size_t i = 0; i < _rankS.size(); i++) {
//         acum += _lazyRank[i];
//         _lazyRank[i] = 0;
//         _rankS[i] += acum;
//     }
//     _changedBitmap = false;
// }
//
// bool BitMap::rankNeedsUpdate()
// {
//     return _changedBitmap;
// }
//
// long BitMap::wrdRank(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return -1;
//
//     bitIdx_t blkIdx = idx/_bitsPerBlk;
//     bitIdx_t ans = _rankS[blkIdx];
//     size_t fstWrd = blkIdx*(_bitsPerBlk/word_s);
//     size_t lstWrd = idx/word_s;
//     
//     for (size_t currWrd = fstWrd; currWrd < lstWrd; currWrd++) {
//         ans += POPCOUNT(_bits[currWrd]);
//     }
//
//     size_t bitPosInWrd = (idx+1) & (word_s - 1);
//     ans += POPCOUNT(_bits[lstWrd] >> (word_s - bitPosInWrd));
//
//     return ans;
// }
//
// long BitMap::rank(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return -1;
//
//     size_t blkIdx = (idx+1)/_bitsPerBlk; // NOTE: idx+1 por si idx es el último elmento del bloque
//     size_t ans = _rankS[blkIdx];
//     bitIdx_t currBit = _bitsPerBlk*blkIdx;
//     if (currBit == (idx+1))
//         return ans;
//
//     size_t wrd = (currBit + word_s - 1) / word_s; // NOTE: Integer ceiling division
//     if (wrd*word_s > idx) {
//         word_t mask = (~0);
//         mask >>= (currBit%word_s);
//         mask &= (word_t)(~0) << ( word_s - ((idx+1) % word_s) );
//         ans += POPCOUNT(_bits[wrd-1] & mask);
//
//         return ans;
//     }
//
//     if (currBit % word_s)
//         ans += POPCOUNT(_bits[wrd-1] << (currBit%word_s));
//
//     size_t lstWrd = (idx+1)/word_s;
//     for (; wrd < lstWrd; wrd++)
//         ans += POPCOUNT(_bits[wrd]);
//
//     currBit = wrd*word_s;
//     if ( wrd*word_s < idx+1 ) {
//         size_t shift = word_s - ((idx+1) % word_s);
//         ans += POPCOUNT(_bits[wrd] >> shift);
//     }
//
//     return ans;
// }
//
// long BitMap::select0(bitIdx_t n)
// {
//     if (n < 1 || _size - _rankS.back() < n)
//         return -1;
//
//     // Binary search in rank structure
//     int l = -1;
//     int r = _rankS.size();
//     while (r > l+1) {
//         int m = (l+r)/2;
//         if ((m*_bitsPerBlk) - _rankS[m] < n) {
//             l = m;
//         }
//         else {
//             r = m;
//         }
//     }
//     bitIdx_t currBit = _bitsPerBlk*l;
//     bitIdx_t cnt = currBit - _rankS[l];
//     if (cnt == n)
//         return currBit-1;
//     
//     size_t wrd = (currBit + word_s - 1) / word_s;
//     for (; cnt < n; wrd++) {
//         cnt += word_s - POPCOUNT(_bits[wrd]);
//     }
//     cnt -= word_s - POPCOUNT(_bits[--wrd]);
//
//     for (currBit = wrd*word_s; cnt < n; currBit++) {
//         if (get(currBit) == 0) {
//             cnt++;
//         }
//     }    
//
//     return currBit-1;
// }
//
// long BitMap::select1(bitIdx_t n)
// {
//     if (n < 1 || _rankS.back() < n)
//         return -1;
//
//     // Binary search in rank structure
//     int l = -1;
//     int r = _rankS.size();
//     while (r > l+1) {
//         int m = (l+r)/2;
//         if (_rankS[m] < n) {
//             l = m;
//         }
//         else {
//             r = m;
//         }
//     }
//     bitIdx_t cnt = _rankS[l];
//     bitIdx_t currBit = _bitsPerBlk*l;
//     if (cnt == n)
//         return currBit-1;
//
//     size_t wrd = (currBit + word_s - 1) / word_s;
//     for (; cnt < n; wrd++) {
//         cnt += POPCOUNT(_bits[wrd]);
//     }
//     cnt -= POPCOUNT(_bits[--wrd]);
//
//     for (currBit = wrd*word_s; cnt < n; currBit++) {
//         if (get(currBit) == 1) {
//             cnt++;
//         }
//     }    
//
//     return currBit-1;
// }
//
// VECTOR OPERATIONS
bitIdx_t BitMap::size()
{
    return _size;
}

std::string BitMap::toString()
{
    std::string bitmap (_size, '0');
    for (bitIdx_t i = 0; i < _size; i++) {
        bitmap[i] = get(i) + '0';
    }
    return bitmap;
}

// __global__ void getBit(bitIdx_t idx, int8_t *bit, BitMap *bmap)
// {
//     *bit = bmap->d_get(idx);
// }

