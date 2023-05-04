#include "coher_internal.h"
#include "directory.h"
#include <math.h>

// NEW PROTOCOLS
// SEND INVALID
// CHANGE ARGS TO INCLUDE PROC NUMBER
// ProcNum: directory NUm (destination)
// RProcNum: requesting proc
void sendRd(uint64_t addr, int procNum, int rprocNum) {
    // If to own directory: place in directory rec queue
    // else place in inter->busReq
    if (procNum == rprocNum) {
        // Go to directory
        direct_sim->directoryReq(BUSRD, addr, procNum, rprocNum);
    } else {
        // INTERCONNECT NEED NEW FUNCTION TONY THIS IS JUST A PLACEHOLDER
        inter_sim->busReq(BUSRD, addr, procNum, rprocNum, -1);
    }
}

void sendWr(uint64_t addr, int procNum, int rprocNum) {
    if (procNum == rprocNum) {
        direct_sim->directoryReq(BUSWR, addr, procNum, rprocNum);
    } else {
        inter_sim->busReq(BUSWR, addr, procNum, rprocNum, -1);
    }

}

void sendDataBack(uint64_t addr, int procNum, int rprocNum) {
    // Cache to Cache
    inter_sim->busReq(DATA, addr, procNum, rprocNum, -1);
}

long bitMask(long highbit, long lowbit)
{
    long mask1 = ((0x01L << 63) >> 63) ^ (0x01L << 63);
    long hmask = mask1 >> (63 - highbit) | (0x01L << highbit);
    long lmask = ((0x01L << 63) >> 63) << lowbit;
    return hmask & lmask;
}

int mylog(int x) {
  int result = 0;
  while (x >>= 1) result++;
  return result;
}

int findHomeProcessor(uint64_t addr, int procNum) {
    int k = mylog(processorCount);
    size_t tag = addr >> (cache_s + cache_b);
    long mask = bitMask(k - 1, 0);
    return mask & tag;
}

coherence_states cacheDirectory(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum) {
    int dest = findHomeProcessor(addr, procNum);
    switch(currentState)
    {
        case INVALID:
        /*
        IF INVALID: NOT CACHED
        IF WITHIN OWN: GO DIRECTLY TO DIRECTORY ELSE INTERCONNECT
        SAME
        */
            *permAvail = 0;
            if (is_read)
            {
                sendRd(addr, procNum, dest);
                return INVALID_SHARED;
            }
            sendWr(addr, procNum, dest);
            return INVALID_MODIFIED;
        case MODIFIED:
        /*
        NOTHING
        */
            *permAvail = 1;
            return MODIFIED;
        case SHARED_STATE:
        /*
        IF READ: NOTHING
        IF WRITE:
        IF WITHIN GO TO DIRECTORY ELSE INTERCONNECT

        */
            if (is_read)
            {
                *permAvail = 1;
                return SHARED_STATE;
            }
            *permAvail = 0;
            sendWr(addr, procNum, dest);
            return SHARED_MODIFIED;
        case INVALID_MODIFIED:
            fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            return INVALID_MODIFIED;
        case INVALID_SHARED:
            fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            return INVALID_SHARED;
        case SHARED_MODIFIED:
            fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            return SHARED_MODIFIED;
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }

    return INVALID;
}

// Directory version of "snooping"
// Cache recieves request from directory
// Not sure why procNum is the origin processor or how else do we send anything back???
// ONLY CAN RECIEVE:
// Invalidates
// Fetch
coherence_states processCache(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum, int rprocNum) {
    *ca = NO_ACTION;
    switch(currentState)
    {
        case INVALID:
            // Cache does not have. go to memory (out of scope)
            return INVALID;
        case MODIFIED:
            if (reqType = FETCH)
            {
                sendDataBack(addr, procNum, rprocNum);
                return SHARED_STATE;
            }
            else if (reqType = IC_INVALIDATE) {
                *ca = INVALIDATE;
                return INVALID;
            }
            return MODIFIED;
        case SHARED_STATE:
            if (reqType = FETCH)
            {
                sendDataBack(addr, procNum, rprocNum);
                return SHARED_STATE;
            }
            else if (reqType = IC_INVALIDATE) {
                *ca = INVALIDATE;
                return INVALID;
            }
            return SHARED_STATE;
        case INVALID_MODIFIED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                return MODIFIED;
            }
            return INVALID_MODIFIED;
        case INVALID_SHARED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                return SHARED_STATE;
            }
            return INVALID_SHARED;
        case SHARED_MODIFIED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                return MODIFIED;
            }
            return SHARED_MODIFIED;
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }
    return INVALID;
}
