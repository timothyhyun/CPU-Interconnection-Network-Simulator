#include <coherence.h>
#include <trace.h>
#include <getopt.h>
#include "coher_internal.h"

#include "stree.h"


typedef struct _cache_req {
    bus_req_type brt;
    uint64_t addr;
    int procNum;
    int replyNum;
    struct _cache_req *next;
} cache_req;


cache_req* pendingRequest = NULL;
cache_req* queuedRequests;
const int CONTROLLER_DELAY = 5;


// TREE = DIRECTORY
tree_t** coherStates = NULL;
int processorCount = 1;
int CADSS_VERBOSE = 0;

int cache_b = 0;
int cache_s = 0;

coher* self = NULL;
interconn* inter_sim = NULL;
direc* direct_sim = NULL;

typedef void(*cacheCallbackFunc)(int, int, int64_t);
cacheCallbackFunc cacheCallback = NULL;

uint8_t busReq(bus_req_type reqType, uint64_t addr, int procNum, int replyNum);
uint8_t permReq(uint8_t is_read, uint64_t addr, int processorNum);

// processorNum: current processor num
// nextProcessorNum: # to reply to
void cacheReq(bus_req_type reqType, uint64_t addr, int procNum, int replyNum);
void registerCacheInterface(void(*callback)(int, int, int64_t));
void registerCacheParameters(int s, int b);

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
    self->cacheReq = cacheReq;
    self->registerCacheInterface = registerCacheInterface;
    self->registerCacheParameters = registerCacheParameters;

    inter_sim->registerCoher(self);
    direct_sim->registerCoher(self);

    queuedRequests = malloc(sizeof(cache_req));
    queuedRequests = NULL;


    return self;
}


void registerCacheInterface(void(*callback)(int, int, int64_t))
{
    cacheCallback = callback;
}

void registerCacheParameters(int s, int b) {
    cache_b = b;
    cache_s = s;
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
uint8_t busReq(bus_req_type reqType, uint64_t addr, int procNum, int replyNum)
{
    if (procNum < 0 || procNum >= processorCount)
    {
        // ERROR
    }

    coherence_states currentState = getState(addr, procNum);
    coherence_states nextState;
    cache_action ca;
    printf("%d cache is processing about %lX and will respond to %d\n", procNum, addr, replyNum);
    nextState = processCache(reqType, &ca, currentState, addr, procNum, replyNum);
    switch(ca)
    {
        case DATA_RECV:
            // callback to cache that request is complete
            cacheCallback(0, procNum, addr);
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
            tree_remove(coherStates[procNum], addr);
        }
    }
    else
    {
        setState(addr, procNum, nextState);
    }

    printf("Finished recieve cache request\n");
    return 0;
}


// TODO - is_read needs to also encompass evictions from cache
//   this is beyond the scope of the next assignment

uint8_t permReq(uint8_t is_read, uint64_t addr, int processorNum)
{
    printf("%d recieving request on %lX\n", processorNum, addr);
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
// rproc: processorNum that sent request
void cacheReq(bus_req_type reqType, uint64_t addr, int procNum, int replyNum)
{
    // Add to pending Queue

    printf("%d recieves request about %lX and will reply to %d\n", procNum, addr, replyNum);
    // printf("Recieving request from interconnect\n");
    // if (pendingRequest == NULL) {
        cache_req* nextReq = calloc(1, sizeof(cache_req));
        nextReq->brt = reqType;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->replyNum = replyNum;
        pendingRequest = nextReq;
    //     countDown = CONTROLLER_DELAY;
    // } else {
    //     cache_req* nextReq = calloc(1, sizeof(cache_req));
    //     nextReq->brt = reqType;
    //     nextReq->addr = addr;
    //     nextReq->procNum = processorNum;
    //     nextReq->nextProcNum = nextProcessorNum;
    //     queuedRequests = nextReq;
    // }


    if (pendingRequest->brt == BUSRD || pendingRequest->brt == BUSWR) {
        printf("%d is forwarding to directory about %lX and will reply to %d\n", procNum, addr, replyNum);
        direct_sim->directoryReq(pendingRequest->brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->replyNum);
    // Is either fetch, invalidate, or data
    } else {
        busReq(pendingRequest->brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->replyNum);
    }
    free(nextReq);
    pendingRequest = NULL;

    printf("cacheReq exit\n");
}





// Departures are simple because they all go through the interconnect queue.
// No need to change anything.
int tick()
{
    // Start processing Queue
    // if (countDown > 0) {
    //     countDown--;
    //     if (countDown == 0) {
    //         if (pendingRequest->brt == BUSRD || pendingRequest->brt == BUSWR) {
    //             direct_sim->directoryReq(pendingRequest->brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->nextProcNum);
    //         // Is either fetch, invalidate, or data
    //         } else {
    //             busReq(pendingRequest->brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->nextProcNum);
    //         }
    //         free(pendingRequest);
    //         pendingRequest = NULL;
    //     }
    // } else {
    //     if (queuedRequests != NULL) {
    //         pendingRequest = queuedRequests;
    //         queuedRequests = NULL;
    //         countDown = CONTROLLER_DELAY;
    //     }
    // }


    // direct_sim->si.tick();
    // printf("tick coherence\n");
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
