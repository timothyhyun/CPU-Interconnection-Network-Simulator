#ifndef COHER_INTERNAL_H
#define COHER_INTERNAL_H


#include "interconnect.h"
#include "coherence.h"


struct _coher;

typedef struct _directory_sim_args {
    struct _interconn* inter;
} directory_sim_args;


typedef enum _directory_status {
    INVALID, 
    SHARED, 
    EXCLUSIVE
} directory_status;


typedef struct _directory_states {
  directory_status state;
  int directory[4];
} directory_states;


typedef struct _directory_sim{
    sim_interface si;
    void (*directoryReq)(bus_req_type reqType, directory_states currentState, uint64_t addr, int procNum);
    void (*registerCoher)(struct _coher* coherComp);
} directory_sim;

#endif
