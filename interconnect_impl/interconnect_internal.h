/**
 * @file interconnect.h
 * @brief Header file which declares all relevant datatypes for interconnect_internal.c
 *
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef INTERCONNECT_INTERNAL_H
#define INTERCONNECT_INTERNAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <interconnect.h>

typedef enum {
  CROSSBAR = 0,
  RING,
  MESH,
  TORUS,
  CUSTOM
} network_type;

typedef enum _ic_req_state {
    NONE,
    QUEUED,
    TRANSFERING,
    WAITING_CACHE,
    WAITING_MEMORY
} ic_req_state;

typedef struct _ic_req {
    bus_req_type brt;
    ic_req_state currentState;
    uint64_t addr;
    int procNum;
    int rprocNum;
    int nextProcNum;
    uint8_t shared;
    uint8_t data;
    struct _ic_req *next;
} ic_req;

typedef struct ic_node {
    int id; // > 1 and < ic_network->order if it's an endpoint, else > order
    ic_req *curr_packet; 
    int *connected;
    int num_neighbors;
    bool busy;
} ic_node_t;

/**
 * Currently size and endpoints fields are not distinct in purpose
 * 
 * In our interconnection network, we abstract away the concept of a link
 * Instead, nodes that wish to send to another node all "register" to send
 * We then perform arbitration by randomly selecting a node to progress, to
 * simulate the actual handling of race conditions. Nodes that fail to send
 * will simply hold their packets in their per-node mailbox.
 */
typedef struct ic_network {
  network_type type;
  ic_node_t *nodes; 
  int endpoints; // only counts endpoints
  int size; // size of nodes
} ic_network_t;

#endif