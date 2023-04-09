#include "processor_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "trace.h"

const int FU0_MAX_AGE = 1;
const int FU1_MAX_AGE = 3;
uint64_t CURR_CYCLE = 0;
uint64_t CURR_PRINT_TAG = 1;

int node_cmp(void *A, void *B);

bool verify_queue(queue_t *Q)
{
  if (Q == NULL)
  {
    printf("Illegal NULL queue input to verify_queue\n");
    return false;
  }
  list_t *curr = Q->q;
  if (!Q->q && !Q->tail)
  {
    return true;
  }
  if (Q->q->prev != NULL || Q->tail->next != NULL)
  {
    return false;
  }
  while (curr)
  {
    if (curr->prev != NULL)
    {
      if (curr->prev->next != curr)
      {
        return false;
      }
    }
    if (curr->data == NULL)
    {
      return false;
    }
    curr = curr->next;
  }
  return true;
}

queue_t *init_q(int cap)
{
  queue_t *new = malloc(sizeof(queue_t));
  if (new == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  new->q = NULL;
  new->tail = NULL;
  new->capacity = cap;
  assert(verify_queue(new));
  return new;
}

bool enq(queue_t *Q, void *data)
{
  assert(verify_queue(Q));
  if (Q == NULL)
  {
    printf("Gave illegal NULL queue input to enq\n");
    return false;
  }
  if (Q->capacity == 0)
  {
    return false;
  }
  if (data == NULL)
  {
    printf("Gave illegal NULL data input to enq\n");
    assert(verify_queue(Q));
    return false;
  }
  list_t *new = malloc(sizeof(list_t));
  if (new == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
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
  if (Q->capacity > 0)
  {
    Q->capacity--;
  }
  assert(verify_queue(Q));
  return true;
}

void *dq(queue_t *Q)
{
  assert(verify_queue(Q));
  if (!Q->q)
  {
    assert(verify_queue(Q));
    return NULL;
  }
  if (Q->q == Q->tail)
    Q->tail = NULL;
  assert(Q->q->data != NULL);
  void *res = Q->q->data;
  list_t *head = Q->q;
  Q->q = Q->q->next;
  if (Q->q)
    Q->q->prev = NULL;
  if (Q->capacity >= 0)
  {
    Q->capacity++;
  }
  free(head);
  assert(verify_queue(Q));
  return res;
}

void delete (queue_t *Q, list_t *node)
{
  assert(verify_queue(Q));
  if (Q == NULL)
  {
    printf("Gave illegal NULL queue input to delete\n");
    return;
  }
  if (!node)
  {
    printf("Gave illegal NULL node input to delete\n");
    assert(verify_queue(Q));
    return;
  }
  if (!Q->q)
  {
    return;
  }
  if (node == Q->q)
  {
    dq(Q);
    return;
  }
  else if (!node->next)
  {
    Q->tail = Q->tail->prev;
    Q->tail->next = NULL;
    free(node);
  }
  else
  {
    list_t *prev = node->prev;
    list_t *next = node->next;
    prev->next = next;
    next->prev = prev;
    free(node);
  }
  if (Q->capacity >= 0)
  {
    Q->capacity++;
  }
  assert(verify_queue(Q));
}

bool enq_sort(queue_t *Q, void *data, int (*cmp)(void *, void *))
{
  assert(verify_queue(Q));
  if (Q == NULL)
  {
    printf("Gave illegal NULL queue input to enq_sort\n");
    return false;
  }
  if (Q->capacity == 0)
  {
    assert(verify_queue(Q));
    return false;
  }
  if (data == NULL)
  {
    printf("Gave illegal NULL data input to enq_sort\n");
    assert(verify_queue(Q));
    return false;
  }
  if (cmp == NULL)
  {
    printf("Gave illegal NULL comparison input to enq_sort\n");
    assert(verify_queue(Q));
    return false;
  }
  if (!Q->q)
  {
    return enq(Q, data);
  }
  list_t *new = malloc(sizeof(list_t));
  if (new == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  new->data = data;
  list_t *curr = Q->q;
  while (curr && (*cmp)(data, curr->data) > 0)
  {
    curr = curr->next;
  }
  if (!curr)
  {
    new->prev = Q->tail;
    new->next = NULL;
    Q->tail->next = new;
    Q->tail = new;
    if (Q->capacity > 0)
    {
      Q->capacity--;
    }
    assert(verify_queue(Q));
    return true;
  }
  if (curr == Q->q) {
    Q->q->prev = new;
    new->next = Q->q;
    new->prev = NULL;
    Q->q = new;
    return true;
  } 
  else
  {
    curr->prev->next = new;
    new->prev = curr->prev;
    curr->prev = new;
    new->next = curr;
    Q->capacity--;
    assert(verify_queue(Q));
    return true;
  }
}

void *peek(queue_t *Q)
{
  return Q->q == NULL ? 0 : Q->q->data;
}

bool empty(queue_t *Q)
{
  return Q->q == NULL;
}

void ins_print(void *data)
{
  if (!data)
  {
    printf("\nNULL ");
    return;
  }
  instruction_t *dat = (instruction_t *)data;
  printf("\n{tag: %i, op: %i} ", dat->tag, dat->op);
}

void print_queue(queue_t *Q, void (*print)(void *data))
{
  list_t *curr = Q->q;
  printf("Head: ");
  while (curr)
  {
    (*print)(curr->data);
    curr = curr->next;
  }
  printf("Tail\n");
}

processor_t *init_p(size_t D, size_t F, size_t m, size_t j, size_t k, size_t c)
{
  processor_t *new = malloc(sizeof(processor_t));
  if (new == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  new->params = malloc(sizeof(p_params));
  if (new->params == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  new->params->D = D;
  new->params->F = F;
  new->params->m = m;
  new->params->j = j;
  new->params->k = k;
  new->params->c = c;
  new->disp_queue = init_q(D * (m * j + m * k));
  new->sche_queue_0 = init_q(m * j);
  new->sche_queue_1 = init_q(m * k);
  new->busQueue = init_q(-1);
  new->funcUnit0 = calloc(sizeof(shortALU_t), j);
  new->funcUnit1 = calloc(sizeof(longALU_t), k);
  new->resBusses = calloc(sizeof(cdBus), c);
  new->defer_queue = init_q(-1);
  new->cache_queue = init_q(-1);
  new->fetch_unit_go = true;
  new->cache_busy = false;
  if (new->funcUnit0 == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  if (new->funcUnit1 == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  if (new->resBusses == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  for (int i = 0; i < 33; i++)
  {
    new->registers[i].tag = 0;
    new->registers[i].busy = false;
  }
  return new;
}

instruction_t *new_ins(trace_op *op, int64_t tag, uint64_t cycle, bool mispredict, int proc)
{
  instruction_t *new = malloc(sizeof(instruction_t));
  if (new == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  if (op == NULL)
  {
    printf("Gave illegal NULL op input to new_ins\n");
    return NULL;
  }
  new->tag = tag;
  new->op = op->op;
  new->executed = false;
  new->ins_ready = false;
  for (int i = 0; i < 2; i++)
  {
    new->src_reg[i] = op->src_reg[i];
  }
  for (int i = 0; i < 2; i++)
  {
    new->src_tag[i] = -1;
  }
  new->dest_reg = op->dest_reg;
  for (int i = 0; i < 2; i++)
  {
    new->ready[i] = false;
  }
  new->mispredict = mispredict;
  new->memAddress = op->memAddress;
  new->size = op->size;
  new->proc = proc;
  new->vdat = malloc(sizeof(verbose_dat));
  if (new->vdat == NULL)
  {
    printf("Ran out of memory at process_internal.c line %i\n", __LINE__);
    exit(1);
  }
  new->vdat->stage_dispatch = -1;
  new->vdat->stage_execute = -1;
  new->vdat->stage_fetch = cycle;
  new->vdat->stage_schedule = -1;
  new->vdat->stage_su = -1;
  return new;
}

bool dispatch_full(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to dispatch_full\n");
    return false;
  }
  return P->disp_queue->capacity == 0;
}

void p_execute(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_sq_update\n");
    return;
  }
  list_t *curr0 = P->sche_queue_0->q;
  int fu0_avail = P->params->j;
  while (curr0 != NULL && fu0_avail != 0)
  {
    instruction_t *curr_ins = (instruction_t *)curr0->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->executed)
    {
      curr0 = curr0->next;
      continue;
    }
    if (curr_ins->ins_ready)
    {
      if (curr_ins->vdat->stage_execute == -1)
      {
        curr_ins->vdat->stage_execute = CURR_CYCLE;
      }
      if (curr_ins->op == MEM_LOAD || curr_ins->op == MEM_STORE) {
        enq(P->cache_queue, (void *)curr0);
        curr0 = curr0->next;
        curr_ins->executed = true;
        continue;
      }
      (&P->funcUnit0[--fu0_avail])->stage1 = curr0;
      curr_ins->executed = true;
    }
    curr0 = curr0->next;
  }
  list_t *curr1 = P->sche_queue_1->q;
  int fu1_avail = P->params->k;
  while (curr1 != NULL && fu1_avail != 0)
  {
    instruction_t *curr_ins = (instruction_t *)curr1->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->executed)
    {
      curr1 = curr1->next;
      continue;
    }
    if (curr_ins->ins_ready)
    {
      if (curr_ins->vdat->stage_execute == -1)
      {
        curr_ins->vdat->stage_execute = CURR_CYCLE;
      }
      (&P->funcUnit1[--fu1_avail])->stage1 = curr1;
      curr_ins->executed = true;
    }
    curr1 = curr1->next;
  }
  if (!P->cache_busy) {
    list_t *most_rec_cache = (list_t *)peek(P->cache_queue);
    if (most_rec_cache == NULL) {
        return;
    }
    instruction_t *ins = (instruction_t *)most_rec_cache->data;
    trace_op *new = malloc(sizeof(trace_op));
    new->op = ins->op;
    new->memAddress = ins->memAddress;
    new->size = ins->size;
    cs->memoryRequest(new, ins->proc, ins->tag, memOpCallback);
    P->cache_busy = true;
  }
}

// Clock update functions

/*  @brief Updates the simulated register files using the CDBs
 */
void p_register_update(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_register_update\n");
    return;
  }
  for (int i = 0; i < P->params->c; i++)
  {
    if (P->resBusses[i].busy)
    {
      int dest = P->resBusses[i].dest;
      int tag = P->resBusses[i].tag;
      // Should only update dest registers if the tag is current
      if (dest >= 0 && P->registers[dest].tag == tag)
      {
        P->registers[dest].busy = false;
      }
    }
  }
}

/*  @brief Marks all ready instructions (i.e. instructions where both src
 *         registers have been marked as ready) to fire
 *
 *  Treats each schedule queue separately for our purposes. Each while loop
 *  breaks out in the case of there being no remaining FU of the type queued.
 */
void p_sq_update(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_sq_update\n");
    return;
  }
  list_t *curr0 = P->sche_queue_0->q;
  while (curr0 != NULL)
  {
    instruction_t *curr_ins = (instruction_t *)curr0->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->executed)
    {
      curr0 = curr0->next;
      continue;
    }
    if (curr_ins->vdat->stage_schedule == -1)
    {
      curr_ins->vdat->stage_schedule = CURR_CYCLE;
    }
    bool ready = true;
    for (int i = 0; i < 2; i++)
    {
      if (curr_ins->src_reg[i] >= 0)
      {
        ready = ready && curr_ins->ready[i];
      }
    }
    curr_ins->ins_ready = ready;
    curr0 = curr0->next;
  }
  list_t *curr1 = P->sche_queue_1->q;
  while (curr1 != NULL)
  {
    instruction_t *curr_ins = (instruction_t *)curr1->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->executed)
    {
      curr1 = curr1->next;
      continue;
    }
    if (curr_ins->vdat->stage_schedule == -1)
    {
      curr_ins->vdat->stage_schedule = CURR_CYCLE;
    }
    bool ready = true;
    for (int i = 0; i < 2; i++)
    {
      if (curr_ins->src_reg[i] >= 0)
      {
        ready = ready && curr_ins->ready[i];
      }
    }
    curr_ins->ins_ready = ready;
    curr1 = curr1->next;
  }
}

/*  @brief Reserves spaces on the scheduling queues for new instructions from
 *         the dispatch queue. Places as many instructions as possible from
 *         the dispatch queue onto the scheduling queue
 *
 *  This function implements both
 *  (a) reserving slots in the schedule queue
 *  (b) dispatch reading the register files into inserted instructions
 */
void p_dispatch_reserve(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_sq_update\n");
    return;
  }
  list_t *pass1 = P->disp_queue->q;
  while (pass1 != NULL) {
    instruction_t *curr_ins = (instruction_t *)pass1->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->vdat->stage_dispatch == -1) {
      curr_ins->vdat->stage_dispatch = CURR_CYCLE;
    }
    pass1 = pass1->next;
  }
  list_t *curr = P->disp_queue->q;
  int queue0_cap = P->sche_queue_0->capacity;
  int queue1_cap = P->sche_queue_1->capacity;
  while (curr != NULL)
  {
    instruction_t *curr_ins = (instruction_t *)curr->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    bool go = true;
    if (curr_ins->vdat->stage_dispatch == -1) {
      curr_ins->vdat->stage_dispatch = CURR_CYCLE;
    }
    switch (curr_ins->op)
    {
    case ALU:
    case BRANCH:
    case MEM_LOAD:
    case MEM_STORE:
      go = (queue0_cap != 0);
      if (!go)
      {
        return;
      }
      else
      {
        enq(P->defer_queue, (void *)curr_ins);
        queue0_cap--;
      }
      break;
    case ALU_LONG:
      go = (queue1_cap != 0);
      if (!go)
      {
        return;
      }
      else
      {
        enq(P->defer_queue, (void *)curr_ins);
        queue1_cap--;
      }
      break;
    }
    list_t *old = curr;
    curr = curr->next;
    delete (P->disp_queue, old);
  }
}

