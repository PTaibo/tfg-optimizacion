#include "bitmap.h"

#include <string>
#include <assert.h>
#include <iostream>

#include "utils.h"

int failed = 0;
int tests = 0;

std::string bmapFile = "/dev/urandom";

void test(std::string msg, bool correct)
{
    tests++;
    std::string dots (30-msg.size(), '.');
    std::cout << msg << dots;
    if (!correct) {
        std::cout << RED << "FAILED\n" << RESET_CLR;
        failed++;
    }
    else {
        std::cout << GREEN << "OK\n" << RESET_CLR;
    }
}

__global__ void getBit(bitIdx_t idx, int8_t *bit, BitMap *bmap) {
    *bit = bmap->d_get(idx);
}

void test_constructor()
{
    BitMap bmap(10, bmapFile);
    bool size = (bmap.size() == 10);
    bool all0 = true;
    int8_t bit;

    BitMap *d_bmap;
    cudaMalloc(&d_bmap, sizeof(BitMap));
    cudaMemcpy(d_bmap, &bmap, sizeof(BitMap), cudaMemcpyHostToDevice);
    int8_t *d_bit;
    cudaMalloc(&d_bit, sizeof(int8_t));

    for (bitIdx_t i = 0; i < bmap.size(); i++) {
        getBit<<<1,1>>>(i, d_bit, d_bmap);
        cudaMemcpy(&bit, d_bit, sizeof(int8_t), cudaMemcpyDeviceToHost);
        if (bmap.get(i) != bit) {
            all0 = false;
            break;
        }
    }
    test("Constructor", size && all0);
    std::cout << "  " << bmap.toString() << " <- bitmap\n";
    test("  Size()", size);
    test("  Initialized correctly", all0);
    std::cout << "-----------------------------------\n";
    cudaFree(d_bmap);
    cudaFree(d_bit);
}

void test_get()
{
    BitMap bitmap(10, bmapFile);
    std::cout << bitmap.toString() << " <- bitmap\n";
    std::vector<int8_t> host(10);
    std::vector<int8_t> dev(10);
    int8_t bit;

    BitMap *d_bmap;
    cudaMalloc(&d_bmap, sizeof(BitMap));
    cudaMemcpy(d_bmap, &bitmap, sizeof(BitMap), cudaMemcpyHostToDevice);
    int8_t *d_bit;
    cudaMalloc(&d_bit, sizeof(int8_t));

    bool passed = true;
    for(size_t i = 0; i < bitmap.size(); i++) {
        getBit<<<1,1>>>(i, d_bit, d_bmap);
        cudaMemcpy(&bit, d_bit, sizeof(int8_t), cudaMemcpyDeviceToHost);
        if (bit != bitmap.get(i)) {
            passed = false;
        }
        host[i] = bitmap.get(i);
        dev[i] = bit;
    }
    for (size_t i = 0; i < 10; i++)
        printf("%d", host[i]);
    printf(" <- host\n");
    for (size_t i = 0; i < 10; i++)
        printf("%d", dev[i]);
    printf(" <- dev\n");

    if (!failed)
        test("Get()", passed);
    test("Get() out of bounds", bitmap.get(100) == -1);
    std::cout << "------------------------" << "\n";

    cudaFree(d_bmap);
    cudaFree(d_bit);
}

void test_rank()
{
    BitMap bmap(24, bmapFile);
    std::string bits = bmap.toString();
    std::vector<int> rank(bits.size(), 0);
    rank[0] += ( bits[0] == '1' ) ? 1 : 0;
    for (size_t i = 1; i < bits.size(); i++) {
        rank[i] = rank[i-1];
        if (bits[i] == '1')
            rank[i]++;
    }

    bool works = true;
    if (bmap.rank(2) != rank[2]) works = false;
    if (bmap.rank(7) != rank[7]) works = false;
    if (bmap.rank(9) != rank[9]) works = false;
    if (bmap.rank(14) != rank[14]) works = false;
    if (bmap.rank(19) != rank[19]) works = false;
    if (bmap.rank(22) != rank[22]) works = false;
    test("rank()", works);
    std::cout << "-----------------------------------\n";
}

void test_random_get(size_t size, int tests, std::string file)
{
    BitMap bmap(size, file);
    BitMap *d_bmap;
    cudaMalloc(&d_bmap, sizeof(BitMap));
    cudaMemcpy(d_bmap, &bmap, sizeof(BitMap), cudaMemcpyHostToDevice);

    int8_t bit;
    int8_t *d_bit;
    cudaMalloc(&d_bit, sizeof(int8_t));

    bool works = true;
    for (int i = 0; i < tests; i++) {
        size_t idx = rand() % size;
        getBit<<<1,1>>>(idx, d_bit, d_bmap);
        cudaMemcpy(&bit, d_bit, sizeof(int8_t), cudaMemcpyDeviceToHost);
        if (bmap.get(idx) != bit) {
            works = false;
            printf("Host: %d | Dev: %d\n", bmap.get(idx), bit);
        }
    }
    test("Random get()", works);
    std::cout << "-----------------------------------\n";
    cudaFree(d_bmap);
    cudaFree(d_bit);
}

void test_random_rank(size_t size, int tests, std::string file)
{
    srand(time(0));
    BitMap bmap(size, file);
    std::string bits = bmap.toString();
    std::vector<long> ones (size, 0);
    ones[0] += ( bits[0] == '1' ) ? 1 : 0;
    for (size_t i = 1; i < bits.size(); i++) {
        ones[i] = ones[i-1];
        if (bits[i] == '1')
            ones[i]++;
    }

    bool works = true;
    for (int i = 0; i < tests; i++) {
        size_t idx = rand() % size;
        if (bmap.rank(idx) != ones[idx]) {
            works = false;
        }
    }
    test("Random rank()", works);
    std::cout << "-----------------------------------\n";
}

int main (void)
{
    failed = 0;
    tests = 0;

    test_constructor();

    test_get();

    test_rank();

    size_t bmap_size = 1000000;
    int tests = 100;
    test_random_get(bmap_size, tests, bmapFile);
    test_random_rank(bmap_size, tests, bmapFile);

    if (!failed) {
        std::cout << GREEN << "PASSED ALL TESTS" << RESET_CLR << "\n";
    }
    else {
        std::cout << RED << "Failed " << failed << "/" << tests << " tests" << "\n";
    }

    return 0;
}

