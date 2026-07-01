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
}

BitMap::~BitMap()
{
    cudaFree(d_att);
    free(_bits);
    cudaFree(d_bits);
    cudaFree(d_rankS);
}

// SINGLE BIT OPERATIONS
int8_t BitMap::get(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t word = idx / _word_s;
    bitIdx_t bit = (_word_s - 1) - (idx % _word_s); // msb - prev words
    word_t mask = (word_t)1 << bit; // Mask with 1 in pos bit

    return (_bits[word] & mask) ? 1 : 0;
}

__device__ int8_t BitMap::d_get(bitIdx_t idx)
{
    if (idx >= d_att->size)
        return -1;

    bitIdx_t word = idx / d_att->word_s;
    bitIdx_t bit = (d_att->word_s - 1) - (idx % d_att->word_s);
    word_t mask = (word_t)1 << bit;

    return (d_bits[word] & mask) ? 1 : 0;
}

// BITMAP OPERATIONS
// long BitMap::rank(bitIdx_t idx)
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
// long BitMap::select(bitIdx_t n)
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

