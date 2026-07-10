#include "bitmap.h"

#include <iostream>

#include "utils.h"
// #include <papi.h>
#include <chrono>

using namespace std::chrono;

// void papi_handle_error(int retval)
// {
//     if (retval != PAPI_OK) {
//         printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
//         exit(1);
//     }
// }

void benchmark_rank(size_t size, int runs, unsigned int seed)
{
    srand(seed);
    BitMap bmap(size, "./bitmap.bin");

    std::vector<size_t> idx(runs);
    for (size_t i = 0; i < runs; i++)
        idx[i] = rand() % size;

    // Warmup
    for (int i = 0; i < 10; i++)
        bmap.rank(idx[i]);
    
    auto start = high_resolution_clock::now();
    for (int i = 0; i < runs; i++) {
        bmap.rank(idx[i]);
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    
    std::cout << duration.count() << " us\n";
}

int main (int argc, char *argv[])
{
    if (argc < 3) {
        printf("No se ha indicado el benchmark a ejecutar o el seed\n");
        return 1;
    }

    size_t bmap_size = 1000000000;
    // int runs = 100000; // Takes too long
    int runs = 1000;

    switch (atoi(argv[1])) {
        case 0:
            benchmark_rank(bmap_size, runs, atoi(argv[2]));
            break;
        default:
            return 2;
    }

    return 0;
}