void p_defer(processor_t *P) {
  instruction_t *curr_ins = NULL;
  while((curr_ins = (instruction_t *)dq(P->defer_queue))) {
    switch (curr_ins->op)
    {
    case ALU:
    case BRANCH:
    case MEM_LOAD:
    case MEM_STORE:
      if(!enq(P->sche_queue_0, (void *)curr_ins)) {
        printf("Deferred too much at %i in processor_internal.c\n", __LINE__);
        exit(1);
      }
      break;
    case ALU_LONG:
      if(!enq(P->sche_queue_1, (void *)curr_ins)) {
        printf("Deferred too much at %i in processor_internal.c\n", __LINE__);
        exit(1);
      }
      break;
    }
    for (int i = 0; i < 2; i++)
    {
      if (!P->registers[curr_ins->src_reg[i]].busy)
      {
        curr_ins->ready[i] = true;
      }
      else
      {
        curr_ins->src_tag[i] = P->registers[curr_ins->src_reg[i]].tag;
        curr_ins->ready[i] = false;
      }
    }
    if (curr_ins->dest_reg >= 0)
    {
      P->registers[curr_ins->dest_reg].tag = curr_ins->tag;
      P->registers[curr_ins->dest_reg].busy = true;
    }
  }
}

/*  @brief takes the values from the CDBs and places them into instructions
 *         on the schedule queue, but does not mark the instructions as ready
 *         to fire, and does not move these instructions into a function unit
 */
