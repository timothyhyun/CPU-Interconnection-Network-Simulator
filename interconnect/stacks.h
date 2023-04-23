/**
 * @file stacks.h
 * @brief stacks interface
 *
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef STACKS_H
#define STACKS_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct stack stack_t;

int pop(stack_t *S);
void push(stack_t *S, int data);

stack_t *new_stack();
bool empty(stack_t *S);

#endif