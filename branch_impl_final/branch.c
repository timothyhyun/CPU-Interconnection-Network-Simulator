#include "branch_internal.h"

#include <branch.h>
#include <trace.h>

#include <getopt.h>
#include <stdlib.h>
#include <assert.h>

branch *self = NULL;

uint64_t branchRequest(trace_op *op, int processorNum);
enum BRANCH_MODEL_TYPE ptype = DEFAULT;
branch_predictor *predictor = NULL;
b_param_t *param = NULL;
bool verbose = false;

branch *init(branch_sim_args *csa)
{
    int op;
    param = malloc(sizeof(b_param_t));
    param->s = 0;
    param->b = 0;

    // TODO - get argument list from assignment
    while ((op = getopt(csa->arg_count, csa->arg_list, "s:b:g:v")) != -1)
    {
        switch (op)
        {
        // predictor size
        case 's':
            param->s = atoi(optarg);
            break;
            // BHR size
        case 'b':
            param->b = atoi(optarg);
            break;
            // predictor model
        case 'g':
            ptype = atoi(optarg);
            break;
        case 'v':
            verbose = true;
        }
    }

    self = malloc(sizeof(branch));
    self->branchRequest = branchRequest;
    self->si.tick = tick;
    self->si.finish = finish;
    self->si.destroy = destroy;

    predictor = init_predict(param);

    return self;
}

// Given a branch operation, return the predicted PC address
uint64_t branchRequest(trace_op *op, int processorNum)
{
    assert(op != NULL);

    uint64_t pcAddress = op->pcAddress;
    if (processorNum) {
        pcAddress += 0xF;   
    }
    uint64_t realNextAddress = op->nextPCAddress;
    if (processorNum) {
        pcAddress += 0xF;   
    }
    uint64_t pred = 0;

    switch (ptype)
    {
    case GSHARE:
        pred = gshare(predictor, pcAddress, realNextAddress);
        if(verbose) printf("Branch %lx -> %lx\n", pcAddress, pred);
        return pred;
    default:
        pred = smith_counter(predictor, pcAddress, realNextAddress);
        if(verbose) printf("Branch %lx -> %lx\n", pcAddress, pred);
        return pred;
    }
}

int tick()
{

    return 1;
}

int finish(int outFd)
{
    printf("Boundary Swap Count: %d\n", BOUNDARY_SWAP_COUNT);
    return 0;
}

int destroy(void)
{
    // free any internally allocated memory here
    return 0;
}
