/**
 * @file processor_internal.h
 * @brief Header file which declares all relevant datatypes for processor.c
 * @author Aaron Tan <ahtan@andrew.cmu.edu>
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef PROCESSOR_INTERNAL_H
#define PROCESSOR_INTERNAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "cache.h"
#include "trace.h"

extern const int FU0_MAX_AGE;
extern const int FU1_MAX_AGE;
extern uint64_t CURR_CYCLE;

extern cache *cs;
extern int64_t *memOpTag;
extern int *pendingMem;
extern uint64_t CURR_PRINT_TAG;

typedef struct vbs_dat
{
  uint64_t stage_fetch;
  uint64_t stage_dispatch;
  uint64_t stage_schedule;
  uint64_t stage_execute;
  uint64_t stage_su;
} verbose_dat;

typedef struct p_params
{
  size_t D;
  size_t F;
  size_t m;
  size_t j;
  size_t k;
  size_t c;
} p_params;

typedef struct instruction
{
  int64_t tag;
  enum op_type op;
  int src_reg[2];
  int64_t src_tag[2];
  int dest_reg;
  bool ready[2];
  verbose_dat *vdat;
  uint64_t memAddress;
  int size;
  int proc;
  int thread;
  bool executed;
  bool ins_ready;
  bool mispredict;
  bool retire;
} instruction_t;

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
  int capacity;
} queue_t;

typedef struct shortALU
{
  list_t *stage1;
  bool busy;
} shortALU_t;

typedef struct longALU
{
  list_t *stage1;
  list_t *stage2;
  list_t *stage3;
  bool busy;
} longALU_t;

typedef struct RegisterFiles
{
  int64_t tag;
  bool busy;
} regFile;

typedef struct CDBus
{
  int64_t tag;
  int dest_proc;
  int dest;
  bool busy;
} cdBus;

typedef struct processor
{
  p_params *params;
  queue_t *disp_queue[2];
  queue_t *sche_queue_0;
  queue_t *sche_queue_1;
  shortALU_t *funcUnit0;
  list_t **fu0_dq;
  list_t **fu1_dq;
  longALU_t *funcUnit1;
  queue_t *busQueue;
  cdBus *resBusses;
  regFile registers[2][33];
  queue_t *defer_queue;
  queue_t *cache_queue;
  bool fetch_unit_go[2];
  bool cache_busy;
  int sche_thread;
} processor_t;

// Queue operations declared in header
queue_t *init_q(int cap);
bool enq(queue_t *Q, void *data);
bool enq_sort(queue_t *Q, void *data, int (*cmp)(void *, void *));
void *dq(queue_t *Q);
void *peek(queue_t *Q);
bool empty(queue_t *Q);

// Support functions for verbose and for non-verbose
void vbs_enq(queue_t *Q, void *data);
void vbs_print(queue_t *Q);
// vbs_dq ensures that we can terminate in non-verbose
void vbs_dq(queue_t *Q);

// Processor initializer
processor_t *init_p(size_t D, size_t F, size_t m, size_t j, size_t k, size_t c);

// Initialize instruction with given params
instruction_t *new_ins(trace_op *op, int64_t tag, uint64_t cycle, bool mispredict, int proc, int thread);

// Useful bool functions
bool dispatch_full(processor_t *P, int idx);

// Callback function from processor.c for cache ops
void memOpCallback(int64_t tag);

// Where does this go?
void p_execute(processor_t *P);

// Clock update functions
void p_register_update(processor_t *P);
void p_sq_update(processor_t *P);
void p_dispatch_reserve(processor_t *P);
void p_sche_q_update(processor_t *P);
void p_state_update(processor_t *P);

// Packages all clock update functions into one function for cleanliness
int processor_update(processor_t *P);

// Instrumentation functions
void print_data();

#endif