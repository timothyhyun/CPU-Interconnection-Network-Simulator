/**
 * @file cache_internal.h
 * @brief Header file which declares all relevant datatypes for cache.c
 * @author Aaron Tan <ahtan@andrew.cmu.edu>
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef CACHE_INTERNAL_H
#define CACHE_INTERNAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief enum that enables stats 
 */
typedef enum {
    HIT  = 0,
    MISS = 1,
    EVICT = 2,
    DEFAULT = 3,
} result;

/**
 * @brief struct containing parameters of cache trace
 */
typedef struct parsed_t {
    size_t s;
    size_t E;
    size_t b;
    size_t k;
    size_t v;
    size_t u;
} parsed_t;

/**
 * @brief Struct representing simulation statistics for a trace
 * 
 * Taken from 15-213 Cachelab
 */
typedef struct {
    size_t hits;            /* number of hits */
    size_t misses;          /* number of misses */
    size_t evictions;       /* number of evictions */
    size_t dirty_bytes;     /* number of dirty bytes in cache
                                      at end of simulation */
    size_t dirty_evictions; /* number of bytes evicted
                                      from dirty lines */
} csim_stats;

/**
 * @brief Array implementation for RRIP caches
 */
typedef struct cline {
    unsigned long tag;
    size_t age;
    bool *valid;
    bool dirty;
    bool valid_line;
} cline;

typedef cline* cset;
typedef cline** cache_t;

/** Function Prototypes */
cache_t *init_cache(parsed_t *cache_parameters);
int load_cache(cache_t *cache, int size, uint64_t memAddress, parsed_t *cache_parameters, void (*cache_update)(cline *, result, int));
int store_cache(cache_t *cache, int size, uint64_t memAddress, parsed_t *cache_parameters, void (*cache_update)(cline *, result, int));

void print_stats(csim_stats *stats);

#endif 
