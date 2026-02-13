#include "bitmap.h"

#include <assert.h>

// CONSTRUCTORS AND DESTRUCTORS

BitMap::BitMap()
{
    _size = 0;
}

BitMap::BitMap(size_t size)
{
    _size = size;
    _bits.resize(size/word_s, 0);
    if (size % word_s != 0) {
        _bits.push_back(0);
    }
}

BitMap::BitMap(std::string bits)
{
    _size = bits.size();
    _bits.resize(bits.size()/word_s, 0);
    if (bits.size() % word_s != 0) {
        _bits.push_back(0);
    }

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
}

BitMap::~BitMap() {}

BitMap::word_t BitMap::getMask(size_t idx)
{
    size_t msb = word_s - 1;
    size_t bit = msb - (idx % word_s);
    return (word_t)1 << bit; // Mask with 1 in pos bit
}

// SINGLE BIT OPERATIONS
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
size_t BitMap::rank(size_t idx)
{
    ulong ans = 0;
    for (size_t i = 0; i < idx+1; i++) {
        if (get(i) == 1) {
            ans++;
        }
    }
    return ans;
}

size_t BitMap::select(size_t n, int8_t bit)
{
    size_t cnt = 0;
    for (size_t ans = 0; ans < _size; ans++) {
        if (get(ans) == bit) {
            cnt++;
            if (cnt == n)
                return ans;
        }
    }
    return -1;
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

