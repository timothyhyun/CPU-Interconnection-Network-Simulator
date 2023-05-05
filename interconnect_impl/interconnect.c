#include <interconnect.h>
#include "interconnect_internal.h"
#include <stdio.h>
#include <getopt.h>
#include "queues.h"




queue_t **pending = NULL;

ic_req* pendingRequest = NULL;
ic_req** queuedRequests;
interconn* self;
coher* coherComp;

int CADSS_VERBOSE = 0;
int processorCount = 1;


const int INTER_DELAY = 10;
const int CACHE_DELAY = 10;
const int CACHE_TRANSFER = 10;
const int BUS_TIME = 90;  // TODO - model using a memory component

void registerCoher(coher* cc);
void busReq(bus_req_type brt, uint64_t addr, int destNum, int sourceNum, int replyNum);

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


    pending = malloc(sizeof(queue_t*) * processorCount);

    for (int i = 0; i < processorCount; i++)
    {
        pending[i] = new_Q();
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

// DEST SOURCE REPLY
/**
 * In a generalized interconnect, busReq will just queue up a request. At each
 * tick of the interconnect, we just deq a rqeuest and place it in the endpoint
 * of procNum.
 *
 */
void busReq(bus_req_type brt, uint64_t addr, int destNum, int sourceNum, int replyNum)
{
    // printf("Interconnect recieves request for %lX from %d to %d and replies %d\n", addr, sourceNum, destNum, replyNum);
    assert(procNum >= 0);
    ic_req* nextReq = calloc(1, sizeof(ic_req));
    nextReq->brt = brt;
    nextReq->currentState = QUEUED;
    nextReq->addr = addr;
    nextReq->destNum = destNum;
    nextReq->sourceNum = sourceNum;
    nextReq->replyNum = replyNum;
    pending[sourceNum]->countDown = INTER_DELAY;

    enq(pending[sourceNum], (void *)nextReq);
}

int tick()
{
    for (int i = 0; i < processorCount; i++) {
        if (pending[i]->countDown > 0) {
            pending[i]->countDown--;
            if (pending[i]->countDown == 0 && !network->nodes[i].busy) {
                ic_req *temp = (ic_req*) dq(pending[i]);
                if (temp != NULL) {
                    // printf("Node %d loads a packet (ADDR %lX) from queue\n", i, temp->addr);
                    network->nodes[i].curr_packet = temp;
                    network->nodes[i].busy = true;
                    pending[i]->countDown = INTER_DELAY;
                }
            }
            assert(network->nodes[i].busy && network->nodes[i].curr_packet != NULL);
        }
        // printf("Node %d waiting for %d ticks\n", i, pending[i]->countDown);
    }

    update(network);

    for (int i = 0; i < network->size; i++) {
        ic_req *curr_packet = network->nodes[i].curr_packet;
        if (curr_packet != NULL && curr_packet->destNum == i) {
            // Send to Cache
            // printf("%lX has been sent to %d from %d and will reply to %d\n", curr_packet->addr, curr_packet->destNum, curr_packet->sourceNum, curr_packet->replyNum);
            coherComp->cacheReq(curr_packet->brt, curr_packet->addr, curr_packet->destNum, curr_packet->replyNum);
            network->nodes[i].curr_packet = NULL;
            network->nodes[i].busy = false;
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
