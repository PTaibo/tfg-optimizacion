#include "bitmap.h"
#include "cuda_utils.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thrust/reduce.h>
#include <thrust/execution_policy.h>

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

    HANDLE_ERROR(cudaMalloc(&d_bmap, sizeof(BitMap)));
    HANDLE_ERROR(cudaMemcpy(d_bmap, this, sizeof(BitMap), cudaMemcpyHostToDevice));
}

BitMap::~BitMap()
{
    cudaFree(d_word_s);
    cudaFree(d_size);
    free(h_bits);
    cudaFree(d_bits);
    cudaFree(d_bmap);
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
__global__ void d_pcnt(BitMap *bmap, long *wrdPcnt, size_t numWords)
{
    size_t w = blockIdx.x * blockDim.x + threadIdx.x;
    size_t jump = blockDim.x * gridDim.x;
    for (; w < numWords; w += jump) {
        wrdPcnt[w] = __popc(bmap->bits(w));
    }
}

long BitMap::rank(bitIdx_t idx)
{
    size_t numWords = (idx + 1) / h_word_s;
    long ans = 0;

    if (numWords > 0) {
        long *d_wrdPcnt;

        cudaMalloc(&d_wrdPcnt, numWords * sizeof(long));

        int thrdsPerBlk = 256;
        int blks = (numWords + thrdsPerBlk - 1) / thrdsPerBlk;
        d_pcnt<<<blks, thrdsPerBlk>>>(d_bmap, d_wrdPcnt, numWords);

        ans = thrust::reduce(thrust::device, d_wrdPcnt,
                             d_wrdPcnt + numWords, 0L,
                             thrust::plus<long>());

        cudaFree(d_wrdPcnt);
    }

    size_t bitInWrd = (idx+1) % h_word_s;
    if (bitInWrd > 0)
        ans += POPCOUNT(h_bits[numWords] >> (h_word_s - bitInWrd));
    
    return ans;
}

// UTILS
bitIdx_t BitMap::size()
{
    return h_size;
}

__device__ BitMap::word_t BitMap::bits(size_t idx)
{
    return d_bits[idx];
}

std::string BitMap::toString()
{
    std::string bitmap (h_size, '0');
    for (bitIdx_t i = 0; i < h_size; i++) {
        bitmap[i] = get(i) + '0';
    }
    return bitmap;
}

