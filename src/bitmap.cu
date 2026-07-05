#include "bitmap.h"
#include "cuda_utils.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

#define RANKBLK (2) // Default number of words per rank block

// CONSTRUCTORS AND DESTRUCTORS
BitMap::BitMap(bitIdx_t size, std::string fileName, bitIdx_t bitsPerRankBlk)
{
    // Initialize attributes (size, word_s, bitsPerBlk...)
    h_size = size;
    cudaMalloc(&d_word_s, sizeof(size_t));
    cudaMalloc(&d_size, sizeof(bitIdx_t));
    cudaMemcpy(d_word_s, &h_word_s, sizeof(size_t), cudaMemcpyHostToDevice);
    cudaMemcpy(d_size, &h_size, sizeof(bitIdx_t), cudaMemcpyHostToDevice);

    // Initialize bitmap
    size_t bytes = ( (h_size + h_word_s - 1) / h_word_s ) * sizeof(word_t); // Int ceiling division: (A + B - 1) / B

    h_bits = (word_t*) malloc(bytes);
    std::ifstream file(fileName, std::ios::binary);
    if (!file)
        throw std::invalid_argument("Couldn't open bitmap file");
    file.read((char*)h_bits, bytes);

    HANDLE_ERROR(cudaMalloc(&d_bits, bytes));
    HANDLE_ERROR(cudaMemcpy(d_bits, h_bits, bytes, cudaMemcpyHostToDevice));
}

BitMap::~BitMap()
{
    cudaFree(d_word_s);
    cudaFree(d_size);
    free(h_bits);
    cudaFree(d_bits);
}

// SINGLE BIT OPERATIONS
int8_t BitMap::get(bitIdx_t idx)
{
    if (idx >= h_size)
        return -1;

    bitIdx_t word = idx / h_word_s;
    bitIdx_t bit = (h_word_s - 1) - (idx % h_word_s); // msb - prev words
    word_t mask = (word_t)1 << bit; // Mask with 1 in pos bit

    return (h_bits[word] & mask) ? 1 : 0;
}

__device__ int8_t BitMap::d_get(bitIdx_t idx)
{
    if (idx >= *d_size)
        return -1;

    bitIdx_t word = idx / *d_word_s;
    bitIdx_t bit = (*d_word_s - 1) - (idx % *d_word_s);
    word_t mask = (word_t)1 << bit;

    return (d_bits[word] & mask) ? 1 : 0;
}

// BITMAP OPERATIONS
// long BitMap::rank(bitIdx_t idx)
// {
//     if (idx >= _size)
//         return -1;
//
//     bitIdx_t blkIdx = idx/_bitsPerBlk;
//     bitIdx_t ans = _rankS[blkIdx];
//     size_t fstWrd = blkIdx*(_bitsPerBlk/word_s);
//     size_t lstWrd = idx/word_s;
//     
//     for (size_t currWrd = fstWrd; currWrd < lstWrd; currWrd++) {
//         ans += POPCOUNT(_bits[currWrd]);
//     }
//
//     size_t bitPosInWrd = (idx+1) & (word_s - 1);
//     ans += POPCOUNT(_bits[lstWrd] >> (word_s - bitPosInWrd));
//
//     return ans;
// }

// VECTOR OPERATIONS
bitIdx_t BitMap::size()
{
    return h_size;
}

// UTILS
bitIdx_t BitMap::size()
{
    return h_size;
}

std::string BitMap::toString()
{
    std::string bitmap (h_size, '0');
    for (bitIdx_t i = 0; i < h_size; i++) {
        bitmap[i] = get(i) + '0';
    }
    return bitmap;
}

