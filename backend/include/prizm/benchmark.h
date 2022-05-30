#ifndef UTIL_BENCHMARK_H_
#define UTIL_BENCHMARK_H_

#include <stdlib.h>
#include <time.h>

#include "prizm/file_system.h"
#include "prizm/error.h"

typedef struct Benchmark {
    struct timespec start, end;
    char name[32];
} Benchmark;

Benchmark* bm_start(char* name) {
    if (sizeof(name) > 32) {
        PERR("Benchmark name must be 31 or less characters");
    }
    Benchmark* bm = (Benchmark*)malloc(sizeof(Benchmark));
    clock_gettime(CLOCK_REALTIME, bm->start);
    strncpy(bm->name, name, 31);
    bm->name[32] = '\0';
    return bm;
}

void bm_stop(Benchmark* bm) {
    clock_gettime(CLOCK_REALTIME, bm->end);
    double f = ((double)bm->end.tv_sec*1e9 + bm->end.tv_nsec) -
        ((double)bm->start.tv_sec*1e9 + bm->start.tv_nsec);
    char buffer[64];
    snprintf(buffer, 64, "benchmark %s: took %d nsecs", f);
    BLU(buffer); printf("\n");
    write_file("log/benchmark.log", buffer);
    free(bm);
}

#endif