void p_sche_q_update(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_sq_update\n");
    return;
  }
  for (int i = 0; i < P->params->c; i++)
  {
    if (!P->resBusses[i].busy)
    {
      continue;
    }
    int64_t tag = P->resBusses[i].tag;
    // printf("Broadcasting tag %li\n", tag);
    list_t *curr0 = P->sche_queue_0->q;
    list_t *curr1 = P->sche_queue_1->q;
    while (curr0)
    {
      instruction_t *curr_ins = (instruction_t *)curr0->data;
      if (curr_ins == NULL)
      {
        printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
        exit(1);
      }
      for (int i = 0; i < 2; i++)
      {
        if (curr_ins->src_tag[i] == tag)
        {
          curr_ins->ready[i] = true;
        }
      }
      curr0 = curr0->next;
    }
    while (curr1)
    {
      instruction_t *curr_ins = (instruction_t *)curr1->data;
      if (curr_ins == NULL)
      {
        printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
        exit(1);
      }
      for (int i = 0; i < 2; i++)
      {
        if (curr_ins->src_tag[i] == tag)
        {
          curr_ins->ready[i] = true;
        }
      }
      curr1 = curr1->next;
    }
  }
}

/*  @brief Instruction "comparison" function to be used in enq_sort
 */
int ins_cmp(void *A, void *B)
{
  if (!A && B)
    return -1;
  if (A && !B)
    return 1;
  if (!A && !B)
    return 0;
  instruction_t *a = (instruction_t *)A;
  instruction_t *b = (instruction_t *)B;
  return (a->tag > b->tag) ? 1 : (a->tag == b->tag ? 0 : -1);
}

