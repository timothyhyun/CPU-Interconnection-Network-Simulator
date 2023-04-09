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
// int64_t *memOpTag = NULL;

processor_t *P = NULL;
queue_t *vbs_q = NULL;
int64_t curr_tag = 1;
bool *p_fetch_go = NULL;
bool do_smt = true;
int curr_thread = 0;

int CYCLES_BRANCH_STALLED = 0;
int CYCLES_CACHE_STALLED = 0;

size_t MAX_CYCLES_RUN = 0;

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

    size_t D = 1, F = 1, m = 1, j = 1, k = 1, c = 1, s = 0;
    // TODO - get argument list from assignment
    while ((op = getopt(psa->arg_count, psa->arg_list, "f:d:m:j:k:c:s:")) != -1)
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
        case 's':
            s = (size_t)strtoul(optarg, NULL, 10);
            if (s)
                do_smt = true;
            break;
        case 'n':
            MAX_CYCLES_RUN = (size_t)strtoul(optarg, NULL, 10);
            break;
        }
    }

    pendingBranch = calloc(processorCount, sizeof(int));
    pendingMem = calloc(processorCount, sizeof(int));
    // memOpTag = calloc(processorCount, sizeof(int64_t));
    p_fetch_go = malloc(processorCount * sizeof(bool));
    for (int i = 0; i < processorCount; i++)
    {
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
    if (most_rec_cache == NULL)
    {
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

/*  In smt_pfetch, we take care of fetch in the case that we end up using SMT.
 *  In particular:
 *  - we make i to a processor number (i / 2) and a thread (i % 2). Thus,
 *    trace i will run on processor i / 2's thread i % 2
 *  - p_fetch_go[i] keeps track of if trace i is stalled by a branch mispredict
 *  - pendingMem keeps track of if trace i is stilled by a memory op
 *  Otherwise, operationally this function is the same as pfetch, but still
 *  should be split into two to avoid spaghetti code.
 */
void smt_pfetch(int i, bool *fetched, int *progress)
{
    // We compute which processor and which thread to use for the given input
    /* NOT NEEDED YET SO COMMENTED OUT
    int procnum = i / 2; // rounds down, only to be used when we get multiprocessor
    int threadnum = i % 2; // modulus tells us which thread we should use
    */

    // For now, operate on the simplifying assumption that only one core is used
    int threadnum = i;

    // If the current thread's dispatch queue is full, or if p_fetch_go is false
    // for thread x, we do the other thread assigned to the core
    if ((P->disp_queue[threadnum]->capacity < P->params->F || !p_fetch_go[threadnum]) && (!tr->is_eof((i + 1) % 2)))
    {
        threadnum = (i + 1) % 2;
    }

    trace_op *nextOp = NULL;
    *fetched = *fetched && (p_fetch_go[0] && p_fetch_go[1]);
    if (pendingMem[i] == 1)
    {
        *progress = 1;
    }

    // In the full processor simulator, the branch is pending until
    //   it has executed. (updated by SU)
    if (pendingBranch[i] > 0)
    {
        *progress = 1;
    }
    // Update both threads
    // processor_update(P);

    // Get and manage ops only if we have space in the dispatch queue and
    // we did not mispredict last cycle. Otherwise, we fetch from the other
    // trace and populate that dispatch queue.
    if (P->disp_queue[threadnum]->capacity >= P->params->F)
    {
        for (int j = 0; j < P->params->F && p_fetch_go[threadnum]; j++)
        {
            nextOp = tr->getNextOp(threadnum);

            if (nextOp == NULL)
            {
                if (tr->is_eof(threadnum) && tr->is_eof((threadnum + 1) % 2))
                {
                    if (p_fetch_go[0] != P->fetch_unit_go[0])
                    {
                        p_fetch_go[0] = P->fetch_unit_go[0];
                        pendingBranch[0] = false;
                    }
                    if (p_fetch_go[1] != P->fetch_unit_go[1])
                    {
                        p_fetch_go[1] = P->fetch_unit_go[1];
                        pendingBranch[1] = false;
                    }
                    return;
                }
                if (tr->is_eof(threadnum))
                {
                    return smt_pfetch((threadnum + 1) % 2, fetched, progress);
                }
                continue;
            }

            *progress = 1;
            instruction_t *curr_ins = NULL;
            switch (nextOp->op)
            {
            case MEM_LOAD:
            case MEM_STORE:
                pendingMem[threadnum] = 0;
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, 0, threadnum);
                enq(P->disp_queue[threadnum], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                break;

            case BRANCH:
                pendingBranch[threadnum] = 1;
                bool mispredict = (bs->branchRequest(nextOp, threadnum) == nextOp->nextPCAddress) ? 0 : 1;
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, mispredict, 0, threadnum);
                enq(P->disp_queue[threadnum], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                p_fetch_go[threadnum] = !mispredict;
                P->fetch_unit_go[threadnum] = !mispredict;
                break;
            case ALU:
            case ALU_LONG:
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, 0, threadnum);
                enq(P->disp_queue[threadnum], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                break;
            }

            free(nextOp);
        }
    }
    if (!p_fetch_go[0] || !p_fetch_go[1]) {
        CYCLES_BRANCH_STALLED++;
    }
    if (p_fetch_go[0] != P->fetch_unit_go[0])
    {
        p_fetch_go[0] = P->fetch_unit_go[0];
        pendingBranch[0] = false;
    }
    if (p_fetch_go[1] != P->fetch_unit_go[1])
    {
        p_fetch_go[1] = P->fetch_unit_go[1];
        pendingBranch[1] = false;
    }
}

/* pfetch has the same functionality as old fetch, but it uses disp_queue 0
   exclusively as only one thread is allowed per processor */
void pfetch(int i, bool *fetched, int *progress)
{
    trace_op *nextOp = NULL;
    *fetched = *fetched && p_fetch_go[i];
    if (pendingMem[i] == 1)
    {
        *progress = 1;
    }

    // In the full processor simulator, the branch is pending until
    //   it has executed. (updated by SU)
    if (pendingBranch[i] > 0)
    {
        *progress = 1;
    }

    // Update all functions
    // processor_update(P);

    // Get and manage ops only if we have space in the dispatch queue and
    // we did not mispredict last cycle
    if (P->disp_queue[0]->capacity >= P->params->F)
    {
        for (int j = 0; j < P->params->F && p_fetch_go[i]; j++)
        {
            nextOp = tr->getNextOp(i);

            if (nextOp == NULL)
                continue;

            *progress = 1;
            instruction_t *curr_ins = NULL;
            switch (nextOp->op)
            {
            case MEM_LOAD:
            case MEM_STORE:
                pendingMem[i] = 0;
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i, 0);
                enq(P->disp_queue[0], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                break;

            case BRANCH:
                pendingBranch[i] = 1;
                bool mispredict = (bs->branchRequest(nextOp, i) == nextOp->nextPCAddress) ? 0 : 1;
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, mispredict, i, 0);
                enq(P->disp_queue[0], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                p_fetch_go[i] = !mispredict;
                P->fetch_unit_go[0] = !mispredict;
                break;
            case ALU:
            case ALU_LONG:
                curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i, 0);
                enq(P->disp_queue[0], (void *)curr_ins);
                enq(vbs_q, (void *)curr_ins);
                curr_tag++;
                break;
            }

            free(nextOp);
        }
    }
    if (p_fetch_go[i] != P->fetch_unit_go[0])
    {
        p_fetch_go[i] = P->fetch_unit_go[0];
        pendingBranch[i] = false;
    }
}

int tick(void)
{
    // if room in pipeline, request op from trace
    //   for the sample processor, it requests an op
    //   each tick until it reaches a branch or memory op
    //   then it blocks on that op

    /* TODO: This doesn't look like it's needed anymore. Delete this? */
    // trace_op *nextOp = NULL;

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
    processor_update(P);
    if (do_smt)
    {
        smt_pfetch(curr_thread, &fetched, &progress);
        curr_thread = (curr_thread + 1) % 2;
    }
    else
    {
        for (int i = 0; i < processorCount; i++)
        {
            pfetch(i, &fetched, &progress);
        }
    }

    /* This old code will exist as junk while we develop the SMT thingy
     * TODO: delete it when done with SMT
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
        if (P->disp_queue[1]->capacity >= P->params->F)
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
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i, 1);
                    enq(P->disp_queue[1], (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    break;

                case BRANCH:
                    pendingBranch[i] = 1;
                    bool mispredict = (bs->branchRequest(nextOp, i) == nextOp->nextPCAddress) ? 0 : 1;
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, mispredict, i, 1);
                    enq(P->disp_queue[1], (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    p_fetch_go[i] = !mispredict;
                    P->fetch_unit_go = !mispredict;
                    break;
                case ALU:
                case ALU_LONG:
                    curr_ins = new_ins(nextOp, curr_tag, CURR_CYCLE, 0, i, 1);
                    enq(P->disp_queue[1], (void *)curr_ins);
                    enq(vbs_q, (void *)curr_ins);
                    curr_tag++;
                    break;
                }

                free(nextOp);
            }
        }
        if (p_fetch_go[i] != P->fetch_unit_go) {
            p_fetch_go[i] = P->fetch_unit_go;
            pendingBranch[i] = false;
        }
    }
    */
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
    if (MAX_CYCLES_RUN != 0 && CURR_PRINT_TAG > MAX_CYCLES_RUN) {
        progress = 0;
    }
    CURR_CYCLE++;
    return progress;
}

int finish(int outFd)
{
    int c = cs->si.finish(outFd);
    int b = bs->si.finish(outFd);

    print_data(P);
    printf("Cycle branch mispredicts stalled: %d\n", CYCLES_BRANCH_STALLED);

    char buf[32];
    size_t charCount = snprintf(buf, 32, "Ticks - %ld\n", tickCount);

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
