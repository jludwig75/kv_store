#include "kv_store_replay.h"

#include "kv_block.h"


int kv_store__replay_log(struct kvstor *store)
{
    uint32_t total_blocks;
    int ret = kv_block_array__get_file_block_count(store->block_array, &total_blocks);
    if (ret != 0)
    {
        return ret;
    }

    struct kv_block block;
    kv_block__init_empty(&block);
    uint32_t b;
    // Scan all of the blocks stored in the file.
    for (b = 0; b < total_blocks; b++)
    {
        // Read the block
        ret = kv_block_array__read_block(store->block_array, b, (uint8_t *)&block);
        if (ret != 0)
        {
            return ret;
        }

        // Only evaluate the block if it is valid
        if (kv_block__validate(&block))
        {
            // Store the key data in the directory
            // It will only be stored if it is the latest
            // version for this key.
            bool set_as_latest_entry_for_key;
            uint32_t replaced_block;
            ret = kv_directory__store_key(store->directory, block.header.key_id, b, block.header.data_bytes, block.header.sequence, &set_as_latest_entry_for_key, &replaced_block);
            if (ret != 0)
            {
                return ret;
            }

            // A block for a previous version of this key was replaced.
            if (replaced_block != UINT32_MAX)
            {
                // Mark the previous block as free.
                kv_block_allocator__free_block(store->block_allocator, replaced_block);
            }

            // This version of the key was the latest and it was stored in the directory.
            if (set_as_latest_entry_for_key)
            {
                // Only mark the block allcoated if it is the latest version
                kv_block_allocator__mark_block_as_allocated(store->block_allocator, b);
            }

            // Keep the append point up-to-date.
            kv_append_point__update_append_point(store->append_point, b, block.header.sequence);

            // Replay the sequence number as well.
            if (block.header.sequence >= store->current_sequence_number)
            {
                // Keep the sequence number one ahead of the highest sequence seen.
                store->current_sequence_number = block.header.sequence + 1;
            }
        }
    }

    return 0;
}
