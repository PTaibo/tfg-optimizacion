#include "bitmap.h"

#include <assert.h>

#define RANKBLK (1) // Default number of words per rank block

// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap(bitIdx_t size, bitIdx_t wordsPerRankBlk)
{
    _size = size;
    // Int ceiling division: (A + B - 1) / B
    _bits.resize((_size + word_s - 1) / word_s, 0); 
    _bitsPerBlk = wordsPerRankBlk*word_s;
    if (!_bitsPerBlk) {
        _bitsPerBlk = RANKBLK*word_s;
    }
    ulong ceiling_div = (_size + _bitsPerBlk - 1) / _bitsPerBlk;
    _rankS.resize(ceiling_div + 1, 0);
    _changedBitmap = false;
    _lazyRank.resize(ceiling_div + 1, 0);
}

BitMap::BitMap(std::string bits, bitIdx_t wordsPerRankBlk)
{
    _size = bits.size();
    _bits.resize((bits.size() + word_s - 1) / word_s, 0);
    _bitsPerBlk = wordsPerRankBlk*word_s;
    if (!_bitsPerBlk) {
        _bitsPerBlk = RANKBLK*word_s;
    }
    ulong ceiling_div = (bits.size() + _bitsPerBlk - 1) / _bitsPerBlk;
    _rankS.resize(ceiling_div + 1, 0);
    _changedBitmap = false;
    _lazyRank.resize(ceiling_div + 1, 0);

    for(bitIdx_t i = 0; i < bits.size(); i++) {
        if (bits[i] == '1') {
            set(i);
        }
    }
}

BitMap::BitMap(const BitMap& bitmap)
{
    _size = bitmap._size;
    _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
    _bitsPerBlk = bitmap._bitsPerBlk;
    _rankS.assign(bitmap._rankS.begin(), bitmap._rankS.end());
    _changedBitmap = bitmap._changedBitmap;
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
        return -1;

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
        return -1;

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

long BitMap::wrdRank(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    bitIdx_t blkIdx = idx/_bitsPerBlk;
    bitIdx_t ans = _rankS[blkIdx];
    size_t fstWrd = blkIdx*(_bitsPerBlk/word_s);
    size_t lstWrd = idx/word_s;
    
    for (size_t currWrd = fstWrd; currWrd < lstWrd; currWrd++) {
        ans += POPCOUNT(_bits[currWrd]);
    }

    size_t bitPosInWrd = (idx+1) & (word_s - 1);
    ans += POPCOUNT(_bits[lstWrd] >> (word_s - bitPosInWrd));

    return ans;
}

long BitMap::rank(bitIdx_t idx)
{
    if (idx >= _size)
        return -1;

    size_t blkIdx = (idx+1)/_bitsPerBlk; // NOTE: idx+1 por si idx es el último elmento del bloque
    size_t ans = _rankS[blkIdx];
    bitIdx_t currBit = _bitsPerBlk*blkIdx;
    if (currBit == (idx+1))
        return ans;

    size_t wrd = (currBit + word_s - 1) / word_s; // NOTE: Integer ceiling division
    if (wrd*word_s > idx) {
        word_t mask = (~0);
        mask >>= (currBit%word_s);
        mask &= (word_t)(~0) << ( word_s - ((idx+1) % word_s) );
        ans += POPCOUNT(_bits[wrd-1] & mask);

        return ans;
    }

    if (currBit % word_s)
        ans += POPCOUNT(_bits[wrd-1] << (currBit%word_s));

    size_t lstWrd = (idx+1)/word_s;
    for (; wrd < lstWrd; wrd++)
        ans += POPCOUNT(_bits[wrd]);

    currBit = wrd*word_s;
    if ( wrd*word_s < idx+1 ) {
        size_t shift = word_s - ((idx+1) % word_s);
        ans += POPCOUNT(_bits[wrd] >> shift);
    }

    return ans;
}

long BitMap::select0(bitIdx_t n)
{
    if (n < 1 || _size - _rankS.back() < n)
        return -1;

    // Binary search in rank structure
    int l = -1;
    int r = _rankS.size();
    while (r > l+1) {
        int m = (l+r)/2;
        if ((m*_bitsPerBlk) - _rankS[m] < n) {
            l = m;
        }
        else {
            r = m;
        }
    }
    bitIdx_t currBit = _bitsPerBlk*l;
    bitIdx_t cnt = currBit - _rankS[l];
    if (cnt == n)
        return currBit-1;
    
    size_t wrd = (currBit + word_s - 1) / word_s;
    for (; cnt < n; wrd++) {
        cnt += word_s - POPCOUNT(_bits[wrd]);
    }
    cnt -= word_s - POPCOUNT(_bits[--wrd]);

    for (currBit = wrd*word_s; cnt < n; currBit++) {
        if (get(currBit) == 0) {
            cnt++;
        }
    }    

    return currBit-1;
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

std::string BitMap::toString()
{
    std::string bitmap (_size, '0');
    for (bitIdx_t i = 0; i < _size; i++) {
        bitmap[i] = get(i) + '0';
    }
    return bitmap;
}

