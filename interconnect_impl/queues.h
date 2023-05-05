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

typedef struct list
{
  void *data;
  struct list *next;
  struct list *prev;
} list_t;

typedef struct queue
{
  list_t *q;
  list_t *tail;
  int countDown;
} queue_t;

void enq(queue_t *Q, void *data);
void *dq(queue_t *Q);

queue_t *new_Q();
bool empty(queue_t *Q);

#endif