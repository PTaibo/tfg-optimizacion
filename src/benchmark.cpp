#include "bitmap.h"

#include <iostream>

#include "utils.h"
#include <papi.h>

void papi_handle_error(int retval)
{
    if (retval != PAPI_OK) {
        printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
        exit(1);
    }
}

void benchmark_select(size_t size, int runs, unsigned int seed)
{
    srand(seed);
    BitMap bmap(size);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 2) {
            bmap.set(i);
        }
    }
    bmap.updateRank();

    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);

    retval = PAPI_start(event_set);
    papi_handle_error(retval);
    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.select(idx);
    }
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    printf("Unhalted clock cycles (select): %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
}

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

    // std::vector<size_t> idx(runs);
    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);
    
    retval = PAPI_start(event_set);
    papi_handle_error(retval);
    for (int i = 0; i < runs; i++) {
        size_t idx = rand() % size;
        bmap.rank(idx);
    }
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    // std::cout << "Rank (s): ";
    printf("Unhalted clock cycles: %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
}

int main (int argc, char *argv[])
{
    if (argc < 2) {
        printf("No se ha indicado el benchmark a ejecutar\n");
        return 1;
    }

    // Init PAPI
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval < 0 || retval != PAPI_VER_CURRENT)
        papi_handle_error(retval);

    size_t bmap_size = 1000000000;
    int runs = 100000;
    switch (atoi(argv[1])) {
        case 0:
            benchmark_select(bmap_size, runs, atoi(argv[2]));
            break;
        case 1:
            benchmark_rank(bmap_size, runs, atoi(argv[2]));
            break;
        default:
            return 2;
    }

    return 0;
}

