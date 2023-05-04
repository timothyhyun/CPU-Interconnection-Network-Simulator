/**
 * @file queues.h
 * @brief queues interface
 *
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef QUEUES_H
#define QUEUES_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct queue queue_t;

int enq(queue_t *Q);
void deq(queue_t *Q, void *data);

queue_t *new_Q();
bool empty(queue_t *Q);

#endif