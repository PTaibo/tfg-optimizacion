#include "bitmap.h"

#include <cstdlib>
#include <iostream>
#include "utils.h"

#define TIME

#ifdef TIME
#include <chrono>
using namespace std::chrono;
#endif

#ifdef PAPI
#include <papi.h>

void papi_handle_error(int retval)
{
    if (retval != PAPI_OK) {
        printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
        exit(1);
    }
}
#endif

void benchmark_select0(size_t size, int runs)
{
    srand(time(0));
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }
    bmap.updateRank();

    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.select0(idx);
    }
    std::cout << "Select0 (s): ";
}

void benchmark_select1(size_t size, int runs, unsigned int seed)
{
    // srand(time(0));
    srand(seed);
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }
    bmap.updateRank();

#ifdef PAPI
    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);

    retval = PAPI_start(event_set);
    papi_handle_error(retval);
#endif

    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.select1(idx);
    }

#ifdef PAPI
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);

    // std::cout << "Select1 (s): ";
    printf("Unhalted clock cycles (select1): %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
#endif
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
    bmap.updateRank();

    std::vector<size_t> idx(runs);
    for (int i = 0; i < runs; i++)
        idx[i] = rand() % size;

    // Warmup
    for (int i = 0; i < 10; i++) {
        bmap.select0(idx[i]);
        bmap.select1(idx[i]);
    }

#ifdef PAPI
    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);
    
    retval = PAPI_start(event_set);
    papi_handle_error(retval);
#endif

    for (int i = 0; i < runs; i++) {
        bmap.select0(idx[i]);
    }

#ifdef PAPI
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    printf("Select0: Unhalted clock cycles: %lld\n", values[0]);

    event_set = PAPI_NULL;
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);

    retval = PAPI_start(event_set);
    papi_handle_error(retval);
#endif

    for (int i = 0; i < runs; i++) {
        bmap.select1(idx[i]);
    }

#ifdef PAPI
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    printf("Select1: Unhalted clock cycles: %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
#endif
}

void benchmark_rank(size_t size, int runs, unsigned int seed)
{
    // srand(time(0));
    srand(seed);
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }
    bmap.updateRank();

    std::vector<size_t> idx(runs);
    for (int i = 0; i < runs; i++)
        idx[i] = rand() % size;

    // Warmup
    for (int i = 0; i < 10; i++)
        bmap.rank(idx[i]);
    
#ifdef PAPI
    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);
    
    retval = PAPI_start(event_set);
    papi_handle_error(retval);
#endif

#ifdef TIME
    auto start = high_resolution_clock::now();
#endif

    // BENCHMARKED CODE
    for (int i = 0; i < runs; i++) {
        bmap.rank(idx[i]);
    }

#ifdef TIME
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << duration.count() << " us\n";
#endif

#ifdef PAPI
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    printf("Unhalted clock cycles: %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
#endif
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
    bmap.updateRank();

    std::vector<size_t> idx(runs);
    for (int i = 0; i < runs; i++) {
        idx[i] = rand() % size;
        bmap.rank(idx[i]);
    }
    std::cout << "Rank (s): ";

    for (int i = 0; i < runs; i++) {
        bmap.wrdRank(idx[i]);
    }
    std::cout << "wrdRank (s): ";
}

int main (int argc, char *argv[])
{
    if (argc < 3) {
        // printf("No se ha indicado el benchmark a ejecutar\n");
        printf("No se ha indicado el benchmark a ejecutar o la semilla\n");
        return 1;
    }

    int runs;

#ifdef PAPI
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval < 0 || retval != PAPI_VER_CURRENT)
        papi_handle_error(retval);
    
    runs = 100000;
#endif
#ifdef TIME
    runs = 100000; // ~Reduce runs for chrono benchmark~. To fast
#endif

    size_t bmap_size = 1000000000;
    size_t bmap_size_small = 1000000000;
    int runs_small = 1000;
    switch (atoi(argv[1])) {
        case 0:
            benchmark_select0(bmap_size, runs);
            break;
        case 1:
            benchmark_select1(bmap_size, runs, atoi(argv[2]));
            break;
        case 2:
            benchmark_rank(bmap_size, runs, atoi(argv[2]));
            break;
        case 3:
            benchmark_select_compare(bmap_size_small, runs_small);
            break;
        case 4:
            benchmark_rank_compare(bmap_size, runs);
            break;
        default:
            return 2;
    }

    return 0;
}

