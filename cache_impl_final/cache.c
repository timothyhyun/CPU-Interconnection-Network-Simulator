#include <cache.h>
#include <trace.h>
#include <coherence.h>
#include "cache_internal.h"
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef HELP_MESSAGE
#define HELP_MESSAGE \
    "Mandatory arguments missing or zero.\n\
    Usage: ./csim -ref [-v] -s <s> -E <E> -b <b> -t <trace>\n\
          ./csim -ref -h\n\
     -h Print this help message and exit\n\
     -v Verbose mode: report effects of each memory operation\n\
     -s <s> Number of set index bits (there are 2**s sets)\n\
     -b <b> Number of block bits (there are 2**b blocks)\n\
     -E <E> Number of lines per set ( associativity )\n\
     -t <trace > File name of the memory trace to process\n\
   The -s, -b, -E, and -t options must be supplied for all simulations.\n"
#endif

/**
 * DATATYPES
 */
cache *self = NULL;
cache_t *cache_sim = NULL;

int processorCount = 1;
int CADSS_VERBOSE = 0;
parsed_t *cache_parameters = NULL;
csim_stats *stats = NULL;
bool use_RRIP = false;

coher* coherComp = NULL;

void coherCallback(int type, int processorNum, int64_t addr);
void memoryRequest(trace_op *op, int processorNum, int64_t tag, void (*callback)(int64_t));

cache *init(cache_sim_args *csa)
{
    int op;
    size_t s = 0, E = 0, b = 0, k = 0, v = 0, u = 0;
    u = 0;

    // TODO - get argument list from assignment
    while ((op = getopt(csa->arg_count, csa->arg_list, "E:s:b:i:R:u:h:v:k")) != -1)
    {
        switch (op)
        {
        // Lines per set
        case 'E':
            E = (size_t)strtoul(optarg, NULL, 10);
            break;

        // Sets per cache
        case 's':
            s = (size_t)strtoul(optarg, NULL, 10);
            break;

        // block size in bits
        case 'b':
            b = (size_t)strtoul(optarg, NULL, 10);
            break;

        // entries in victim cache
        case 'i':
            v = (size_t)strtoul(optarg, NULL, 10);
            break;

        // bits in a RRIP-based replacement policy
        case 'R':
            use_RRIP = true;
            k = (size_t)strtoul(optarg, NULL, 10);
            break;

        // 2^U subblocks per block
        case 'u':
            u = (size_t)strtoul(optarg, NULL, 10);
            break;

        case 'h':
            printf(HELP_MESSAGE);
            return 0;

        default:
            printf(HELP_MESSAGE);
            return 0;
        }
    }

    cache_parameters = malloc(sizeof(parsed_t));
    cache_parameters->b = b;
    cache_parameters->E = E;
    cache_parameters->k = k;
    cache_parameters->v = v;
    cache_parameters->s = s;
    cache_parameters->u = u;

    cache_sim = init_cache(cache_parameters);

    stats = malloc(sizeof(csim_stats));
    stats->dirty_bytes = 0;
    stats->dirty_evictions = 0;
    stats->evictions = 0;
    stats->hits = 0;
    stats->misses = 0;

    self = malloc(sizeof(cache));
    self->memoryRequest = memoryRequest;
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;

    return self;
}

int64_t pendingTag = 0;
int countDown = 0;
void (*memCallback)(int64_t) = NULL;

/**
 *  With coherence we might now want to keep track of pending memory operations
 *  as there are rules that must be followed wrt when operations/coherence ops
 *  can happen and in what order
 * 
 *  Linked list of outstanding transactions
 */
typedef struct _pendingRequest {
    int64_t tag;
    int64_t addr;
    int type; // perhaps a coherence operation?
    int processorNum;
    void(*callback)(int64_t);
    struct _pendingRequest* next;
} pendingRequest;

/**
 *  Not actually sure if this is the correct way to do this. 
 */
pendingRequest* readyReq = NULL;
pendingRequest* pendReq = NULL;

/**
 * theoretically this should be defined by the coherence component but for now
 * we will just set this to be a bus coherence callback by default
 */
void coherCallback(int type, int processorNum, int64_t addr)
{
    assert(pendReq != NULL);
    assert(processorNum < processorCount);
    
    if (pendReq->processorNum == processorNum &&
        pendReq->addr == addr)
    {
        pendingRequest* pr = pendReq;
        pendReq = pendReq->next;
        
        pr->next = readyReq;
        readyReq = pr;
    }
    else
    {
        pendingRequest* prevReq = pendReq;
        pendingRequest* pr = pendReq->next;
        
        while (pr != NULL)
        {
            if (pr->processorNum == processorNum &&
                pr->addr == addr)
            {
                prevReq->next = pr->next;
                
                pr->next = readyReq;
                readyReq = pr;
                break;
            }
            pr = pr->next;
            prevReq = prevReq->next;
        }
        
        if (pr == NULL && CADSS_VERBOSE == 1)
        {
            pr = pendReq;
            while (pr != NULL)
            {
                printf("W: %p (%lx %d)\t", pr, pr->addr, pr->processorNum);
                pr = pr->next;
            }
            
        }
        assert(pr != NULL);
    }
}

void lru_update(cline *currline, result op, int def)
{
    switch (op)
    {
    case MISS:
    case HIT:
    case EVICT:
        currline->age = 0;
        break;
    default:
        currline->age++;
        break;
    }
}

void rrip_update(cline *currline, result op, int def)
{
    switch (op)
    {
    case MISS:
        currline->age = (1 << cache_parameters->k) - 2;
        break;
    case EVICT:
        currline->age = (1 << cache_parameters->k) - 1;
        break;
    case HIT:
        currline->age = 0;
        break;
    default:
        currline->age += def;
        break;
    }
}

void memoryRequest(trace_op *op, int processorNum, int64_t tag, void (*callback)(int64_t))
{
    assert(op != NULL);
    assert(callback != NULL);

    int result = 0;
    switch (op->op)
    {
    case MEM_LOAD:
        if (use_RRIP)
        {
            result = load_cache(cache_sim, op->size, op->memAddress, cache_parameters, &rrip_update);
        }
        else
        {
            result = load_cache(cache_sim, op->size, op->memAddress, cache_parameters, &lru_update);
        }
        break;
    case MEM_STORE:
        if (use_RRIP)
        {
            result = store_cache(cache_sim, op->size, op->memAddress, cache_parameters, &rrip_update);
        }
        else
        {
            result = store_cache(cache_sim, op->size, op->memAddress, cache_parameters, &lru_update);
        }
        break;
    }

    // Simple model to only have one outstanding memory operation
    if (countDown != 0)
    {
        memCallback(pendingTag);
    }

    pendingTag = tag;
    memCallback = callback;

    // In a real cache simulator, the delay is based on
    //   whether the request is a hit or miss
    countDown += result;
}

int tick()
{
    if (countDown > 0)
    {
        countDown--;
        if (countDown == 0)
        {
            assert(memCallback != NULL);
            memCallback(pendingTag);
        }
    }

    return 1;
}

int finish(int outFd)
{
    print_stats(stats);
    return 0;
}

int destroy(void)
{
    // free any internally allocated memory here
    return 0;
}
