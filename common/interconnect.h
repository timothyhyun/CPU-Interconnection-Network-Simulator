#ifndef INTERCONNECT_H
#define INTERCONNECT_H

struct _coher;

typedef enum _bus_req_type {
    // NOT NEEDED
    NO_REQ,
    BUSRD,
    BUSWR,
    DATA,
    FETCH, 
    INVALIDATE,
    // NOT NEEDED RN 
    SHARED,
    // NEVER USED
    FLUSH
} bus_req_type;

#include "coherence.h"

typedef struct _inter_sim_args {
    int arg_count;
    char** arg_list;
} inter_sim_args;

typedef struct _interconn {
    sim_interface si;
    void (*busReq)(bus_req_type brt, uint64_t addr, int procNum, int rprocNum, int nextProcNum);
    void (*registerCoher)(struct _coher* coherComp);
} interconn;

#endif