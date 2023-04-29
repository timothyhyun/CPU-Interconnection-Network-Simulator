#include <interconnect.h>
#include <stdio.h>
#include <getopt.h>

typedef enum _bus_req_state {
    NONE,
    QUEUED,
    TRANSFERING,
    WAITING_CACHE,
    WAITING_MEMORY
} bus_req_state;

typedef struct _bus_req {
    bus_req_type brt;
    bus_req_state currentState;
    uint64_t addr;
    int procNum;
    int rprocNum;
    int nextProcNum;
    uint8_t shared;
    uint8_t data;
    struct _bus_req *next;
} bus_req;

bus_req* pendingRequest = NULL;
bus_req** queuedRequests;
interconn* self;
coher* coherComp;

int CADSS_VERBOSE = 0;
int processorCount = 1;

const int CACHE_DELAY = 10;
const int CACHE_TRANSFER = 10;
const int BUS_TIME = 90;  // TODO - model using a memory component

void registerCoher(coher* cc);
void busReq(bus_req_type brt, uint64_t addr, int procNum, int rprocNum, int nextProcNum);

interconn* init(inter_sim_args* isa)
{
    int op;
    
    while ((op = getopt(isa->arg_count, isa->arg_list, "v")) != -1)
    {
        switch (op)
        {
            default:
                break;
        }
    }
    
    queuedRequests = malloc(sizeof(bus_req*) * processorCount);
    for (int i = 0; i < processorCount; i++)
    {
        queuedRequests[i] = NULL;
    }
    
    self = malloc(sizeof(interconn));
    self->busReq = busReq;
    self->registerCoher = registerCoher;
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;
    
    return self;
}

int countDown = 0;
int lastProc = 0; // for round robin arbitration

void registerCoher(coher* cc)
{
    coherComp = cc;
}

// procNum: destination
// rprocNum: sender
// nextProcNum: nextProcNum (if fetch this is the is the processor to reply to)

void busReq(bus_req_type brt, uint64_t addr, int procNum, int rprocNum, int nextProcNum)
{
    if (pendingRequest == NULL)
    {
        assert(brt != SHARED);
        
        bus_req* nextReq = calloc(1, sizeof(bus_req));
        nextReq->brt = brt;
        nextReq->currentState = WAITING_CACHE;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->rprocNum = rprocNum;
        nextReq->nextProcNum = nextProcNum;
        
        pendingRequest = nextReq;
        countDown = CACHE_DELAY;
        return;
    }
    else if (brt == SHARED && pendingRequest->addr == addr)
    {
        pendingRequest->shared = 1;
        return;
    }
    else if (brt == DATA && pendingRequest->addr == addr)
    {
        assert(pendingRequest->currentState == WAITING_MEMORY);
        pendingRequest->data = 1;
        pendingRequest->currentState = TRANSFERING;
        countDown = CACHE_TRANSFER;
        return;
    }
    else
    {
        assert(brt != SHARED);
        assert(queuedRequests[procNum] == NULL);
        
        bus_req* nextReq = calloc(1, sizeof(bus_req));
        nextReq->brt = brt;
        nextReq->currentState = QUEUED;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->rprocNum = rprocNum;
        nextReq->nextProcNum = nextProcNum;
        
        queuedRequests[procNum] = nextReq;
    }
}

int tick()
{
    if (countDown > 0)
    {
        assert(pendingRequest != NULL);
        countDown--;
        if (countDown == 0)
        {
            if (pendingRequest->currentState == WAITING_CACHE)
            {
                countDown = BUS_TIME;
                pendingRequest->currentState = WAITING_MEMORY;
                for (int i = 0; i < processorCount; i++)
                {
                    if (pendingRequest->procNum != i)
                    {
                        // This is the broadcast I am assuming 
                        coherComp->cacheReq(pendingRequest->brt, pendingRequest->addr, i, pendingRequest->nextProcNum);
                    }
                }
                if (pendingRequest->data == 1)
                {
                    pendingRequest->brt = DATA;
                }
            }
            else if (pendingRequest->currentState == WAITING_MEMORY)
            {
                bus_req_type brt = DATA;
                if (pendingRequest->shared == 1) brt = SHARED;
                coherComp->cacheReq(brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->nextProcNum);
                free(pendingRequest);
                pendingRequest = NULL;
            }
            else if (pendingRequest->currentState == TRANSFERING)
            {
                bus_req_type brt = pendingRequest->brt;
                if (pendingRequest->shared == 1) brt = SHARED;
                coherComp->cacheReq(brt, pendingRequest->addr, pendingRequest->procNum, pendingRequest->nextProcNum);
                free(pendingRequest);
                pendingRequest = NULL;
            }
        }
        
    }
    else if (countDown == 0)
    {
        for (int i = 0; i < processorCount; i++)
        {
            int pos = (i + lastProc) % processorCount;
            if (queuedRequests[pos] != NULL)
            {
                pendingRequest = queuedRequests[pos];
                queuedRequests[pos] = NULL;
                countDown = CACHE_DELAY;
                pendingRequest->currentState = WAITING_CACHE;
                
                lastProc = (pos + 1) % processorCount;
                break;
            }
        }
    }
    
    return 0;
}

int finish(int outFd)
{
    return 0;
}

int destroy(void)
{
    // TODO
    
    return 0;
}