/*  @brief Node "comparison" function to be used in enq_sort
 */
int node_cmp(void *A, void *B)
{
  if (!A && B)
    return -1;
  if (A && !B)
    return 1;
  if (!A && !B)
    return 0;
  list_t *a = (list_t *)A;
  list_t *b = (list_t *)B;
  instruction_t *ins_a = (instruction_t *)a->data;
  instruction_t *ins_b = (instruction_t *)b->data;
  return (ins_a->tag > ins_b->tag) ? 1 : (ins_a->tag == ins_b->tag ? 0 : -1);
}

void update_longALU(longALU_t *alu) {
  if (alu->stage2 != NULL)
  {
    alu->stage3 = alu->stage2;
    alu->stage2 = NULL;
  }
  if (alu->stage1 != NULL)
  {
    alu->stage2 = alu->stage1;
    alu->stage1 = NULL;
  }
}

void p_state_update(processor_t *P)
{
  if (!P)
  {
    printf("Gave illegal NULL processor input to p_sq_update\n");
    return;
  }
  // Clear the FUs at the end of cycle, place into unbound queue in tag order
  for (int i = 0; i < P->params->j; i++)
  {
    if (P->funcUnit0[i].stage1 != NULL)
    {
      instruction_t *curr_ins = (instruction_t *)P->funcUnit0[i].stage1->data;
      curr_ins->vdat->stage_su = CURR_CYCLE;
      P->funcUnit0[i].busy = false;
      enq_sort(P->busQueue, P->funcUnit0[i].stage1, &node_cmp);
      // printf("Tag %i completed\n", ((instruction_t *)P->funcUnit0[i].stage1->data)->tag);
      // delete (P->sche_queue_0, P->funcUnit0[i].stage1);
      P->funcUnit0[i].stage1 = NULL;
    }
  }
  for (int i = 0; i < P->params->k; i++)
  {
    if (P->funcUnit1[i].stage3 != NULL)
    {
      instruction_t *curr_ins = (instruction_t *)P->funcUnit1[i].stage3->data;
      curr_ins->vdat->stage_su = CURR_CYCLE;
      enq_sort(P->busQueue, P->funcUnit1[i].stage3, &node_cmp);
      // printf("Tag %i completed\n", ((instruction_t *)P->funcUnit1[i].stage3->data)->tag);
      // delete (P->sche_queue_1, P->funcUnit1[i].stage3);
      P->funcUnit1[i].stage3 = NULL;
    }
  }

  for (int i = 0; i < P->params->k; i++)
  {
    update_longALU(&P->funcUnit1[i]);
  }

  // First, clear the CDBs at the start of every state update cycle
  for (int i = 0; i < P->params->c; i++)
  {
    (&P->resBusses[i])->busy = false;
  }

  for (int i = 0; i < P->params->c; i++)
  {
    cdBus *curr_bus = &P->resBusses[i];
    if (!curr_bus->busy)
    {
      list_t *curr_dat = (list_t *)dq(P->busQueue);
      if (curr_dat == NULL) {
        return;
      }
      instruction_t *curr_ins = (instruction_t *)curr_dat->data;
      if (curr_ins == NULL)
      {
        return;
      }
      switch (curr_ins->op)
      {
      case MEM_LOAD:
      case MEM_STORE:
        curr_ins->vdat->stage_su = CURR_CYCLE;
        delete (P->sche_queue_0, curr_dat);
        break;
      case BRANCH:
        if (curr_ins->mispredict) {
          P->fetch_unit_go = true;
        }
      case ALU:
        delete (P->sche_queue_0, curr_dat);
        break;
      case ALU_LONG:
        delete (P->sche_queue_1, curr_dat);
        break;
      }
      curr_bus->busy = true;
      curr_bus->tag = curr_ins->tag;
      curr_bus->dest = curr_ins->dest_reg;
    }
  }
} 


