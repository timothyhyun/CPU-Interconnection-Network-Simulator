#include <coherence.h>
#include <trace.h>
#include <getopt.h>
#include "coher_internal.h"

#include "stree.h"


typedef struct _cache_req {
    bus_req_type brt;
    uint64_t addr;
    int procNum;
    int nextProcNum; 
    struct _cache_req *next;
} cache_req;


cache_req* pendingRequest = NULL;
cache_req* queuedRequests;
const int CONTROLLER_DELAY = 5;


// TREE = DIRECTORY
tree_t** coherStates = NULL;
int processorCount = 1;
int CADSS_VERBOSE = 0;

coher* self = NULL;
interconn* inter_sim = NULL;
directory_sim* direct_sim = NULL;

typedef void(*cacheCallbackFunc)(int, int, int64_t);
cacheCallbackFunc cacheCallback = NULL;

uint8_t busReq(bus_req_type reqType, uint64_t addr, int processorNum);
uint8_t permReq(uint8_t is_read, uint64_t addr, int processorNum);
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
    direct_sim = csa->direct;
    self = malloc(sizeof(coher));
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;
    self->permReq = permReq;
    self->busReq = busReq;
    self->registerCacheInterface = registerCacheInterface;
    
    inter_sim->registerCoher(self);
    direct_sim->registerCoher(self);

    
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

// processorNum is dest proc
// Do I need to add origin processor? Or else how do I send data back????
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

uint8_t permReq(uint8_t is_read, uint64_t addr, int processorNum)
{
    if (processorNum < 0 || processorNum >= processorCount)
    {
        // ERROR
    }
    
    coherence_states currentState = getState(addr, processorNum);
    coherence_states nextState;
    uint8_t permAvail = 0;
    nextState = cacheDirectory(is_read, &permAvail, currentState, addr, processorNum);
    setState(addr, processorNum, nextState);
    
    return permAvail;
}


// Takes request from interconnect and directory and places them in queue
// all go to processCache
void cacheReq(bus_req_type reqType, uint64_t addr, int processorNum, int nextProcessorNum) 
{
    // Add to pending Queue
}





// Departures are simple because they all go through the interconnect queue.
// No need to change anything. 
int tick()
{
    // Start processing Queue


    direct_sim->si.tick();
    return inter_sim->si.tick();
    

}

int finish(int outFd)
{
    direct_sim->si.finish(outFd);
    return inter_sim->si.finish(outFd);
}

int destroy(void)
{
    // TODO
    direct_sim->si.destroy();
    return inter_sim->si.destroy();
}