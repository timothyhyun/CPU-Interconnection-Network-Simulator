/**
 * @file branch_internal.h
 * @brief Header file which declares all relevant datatypes for branch.c
 * @author Aaron Tan <ahtan@andrew.cmu.edu>
 * @author Tony Yu   <tonyy@andrew.cmu.edu>
 */

#ifndef BRANCH_INTERNAL_H
#define BRANCH_INTERNAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

extern int BOUNDARY_SWAP_COUNT;

typedef struct branch_params {
    int s;
    int b;
} b_param_t;

typedef struct btb_entry {
    uint64_t pred_addr;
    uint64_t predictor;
} btb_entry;

typedef struct branch_predictor {
    btb_entry *BTB;
    uint64_t BHR; // global branch history register
    uint64_t addr_mask;
    uint64_t bhr_mask;
} branch_predictor;

branch_predictor *init_predict (b_param_t *parameters);
uint64_t smith_counter (branch_predictor *p, uint64_t pcAddress, uint64_t realNextAddress);
uint64_t gshare (branch_predictor *p, uint64_t pcAddress, uint64_t realNextAddress);


#endif