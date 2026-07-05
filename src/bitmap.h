#pragma once

#include <cstdint>
#include <uchar.h>
#include <vector>
#include <string>

#define POPCOUNT(x) __builtin_popcountll(x)
using bitIdx_t = size_t;

/// @brief Basic implementation of a bitmap
class BitMap
{
    // ATTRIBUTES
    private:
        using word_t = uint32_t; //!< Type used for bit vector

        size_t h_word_s = sizeof(word_t)*8;
        size_t *d_word_s;
        bitIdx_t h_size; //!< Size in bits of the bitmap (CPU)
        bitIdx_t *d_size; //!< Size in bits of the bitmap (GPU)
        word_t *h_bits; //!< Bit vector
        word_t *d_bits; //!< Bit vector
        BitMap *d_bmap; //!< GPU copy of the bitmap

    // METHODS
    public:
        // CONSTRUCTORS AND DESTRUCTORS
        // @param size Size of bitmap in bits
        // @return Bitmap initialized to 0
        BitMap(bitIdx_t size, std::string fileName, bitIdx_t bitsPerRankBlk = 0);

        ~BitMap();

        // SINGLE BIT OPERATIONS
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        int8_t get(bitIdx_t idx);
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        __device__ int8_t d_get(bitIdx_t idx);

        // BITMAP OPERATIONS
        // @param idx Position between 0 and size()-1
        // @return Number of ones up to that idx (included)
        long rank(bitIdx_t idx);
        // @brief Get position of the nth 1
        // @return Position of the bit or -1 if not found
        long select(bitIdx_t n);

        // UTILS
        // @return The size of the bitmap in bits (CPU)
        bitIdx_t size();
        // @param idx Position between 0 and bits.size() - 1
        // @return Word_t element in that position of the array
        __device__ word_t bits(size_t idx);
        // @return Bitmap as string of 0s and 1s
        std::string toString();
};

