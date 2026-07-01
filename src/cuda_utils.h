#pragma once
#include <cstdlib>
#include <stdio.h>
#include <string>

static void HandleError( cudaError_t err,
                         const char *file,
                         int line ) {
    if (err != cudaSuccess) {
        printf( "%s in %s at line %d\n", cudaGetErrorString( err ),
                file, line );
        exit( EXIT_FAILURE );
    }
}
#define HANDLE_ERROR( err ) (HandleError( err, __FILE__, __LINE__ ))

static void ErrorMsg ( char msg[], cudaError_t err,
                       const char *file, int line)
{
    if (err != cudaSuccess) {
        printf("%s in file %s at line %d failed with error \"%s\".\n",
                msg, file, line, cudaGetErrorString(err));
        exit ( EXIT_FAILURE );
    }
}
#define HANDLE_ERROR_MSG( msg, err ) (ErrorMsg( msg, err, __FILE__, __LINE__ ))

static void getStatus (char msg[], cudaError_t err,
                       const char *file, int line)
{
    if (err != cudaSuccess) {
        printf("%s in file %s at line %d failed with error \"%s\".\n",
                msg, file, line, cudaGetErrorString(err));
        exit ( EXIT_FAILURE );
    }
    else {
        printf("Passed %s in file %s\n", msg, file);
    }
}
#define GET_GPU_STATUS( msg, err ) (getStatus( msg, err, __FILE__, __LINE__ ))

inline void cudaInitTimer (cudaEvent_t *start, cudaEvent_t *stop)
{
    HANDLE_ERROR( cudaEventCreate(start) );
    HANDLE_ERROR( cudaEventCreate(stop) );
}

inline void cudaStartTimer (cudaEvent_t start)
{
    HANDLE_ERROR( cudaEventRecord(start, 0) );
}

inline void cudaStopTimer (cudaEvent_t start, cudaEvent_t stop, float *time)
{
    HANDLE_ERROR( cudaEventRecord(stop, 0) );
    HANDLE_ERROR( cudaEventSynchronize(stop) );
    HANDLE_ERROR( cudaEventElapsedTime(time, start, stop) );
}

inline void cudaGetTime (cudaEvent_t start, cudaEvent_t stop, float *time)
{
    HANDLE_ERROR( cudaEventElapsedTime(time, start, stop) );
}

inline void cudaDeleteTimer (cudaEvent_t start, cudaEvent_t stop)
{
    HANDLE_ERROR( cudaEventDestroy(start) );
    HANDLE_ERROR( cudaEventDestroy(stop) );
}

