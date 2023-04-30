#include "coher_internal.h"

void sendBusRd(uint64_t addr, int procNum)
{
    inter_sim->busReq(BUSRD, addr, procNum);
}

void sendBusWr(uint64_t addr, int procNum)
{
    inter_sim->busReq(BUSWR, addr, procNum);
}

void sendData(uint64_t addr, int procNum)
{
    inter_sim->busReq(DATA, addr, procNum);
}

void indicateShared(uint64_t addr, int procNum)
{
    inter_sim->busReq(SHARED, addr, procNum);
}

coherence_states cacheMSI(uint8_t is_read, uint8_t* permAvail, coherence_states currentState, uint64_t addr, int procNum)
{
    switch(currentState)
    {
        case INVALID:
            *permAvail = 0;
            if (is_read) 
            {
                sendBusRd(addr, procNum);
                return INVALID_SHARED;
            } 
            // TODO: what if we evict an INVALID line?
            if(is_read & 0x2) {
                return INVALID;
            }
            sendBusWr(addr, procNum);
            return INVALID_MODIFIED;
        case MODIFIED:
            *permAvail = 1;
            // TODO: eviction case
            if(is_read & 0x2) { /* how will we determine if it's an evict or not? */
                // flush
                return MODIFIED_INVALID; // new MI state
            }
            return MODIFIED;
        case SHARED_STATE:
            if (is_read) 
            {
                *permAvail = 1;
                return SHARED_STATE;
            } 
            // TODO: eviction case
            if (is_read & 0x2) { /* how will we determine if it's an evict or not? */
                return INVALID;
            }
            *permAvail = 0;
            sendBusWr(addr, procNum);
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

coherence_states snoopMSI(bus_req_type reqType, cache_action* ca, coherence_states currentState, uint64_t addr, int procNum)
{
    *ca = NO_ACTION;
    switch(currentState)
    {
        case INVALID:
            return INVALID;
        case MODIFIED:
            if (reqType = BUSWR) 
            {
                sendData(addr, procNum);
                //indicateShared(addr, procNum); // Needed for E state
                *ca = INVALIDATE;
                return INVALID;
            } 
            else if (reqType = BUSRD) {
                sendData(addr, procNum);
                return SHARED_STATE;
            }
            return MODIFIED;
        case SHARED_STATE:
            if (reqType == BUSWR) 
            {
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
        // Handle eviction intermediates here
        default:
            fprintf(stderr, "State %d not supported, found on %lx\n", currentState, addr);
            break;
    }
    
    return INVALID;
}