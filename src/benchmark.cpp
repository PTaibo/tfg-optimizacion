#include "bitmap.h"

#include <iostream>

#include "utils.h"
#include "polybench.h"

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

int main(void)
{
    polybench_prepare_instruments();
    size_t bmap_size = 10000000;
    int runs = 10000;
    benchmark_select_compare(bmap_size, runs);
    // TODO: Probar diferencia compares con las diferentes longitudes de palabra

    return 0;
}

