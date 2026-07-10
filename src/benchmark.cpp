#include "bitmap.h"
#include <iostream>
#include "utils.h"

#define TIME

#ifdef PAPI
#include <papi.h>
#endif

#ifdef TIME
#include <chrono>
using namespace std::chrono;
#endif

#ifdef PAPI
void papi_handle_error(int retval)
{
    if (retval != PAPI_OK) {
        printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
        exit(1);
    }
}
#endif

void benchmark_rank(size_t size, int runs, unsigned int seed)
{
    srand(seed);
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }
    bmap.updateRank();

    std::vector<size_t> idx(runs);
    for (size_t i = 0; i < runs; i++)
        idx[i] = rand() % size;

    // Warmup
    for (int i = 0; i < 10; i++) {
        bmap.rank(idx[i]);
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

int main (int argc, char *argv[])
{
    if (argc < 2) {
        printf("No se ha indicado el benchmark a ejecutar\n");
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
    runs = 1000; // Reduce runs for chrono benchmark
#endif

    size_t bmap_size = 1000000000;
    switch (atoi(argv[1])) {
        case 1:
            benchmark_rank(bmap_size, runs, atoi(argv[2]));
            break;
        default:
            return 2;
    }

    return 0;
}

