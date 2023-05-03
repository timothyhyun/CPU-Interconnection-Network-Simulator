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
            for (int i = 0; i < numProc; i++) {
                res->nodes[i].busy = false;
                res->nodes[i].curr_packet = NULL;
                res->nodes[i].id = i;
                res->nodes[i].num_neighbors = numProc - 1;
                res->nodes[i].connected = malloc(sizeof(int) * numProc - 1);
                for (int j = 0; j < numProc - 1; j++) {
                    if (j >= i) {
                        res->nodes[i].connected[j] = j + 1;
                    } else {
                        res->nodes[i].connected[j] = j;
                    }
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
                res->nodes[i].connected = malloc(sizeof(int) * 2);
                res->nodes[i].connected = (i + 1) % numProc;
                res->nodes[i].connected = (i - 1) < 0 ? (i - 1) + numProc : (i - 1);
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
            int n1 = graph->nodes[start].connected[0];
            int n2 = graph->nodes[start].connected[1];
            if (!graph->nodes[n1].busy) {
                return n1;
            } else if (!graph->nodes[n2].busy) {
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

    // Candidate arrays keep track of everyone that tries to do an action to i
    // We randomly select one actor to "win" and everyone else fails.
    int **candidates = malloc(sizeof(int *) * numNodes);
    for (int i = 0; i < numNodes; i++) {
      candidates[i] = calloc(sizeof(int), numNodes);
    }
    int *candc = calloc(sizeof(int), numNodes);
    for (int i = 0; i < numNodes; i++) {
        ic_req *packet = graph->nodes[i].curr_packet;
        if (packet == NULL) {
          continue;
        }
        int next_node = route(i, packet->procNum, graph);

        // Keep track of the fact that i tried to touch next_node
        if (!graph->nodes[next_node].busy) {
            int idx = candc[next_node]++;
            candidates[next_node][idx] = i;
        }
    }
    
    /**
     * This part of the simulation is particularly crude and technically
     * not 100% faithful to how things really work, but oh well.
     * In particular, this is because we abstracted links when we really
     * should model them literally
     * 
     * Also fails to capture the idea of simultaneous update.
     */
    for (int i = 0; i < numNodes; i++) {
        int m = candc[i];
        int winner = (int)rand_max_n((uint64_t) m);
        assert(!graph->nodes[i].busy);
        graph->nodes[i].curr_packet = graph->nodes[winner].curr_packet;
        graph->nodes[i].busy = true;
        graph->nodes[winner].curr_packet = NULL;
        graph->nodes[winner].busy = false;
    }
}