#include <coherence.h>
#include <trace.h>
#include <getopt.h>
#include "coher_internal.h"

#include "stree.h"

tree_t** coherStates = NULL;
int processorCount = 1;
int CADSS_VERBOSE = 0;
coherence_scheme cs = MI;

coher* self = NULL;
interconn* inter_sim = NULL;

typedef void(*cacheCallbackFunc)(int, int, int64_t);
cacheCallbackFunc cacheCallback = NULL;

uint8_t busReq(bus_req_type reqType, uint64_t addr, int processorNum);
void registerCacheInterface(void(*callback)(int, int, int64_t));

coher* init(coher_sim_args* csa)
{
    int op;
    
    
    if (processorCount < 1 || processorCount > 256)
    {
        fprintf(stderr, "Error: processorCount outside valid range - %d specified\n", processorCount);
        return NULL;
    }
    
    coherStates = malloc(sizeof(tree_t*) * processorCount);
    for (int i = 0; i < processorCount; i++)
    {
        coherStates[i] = tree_new();
    }
    
    inter_sim = csa->inter;
    
    self = malloc(sizeof(coher));
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;
    self->permReq = permReq;
    self->busReq = busReq;
    self->registerCacheInterface = registerCacheInterface;
    
    inter_sim->registerCoher(self);
    
    return self;
}

void registerCacheInterface(void(*callback)(int, int, int64_t))
{
    cacheCallback = callback;
}

coherence_states getState(uint64_t addr, int processorNum)
{
    coherence_states lookState = (coherence_states) tree_find(coherStates[processorNum], addr);
    if (lookState == UNDEF) return INVALID;
    
    return lookState;
}

void setState(uint64_t addr, int processorNum, coherence_states nextState)
{
    tree_insert(coherStates[processorNum], addr, (void*)nextState);
}

uint8_t busReq(bus_req_type reqType, uint64_t addr, int processorNum)
{
    if (processorNum < 0 || processorNum >= processorCount)
    {
        // ERROR
    }
    
    coherence_states currentState = getState(addr, processorNum);
    coherence_states nextState;
    cache_action ca;

    nextState = directory(reqType, &ca, currentState, addr, processorNum);
    

    
    switch(ca)
    {
        case DATA_RECV:
            // callback to cache that request is complete
            cacheCallback(0, processorNum, addr);
            break;
        case INVALIDATE:
            // callback to cache that line has been invalidated
            // Do nothing, cache needs to be informed when it
            //   models replacement
            break;
        case NONE:
            break;
    }
    
    // IF the destination state is invalid, that is an implicit state
    //   and does not need to be stored in the tree.
    if (nextState == INVALID)
    {
        if (currentState != INVALID)
        {
            tree_remove(coherStates[processorNum], addr);
        }
    }
    else
    {
        setState(addr, processorNum, nextState);
    }
    
    return 0;
}

// TODO - is_read needs to also encompass evictions from cache
//   this is beyond the scope of the next assignment

int tick()
{
    return inter_sim->si.tick();
}

int finish(int outFd)
{
    return inter_sim->si.finish(outFd);
}

int destroy(void)
{
    // TODO
    
    return inter_sim->si.destroy();
}