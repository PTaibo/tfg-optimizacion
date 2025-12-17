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
    private:
        // @param idx Bit position between 0 and _size-1
        // @return Word mask for the bit in that position
        word_t getMask(size_t idx);

    public:
        // CONSTRUCTORS AND DESTRUCTORS
        // @return Empty bitmap
        BitMap();
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
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        int8_t get(size_t idx);
        // @brief Sets bit to 1
        // @param idx Position between 0 and size() - 1
        // @return 0 if out of bounds (error)
        int8_t set(size_t idx);
        // @brief Sets bit to 0
        // @param idx Position between 0 and size() - 1
        // @return 0 if out of bounds (error)
        int8_t clear(size_t idx);
        // @brief Toggles bit
        // @param idx Position between 0 and size() - 1
        // @return New value of bit or -1 if out of bounds
        int8_t toggle(size_t idx);
        // @brief Appends a bit at the end of the bitmap
        // @param bit Value of the bit
        void push_back(int8_t bit);
        // @brief Removes a bit from the end of the bitmap
        void pop_back();

        // BASIC VECTOR OPERATIONS
        bool isEmpty();
        // @return The size of the bitmap in bits
        size_t size();
        // @brief Prints bitmap to standard output
        void print();
        // @return Bitmap as string of 0s and 1s
        std::string toString();
};
