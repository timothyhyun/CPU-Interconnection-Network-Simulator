#include <directory.h>
#include "direc_internal.h"
#include "stree.h"

typedef struct _directory_req {
    bus_req_type brt;
    uint64_t addr;
    int procNum;
    // When directed to directory, this is the originating processor.
    // When directed to cache, this is the procNum to send data to.
    int rprocNum;
    struct _directory_req *next;
} directory_req;

// Not sure if I need req state enum yet



directory_req* pendingRequest = NULL;
// Just one Queue Needed
directory_req* queuedRequests;
tree_t** directoryStates = NULL;
int processorCount = 1;
interconn* inter_sim = NULL;
direc* self = NULL;
coher* coherComp = NULL;

const int CONTROLLER_DELAY = 5;



int countDown = 0;

// Sending Functions:

// send from directory to cache
// directoryNum sending fetch request to procNum which will send data back to rprocNum
void sendFetch(uint64_t addr, int procNum, int directoryNum, int rprocNum) {
    // printf("sendFetch enter ");
    // need additional arg in busreq for destination
    if (procNum == directoryNum) {
        coherComp->cacheReq(FETCH, addr, procNum, rprocNum);
    } else {
        // interconnect request
        inter_sim->busReq(FETCH, addr, procNum, directoryNum, rprocNum);
    }
    // printf("sendFetch exit\n");
}

// directoryNum sending invalidate to procNum
void sendInvalidate(uint64_t addr, int procNum, int directoryNum){
    // printf("sendInvalidate enter ");
    if (procNum == directoryNum) {
        coherComp->cacheReq(IC_INVALIDATE, addr, procNum, -1);
    } else {
        // Interconnect request
        // No reply needed
        inter_sim->busReq(IC_INVALIDATE, addr, procNum, directoryNum, -1);
    }
    // printf("sendInvalidate exit\n");
}



void directoryReq(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum);
void registerCoher(coher *cc);

direc* init(direc_sim_args* dsa)
{
    directoryStates = malloc(sizeof(tree_t*) * processorCount);

    for (int i = 0; i < processorCount; i++)
    {
        directoryStates[i] = tree_new();
    }
    inter_sim = dsa->inter;
    coherComp = dsa->coher;
    self = malloc(sizeof(direc));
    self->directoryReq = directoryReq;
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;
    self->registerCoher = registerCoher;

    queuedRequests = malloc(sizeof(directory_req));
    queuedRequests = NULL;


    return self;
}


void registerCoher(coher* cc)
{
    coherComp = cc;
}

directory_states *getDirectoryState(uint64_t addr, int procNum) {
    // printf("getDirectoryState enter ");
    directory_states *lookState = (directory_states *) tree_find(directoryStates[procNum], addr);
    if (lookState == NULL){
        // In practice we should have an init function for this
        lookState = malloc(sizeof(directory_states));
        lookState->state = D_INVALID;
        lookState->directory = calloc(sizeof(bool), processorCount);
        tree_insert(directoryStates[procNum], addr, (void *)lookState);
    }
    // printf("getDirectoryState exit\n");
    return lookState;
}

void setDirectoryState(uint64_t addr, int processorNum, directory_states *nextState)
{
    // printf("setDirectory enter ");
    tree_insert(directoryStates[processorNum], addr, (void*)nextState);
    // printf("setDirectory exit ");
}

