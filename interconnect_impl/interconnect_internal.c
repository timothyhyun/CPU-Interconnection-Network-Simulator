#include "interconnect_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Nice little hack to stage computation, I think
int l_dim = 0;


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

void route(int start, int dest) {

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
        // implement XY routing logic here, but here is the catch
        int next_node; // TODO: = some logic here

        // Keep track of the fact that i tried to touch next_node
        if (!graph->nodes[next_node].busy) {
            int idx = candc[next_node]++;
            candidates[next_node][idx] = i;
        }
    }
}