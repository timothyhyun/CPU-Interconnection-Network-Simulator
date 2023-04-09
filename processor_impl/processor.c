#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "processor.h"
#include "trace.h"
#include "cache.h"
#include "branch.h"
#include "processor_internal.h"

trace_reader *tr = NULL;
cache *cs = NULL;
branch *bs = NULL;

int processorCount = 1;
int CADSS_VERBOSE = 0;

int *pendingMem = NULL;
int *pendingBranch = NULL;
//int64_t *memOpTag = NULL;

processor_t *P = NULL;
queue_t *vbs_q = NULL;
int64_t curr_tag = 1;
bool *p_fetch_go = NULL;

int CYCLES_BRANCH_STALLED = 0;

//
// init
//
//   Parse arguments and initialize the processor simulator components
//
processor *init(processor_sim_args *psa)
{
    int op;

    tr = psa->tr;
    cs = psa->cache_sim;
    bs = psa->branch_sim;

    size_t D = 1, F = 1, m = 1, j = 1, k = 1, c = 1;
    // TODO - get argument list from assignment
    while ((op = getopt(psa->arg_count, psa->arg_list, "f:d:m:j:k:c:")) != -1)
    {
        switch (op)
        {
        // fetch rate
        case 'f':
            F = (size_t)strtoul(optarg, NULL, 10);
            break;

        // dispatch queue multiplier
        case 'd':
            D = (size_t)strtoul(optarg, NULL, 10);
            break;

        // Schedule queue multiplier
        case 'm':
            m = (size_t)strtoul(optarg, NULL, 10);
            break;

        // Number of fast ALUs
        case 'j':
            j = (size_t)strtoul(optarg, NULL, 10);
            break;

        // Number of long ALUs
        case 'k':
            k = (size_t)strtoul(optarg, NULL, 10);
            break;

        // Number of CDBs
        case 'c':
            c = (size_t)strtoul(optarg, NULL, 10);
            break;
        }
    }

    pendingBranch = calloc(processorCount, sizeof(int));
    pendingMem = calloc(processorCount, sizeof(int));
    //memOpTag = calloc(processorCount, sizeof(int64_t));
    p_fetch_go = malloc(processorCount * sizeof(bool));
    for (int i = 0; i < processorCount; i++) {
        p_fetch_go[i] = true; 
    }

    P = init_p(D, F, m, j, k, c);
    vbs_q = init_q(-1);

    return 0;
}

const int64_t STALL_TIME = 100000;
int64_t tickCount = 0;
int64_t stallCount = -1;

int64_t makeTag(int procNum, int64_t baseTag)
{
    return ((int64_t)procNum) | (baseTag << 8);
}

static int node_cmp(void *A, void *B)
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

void memOpCallback(int64_t tag)
{   
    list_t *most_rec_cache = (list_t *)peek(P->cache_queue);
    if (most_rec_cache == NULL) {
        return;
    }
    instruction_t *ins = (instruction_t *)most_rec_cache->data;
    int proc_num = ins->proc;
    // Is the completed memop one that is pending?
    if (tag == ins->tag)
    {
        pendingMem[proc_num] = 0;
        stallCount = tickCount + STALL_TIME;
        dq(P->cache_queue);
        enq_sort(P->busQueue, most_rec_cache, &node_cmp);
        P->cache_busy = false;
    }
    else
    {
        printf("memopTag: %ld != tag %ld\n", tag, ins->tag);
        P->cache_busy = false;
    }
}

int tick(void)
{
    // if room in pipeline, request op from trace
    //   for the sample processor, it requests an op
    //   each tick until it reaches a branch or memory op
    //   then it blocks on that op

    trace_op *nextOp = NULL;

    // Pass along to the branch predictor and cache simulator that time ticked
    bs->si.tick();
    cs->si.tick();
    tickCount++;

    if (tickCount == stallCount)
    {
        printf("Processor may be stalled.  Now at tick - %ld, last op at %ld\n",
               tickCount, tickCount - STALL_TIME);
        int i;
        for (i = 0; i < processorCount; i++)
        {
            if (pendingMem[i] == 1)
            {
                printf("Processor %d is waiting on memory\n", i);
            }
        }
    }

    bool fetched = true;
    int progress = 0;
    int i;
    for (i = 0; i < processorCount; i++)
    {
        fetched = fetched && p_fetch_go[i];
        if (pendingMem[i] == 1)
        {
            progress = 1;
        }

        // In the full processor simulator, the branch is pending until
        //   it has executed. (updated by SU)
        if (pendingBranch[i] > 0)
        {
            progress = 1;
        }

        // Update all functions
        processor_update(P);

        // Get and manage ops only if we have space in the dispatch queue and
        // we did not mispredict last cycle
        if (P->disp_queue->capacity >= P->params->F)
        {
            for (int j = 0; j < P->params->F && p_fetch_go[i]; j++)
            {
                nextOp = tr->getNextOp(i);

                if (nextOp == NULL)
                    continue;

                progress = 1;
                instruction_t *curr_ins = NULL;
                switch (nextOp->op)
                {
                case MEM_LOAD:
                case MEM_STORE:
                    pendingMem[i] = 0;
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i);
                    enq(P->disp_queue, (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    break;

                case BRANCH:
                    pendingBranch[i] = 1;
                    bool mispredict = (bs->branchRequest(nextOp, i) == nextOp->nextPCAddress) ? 0 : 1;
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, mispredict, i);
                    enq(P->disp_queue, (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    p_fetch_go[i] = !mispredict;
                    P->fetch_unit_go = !mispredict;
                    break;
                case ALU:
                case ALU_LONG:
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i);
                    enq(P->disp_queue, (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    break;
                }

                free(nextOp);
            }
        } 
        if (!p_fetch_go[i]) {
            CYCLES_BRANCH_STALLED++;
        }
        if (p_fetch_go[i] != P->fetch_unit_go) {
            p_fetch_go[i] = P->fetch_unit_go;
            pendingBranch[i] = false;
        }
    }   
    if (CADSS_VERBOSE)
    {
        vbs_print(vbs_q);
    }
    else
    {
        vbs_dq(vbs_q);
    }
    if (empty(vbs_q) && fetched)
    {
        progress = 0;
    }
    else
    {
        progress = 1;
    }
    CURR_CYCLE++;
    return progress;
}

int finish(int outFd)
{
    int c = cs->si.finish(outFd);
    int b = bs->si.finish(outFd);

    char buf[32];
    size_t charCount = snprintf(buf, 32, "Ticks - %ld\n", tickCount);

    printf("Cycle branch mispredicts stalled: %d\n", CYCLES_BRANCH_STALLED);
    write(outFd, buf, charCount + 1);

    if (b || c)
        return 1;
    return 0;
}

int destroy(void)
{
    int c = cs->si.destroy();
    int b = bs->si.destroy();

    if (b || c)
        return 1;
    return 0;
}
