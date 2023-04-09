#include "branch_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int BOUNDARY_SWAP_COUNT = 0;

/** @brief Computes a bitmask of all 1s starting from highbit and ending
 *         at lowbit, inclusive.
 *
 * Taken from tonyy's datalab submission.
 */
long bitMask(long highbit, long lowbit) {
    long mask1 = ((0x01L << 63) >> 63) ^ (0x01L << 63);
    long hmask = mask1 >> (63 - highbit) | (0x01L << highbit);
    long lmask = ((0x01L << 63) >> 63) << lowbit;
    return hmask & lmask;
}

branch_predictor *init_predict (b_param_t *parameters) {
    branch_predictor *p = malloc(sizeof(branch_predictor));
    p->BTB = malloc(sizeof(btb_entry) * (1 << parameters->s));
    for (int i = 0; i < (1 << parameters->s); i++) {
        p->BTB[i].pred_addr = 0;
        p->BTB[i].predictor = 1;
    }
    p->BHR = 0;
    p->addr_mask = bitMask(parameters->s - 1, 0);
    p->bhr_mask = bitMask(parameters->b - 1, 0);
    return p;
}

uint64_t smith_counter (branch_predictor *p, uint64_t pcAddress, uint64_t realNextAddress) {
    uint64_t prediction;
    uint64_t idx = (pcAddress >> 3);
    idx &= p->addr_mask;
    // get the prediction
    if (p->BTB[idx].predictor > 1) {
        prediction = p->BTB[idx].pred_addr;
    } else {
        prediction = pcAddress + 4;
    }

    // Update predictor if it needs updating
    if (realNextAddress != pcAddress + 4) {
        p->BTB[idx].predictor = (p->BTB[idx].predictor == 3 ? 3 : p->BTB[idx].predictor + 1);
        p->BTB[idx].pred_addr = realNextAddress;
    } else {
        p->BTB[idx].predictor = (p->BTB[idx].predictor == 0 ? 0 : p->BTB[idx].predictor - 1);
    }
    return prediction;
}

/** @brief Implements the gshare branch predictor algorithm
 *
 */
uint64_t gshare (branch_predictor *p, uint64_t pcAddress, uint64_t realNextAddress) {
    uint64_t prediction;
    uint64_t idx = (pcAddress >> 3);
    idx ^= p->BHR;
    idx &= p->addr_mask;
    

    // get the prediction
    if (p->BTB[idx].predictor > 1) {
        prediction = p->BTB[idx].pred_addr;
    } else {
        prediction = pcAddress + 4;
    }

    if (realNextAddress != prediction && (p->BTB[idx].predictor == 2 || p->BTB[idx].predictor == 1)) {
        BOUNDARY_SWAP_COUNT++;
    }

    // Update predictor if it needs updating
    if (realNextAddress != pcAddress + 4) {
        p->BTB[idx].predictor = (p->BTB[idx].predictor == 3 ? 3 : p->BTB[idx].predictor + 1);
        p->BHR = ((p->BHR << 1) | 1) & p->bhr_mask;
        p->BTB[idx].pred_addr = realNextAddress;
    } else {
        p->BTB[idx].predictor = (p->BTB[idx].predictor == 0 ? 0 : p->BTB[idx].predictor - 1);
        p->BHR = ((p->BHR << 1) | 0) & p->bhr_mask;
    }
    return prediction;
}
