#include "interconnect_internal.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Nice little hack to stage computation, I think
int l_dim = 0;

/**
 * From Stack Overflow: 
 * https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
 * 
 * So our RNG isn't.. horrific?
 * 
 * Again, we use random() as per cppreference's note on how bad rand() is
 */
uint64_t rand_max_n (uint64_t n) {
    srandom(time(NULL));
    uint64_t buckets = n;
    uint64_t range = (uint64_t) RAND_MAX + 1;
    uint64_t bucket_size = range / buckets;
    uint64_t round = range % buckets;

    int64_t r;
    do {
        r = random();
    } while (range - round <= (uint64_t)r);
    return r / bucket_size;
}

/**
 * Functions to support gridding of the processors
 */
inline int index_to_x(int index) {
    assert(l_dim != 0);
    return index % l_dim;
}

inline int index_to_y(int index) {
    assert(l_dim != 0);
    return index / l_dim;
}

inline int xy_to_index(int x, int y) {
    assert(l_dim != 0);
    return x + y * l_dim;
}

/**
 * Generates a new interconnection network
 * 
 * We will set up the network to properly use the so-called 'XY' routing 
 * algorithm. This is done by laying n many nodes on a square or rectangular
 * grid, and then building the links based on the topology.
 * 
 * For custom-built interconnection networks we resort to using Valiant
 * routing, lest we use static routing, which is easy to accomplish 
 * with a basic use of DFS/BFS inserted into a lookup table. But for us, 
 * randomized routing is more than sufficient.
 */
ic_network_t *new_network(int numProc, network_type type) {
    // We form the grid with some math.h magic:
    l_dim = (int) floor(sqrt(4.0));
    ic_network_t *res = malloc(sizeof(ic_network_t));
    res->type = type;
    res->nodes = malloc(sizeof(ic_node_t) * numProc); // We need to handle this differently in the custom case
    res->endpoints = numProc;
    res->size = numProc;
    switch (type) {
        case CROSSBAR:
            /**
             * In our case, we technically abstract away the concept of crossbar
             * switching and just assume that every processor is connected to
             * every other processor for simplicity
             */
            for (int i = 0; i < numProc; i++) {
                // TODO: double check all of this for type errors
                res->nodes[i].busy = false;
                res->nodes[i].curr_packet = NULL;
                res->nodes[i].id = i;
                res->nodes[i].num_neighbors = numProc - 1;
                res->nodes[i].links = malloc(sizeof(ic_link_t *) * numProc - 1);
                for (int j = 0; j < numProc - 1; j++) {
                    ic_link_t *link = calloc(sizeof(ic_link_t), 1);
                    link->start = i;
                    if (j >= i) {
                        assert(j + 1 < numProc);
                        link->dest = j + 1;
                    } else {
                        link->dest = j;
                    }
                    res->nodes[i].links[j] = link;
                }
            }
            break;
        case RING:
            // For Rings, we actually aim to simulate a double ring
            for (int i = 0; i < numProc; i++) {
                res->nodes[i].busy = false;
                res->nodes[i].curr_packet = NULL;
                res->nodes[i].id = i;
                res->nodes[i].num_neighbors = 2;
                res->nodes[i].links = malloc(sizeof(ic_link_t *) * 2);
                ic_link_t *link0 = calloc(sizeof(ic_link_t), 1);
                ic_link_t *link1 = calloc(sizeof(ic_link_t), 1);
                link0->start = i;
                link0->dest = (i + 1) % numProc;
                link1->start = i;
                link1->dest = (i - 1) < 0 ? (i - 1) + numProc : (i - 1);
                res->nodes[i].links[0] = link0;
                res->nodes[i].links[1] = link1;
            }
            break;
        case MESH:
            // need to construct a grid for this???
            break;
        case TORUS:
            // you too, we won't test this for now???
            break;
        case CUSTOM:
        // We construct the graph from a file in this case
        default:

            break;
    }
    return res;
}

int route(int start, int dest, ic_network_t *graph) {
    if (start == dest) {
        return dest;
    }
    switch(graph->type) {
        case RING:
            ic_link_t *n1 = graph->nodes[start].links[0];
            ic_link_t *n2 = graph->nodes[start].links[1];
            if (n1 && !n1->busy) {
                return n1;
            } else if (n2 && !n2->busy) {
                return n2;
            } else {
              return start;
            }
            break;
        case CROSSBAR:
            break;
        default:
            return -1;
    }
}

void update(ic_network_t *graph) {
    int numNodes = graph->size;

    
}