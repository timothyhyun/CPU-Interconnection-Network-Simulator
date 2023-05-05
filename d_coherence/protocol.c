#include "coher_internal.h"
#include "directory.h"
#include <math.h>

// NEW PROTOCOLS
// SEND INVALID
// CHANGE ARGS TO INCLUDE PROC NUMBER
// sourceNum: cache procNUm
// Dest:DUH
void sendRd(uint64_t addr, int destNum, int sourceNum) {
    // If to own directory: place in directory rec queue
    // else place in inter->busReq
    printf("%d sourceNum sending read %lX to %d over", sourceNum, addr, destNum);
    if (sourceNum == destNum) {
        // Go to directory
        printf("directly\n");
        direct_sim->directoryReq(BUSRD, addr, destNum, sourceNum);
    } else {
        // INTERCONNECT NEED NEW FUNCTION TONY THIS IS JUST A PLACEHOLDER
        printf("interconnect\n");
        inter_sim->busReq(BUSRD, addr, destNum, sourceNum, sourceNum);
    }
}

void sendWr(uint64_t addr, int destNum, int sourceNum) {
    printf("%d sourceNum sending write %lX to %d over", sourceNum, addr, destNum);

    if (sourceNum == destNum) {
        printf("directly\n");
        direct_sim->directoryReq(BUSWR, addr, destNum, sourceNum);
    } else {
        printf("interconnect\n");
        inter_sim->busReq(BUSWR, addr, destNum, sourceNum, sourceNum);
    }

}

void sendDataBack(uint64_t addr, int destNum, int sourceNum) {
    printf("%d is sending to %d about %lX\n", sourceNum, destNum, addr);
    inter_sim->busReq(DATA, addr, destNum, sourceNum, sourceNum);
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
    printf("%d recieves request on %lX\n", procNum, addr);
    int destNum = findHomeProcessor(addr, procNum);
    switch(currentState)
    {
        case INVALID:
        /*
        IF INVALID: NOT CACHED
        IF WITHIN OWN: GO DIRECTLY TO DIRECTORY ELSE INTERCONNECT
        SAME
        */
            *permAvail = 0;
            if (destNum == procNum) {
                *permAvail = 1;

            }
            if (is_read) {
                // DEST, SOURCE
                sendRd(addr, destNum, procNum);
                return INVALID_SHARED;
            }
            // DEST, SOURCE
            sendWr(addr, destNum, procNum);
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
            if (destNum == procNum) {
                *permAvail = 1;
            }
            // DEST SOURCE
            sendWr(addr, destNum, procNum);
            return SHARED_MODIFIED;
        case INVALID_MODIFIED:
            fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            printf("cacheDirectory exit\n");
            return INVALID_MODIFIED;
        case INVALID_SHARED:
            fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            printf("cacheDirectory exit\n");
            return INVALID_SHARED;
        case SHARED_MODIFIED:
            fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
            *permAvail = 0;
            printf("cacheDirectory exit\n");
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
coherence_states processCache(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum, int replyNum) {
    *ca = NO_ACTION;
    switch(currentState)
    {
        case INVALID:
            // Cache does not have. go to memory (out of scope)
            if (reqType == FETCH) {
                printf("This is a fetch reply");
                sendDataBack(addr, replyNum, procNum);
            }
            printf("processCache exit, %d\n", __LINE__);
            return INVALID;
        case MODIFIED:
            if (reqType = FETCH)
            {
                sendDataBack(addr, replyNum, procNum);
                printf("processCache exit\n");
                return SHARED_STATE;
            }
            else if (reqType = IC_INVALIDATE) {
                *ca = INVALIDATE;
                printf("processCache exit\n");
                return INVALID;
            }
            printf("processCache exit\n");
            return MODIFIED;
        case SHARED_STATE:
            if (reqType = FETCH)
            {
                sendDataBack(addr, replyNum, procNum);
                printf("processCache exit\n");
                return SHARED_STATE;
            }
            else if (reqType = IC_INVALIDATE) {
                *ca = INVALIDATE;
                printf("processCache exit\n");
                return INVALID;
            }
            return SHARED_STATE;
        case INVALID_MODIFIED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                printf("processCache exit\n");
                return MODIFIED;
            }
            printf("processCache exit\n");
            return INVALID_MODIFIED;
        case INVALID_SHARED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                printf("processCache exit\n");
                return SHARED_STATE;
            }
            printf("processCache exit\n");
            return INVALID_SHARED;
        case SHARED_MODIFIED:
            if (reqType == DATA || reqType == SHARED)
            {
                *ca = DATA_RECV;
                printf("processCache exit\n");
                return MODIFIED;
            }
            printf("processCache exit\n");
            return SHARED_MODIFIED;
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }
    return INVALID;
}
