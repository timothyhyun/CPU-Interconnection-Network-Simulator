#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef struct list {
  int data;
  struct list *next;
} list_t;

typedef struct stack {
  list_t *head;
  list_t *tail; // Always points to last element of the list
} stack_t;

int pop(stack_t *S) {
  if (S->head == NULL) {
    return NULL;
  } 
  int ret = S->head->data;
  list_t *tmp = S->head;
  if (S->head == S->tail) {
    assert(S->tail->next == S->head->next && S->head->next == NULL);
    S->tail = NULL;
  }
  S->head = S->head->next;
  free(tmp);
  return ret;
}

void push(stack_t *S, int data) {
  if (S->head == NULL) {
    assert(S->tail == NULL);
    list_t *new = malloc(sizeof(list_t));
    new->data = data;
    new->next = NULL;
    S->head = new;
    S->tail = new;
    return;
  }
  list_t *new = malloc(sizeof(list_t));
  new->data = data;
  new->next = NULL;
  S->tail->next = new;
  S->tail = new;
}

stack_t *new_stack() {
  stack_t *new = malloc(sizeof(stack_t));
  new->head = NULL;
  new->tail = NULL;
}

bool empty(stack_t *S) {
  return S->head == NULL;
}