#include "bitmap.h"

#include <assert.h>
#include <immintrin.h>

#define RANKBLK (20) // Default number of words per rank block

// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap(bitIdx_t size, bitIdx_t wordsPerRankBlk)
{
    _size = size;
    if (!wordsPerRankBlk) {
        _bitsPerBlk = RANKBLK*word_s;
    }
    else {
        _bitsPerBlk = wordsPerRankBlk*word_s;
    }
    ulong rankBlkNum = (_size + _bitsPerBlk - 1) / _bitsPerBlk;
    _changedBitmap = false;

    // Int ceiling division: (A + B - 1) / B
    _bits.resize((_size + word_s - 1) / word_s, 0); 
    _rankS.resize(rankBlkNum + 1, 0);
    _lazyRank.resize(rankBlkNum + 1, 0);
}

BitMap::BitMap(std::string bits, bitIdx_t wordsPerRankBlk)
{
    _size = bits.size();
    if (!wordsPerRankBlk)
        _bitsPerBlk = RANKBLK*word_s;
    else
        _bitsPerBlk = wordsPerRankBlk*word_s;
    ulong rankBlkNum = (bits.size() + _bitsPerBlk - 1) / _bitsPerBlk;
    _changedBitmap = false;

    _bits.resize((bits.size() + word_s - 1) / word_s, 0);
    _rankS.resize(rankBlkNum + 1, 0);
    _lazyRank.resize(rankBlkNum + 1, 0);

    for(bitIdx_t i = 0; i < bits.size(); i++) {
        if (bits[i] == '1') {
            set(i);
        }
    }
}

BitMap::BitMap(const BitMap& bitmap)
{
    _size = bitmap._size;
    _bitsPerBlk = bitmap._bitsPerBlk;
    _changedBitmap = bitmap._changedBitmap;

    _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
    _rankS.assign(bitmap._rankS.begin(), bitmap._rankS.end());
    _lazyRank.assign(bitmap._lazyRank.begin(), bitmap._lazyRank.end());
}

BitMap::~BitMap() {}

// SINGLE BIT OPERATIONS
BitMap::word_t BitMap::getMask(bitIdx_t idx)
{
    bitIdx_t msb = word_s - 1;
    bitIdx_t bit = msb - (idx % word_s);
    return (word_t)1 << bit; // Mask with 1 in pos bit
}

int8_t BitMap::get(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t word = idx / word_s;
    word_t mask = getMask(idx);
    return (_bits[word] & mask) ? 1 : 0;
}

int8_t BitMap::set(bitIdx_t idx)
{
    if (idx >= _size)
        return 0;

    bitIdx_t word = idx / word_s;
    word_t mask = getMask(idx);
    _bits[word] = _bits[word] | mask;
    _changedBitmap = true;
    _lazyRank[(idx/_bitsPerBlk)+1]++;

    return 1;
}

int8_t BitMap::clear(bitIdx_t idx)
{
    if (idx >= _size)
        return 0;

    bitIdx_t word = idx / word_s;
    word_t mask = ~getMask(idx);
    _bits[word] = _bits[word] & mask;
    _changedBitmap = true;
    _lazyRank[(idx/_bitsPerBlk)+1]--;

    return 1;
}

int8_t BitMap::toggle(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t word = idx / word_s;
    word_t mask = getMask(idx);
    _bits[word] = _bits[word] ^ mask;
    _changedBitmap = true;

    if (_bits[word] & mask) {
        _lazyRank[(idx/_bitsPerBlk)+1]++;
        return 1;
    }
    else {
        _lazyRank[(idx/_bitsPerBlk)+1]--;
        return 0;
    }
}

// BITMAP OPERATIONS
void BitMap::updateRank()
{
    uint64_t acum = 0;
    for (size_t i = 0; i < _rankS.size(); i++) {
        acum += _lazyRank[i];
        _lazyRank[i] = 0;
        _rankS[i] += acum;
    }
    _changedBitmap = false;
}

bool BitMap::rankNeedsUpdate()
{
    return _changedBitmap;
}

long BitMap::rank(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t blkIdx = idx/_bitsPerBlk;
    bitIdx_t ans = _rankS[blkIdx];
    size_t fstWrd = blkIdx*(_bitsPerBlk/word_s);
    size_t lstWrd = idx/word_s;
    size_t currWrd = fstWrd;
    
    __m256i v_bits, v_pcount;
    __m256i v_sum = _mm256_setzero_si256();
    for (; currWrd+7 < lstWrd; currWrd += 8) {
        v_bits = _mm256_set_epi32(_bits[currWrd+7], _bits[currWrd+6],
                                  _bits[currWrd+5], _bits[currWrd+4],
                                  _bits[currWrd+3], _bits[currWrd+2],
                                  _bits[currWrd+1], _bits[currWrd]);
        v_pcount = _mm256_popcnt_epi32(v_bits);
        v_sum = _mm256_add_epi32(v_sum, v_pcount);
    }
    alignas(32) uint32_t sum[8];
    _mm256_store_si256((__m256i*)sum, v_sum);
    for (size_t i = 0; i < 8; i++) {
        ans += sum[i];
    }

    for (; currWrd < lstWrd; currWrd++) {
        ans += POPCOUNT(_bits[currWrd]);
    }
    size_t bitPosInWrd = (idx+1) & (word_s - 1);
    ans += POPCOUNT(_bits[lstWrd] >> (word_s - bitPosInWrd));

    return ans;
}

long BitMap::select1(bitIdx_t n)
{
    if (n < 1 || _rankS.back() < n)
        return -1;

    // Binary search in rank structure
    int l = -1;
    int r = _rankS.size();
    while (r > l+1) {
        int m = (l+r)/2;
        if (_rankS[m] < n) {
            l = m;
        }
        else {
            r = m;
        }
    }
    bitIdx_t cnt = _rankS[l];
    bitIdx_t currBit = _bitsPerBlk*l;
    if (cnt == n)
        return currBit-1;

    size_t wrd = (currBit + word_s - 1) / word_s;
    for (; cnt < n; wrd++) {
        cnt += POPCOUNT(_bits[wrd]);
    }
    cnt -= POPCOUNT(_bits[--wrd]);

    for (currBit = wrd*word_s; cnt < n; currBit++) {
        if (get(currBit) == 1) {
            cnt++;
        }
    }    

    return currBit-1;
}

// VECTOR OPERATIONS
bitIdx_t BitMap::size()
{
    return _size;
}

