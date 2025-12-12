#pragma once

#include <cstdint>
#include <uchar.h>
#include <vector>
#include <string>

/// @brief Basic implementation of a bitmap

class BitMap
{
    // ATTRIBUTES
    private:
        using word_t = uint8_t; //!< Type used for bit vector
        const size_t word_s = 8; //!< Size in bits of word_t

        size_t _size = 0; //!< Size in bits of the bitmap
        std::vector<word_t> _bits; //!< Bit vector

    // METHODS
    public:
        // CONSTRUCTORS AND DESTRUCTORS
        // @param size Size of bitmap in bits
        // @return Bitmap initialized to 0
        BitMap(size_t size);
        // @brief Converts the string into a bitmap
        // @param bits String of 0s and 1s
        BitMap(std::string bits);
        // @brief Creates a copy of bitmap
        BitMap(const BitMap& bitmap);

        ~BitMap();

        // SINGLE BIT OPERATIONS
        // @param Position between 0 and size() - 1
        // @return Value of bit
        int8_t getBit(size_t idx);
        // @brief Sets bit to 1
        // @param Position between 0 and size() - 1
        void setBit(size_t idx);
        // @brief Sets bit to 0
        // @param Position between 0 and size() - 1
        void clearBit(size_t idx);
        // @brief Toggles bit
        // @brief Position between 0 and size() - 1
        // @return New value of bit
        int8_t toggleBit(size_t idx);

        // VECTOR OPERATIONS
        bool isEmpty();
        // @return The size of the bitmap in bits
        size_t size();
        // @brief Prints bitmap to standard output
        void print();
        // void clear();
};
