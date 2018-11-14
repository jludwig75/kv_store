/*
 * BlockAllocator.cpp
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */
#include "kv_block_allocator.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

struct kv_block_allocator
{
    uint32_t number_of_blocks;
    uint8_t *bits;
};


int kv_block_allocator__init(struct kv_block_allocator **block_allocator, uint32_t number_of_blocks)
{
    *block_allocator = (struct kv_block_allocator *)malloc(sizeof(struct kv_block_allocator));
    if (!*block_allocator)
    {
        return -ENOMEM;
    }

    (*block_allocator)->number_of_blocks = number_of_blocks;
    size_t bit_array_bytes = (number_of_blocks + 7) / 8;
    (*block_allocator)->bits = (uint8_t *)malloc(bit_array_bytes);
    memset((*block_allocator)->bits, 0, bit_array_bytes);

    return 0;
}

void kv_block_allocator__cleanup(struct kv_block_allocator **block_allocator)
{
    free((*block_allocator)->bits);
    free(*block_allocator);
    *block_allocator = NULL;
}


static size_t kv_block_allocator__block_to_byte(const struct kv_block_allocator *block_allocator, const uint32_t block)
{
    assert(block < block_allocator->number_of_blocks);

    return block / 8;
}

static size_t kv_block_allocator__block_to_bit(const struct kv_block_allocator *block_allocator, uint32_t block)
{
    assert(block < block_allocator->number_of_blocks);

    return block % 8;
}

static bool kv_block_allocator__is_set(const struct kv_block_allocator *block_allocator, uint32_t block)
{
    return ((1 << kv_block_allocator__block_to_bit(block_allocator, block)) & block_allocator->bits[kv_block_allocator__block_to_byte(block_allocator, block)]) != 0;
}

static void kv_blocmk_allocator__set(struct kv_block_allocator *block_allocator, uint32_t block)
{
    block_allocator->bits[kv_block_allocator__block_to_byte(block_allocator, block)] |= (1 << kv_block_allocator__block_to_bit(block_allocator, block));
}

static void kv_blocmk_allocator__clear(struct kv_block_allocator *block_allocator, uint32_t block)
{
    block_allocator->bits[kv_block_allocator__block_to_byte(block_allocator, block)] &= ~(1 << kv_block_allocator__block_to_bit(block_allocator, block));
}


uint32_t kv_block_allocator__find_next_free_block(const struct kv_block_allocator *block_allocator, uint32_t starting_block)
{
    assert(starting_block <= block_allocator->number_of_blocks);
    if (starting_block > block_allocator->number_of_blocks)
    {
        return UINT32_MAX;
    }

    if (starting_block == block_allocator->number_of_blocks)
    {
        starting_block = 0;
    }

    // Scan from starting block to the end of the bitmap.
    uint32_t b;
    for (b = starting_block; b < block_allocator->number_of_blocks; b++)
    {
        if (!kv_block_allocator__is_set(block_allocator, b))
        {
            return b;
        }
    }

    // We didn't find a starting block at the end of the bitmap
    if (starting_block > 0)
    {
        // If we didn't already start at block 0,
        // Start at the beginning of the log and scan
        // up to starting_block, non-inclusive.
        for (b = 0; b < starting_block; b++)
        {
            if (!kv_block_allocator__is_set(block_allocator, b))
            {
                return b;
            }
        }
    }
    return UINT32_MAX;
}

void kv_block_allocator__free_block(struct kv_block_allocator *block_allocator, uint32_t block)
{
    assert(block < block_allocator->number_of_blocks);

    kv_blocmk_allocator__clear(block_allocator, block);
}

void kv_block_allocator__mark_block_as_allocated(struct kv_block_allocator *block_allocator, uint32_t block)
{
    assert(block < block_allocator->number_of_blocks);

    kv_blocmk_allocator__set(block_allocator, block);
}
