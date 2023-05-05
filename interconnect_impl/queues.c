#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "queues.h"

void enq(queue_t *Q, void *data)
{
  list_t *new = malloc(sizeof(list_t));
  if (new == NULL)
  {
    exit(1);
  }
  new->data = data;
  new->next = NULL;
  new->prev = Q->tail;
  if (!Q->q)
  {
    Q->tail = new;
    Q->q = new;
  }
  else
  {
    Q->tail->next = new;
    Q->tail = new;
  }
}

void *dq(queue_t *Q)
{
  if (!Q->q)
  {
    return NULL;
  }
  if (Q->q == Q->tail)
    Q->tail = NULL;
  void *res = Q->q->data;
  list_t *head = Q->q;
  Q->q = Q->q->next;
  if (Q->q)
    Q->q->prev = NULL;
  free(head);
  return res;
}

queue_t *new_Q()
{
  queue_t *new = malloc(sizeof(queue_t));
  if (new == NULL)
  {
    exit(1);
  }
  new->q = NULL;
  new->tail = NULL;
  return new;
}

bool empty(queue_t *Q)
{
  return Q->q == NULL;
}
