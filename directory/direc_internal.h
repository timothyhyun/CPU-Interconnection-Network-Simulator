#ifndef COHER_INTERNAL_H
#define COHER_INTERNAL_H


#include "interconnect.h"
#include "coherence.h"


struct _coher;

typedef enum _directory_status {
    INVALID, 
    SHARED, 
    EXCLUSIVE
} directory_status;


typedef struct _directory_states {
  directory_status state;
  // Im just assuming 4 processors we can change this later
  int directory[4];
} directory_states;


#endif
