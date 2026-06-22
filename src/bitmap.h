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
        const size_t word_s = sizeof(word_t)*8; //!< Size in bits of word_t

        bitIdx_t _size = 0; //!< Size in bits of the bitmap
        std::vector<word_t> _bits; //!< Bit vector
        bitIdx_t _bitsPerBlk = 0; //!< Number of bits per block of rankS (HAS TO BE MULTIPLE OF WORD_S)
        std::vector<uint32_t> _rankS; //!< Rank helper structure
        bool _changedBitmap = false;
        std::vector<int32_t> _lazyRank; //!< Records bit changes

    // METHODS
    private:
        // @param idx Bit position between 0 and _size-1
        // @return Word mask for the bit in that position
        word_t getMask(bitIdx_t idx);

    public:
        // @brief Checks if there are pending changes for rankS
        // @return True if an updateRank is necessary
        bool rankNeedsUpdate();
        // @brief Updates rankS using the changes stored in lazyRank
        void updateRank();
        // CONSTRUCTORS AND DESTRUCTORS
        // @param size Size of bitmap in bits
        // @return Bitmap initialized to 0
        BitMap(bitIdx_t size, bitIdx_t bitsPerRankBlk = 0);
        // @brief Converts the string into a bitmap
        // @param bits String of ASCII 0s and 1s
        BitMap(std::string bits, bitIdx_t bitsPerRankBlk = 0);
        // @brief Creates a copy of bitmap
        BitMap(const BitMap& bitmap);

        ~BitMap();

        // SINGLE BIT OPERATIONS
        // @param idx Position between 0 and size()-1
        // @return Value of bit or -1 if out of bounds
        int8_t get(bitIdx_t idx);
        // @brief Sets bit to 1. Necessary to update rankS
        // @param idx Position between 0 and size()-1
        // @return 0 if out of bounds (error)
        int8_t set(bitIdx_t idx);
        // @brief Sets bit to 0. Necessary to update rankS
        // @param idx Position between 0 and size()-1
        // @return 0 if out of bounds (error)
        int8_t clear(bitIdx_t idx);

        // BITMAP OPERATIONS
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