int processor_update(processor_t *P)
{
  p_dispatch_reserve(P);
  p_state_update(P);
  p_register_update(P);
  p_execute(P);
  p_sq_update(P);
  p_sche_q_update(P);
  p_defer(P);
  // print_all(P);
}

void print_FU0s(processor_t *P)
{
  for (int i = 0; i < P->params->j; i++)
  {
    shortALU_t *curr = &P->funcUnit0[i];
    list_t *stage1 = (list_t *)curr->stage1;
    printf("FU0: stage1: %ld, busy: %i\n", stage1 != NULL ? ((instruction_t *)stage1->data)->tag : 0, curr->busy);
  }
}

void print_FU1s(processor_t *P)
{
  for (int i = 0; i < P->params->k; i++)
  {
    longALU_t *curr = &P->funcUnit1[i];
    list_t *stage1 = (list_t *)curr->stage1;
    list_t *stage2 = (list_t *)curr->stage2;
    list_t *stage3 = (list_t *)curr->stage3;
    printf("FU1: stage1: %ld, stage2: %ld, stage3: %ld, busy: %i\n", stage1 != NULL ? ((instruction_t *)stage1->data)->tag : 0, stage2 != NULL ? ((instruction_t *)stage2->data)->tag : 0, stage3 != NULL ? ((instruction_t *)stage3->data)->tag : 0, curr->busy);
  }
}

