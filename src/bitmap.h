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
        typedef struct {
            size_t word_s; //!< Size in bits of word_t
            bitIdx_t size; //!< Size in bits of the bitmap
            bitIdx_t bitsPerBlk; //!< Number of bits per block of rankS (HAS TO BE MULTIPLE OF WORD_S)
            bool changedBitmap;
        } attributes;

        size_t _word_s = sizeof(word_t)*8;
        bitIdx_t _size; //!< Size in bits of the bitmap
        attributes *d_att;
        word_t *_bits; //!< Bit vector
        word_t *d_bits; //!< Bit vector
        uint32_t *d_rankS; //!< Rank helper structure
        int32_t *d_lazyRank; //!< Records bit changes

    // METHODS
    private:
        // @param idx Bit position between 0 and _size-1
        // @return Word mask for the bit in that position
        word_t getMask(bitIdx_t idx);
        __device__ word_t d_getMask(bitIdx_t idx);

    public:
        // CONSTRUCTORS AND DESTRUCTORS
        // @param size Size of bitmap in bits
        // @return Bitmap initialized to 0
        BitMap(bitIdx_t size, std::string fileName, bitIdx_t bitsPerRankBlk = 0);
        // @brief Creates a copy of bitmap
        BitMap(const BitMap& bitmap);
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        __device__ int8_t d_get(bitIdx_t idx);

        ~BitMap();

        // SINGLE BIT OPERATIONS
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        int8_t get(bitIdx_t idx);

        // BITMAP OPERATIONS
        // @brief Checks if there are pending changes for rankS
        // @return True if an updateRank is necessary
        bool rankNeedsUpdate();
        // @brief Updates rankS using the changes stored in lazyRank
        void updateRank();
        // @param idx Position between 0 and size()-1
        // @return Number of ones up to that idx (included)
        long rank(bitIdx_t idx);
        // @brief Get position of the nth 1
        // @return Position of the bit or -1 if not found
        long select(bitIdx_t n);

        // BASIC VECTOR OPERATIONS
        // @return The size of the bitmap in bits
        bitIdx_t size();
        // @return Bitmap as string of 0s and 1s
        std::string toString();
};

// CUDA KERNELS

