#include "coher_internal.h"
#include "directory.h"


// void sendBusRd(uint64_t addr, int procNum)
// {
//     inter_sim->busReq(BUSRD, addr, procNum);
// }

// void sendBusWr(uint64_t addr, int procNum)
// {
//     inter_sim->busReq(BUSWR, addr, procNum);
// }

// void sendData(uint64_t addr, int procNum)
// {
//     inter_sim->busReq(DATA, addr, procNum);
// }

// void indicateShared(uint64_t addr, int procNum)
// {
//     inter_sim->busReq(SHARED, addr, procNum);
// }

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



int findHomeProcessor(uint64_t addr, int procNum) {
    return procNum;
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
            else if (reqType = INVALIDATE) {
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
            else if (reqType = INVALIDATE) {
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

// BELOW IS SNOOPING NOT NEEDED

// coherence_states cacheMI(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
// {
//     switch(currentState)
//     {
//         case INVALID:
//             *permAvail = 0;
//             sendBusWr(addr, procNum);
//             return INVALID_MODIFIED;
//         case MODIFIED:
//             *permAvail = 1;
//             return MODIFIED;
//         case INVALID_MODIFIED:
//             fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states snoopMI(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
// {
//     *ca = NO_ACTION;
//     switch(currentState)
//     {
//         case INVALID:
//             return INVALID;
//         case MODIFIED:
//             sendData(addr, procNum);
//             //indicateShared(addr, procNum); // Needed for E state
//             *ca = INVALIDATE;
//             return INVALID;
//         case INVALID_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }

//             return INVALID_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states cacheMSI(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
// {
//     switch(currentState)
//     {
//         case INVALID:
//             *permAvail = 0;
//             if (is_read)
//             {
//                 sendBusRd(addr, procNum);
//                 return INVALID_SHARED;
//             }
//             sendBusWr(addr, procNum);
//             return INVALID_MODIFIED;
//         case MODIFIED:
//             *permAvail = 1;
//             return MODIFIED;
//         case SHARED_STATE:
//             if (is_read)
//             {
//                 *permAvail = 1;
//                 return SHARED_STATE;
//             }
//             *permAvail = 0;
//             sendBusWr(addr, procNum);
//             return SHARED_MODIFIED;
//         case INVALID_MODIFIED:
//             fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }
// // MOVES Transistion states to actual
// coherence_states snoopMSI(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
// {
//     *ca = NO_ACTION;
//     switch(currentState)
//     {
//         case INVALID:
//             return INVALID;
//         case MODIFIED:
//             if (reqType = BUSWR)
//             {
//                 sendData(addr, procNum);
//                 //indicateShared(addr, procNum); // Needed for E state
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             else if (reqType = BUSRD) {
//                 sendData(addr, procNum);
//                 return SHARED_STATE;
//             }
//             return MODIFIED;
//         case SHARED_STATE:
//             if (reqType == BUSWR)
//             {
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             return SHARED_STATE;
//         case INVALID_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return SHARED_STATE;
//             }
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states cacheMESI(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
// {
//     switch(currentState)
//     {
//         case INVALID:
//             *permAvail = 0;
//             if (is_read)
//             {
//                 sendBusRd(addr, procNum);
//                 return INVALID_SHARED;
//             }
//             sendBusWr(addr, procNum);
//             return INVALID_MODIFIED;
//         case MODIFIED:
//             *permAvail = 1;
//             return MODIFIED;
//         case SHARED_STATE:
//             if (is_read)
//             {
//                 *permAvail = 1;
//                 return SHARED_STATE;
//             }
//             *permAvail = 0;
//             sendBusWr(addr, procNum);
//             return SHARED_MODIFIED;
//         case EXCLUSIVE:
//             *permAvail = 1;
//             if (is_read) {
//                 return EXCLUSIVE;
//             }
//             return MODIFIED;
//         case INVALID_MODIFIED:
//             fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states snoopMESI(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
// {
//     *ca = NO_ACTION;
//     switch(currentState)
//     {
//         case INVALID:
//             return INVALID;
//         case MODIFIED:
//             if (reqType = BUSWR)
//             {
//                 sendData(addr, procNum);
//                 // indicateShared(addr, procNum); // When Do we need this?
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             // TODO: flushed?
//             sendData(addr, procNum);
//             return SHARED_STATE;
//         case SHARED_STATE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate?
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             return SHARED_STATE;
//         case EXCLUSIVE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate?
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             indicateShared(addr, procNum);
//             return SHARED_STATE;
//         case INVALID_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             if (reqType == DATA)
//             {
//                 *ca = DATA_RECV;
//                 return EXCLUSIVE;
//             }
//             if (reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 indicateShared(addr, procNum);
//                 return SHARED_STATE;
//             }
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states cacheMOESI(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
// {
//     switch(currentState)
//     {
//         case INVALID:
//             *permAvail = 0;
//             if (is_read)
//             {
//                 sendBusRd(addr, procNum);
//                 return INVALID_SHARED;
//             }
//             sendBusWr(addr, procNum);
//             return INVALID_MODIFIED;
//         case MODIFIED:
//             *permAvail = 1;
//             return MODIFIED;
//         case SHARED_STATE:
//             if (is_read)
//             {
//                 *permAvail = 1;
//                 return SHARED_STATE;
//             }
//             *permAvail = 0;
//             sendBusWr(addr, procNum);
//             return SHARED_MODIFIED;
//         case EXCLUSIVE:
//             *permAvail = 1;
//             if (is_read) {
//                 return EXCLUSIVE;
//             }
//             return MODIFIED;
//         case OWNED:
//             if (is_read) {
//                 *permAvail = 1;
//                 return OWNED;
//             }
//             *permAvail = 0;
//             // TODO: Something here?
//             sendBusWr(addr, procNum);
//             return OWNED_MODIFIED;
//         case INVALID_MODIFIED:
//             fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states snoopMOESI(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
// {
//     *ca = NO_ACTION;
//     switch(currentState)
//     {
//         case INVALID:
//             return INVALID;
//         case MODIFIED:
//             if (reqType = BUSWR)
//             {
//                 sendData(addr, procNum);
//                 // indicateShared(addr, procNum); // Needed for E state
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             // TODO: Flushed?
//             sendData(addr, procNum);
//             return OWNED;
//         case SHARED_STATE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate + flush?
//                 sendData(addr, procNum);
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             return SHARED_STATE;
//         case EXCLUSIVE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate + flush?
//                 sendData(addr, procNum);
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             // TODO: do we need to flush?
//             sendData(addr, procNum);
//             indicateShared(addr, procNum);
//             return SHARED_STATE;
//         case OWNED:
//             if (reqType == BUSRD) {
//                 // TODO: flushed?
//                 sendData(addr, procNum);
//                 return OWNED;
//             }
//             sendData(addr, procNum);
//             // indicateShared(addr, procNum);
//             *ca = INVALIDATE;
//             return INVALID;
//         case INVALID_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             if (reqType == DATA)
//             {
//                 *ca = DATA_RECV;
//                 return EXCLUSIVE;
//             }
//             if (reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 indicateShared(addr, procNum);
//                 return SHARED_STATE;
//             }
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return SHARED_MODIFIED;
//         case OWNED_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return OWNED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states cacheMESIF(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
// {
//     switch(currentState)
//     {
//         case INVALID:
//             *permAvail = 0;
//             if (is_read)
//             {
//                 sendBusRd(addr, procNum);
//                 return INVALID_SHARED;
//             }
//             sendBusWr(addr, procNum);
//             return INVALID_MODIFIED;
//         case MODIFIED:
//             *permAvail = 1;
//             return MODIFIED;
//         case SHARED_STATE:
//             // *permAvail = 1; <-- do we nede this?
//             if (is_read)
//             {
//                 *permAvail = 1;
//                 return SHARED_STATE;
//             }
//             *permAvail = 0;
//             sendBusWr(addr, procNum);
//             return SHARED_MODIFIED;
//         case EXCLUSIVE:
//             *permAvail = 1;
//             if (is_read) {
//                 return EXCLUSIVE;
//             }
//             return MODIFIED;
//         case FORWARD:
//             *permAvail = 1;
//             if (is_read) {
//                 return FORWARD;
//             }
//             sendBusWr(addr, procNum);
//             return FORWARD_MODIFIED;
//         case INVALID_MODIFIED:
//             fprintf(stderr, "IM state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             fprintf(stderr, "IS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             fprintf(stderr, "MS state on %lx, but request %d\n", addr, is_read);
//             *permAvail = 0;
//             return SHARED_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }

// coherence_states snoopMESIF(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
// {
//     *ca = NO_ACTION;
//     switch(currentState)
//     {
//         case INVALID:
//             return INVALID;
//         case MODIFIED:
//             if (reqType = BUSWR)
//             {
//                 sendData(addr, procNum);
//                 // indicateShared(addr, procNum); // When Do we need this?
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             // TODO: flushed?
//             sendData(addr, procNum);
//             return SHARED_STATE;
//         case SHARED_STATE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate?
//                 sendData(addr, procNum);
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             return SHARED_STATE;
//         case EXCLUSIVE:
//             if (reqType == BUSWR)
//             {
//                 // TODO: do we need to invalidate?
//                 sendData(addr, procNum);
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             if (reqType == BUSRD) return SHARED_STATE;
//             return EXCLUSIVE;
//         case FORWARD:
//             if (reqType == BUSWR)
//             {
//                 sendData(addr, procNum);
//                 *ca = INVALIDATE;
//                 return INVALID;
//             }
//             if (reqType == BUSRD) {
//                 sendData(addr, procNum);
//                 return SHARED_STATE;
//             }
//             return FORWARD;
//         case INVALID_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return INVALID_MODIFIED;
//         case INVALID_SHARED:
//             if (reqType == DATA)
//             {
//                 *ca = DATA_RECV;
//                 return EXCLUSIVE;
//             }
//             if (reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return FORWARD;
//             }
//             return INVALID_SHARED;
//         case SHARED_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return SHARED_MODIFIED;
//         case FORWARD_MODIFIED:
//             if (reqType == DATA || reqType == SHARED)
//             {
//                 *ca = DATA_RECV;
//                 return MODIFIED;
//             }
//             return FORWARD_MODIFIED;
//         default:
//             fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
//             break;
//     }

//     return INVALID;
// }
