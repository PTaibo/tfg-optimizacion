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

void benchmark_rank(size_t size, int runs, unsigned int seed)
{
    srand(seed);
    BitMap bmap(size, "./bitmap.bin");

    std::vector<size_t> idx(runs);
    for (size_t i = 0; i < runs; i++)
        idx[i] = rand() % size;

    int event_set = PAPI_NULL;
    long long values[1];
    int retval = PAPI_create_eventset(&event_set);
    papi_handle_error(retval);

    const char *event_name = "UNHALTED_CORE_CYCLES";
    retval = PAPI_add_named_event(event_set, event_name);
    papi_handle_error(retval);

    // Warmup
    for (int i = 0; i < 10; i++)
        bmap.rank(idx[i]);
    
    retval = PAPI_start(event_set);
    papi_handle_error(retval);
    for (int i = 0; i < runs; i++) {
        bmap.rank(idx[i]);
    }
    retval = PAPI_read(event_set, &values[0]);
    papi_handle_error(retval);
    
    printf("Unhalted clock cycles: %lld\n", values[0]);

    retval = PAPI_stop(event_set, NULL);
    papi_handle_error(retval);
}

int main (int argc, char *argv[])
{
    if (argc < 3) {
        printf("No se ha indicado el benchmark a ejecutar o el seed\n");
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
            benchmark_rank(bmap_size, runs, atoi(argv[2]));
            break;
        default:
            return 2;
    }

    return 0;
}

