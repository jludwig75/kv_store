#include "kv_store_write_block.h"

#include "kv_block.h"

#include <errno.h>
#include <assert.h>


int kv_write_key_block(struct kvstor *store, const struct key *k, size_t value_bytes, const char *value_buffer)
{
    if (value_bytes == 0)
    {
        assert(value_buffer == NULL);
        if (value_buffer != NULL)
        {
            return -EINVAL;
        }
    }
    if (value_buffer == NULL)
    {
        assert(value_bytes == 0);
        if (value_bytes != 0)
        {
            return -EINVAL;
        }
    }
    
    uint64_t my_sequence = ++store->current_sequence_number;
    
    /// @todo Allocate on heap rather than stack
    struct kv_block block_data;
    kv_block__init(&block_data, k->id, (uint32_t)value_bytes, value_buffer, my_sequence);

    uint32_t destination_block = kv_append_point__get_append_point(store->append_point);
    if (destination_block == UINT32_MAX)
    {
        return -ENOSPC;
    }

    int ret = kv_block_array__write_block(store->block_array, destination_block, (const uint8_t *)&block_data);
    if (ret != 0)
    {
        return ret;
    }

    bool set_as_latest_entry_for_key;
    uint32_t replaced_block;
    ret = kv_directory__store_key(store->directory, k->id, destination_block, value_bytes, my_sequence, &set_as_latest_entry_for_key, &replaced_block);
    if (ret != 0)
    {
        return ret;
    }

    if (replaced_block != UINT32_MAX)
    {
        kv_block_allocator__free_block(store->block_allocator, replaced_block);
    }

    assert(set_as_latest_entry_for_key);
    kv_block_allocator__mark_block_as_allocated(store->block_allocator, destination_block);
    
    return 0;
}
