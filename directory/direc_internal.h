#ifndef COHER_INTERNAL_H
#define COHER_INTERNAL_H


#include "interconnect.h"
#include "coherence.h"


struct _coher;

typedef enum _directory_status {
    D_INVALID,
    D_SHARED,
    D_EXCLUSIVE
} directory_status;


typedef struct _directory_states {
  directory_status state;
  
  int *directory;
} directory_states;


#endif
