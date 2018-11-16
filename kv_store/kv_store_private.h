#pragma once

#include "kv_store.h"
#include "kv_block_allocator.h"
#include "kv_block_array.h"
#include "kv_directory.h"
#include "kv_append_point.h"

#include <stdint.h>


struct kvstor
{
    struct kv_block_array *block_array;
    struct kv_directory *directory;
    struct kv_block_allocator *block_allocator;
    struct kv_append_point *append_point;

    uint64_t current_sequence_number;
};
