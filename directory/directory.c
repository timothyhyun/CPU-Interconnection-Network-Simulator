#include <directory.h>
#include "stree.h"



tree_t** directoryStates = NULL;
int processorCount = 1;
interconn* inter_sim = NULL;
directory_sim* self = NULL;
coher* coherComp;

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


    return self;
} 


void registerCoher(coher* cc)
{
    coherComp = cc;
}



directory_states getDirectoryState() {
    directory_states lookState = (directory_states) tree_find(directoryStates[processorNum], addr);
    if (lookState.state == UNDEF) return INVALID;

    return lookState;
}

void setDirectoryState(uint64_t addr, int processorNum, directory_states nextState) 
{
    tree_insert(directoryStates[processorNum], addr, (void*)nextState);
}


// procnum: current processor number
directory_states directory(bus_req_type reqType, directory_states currentState, uint64_t addr, int procNum) {
    switch(currentState.state) {
        case EXCLUSIVE:
            if (reqType == BUSRD) {
                // SEND FETCH
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        sendFetch(addr, procNum, i);
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
                        sendFetch(addr, procNum, i);
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
                        sendFetch(addr, procNum, i);
                        break;
                    }
                }
                currentState.directory[procNum] = 1;
            } else {
                // SEND Invalidates to everybody
                for (int i = 0; i < 4; i++) {
                    if (currentState.directory[i] == 1) {
                        sendFetch(addr, procNum, i);
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
    return currentState;
}













// Takes requests from interconnect and cache controller and places them in queue
// all go to directory
void directoryReq(bus_req_type reqType, directory_states currentState, uint64_t addr, int procNum)
{
    // Add to pending Queue


}

int tick() 
{
    // Start Processing Queue and calls directory()
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
