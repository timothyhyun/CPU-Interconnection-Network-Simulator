#include "cache_internal.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern csim_stats *stats; /* stats struct which is declared extern in cache.c */

/** @brief Computes a bitmask of all 1s starting from highbit and ending
 *         at lowbit, inclusive.
 *
 * Taken from my datalab submission. Used to compute a bitmask for the
 * tag and for the set index.
 */
long bitMask(long highbit, long lowbit)
{
    long mask1 = ((0x01L << 63) >> 63) ^ (0x01L << 63);
    long hmask = mask1 >> (63 - highbit) | (0x01L << highbit);
    long lmask = ((0x01L << 63) >> 63) << lowbit;
    return hmask & lmask;
}

/** @brief Zero-initialize a cache line
 *
 * See the declaration of '' directly above for what output is
 * produced as a result of the function.
 */
cline *init_line(parsed_t *cache_parameters)
{
    cline *line = malloc(sizeof(cline));
    line->tag = 0;
    line->age = 0;
    line->dirty = false;
    line->valid = calloc(1 << cache_parameters->u, sizeof(bool));
    return line;
}

/** @brief Zero-initializes the cache, which is represented as an 2^s * E array
 *         of cache lines.
 */
cache_t *init_cache(parsed_t *cache_parameters)
{
    size_t s = 1 << cache_parameters->s;
    size_t E = cache_parameters->E;
    cache_t *sets = malloc(sizeof(cset) * s);
    for (size_t i = 0; i < s; i++)
    {
        sets[i] = malloc(sizeof(cset) * E);
        for (size_t j = 0; j < E; j++)
        {
            sets[i][j] = init_line(cache_parameters);
        }
    }
    return sets;
}

void print_stats(csim_stats *stats)
{
    printf("{hits : %ld, misses : %ld, evictions : %ld}\n",
           stats->hits, stats->misses, stats->evictions);
}

/** @brief Simulates a load instruction
 *
 */
int load_cache(cache_t *cache, int size, uint64_t memAddress, parsed_t *cache_parameters, void (*cache_update)(cline *, result, int))
{
    if (size <= 0)
    {
        return 0;
    }
    size_t tag = (memAddress >> (cache_parameters->s + cache_parameters->b + cache_parameters->u));
    size_t set_idx = ((tag << cache_parameters->s) ^
                      (memAddress >> (cache_parameters->b + cache_parameters->u)));
    size_t sub_block = (memAddress >> cache_parameters->b) ^
                       (((set_idx << cache_parameters->u) | (tag << (cache_parameters->s + cache_parameters->u))));
    size_t block_offset = (((tag << (cache_parameters->s + cache_parameters->b + cache_parameters->u)) |
                            (set_idx << cache_parameters->b + cache_parameters->u)) |
                           (sub_block << cache_parameters->b)) ^
                          memAddress;
    size_t victim = 0;
    size_t eldest = 0;
    size_t U = 1 << cache_parameters->u;
    size_t B = 1 << cache_parameters->b;
    int return_stack = 0;
    int size_tmp = size;
    for (size_t i = 0; i < cache_parameters->E; i++)
    {
        cline *currline = cache[set_idx][i];
        if (currline->age > eldest)
        {
            eldest = currline->age;
            victim = i;
        }
        // Cold misses
        if (!currline->valid_line)
        {
            stats->misses++;
            currline->tag = tag;
            currline->valid_line = true;
            for (size_t j = sub_block; j < U && size_tmp > 0; j++)
            {
                currline->valid[j] = true;
                return_stack += 100;
                size_tmp -= B;
            }
            for (size_t j = 0; j < cache_parameters->E; j++)
            {
                (*cache_update)(cache[set_idx][j], DEFAULT, 1);
            }
            (*cache_update)(currline, MISS, 0);
            // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
            if ((size + block_offset) > B * (U - sub_block))
            {
                int jump_size = B * (U - sub_block) - block_offset;
                return_stack += load_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
            }
            return return_stack;
        }
        else if (currline->tag == tag)
        {
            stats->hits++;
            for (size_t j = sub_block; j < U && size_tmp > 0; j++)
            {
                if (currline->valid[j])
                {
                    return_stack += 1;
                }
                else
                {
                    currline->valid[j] = true;
                    return_stack += 100;
                }
                size_tmp -= B;
            }
            for (size_t j = 0; j < cache_parameters->E; j++)
            {
                (*cache_update)(cache[set_idx][j], DEFAULT, 1);
            }
            (*cache_update)(currline, HIT, 0);
            // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
            if ((size + block_offset) > B * (U - sub_block))
            {
                int jump_size = B * (U - sub_block) - block_offset;
                return_stack += load_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
            }
            return return_stack;
        }
    }
    // This case is for conflict miss/eviction
    int diff = (1 << cache_parameters->k) - 1 - eldest;
    for (size_t i = 0; i < cache_parameters->E; i++)
    {
        cline *currline = cache[set_idx][i];
        (*cache_update)(currline, DEFAULT, diff);
    }
    cline *currline = cache[set_idx][victim];
    currline->tag = tag;
    for (int i = 0; i < U; i++)
    {
        currline->valid[i] = false;
        if (currline->dirty)
        {
            return_stack += 50;
        }
        currline->dirty = false;
    }
    for (size_t j = sub_block; j < U && size_tmp > 0; j++)
    {
        currline->valid[j] = true;
        return_stack += 100;
        size_tmp -= B;
    }
    // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
    (*cache_update)(currline, EVICT, 0);
    stats->evictions++;
    stats->misses++;
    // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
    if ((size + block_offset) > B * (U - sub_block))
    {
        int jump_size = B * (U - sub_block) - block_offset;
        return_stack += load_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
    }
    return return_stack;
}