void print_all(processor_t *P)
{
  printf("Cycle %li: \n", CURR_CYCLE);
  printf("Dispatch queue: ");
  print_queue(P->disp_queue, &ins_print);
  printf("Schedule queue type 0: ");
  print_queue(P->sche_queue_0, &ins_print);
  printf("Schedule queue type 1: ");
  print_queue(P->sche_queue_1, &ins_print);
  print_FU0s(P);
  print_FU1s(P);
}

void vbs_enq(queue_t *Q, void *data)
{
  enq_sort(Q, data, &ins_cmp);
}

void vbs_print(queue_t *Q)
{
  list_t *curr = Q->q;
  while (curr)
  {
    instruction_t *curr_ins = (instruction_t *)curr->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->tag != CURR_PRINT_TAG || curr_ins->vdat->stage_su == -1)
    {
      return;
    }
    curr = curr->next;
    dq(Q);
    printf("%li %li %li %li %li %li\n",
           curr_ins->tag, curr_ins->vdat->stage_fetch,
           curr_ins->vdat->stage_dispatch, curr_ins->vdat->stage_schedule,
           curr_ins->vdat->stage_execute, curr_ins->vdat->stage_su);
    CURR_PRINT_TAG++;
  }
  fflush(stdout);
}

void vbs_dq(queue_t *Q)
{
  list_t *curr = Q->q;
  while (curr)
  {
    instruction_t *curr_ins = (instruction_t *)curr->data;
    if (curr_ins == NULL)
    {
      printf("Illegal NULL instruction encoutered at line %i in processor_internal.c\n", __LINE__);
      exit(1);
    }
    if (curr_ins->tag != CURR_PRINT_TAG || curr_ins->vdat->stage_su == -1)
    {
      return;
    }
    curr = curr->next;
    dq(Q);
    CURR_PRINT_TAG++;
  }
}