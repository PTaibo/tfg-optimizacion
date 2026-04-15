#include "bitmap.h"

#include <iostream>

#include "utils.h"
#include "polybench.h"

void benchmark_select0(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }

    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.select0(idx);
    }
    polybench_stop_instruments;
    std::cout << "Select0 (s): ";
    polybench_print_instruments;
}

void benchmark_select1(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }

    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.select1(idx);
    }
    polybench_stop_instruments;
    std::cout << "Select1 (s): ";
    polybench_print_instruments;
}

void benchmark_select_compare(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }

    std::vector<size_t> idx(runs);
    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        idx[i] = rand() % size;
        bmap.select0(idx[i]);
    }
    polybench_stop_instruments;
    std::cout << "Select0 (s): ";
    polybench_print_instruments;

    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        bmap.select1(idx[i]);
    }
    polybench_stop_instruments;
    std::cout << "Select1 (s): ";
    polybench_print_instruments;
}

void benchmark_rank(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }

    // std::vector<size_t> idx(runs);
    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.rank(idx);
    }
    polybench_stop_instruments;
    std::cout << "Rank (s): ";
    polybench_print_instruments;
}

void benchmark_rank_compare(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }

    std::vector<size_t> idx(runs);
    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        idx[i] = rand() % size;
        bmap.rank(idx[i]);
    }
    polybench_stop_instruments;
    std::cout << "Rank (s): ";
    polybench_print_instruments;

    polybench_start_instruments;
    for (int i = 0; i < runs; i++) {
        bmap.wrdRank(idx[i]);
    }
    polybench_stop_instruments;
    std::cout << "wrdRank (s): ";
    polybench_print_instruments;
}

int main (int argc, char *argv[])
{
    if (argc < 2) {
        printf("No se ha indicado el benchmark a ejecutar\n");
        return 1;
    }

    polybench_prepare_instruments();
    size_t bmap_size = 1000000000;
    int runs = 100000;
    switch (atoi(argv[1])) {
        case 0:
            benchmark_select0(bmap_size, runs);
            break;
        case 1:
            benchmark_select1(bmap_size, runs);
            break;
        case 2:
            benchmark_rank(bmap_size, runs);
            break;
        case 3:
            benchmark_select_compare(bmap_size, runs);
            break;
        case 4:
            benchmark_rank_compare(bmap_size, runs);
        default:
            return 2;
    }

    return 0;
}

