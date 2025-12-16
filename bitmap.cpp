#include "bitmap.h"

#include <assert.h>

// CONSTRUCTORS AND DESTRUCTORS

BitMap::BitMap(size_t size)
{
    _size = size;
    _bits.resize(size/8, 0);
    if (size % word_s != 0) {
        _bits.push_back(0);
    }
}

BitMap::BitMap(std::string bits)
{
    _size = bits.size();
    _bits.resize(bits.size()/8, 0);
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
    return 1 << bit; // Mask with 1 in pos bit
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
    word_t mask = getMask(idx);
    mask = ~mask;
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

// VECTOR OPERATIONS
bool BitMap::isEmpty()
{
    return _size == 0;
}
size_t BitMap::size()
{
    return _size;
}

void BitMap::print()
{
    for (size_t i = 0; i < _size; i++) {
        printf("%d", get(i));
    }
    printf("\n");
}