// rprocnum: originating processor
// procnum: current processor number
directory_status directory(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum) {
    // printf("directory call enter ");
    directory_states *currentState = getDirectoryState(addr, procNum);
    switch(currentState->state) {
        case D_EXCLUSIVE:
            if (reqType == BUSRD) {
                // SEND FETCH
                for (int i = 0; i < 4; i++) {
                    if (currentState->directory[i] == 1) {
                        // send fetch to process 1 (currently at procNum), reply to rprocNum
                        sendFetch(addr, i, procNum, rprocNum);
                        break;
                    }
                }
                // DEMOTE TO SHARED AND ADD PROCESSOR
                currentState->directory[procNum] = 1;
                currentState->state = D_SHARED;
            } else {
                // SEND Invalidates to everybody
                for (int i = 0; i < 4; i++) {
                    if (currentState->directory[i] == 1) {
                        currentState->directory[i] = 0;
                        sendFetch(addr, i, procNum, rprocNum);
                        sendInvalidate(addr, procNum, i);
                        break;
                    }
                }
                currentState->directory[procNum] = 1;
            }
            break;
        case D_SHARED:
            if (reqType == BUSRD) {
                // SEND READ
                for (int i = 0; i < 4; i++) {
                    if (currentState->directory[i] == 1) {
                        sendFetch(addr, i, procNum, rprocNum);
                        break;
                    }
                }
                currentState->directory[procNum] = 1;
            } else {
                // SEND Invalidates to everybody
                for (int i = 0; i < 4; i++) {
                    if (currentState->directory[i] == 1) {
                        sendFetch(addr, i, procNum, rprocNum);
                        break;
                    }
                }
                for (int i = 0; i < 4; i++) {
                    if (currentState->directory[i] == 1) {
                        currentState->directory[i] = 0;
                        sendInvalidate(addr, procNum, i);
                    }
                }
                currentState->directory[procNum] = 1;
                currentState->state = D_EXCLUSIVE;
            }
            break;
        case D_INVALID:
            currentState->directory[procNum] = 1;
            coherComp->cacheReq(FETCH, addr, procNum, rprocNum);
            // add to cache recieving queue - Fetch?
            if (reqType == BUSRD) {
                // SEND DATA TO ASKING PROC
                //coherComp->cacheReq(FETCH, addr, procNum, rprocNum);

                // return SHARED;

                // Send reads to other procs
                currentState->state = D_SHARED;
            } else {
                // MAYBE SEND DATA
                // coherComp->cacheReq(FETCH, addr, procNum, rprocNum);

                // return EXCLUSIVE;

                // Send invalidates to all other copies
                currentState->state = D_EXCLUSIVE;
            }
            break;
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }
    setDirectoryState(addr, procNum, currentState);
    // printf("directory call exit\n");
}


// Takes requests from interconnect and cache controller and places them in queue
// all go to directory
void directoryReq(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum)
{
    //printf("directory req enter ");
    // Add to pending Queue
    // if (pendingRequest == NULL) {
        directory_req* nextReq = calloc(1, sizeof(directory_req));
        nextReq->brt = reqType;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->rprocNum = rprocNum;
        pendingRequest = nextReq;
        countDown = CONTROLLER_DELAY;
        directory(pendingRequest->brt, pendingRequest-> addr, pendingRequest->procNum, pendingRequest->rprocNum);
        free(pendingRequest);
        pendingRequest = NULL;
        // printf("directory req exit\n");
        return;
    // } else {
    //     directory_req* nextReq = calloc(1, sizeof(directory_req));
    //     nextReq -> brt = reqType;
    //     nextReq->addr = addr;
    //     nextReq->procNum = procNum;
    //     nextReq->rprocNum = rprocNum;
    //     // Uhhhh idk lol. The ref doesnt look like it should work so I will wait on this lol
    //     queuedRequests = nextReq;
    // }


}

int tick()
{
    // Start Processing Queue and calls directory()
    // if (countDown > 0)
    // {
    //     countDown--;
    //     if (countDown == 0) {
    //         directory(pendingRequest->brt, pendingRequest-> addr, pendingRequest->procNum, pendingRequest->rprocNum);
    //         free(pendingRequest);
    //         pendingRequest = NULL;
    //     }
    // } else {
    //     if (queuedRequests != NULL) {
    //         pendingRequest = queuedRequests;
    //         // All this may need to be changed
    //         queuedRequests = NULL;
    //         countDown = CONTROLLER_DELAY;
    //     }
    // }
    // printf("tick directory\n");
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
