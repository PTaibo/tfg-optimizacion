#include "bitmap.h"

#include <assert.h>
#include <cmath>



// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap()
{
    _size = 0;
}

BitMap::BitMap(size_t size, size_t rankBlkSize)
{
    _size = size;
    // Int ceiling division: (A + B - 1) / B
    _bits.resize((_size + word_s - 1) / word_s, 0); 
    _rankBlk = rankBlkSize;
    if (!_rankBlk) {
        _rankBlk = log2(_size)/2;
    }
    ulong ceiling_div = (_size + _rankBlk - 1) / _rankBlk;
    _rankS.resize(ceiling_div + 1, 0);
}

BitMap::BitMap(std::string bits, size_t rankBlkSize)
{
    _size = bits.size();
    _bits.resize((bits.size() + word_s - 1) / word_s, 0);
    _rankBlk = rankBlkSize;
    if (!_rankBlk) {
        _rankBlk = log2(bits.size())/2;
    }
    ulong ceiling_div = (bits.size() + _rankBlk - 1) / _rankBlk;
    _rankS.resize(ceiling_div + 1, 0);

    for(size_t i = 0; i < bits.size(); i++) {
        if (bits[i] == '1') {
            set(i);
            _rankS[(i/_rankBlk)+1]++;
        }
    }
    for (size_t i = 1; i < _rankS.size(); i++) {
        _rankS[i] += _rankS[i-1];
    }
}

BitMap::BitMap(const BitMap& bitmap)
{
    _size = bitmap._size;
    _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
    _rankBlk = bitmap._rankBlk;
    _rankS.assign(bitmap._rankS.begin(), bitmap._rankS.end());
}

void BitMap::createRankS()
{
    ulong ceiling_div = (_size + _rankBlk - 1) / _rankBlk;
    _rankS.clear();
    _rankS.resize(ceiling_div + 1, 0);

    for(size_t i = 0; i < _size; i++) {
        if (get(i) == 1) {
            _rankS[(i/_rankBlk)+1]++;
        }
    }
    for (size_t i = 1; i < _rankS.size(); i++) {
        _rankS[i] += _rankS[i-1];
    }
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
    return 1;
}

int8_t BitMap::clear(size_t idx)
{
    if (idx >= _size)
        return 0;

    size_t word = idx / word_s;
    word_t mask = ~getMask(idx);
    _bits[word] = _bits[word] & mask;
    return 1;
}

int8_t BitMap::toggle(size_t idx)
{
    if (idx >= _size)
        return -1;

    size_t word = idx / word_s;
    word_t mask = getMask(idx);
    _bits[word] = _bits[word] ^ mask;
    return (_bits[word] & mask) ? 1 : 0;
}

void BitMap::push_back(int8_t bit)
{
    _size++;
    if (_size % word_s == 1) {
        _bits.push_back(0);
    }
    if (bit == 1) {
        set(_size-1);
    } else {
        clear(_size-1);
    }
}

void BitMap::pop_back()
{
    _size--;
    if (_size % word_s == 0) {
        _bits.pop_back();
    }
}

// BITMAP OPERATIONS
long BitMap::rank(size_t idx)
{
    if (idx >= _size)
        return -1;

    size_t ans = _rankS[idx/_rankBlk];
    for (size_t i = idx - (idx%_rankBlk); i < idx+1; i++) {
        if (get(i) == 1) {
            ans++;
        }
    }    

    return ans;
}

long BitMap::select(size_t n, int8_t bit)
{
    if (n < 1 || _rankS.back() < n) {
        return -1;
    }

    size_t cnt = 0;
    size_t i, pcount;
    for (i = 0; i < _bits.size(); i++) {
        pcount = POPCOUNT(_bits[i]);
        if (cnt + pcount >= n) {
            break;
        }
        cnt += pcount;
    }
    for (size_t ans = i * word_s; ans < _size; ans++) {
        if (get(ans) == bit) {
            cnt++;
            if (cnt == n)
                return ans;
        }
    }
    return -1;
}

long BitMap::select1(size_t n)
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

    size_t i;
    size_t cnt = _rankS[l];
    for (i = l * _rankBlk; cnt < n; i++) {
        if (get(i) == 1) {
            cnt++;
        }
    }    

    return i-1;
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

