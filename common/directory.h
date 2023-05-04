#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "coherence.h"
#include "cache.h"

typedef struct _direc_sim_args {
    int arg_count;
    char** arg_list;
    struct _interconn* inter;
    struct _coher* coher;
    struct _cache* cache;
} direc_sim_args;

typedef struct _direc{
    sim_interface si;
    void (*directoryReq)(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum);
    void (*registerCoher)(struct _coher* coherComp);
} direc;

#endif
