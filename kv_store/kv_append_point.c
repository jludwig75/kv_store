/*
 * AppendPoint.cpp
 *
 *  Created on: Nov 9, 2018
 *      Author: jludwig
 */
#include "kv_append_point.h"

#include "kv_block_allocator.h"

#include <errno.h>

struct kv_append_point
{
    struct kv_block_allocator *block_allocator;
    uint32_t current_append_point;
    uint64_t last_scanned_sequence_number;
};

int kv_append_point__init(struct kv_append_point **append_point, struct kv_block_allocator *block_allocator)
{
    *append_point = (struct kv_append_point *)malloc(sizeof(struct kv_append_point));
    if (!*append_point)
    {
        return -ENOMEM;
    }

    (*append_point)->block_allocator = block_allocator;
    (*append_point)->current_append_point = 0;
    (*append_point)->last_scanned_sequence_number = 0;

    return 0;
}

void kv_append_point__cleanup(struct kv_append_point **append_point)
{
    free(*append_point);
    *append_point = NULL;
}


uint32_t kv_append_point__get_append_point(struct kv_append_point *append_point)
{
    if (append_point->current_append_point == UINT32_MAX)
    {
        // If we ran out of space before, start back at 0.
        append_point->current_append_point = 0;
    }

    // Start from the last append point used (or seen at replay) and find the next free block.
    append_point->current_append_point = kv_block_allocator__find_next_free_block(append_point->block_allocator, append_point->current_append_point);
    return append_point->current_append_point;
}

void kv_append_point__update_append_point(struct kv_append_point *append_point, uint32_t block, uint64_t sequence)
{
    // See if this sequence was higher than the last seen.
    if (sequence >= append_point->last_scanned_sequence_number)    // >= to handle only block 0 written.
    {
        // This sequence is higher. Use this block as the append point.
        // The call to get_append_point will take care of advancing it past this block.
        append_point->current_append_point = block;
        // Keep track of this sequence until we find a higher one.
        append_point->last_scanned_sequence_number = sequence;
    }
}
