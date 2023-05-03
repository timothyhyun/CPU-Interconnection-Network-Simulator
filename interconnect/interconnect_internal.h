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
  NONE = 0
} i_req_status;

typedef struct packet {
  int src;
  int dest;
  int size; // useless and we ignore this for simplicity
} packet_t;

// What will we use this for?
typedef struct i_req {
    bus_req_type irt;
    i_req_status currentState;
    uint64_t addr;
    int procNum;
    uint8_t shared;
    uint8_t data;
    struct i_req *next; // not sure if this is needed rn
} i_req_t;

typedef struct ic_node {
    int id; // > 1 and < ic_network->order if it's an endpoint, else > order
    packet_t *curr_packet;
    int *connected;
    int num_neighbors;
    bool busy;
} ic_node_t;

typedef struct ic_network {
  ic_node_t *nodes; 
  int endpoints; // only counts endpoints
  int size; // includes all intermediate nodes
} ic_network_t;

#endif