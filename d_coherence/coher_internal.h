#ifndef COHER_INTERNAL_H
#define COHER_INTERNAL_H

#include <directory.h>
#include <interconnect.h>
#include <stdio.h>

extern interconn* inter_sim;
extern direc* direct_sim;
extern int processorCount;
extern int cache_b;
extern int cache_s;

typedef enum _coherence_states {
  UNDEF = 0,  // As tree find returns NULL, we need an unused for NULL
  MODIFIED,
  INVALID,
  INVALID_MODIFIED,
  SHARED_STATE,
  INVALID_SHARED,
  SHARED_MODIFIED,
  EXCLUSIVE,
  OWNED,
  OWNED_MODIFIED,
  FORWARD,
  FORWARD_MODIFIED
} coherence_states;

typedef enum _coherence_scheme {
  MI,
  MSI,
  MESI,
  MOESI,
  MESIF
} coherence_scheme;

typedef enum _cache_action {
  NO_ACTION,
  DATA_RECV,
  INVALIDATE
} cache_action;


// Change Cache State then Send to Directory or Interconnect
coherence_states cacheDirectory(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum);
coherence_states processCache(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum, int replyNum);

int findHomeProcessor(uint64_t addr, int procNum);


#endif
