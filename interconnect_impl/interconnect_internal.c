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
 * We will assume numProc is a perfect square
 */
ic_network_t *new_network(int numProc, network_type type) {
    // We form the grid with some math.h magic:
    l_dim = (int) floor(sqrt(numProc));
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
                link0->busy = false;
                link1->start = i;
                link1->dest = (i - 1) < 0 ? (i - 1) + numProc : (i - 1);
                link1->busy  = false;
                res->nodes[i].links[0] = link0;
                res->nodes[i].links[1] = link1;
            }
            break;
        case MESH:
            // We would have 4 links in middle. Borders have 3. Corners have 2. Ordered as followed:
            //   0
            // 3 x 1
            //   2
            // need to construct a grid for this???
            for (int i = 0; i < numProc; i++) {
                res->nodes[i].busy = false;
                res->nodes[i].curr_packet = NULL;
                res->nodes[i].id = i;
                int x = index_to_x(i);
                int y = index_to_y(i);
                int numNeighbors = 0;
                res->nodes[i].links = malloc(sizeof(ic_link_t) * 4);
                if (y > 0) {
                    numNeighbors++;
                    ic_link_t *link0 = calloc(sizeof(ic_link_t), 1);
                    link0->start = i;
                    link0->dest = xy_to_index(x, y-1);
                    link0->busy = false;
                    res->nodes[i].links[0] = link0;
                }
                if (x < l_dim-1) {
                    numNeighbors++;
                    ic_link_t *link1 = calloc(sizeof(ic_link_t), 1);
                    link1->start = i;
                    link1->dest = xy_to_index(x+1, y);
                    link1->busy = false;
                    res->nodes[i].links[1] = link1;
                }
                if (y < l_dim-1) {
                    numNeighbors++;
                    ic_link_t *link2 = calloc(sizeof(ic_link_t), 1);
                    link2->start = i;
                    link2->dest = xy_to_index(x, y+1);
                    link2->busy = false;
                    res->nodes[i].links[2] = link2;
                }
                if (x > 0) {
                    numNeighbors++;
                    ic_link_t *link3 = calloc(sizeof(ic_link_t), 1);
                    link3->start = i;
                    link3->dest = xy_to_index(x-1, y);
                    link3->busy = false;
                    res->nodes[i].links[3] = link3;
                }
                res->nodes[i].num_neighbors = numNeighbors;
            }
            break;
        case TORUS:
            for (int i = 0; i < numProc; i++) {
                res->nodes[i].busy = false;
                res->nodes[i].curr_packet = NULL;
                res->nodes[i].id = i;
                res->nodes[i].num_neighbors = 4;
                res->nodes[i].links = malloc(sizeof(ic_link_t *) * 4);
                ic_link_t *link0 = calloc(sizeof(ic_link_t), 1);
                ic_link_t *link1 = calloc(sizeof(ic_link_t), 1);
                ic_link_t *link2 = calloc(sizeof(ic_link_t), 1);
                ic_link_t *link3 = calloc(sizeof(ic_link_t), 1);
                int x = index_to_x(i);
                int y = index_to_y(i);
                link0->start = i;
                link1->start = i;
                link2->start = i;
                link3->start = i;
                link0->dest = xy_to_index((x - 1) % l_dim, y);
                link1->dest = xy_to_index(x, (y + 1) % (numProc / l_dim));
                link2->dest = xy_to_index((x + 1) < 0 ? l_dim - 1 : (x - 1), y);
                link3->dest = xy_to_index(x, (y - 1) < 0 ? (numProc / l_dim) - 1 : (y - 1));
                // link[0] goes left, 1 down, 2 right, 3 up
                res->nodes[i].links[0] = link0;
                res->nodes[i].links[1] = link1;
                res->nodes[i].links[2] = link2;
                res->nodes[i].links[3] = link3;
            }
            break;
        case CUSTOM:
        // We construct the graph from a file in this case
        default:
            break;
    }
    return res;
}


// Route returns the link we choose to use.
ic_link_t *route(int start, int dest, ic_network_t *graph) {
    // This should not exist but just in case
    if (start == dest) {
        return NULL;
    }
    int startx;
    int starty;
    int destx;
    int desty;
    switch(graph->type) {
        case RING:
            ic_link_t *n1 = graph->nodes[start].links[0];
            ic_link_t *n2 = graph->nodes[start].links[1];
            if (n1 && !n1->busy) {
                return n1;
            } else if (n2 && !n2->busy) {
                return n2;
            } else {
              return NULL;
            }
            break;
        case CROSSBAR:
            ic_link_t *n;
            if (dest > start) {
                n = graph->nodes[start].links[dest-1];
            } else {
                n = graph->nodes[start].links[dest];
            }
            return n;
            break;
        case MESH:
            startx = index_to_x(start);
            starty = index_to_y(start);
            destx = index_to_x(dest);
            desty = index_to_y(dest);
            if (destx > startx) {
                return graph->nodes[start].links[1];
            } else if (destx < startx) {
                return graph->nodes[start].links[3];
            } else if (desty > starty) {
                return graph->nodes[start].links[2];
            } else {
                return graph->nodes[start].links[0];
            }
            break;
        case TORUS:
            startx = index_to_x(start);
            starty = index_to_y(start);
            destx = index_to_x(dest);
            desty = index_to_y(dest);
            if (destx != startx) {
                return graph->nodes[start].links[2];
            } else {
                return graph->nodes[start].links[1];
            }
            break;
        default:
            return NULL;
    }
}

void update(ic_network_t *graph) {
    // Have to load in request from queue.
    // Iterate

    int numNodes = graph->size;

    uintptr_t **candidates = malloc(sizeof(uintptr_t *) * numNodes);
     for (int i = 0; i < numNodes; i++) {
       candidates[i] = calloc(sizeof(uintptr_t), numNodes);
     }
     int *candc = calloc(sizeof(int), numNodes);

    for (int i = 0; i < numNodes; i++) {
        ic_req *packet = graph->nodes[i].curr_packet;
        if (packet != NULL && graph->nodes[i].busy) {
            // Finds link to go down
            ic_link_t *next_link = route(i, packet->destNum, graph);
            if (next_link != NULL) {
                // candidates[i] = list of links that want send to node i.
                next_link->busy = true;
                next_link->curr_packet = packet;
                int idx = candc[next_link->dest]++;
                candidates[next_link->dest][idx] = (uintptr_t)next_link;
                // Move request out of graph to link. Now have to make sure to keep links.

            }
        }
    }

    for (int i = 0; i < numNodes; i++) {
        // pick a winner for each node
        int count = candc[i];
        if (count > 0 && graph->nodes[i].curr_packet == NULL && !graph->nodes[i].busy) {
            int winner = (int)rand_max_n((uint64_t)count);
            ic_link_t *link = candidates[i][winner];
            graph->nodes[i].curr_packet = link->curr_packet;
            graph->nodes[i].busy = true;
            graph->nodes[link->start].curr_packet = NULL;
            graph->nodes[link->start].busy = false;
            link->curr_packet = NULL;
            link->busy = false;
            printf("Moving a packet\n");

        }
    }
    free(candc);
    for (int l = 0; l < numNodes; l++) {
        free(candidates[l]);
    }
    free(candidates);
}
