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

/**
 * Simulates a one-directional link from a -> b
 * So for a <-> b we have two one-directional links for simplicity
 * Implies that all of our links are multiplexed too, which is cool.
 */
typedef struct ic_link {
    int start;
    int dest;
    ic_req *curr_packet;
    bool busy; // This exists purely as a safety check
} ic_link_t;

typedef struct ic_node {
  // Order of Graph: number of vertices
    int id; // > 1 and < ic_network->order if it's an endpoint, else > order
    ic_req *curr_packet; 
    ic_link_t **links;
    int num_neighbors;
    bool busy;
} ic_node_t;

/**
 * Currently size and endpoints fields are not distinct in purpose
 */
typedef struct ic_network {
  network_type type;
  ic_node_t *nodes; 
  int endpoints; // only counts endpoints
  int size; // size of nodes
  // ic_link_t ***in_links; // record all links into nodes
} ic_network_t;

ic_network_t *new_network(int numProc, network_type type);
void update(ic_network_t *graph);

#endif