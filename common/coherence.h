#ifndef COHERENCE_H
#define COHERENCE_H

#include <common.h>
#include "interconnect.h"

#define READ_PERM 0
#define WRITE_PERM 1

struct _interconn;

typedef struct _coher_sim_args {
    int arg_count;
    char** arg_list;
    struct _interconn* inter;
    struct _directory_sim* direct;
} coher_sim_args;

typedef struct _coher {
    sim_interface si;
    void (*registerCacheInterface)(void(*callback)(int, int, int64_t));
    uint8_t (*permReq)(uint8_t is_read, uint64_t addr, int processorNum);
    uint8_t (*busReq)(bus_req_type reqType, uint64_t addr, int processorNum);
    void (*cacheReq)();
} coher;

#endif