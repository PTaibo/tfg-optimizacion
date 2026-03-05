#include "bitmap.h"

#include <assert.h>

#define RANKBLK (600) // Default rank block size

// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap(size_t size, size_t rankBlkSize)
{
    _size = size;
    // Int ceiling division: (A + B - 1) / B
    _bits.resize((_size + word_s - 1) / word_s, 0); 
    _rankBlk = rankBlkSize;
    if (!_rankBlk) {
        _rankBlk = RANKBLK;
    }
    ulong ceiling_div = (_size + _rankBlk - 1) / _rankBlk;
    _rankS.resize(ceiling_div + 1, 0);
    _changedBitmap = false;
    _lazyRank.resize(ceiling_div + 1, 0);
}

BitMap::BitMap(std::string bits, size_t rankBlkSize)
{
    _size = bits.size();
    _bits.resize((bits.size() + word_s - 1) / word_s, 0);
    _rankBlk = rankBlkSize;
    if (!_rankBlk) {
        _rankBlk = RANKBLK;
    }
    ulong ceiling_div = (bits.size() + _rankBlk - 1) / _rankBlk;
    _rankS.resize(ceiling_div + 1, 0);
    _changedBitmap = false;
    _lazyRank.resize(ceiling_div + 1, 0);

    for(size_t i = 0; i < bits.size(); i++) {
        if (bits[i] == '1') {
            set(i);
        }
    }
}

BitMap::BitMap(const BitMap& bitmap)
{
    _size = bitmap._size;
    _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
    _rankBlk = bitmap._rankBlk;
    _rankS.assign(bitmap._rankS.begin(), bitmap._rankS.end());
    _changedBitmap = bitmap._changedBitmap;
    _lazyRank.assign(bitmap._lazyRank.begin(), bitmap._lazyRank.end());
}

BitMap::~BitMap() {}

// SINGLE BIT OPERATIONS
BitMap::word_t BitMap::getMask(size_t idx)
{
    size_t msb = word_s - 1;
    size_t bit = msb - (idx % word_s);
    return (word_t)1 << bit; // Mask with 1 in pos bit
}

int8_t BitMap::get(size_t idx)
{
    if (idx >= _size)
        return -1;

    size_t word = idx / word_s;
    word_t mask = getMask(idx);
    return (_bits[word] & mask) ? 1 : 0;
}

int8_t BitMap::set(size_t idx)
{
    if (idx >= _size)
        return 0;

    size_t word = idx / word_s;
    word_t mask = getMask(idx);
    _bits[word] = _bits[word] | mask;
    _changedBitmap = true;
    _lazyRank[(idx/_rankBlk)+1]++;

    return 1;
}

int8_t BitMap::clear(size_t idx)
{
    if (idx >= _size)
        return 0;

    size_t word = idx / word_s;
    word_t mask = ~getMask(idx);
    _bits[word] = _bits[word] & mask;
    _changedBitmap = true;
    _lazyRank[(idx/_rankBlk)+1]--;

    return 1;
}

int8_t BitMap::toggle(size_t idx)
{
    if (idx >= _size)
        return -1;

    size_t word = idx / word_s;
    word_t mask = getMask(idx);
    _bits[word] = _bits[word] ^ mask;
    _changedBitmap = true;

    if (_bits[word] & mask) {
        _lazyRank[(idx/_rankBlk)+1]++;
        return 1;
    }
    else {
        _lazyRank[(idx/_rankBlk)+1]--;
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

long BitMap::wrd_rank(size_t idx)
{
    if (idx >= _size)
        return -1;

    if (_changedBitmap)
        updateRank();

    size_t blkIdx = idx/_rankBlk;
    size_t ans = _rankS[blkIdx];
    size_t fstWrd = blkIdx*(_rankBlk/word_s);
    size_t lstWrd = (idx+1)/word_s;
    
    for (size_t currWrd = fstWrd; currWrd < lstWrd; currWrd++) {
        ans += POPCOUNT(_bits[currWrd]);
    }

    for (size_t i = lstWrd*word_s; i < idx+1; i++) {
        if (get(i) == 1) {
            ans++;
        }
    }

    return ans;
}

long BitMap::rank(size_t idx)
{
    if (idx >= _size)
        return -1;

    if (_changedBitmap)
        updateRank();

    size_t blkIdx = (idx+1)/_rankBlk; // NOTE: idx+1 por si idx es el último elmento del bloque
    size_t ans = _rankS[blkIdx];
    size_t currBit = _rankBlk*blkIdx;

    size_t fstWrd = (currBit + word_s - 1) / word_s; // NOTE: Integer ceiling division
    if (fstWrd*word_s < idx+1) {
        for (; currBit < fstWrd*word_s; currBit++) {
            if (get(currBit) == 1)
                ans++;
        }

        size_t lstWrd = (idx+1)/word_s;
        for (size_t currWrd = fstWrd; currWrd < lstWrd; currWrd++) {
            ans += POPCOUNT(_bits[currWrd]);
        }
        currBit = lstWrd*word_s;
    }

    for (size_t i = currBit; i < idx+1; i++) {
        if (get(i) == 1)
            ans++;
    }

    return ans;
}

long BitMap::select0(size_t n)
{
    if (_changedBitmap)
        updateRank();

    if (n < 1 || _size - _rankS.back() < n) {
        return -1;
    }

    size_t cnt = 0;
    size_t i, pcount;
    for (i = 0; i < _bits.size(); i++) {
        pcount = word_s - POPCOUNT(_bits[i]);
        if (cnt + pcount >= n) {
            break;
        }
        cnt += pcount;
    }
    for (size_t ans = i * word_s; ans < _size; ans++) {
        if (get(ans) == 0) {
            cnt++;
            if (cnt == n)
                return ans;
        }
    }
    return -1;
}

long BitMap::select1(size_t n)
{
    if (_changedBitmap)
        updateRank();

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

    size_t cnt = _rankS[l];
    size_t currBit = _rankBlk*l;
    size_t wrd = (currBit + word_s - 1) / word_s;
    for (; currBit < wrd*word_s; currBit++) {
        if (get(currBit) == 1) {
            cnt++;
            if (cnt == n)
                return currBit;
        }
    }

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
bool BitMap::isEmpty()
{
    return _size == 0;
}
size_t BitMap::size()
{
    return _size;
}

std::string BitMap::toString()
{
    std::string bitmap (_size, '0');
    for (size_t i = 0; i < _size; i++) {
        bitmap[i] = get(i) + '0';
    }
    return bitmap;
}