/** @brief Simulates a store instruction
 *
 */
int store_cache(cache_t *cache, int size, uint64_t memAddress, parsed_t *cache_parameters, void (*cache_update)(cline *, result, int))
{
    if (size <= 0)
    {
        return 0;
    }
    size_t tag = (memAddress >> (cache_parameters->s + cache_parameters->b + cache_parameters->u));
    size_t set_idx = ((tag << cache_parameters->s) ^
                      (memAddress >> (cache_parameters->b + cache_parameters->u)));
    size_t sub_block = (memAddress >> cache_parameters->b) ^
                       (((set_idx << cache_parameters->u) | (tag << (cache_parameters->s + cache_parameters->u))));
    size_t block_offset = (((tag << (cache_parameters->s + cache_parameters->b + cache_parameters->u)) |
                            (set_idx << cache_parameters->b + cache_parameters->u)) |
                           (sub_block << cache_parameters->b)) ^
                          memAddress;
    size_t victim = 0;
    size_t eldest = 0;
    size_t U = 1 << cache_parameters->u;
    size_t B = 1 << cache_parameters->b;
    int size_tmp = size;
    int return_stack = 0;
    for (size_t i = 0; i < cache_parameters->E; i++)
    {
        cline *currline = cache[set_idx][i];
        if (currline->age > eldest)
        {
            eldest = currline->age;
            victim = i;
        }
        if (!currline->valid_line)
        {
            stats->misses++;
            currline->tag = tag;
            currline->valid_line = true;
            for (size_t j = sub_block; j < U && size_tmp > 0; j++)
            {
                currline->dirty = true;
                currline->valid[j] = true;
                return_stack += 100;
                size_tmp -= B;
            }
            for (size_t j = 0; j < cache_parameters->E; j++)
            {
                (*cache_update)(cache[set_idx][j], DEFAULT, 0);
            }
            (*cache_update)(currline, MISS, 0);
            if ((size + block_offset) > B * (U - sub_block))
            {
                int jump_size = B * (U - sub_block) - block_offset;
                return_stack += store_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
            }
            // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
            return return_stack;
        }
        else if (currline->tag == tag)
        {
            stats->hits++;
            for (size_t j = sub_block; j < U && size_tmp > 0; j++)
            {
                currline->dirty = true;
                if (currline->valid[j])
                {
                    return_stack += 1;
                }
                else
                {
                    currline->valid[j] = true;
                    return_stack += 100;
                }
                size_tmp -= B;
            }
            for (size_t j = 0; j < cache_parameters->E; j++)
            {
                (*cache_update)(cache[set_idx][j], DEFAULT, 0);
            }
            (*cache_update)(currline, HIT, 0);
            // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
            if ((size + block_offset) > B * (U - sub_block))
            {
                int jump_size = B * (U - sub_block) - block_offset;
                return_stack += store_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
            }
            return return_stack;
        }
    }
    // This case is for conflict miss/eviction
    int diff = (1 << cache_parameters->k) - 1 - eldest;
    for (size_t i = 0; i < cache_parameters->E; i++)
    {
        cline *currline = cache[set_idx][i];
        (*cache_update)(currline, DEFAULT, diff);
    }
    cline *currline = cache[set_idx][victim];
    currline->tag = tag;
    for (int i = 0; i < U; i++)
    {
        currline->valid[i] = false;
        if (currline->dirty)
        {
            return_stack += 50;
        }
        currline->dirty = false;
    }
    for (size_t j = sub_block; j < U && size_tmp > 0; j++)
    {
        currline->dirty = true;
        currline->valid[j] = true;
        return_stack += 100;
        size_tmp -= B;
    }
    // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
    (*cache_update)(currline, EVICT, 0);
    stats->evictions++;
    stats->misses++;
    if ((size + block_offset) > B * (U - sub_block))
    {
        int jump_size = B * (U - sub_block) - block_offset;
        return_stack += store_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
    }
    // printf("return_stack: %ld, line %ld\n", return_stack, __LINE__);
    if ((size + block_offset) > B * (U - sub_block))
    {
        int jump_size = B * (U - sub_block) - block_offset;
        return_stack += load_cache(cache, size - jump_size, memAddress + jump_size, cache_parameters, cache_update);
    }
    return return_stack;
}
