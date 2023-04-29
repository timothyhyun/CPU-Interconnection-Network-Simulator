#include <directory.h>
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
directory_sim* self = NULL;
coher* coherComp;

const int CONTROLLER_DELAY = 5;


// Sending Functions: 

// send from directory to cache
// directoryNum sending fetch request to procNum which will send data back to rprocNum
void sendFetch(uint64_t addr, int procNum, int rprocNum, int directoryNum) {
    // need additional arg in busreq for destination
    if (procNum == directoryNum) {
        coherComp->cacheReq(FETCH, addr, procNum, rprocNum);
    } else {
        // interconnect request
    }
}

// directoryNum sending invalidate to procNum
void sendInvalidate(uint64_t addr, int procNum, int directoryNum){
    if (procNum == directoryNum) {
        coherComp->cacheReq(INVALIDATE, addr, procNum, -1);
    } else {
        // Interconnect request
    }
}



void directoryReq(bus_req_type reqType, directory_states currentState, uint64_t addr, int procNum);
void registerCoher(coher *cc);

directory_sim* init(directory_sim_args* dsa) 
{
    directoryStates = malloc(sizeof(tree_t*) * processorCount);

    for (int i = 0; i < processorCount; i++)
    {
        directoryStates[i] = tree_new();
    }
    inter_sim = dsa->inter;
    self = malloc(sizeof(directory_sim));
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

directory_states getDirectoryState(uint64_t addr, int procNum) {
    directory_states lookState = (directory_states) tree_find(directoryStates[procNum], addr);
    if (lookState == NULL){
        lookState.state = INVALID;
        lookState.directory = [0,0,0,0];
    }

    return lookState;
}

void setDirectoryState(uint64_t addr, int processorNum, directory_states nextState) 
{
    tree_insert(directoryStates[processorNum], addr, (void*)nextState);
}

// rprocnum: originating processor 
// procnum: current processor number
directory_states directory(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum) {
    directory_states currentState = getDirectoryState(addr, procNum);
    switch(currentState.state) {
        case EXCLUSIVE:
            if (reqType == BUSRD) {
                // SEND FETCH
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        sendFetch(addr, rprocNum, i, procNum);
                        break;
                    }
                }
                // DEMOTE TO SHARED AND ADD PROCESSOR
                currentState.directory[procNum] = 1;
                currentState.state = SHARED_STATE;
            } else {
                // SEND Invalidates to everybody
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        currentState.directory[i] = 0;
                        sendFetch(addr, rprocNum, i, procNum);
                        sendInvalidate(addr, procNum, i);
                        break;
                    }
                }
                currentState.directory[procNum] = 1;
            }
            break;
        case SHARED_STATE:
            if (reqType == BUSRD) {
                // SEND READ
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        sendFetch(addr, rprocNum, i, procNum);
                        break;
                    }
                }
                currentState.directory[procNum] = 1;
            } else {
                // SEND Invalidates to everybody
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        sendFetch(addr, rprocNum, i, procNum);
                        break;
                    }
                }
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        currentState.directory[i] = 0;
                        sendInvalidate(addr, procNum, i);
                    }
                }
                currentState.directory[procNum] = 1;
                currentState.state = EXCLUSIVE;
            }
            break;
        case INVALID:
            currentState.directory[procNum] = 1;
            // add to cache recieving queue - Fetch?
            if (reqType == BUSRD) {
                // SEND DATA TO ASKING PROC
                return SHARED_STATE;
            } else {
                // MAYBE SEND DATA
                return EXCLUSIVE;
            }
            break;
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }
    setDirectoryState(addr, procNum, currentState);
}


// Takes requests from interconnect and cache controller and places them in queue
// all go to directory
void directoryReq(bus_req_type reqType, uint64_t addr, int procNum, int rprocNum)
{
    // Add to pending Queue
    if (pendingRequest == NULL) {
        directory_req* nextReq = calloc(1, sizeof(directory_req));
        nextReq->brt = reqType;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->rprocNum = rprocNum;
        pendingRequest = nextReq;
        countDown = CONTROLLER_DELAY;
        return;
    } else {
        directory_req* nextReq = calloc(1, sizeof(directory_req));
        nextReq -> brt = reqType;
        nextReq->addr = addr;
        nextReq->procNum = procNum;
        nextReq->rprocNum = rprocNum;
        // Uhhhh idk lol. The ref doesnt look like it should work so I will wait on this lol
        queuedRequests = nextReq;
    }


}

int tick() 
{
    // Start Processing Queue and calls directory()
    if (countDown > 0)
    {
        countDown--;
        if (countDown == 0) {
            directory(pendingRequest->brt, pendingRequest-> addr, pendingRequest->procNum, pendingRequest->rprocNum);
            free(pendingRequest);
            pendingRequest = NULL;
        }
    } else {
        if (queuedRequests != NULL) {
            pendingRequest = queuedRequests;
            // All this may need to be changed
            queuedRequests = NULL;
            countDown = CONTROLLER_DELAY;
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
