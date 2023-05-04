#include <interconnect.h>
#include "interconnect_internal.h"
#include <stdio.h>
#include <getopt.h>

ic_req* pendingRequest = NULL;
ic_req** queuedRequests;
interconn* self;
coher* coherComp;

int CADSS_VERBOSE = 0;
int processorCount = 1;

const int CACHE_DELAY = 10;
const int CACHE_TRANSFER = 10;
const int BUS_TIME = 90;  // TODO - model using a memory component

void registerCoher(coher* cc);
void busReq(bus_req_type brt, uint64_t addr, int procNum, int rprocNum, int nextProcNum);

ic_network_t *network = NULL;
network_type nt = CROSSBAR;

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
    
    queuedRequests = malloc(sizeof(ic_req*) * processorCount);
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

    network = new_network(processorCount, nt);
    
    return self;
}

int countDown = 0;

void registerCoher(coher* cc)
{
    coherComp = cc;
}

// procNum: destination
// rprocNum: sender
// nextProcNum: nextProcNum (if fetch this is the is the processor to reply to)

/**
 * In a generalized interconnect, busReq will just queue up a request. At each
 * tick of the interconnect, we just deq a rqeuest and place it in the endpoint
 * of procNum. 
 * 
 */
void busReq(bus_req_type brt, uint64_t addr, int procNum, int rprocNum, int nextProcNum)
{
    ic_req* nextReq = calloc(1, sizeof(ic_req));
    nextReq->brt = brt;
    nextReq->currentState = QUEUED;
    nextReq->addr = addr;
    nextReq->procNum = procNum;
    nextReq->rprocNum = rprocNum;
    nextReq->nextProcNum = nextProcNum;

    queuedRequests[procNum] = nextReq;
}

int tick()
{
    update(network);

    for (int i = 0; i < network->size; i++) {
        ic_req *curr_packet = network->nodes[i].curr_packet;
        if (curr_packet != NULL && curr_packet->procNum == i) {
            // Send to Cache
            coherComp->cacheReq();
        }
    }
    
    return 1;
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