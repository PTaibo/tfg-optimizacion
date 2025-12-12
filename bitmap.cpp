#include "bitmap.h"

#include <iostream>
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

    for(int i = 0; i < bits.size(); i++) {
        if (bits[i] == 1) {
            setBit(i);
        }
    }
}

BitMap::BitMap(const BitMap& bitmap)
{
    _size = bitmap._size;
    _bits.assign(bitmap._bits.begin(), bitmap._bits.end());
}

BitMap::~BitMap() {}

// SINGLE BIT OPERATIONS
int8_t BitMap::getBit(size_t idx)
{
    size_t el = idx / word_s;
    size_t bit = word_s - (idx % word_s);
    word_t mask = 1 << ((word_s-1) - bit); // Mask with 1 in pos bit

    return (_bits[el] & mask) ? 1 : 0;
}

void BitMap::setBit(size_t idx)
{
    size_t word = idx / word_s;
    size_t bit = word_s - (idx % word_s);
    word_t mask = 1 << ((word_s-1) - bit); // Mask with 1 in pos bit
    _bits[word] = _bits[word] | mask;
}

void BitMap::clearBit(size_t idx)
{
    size_t word = idx / word_s;
    size_t bit = word_s - (idx % word_s);
    word_t mask = 1 << ((word_s-1) - bit); // Mask with 1 in pos bit
    mask = ~mask;
    _bits[word] = _bits[word] & mask;
}

int8_t BitMap::toggleBit(size_t idx)
{
    if (idx >= _size)
        return -1;

    size_t word = idx / word_s;
    size_t bit = word_s - (idx % word_s);
    word_t mask = 1 << ((word_s-1) - bit); // Mask with 1 in pos bit
    _bits[word] = _bits[word] ^ mask;
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
    for (int i = 0; i < _bits.size()-1; i++) {
        std::cout << _bits[i];
    }
    for (int i = (_bits.size()-1)*word_s; i < _size; i++) {
        std::cout << getBit(i);
    }
    std::cout << "\n";
}

