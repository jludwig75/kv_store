#include "kv_block.h"

#include <assert.h>
#include <string.h>


static const uint64_t kv_block_signature = 0xB1423CB4C5BCBC4ALL;


void kv_block_header__init_empty(struct kv_block_header *header)
{
    header->signature = 0;
    header->key_id = 0;
    header->sequence = 0;
    header->data_bytes = 0;
}

void kv_block_header__init(struct kv_block_header *header, uint64_t key_id, uint32_t data_bytes, uint64_t sequence)
{
    header->signature = kv_block_signature;
    header->key_id = key_id;
    header->sequence = sequence;
    header->data_bytes = data_bytes;
}

bool kv_block_header__is_allocated(const struct kv_block_header *header)
{
    return header->data_bytes > 0;
}

bool kv_block_header__validate(const struct kv_block_header *header)
{
    return header->signature == kv_block_signature;
}



void kv_block__init_empty(struct kv_block *kv_block)
{
    kv_block_header__init_empty(&kv_block->header);
}

void kv_block__init(struct kv_block *kv_block, uint64_t key_id, uint32_t data_bytes, const char *value_data, uint64_t sequence)
{
    kv_block_header__init(&kv_block->header, key_id, data_bytes, sequence);
    assert(data_bytes <= 4096);
    if (value_data)
    {
        memcpy(kv_block->data, value_data, data_bytes);
    }
}

bool kv_block__is_allocated(const struct kv_block *kv_block)
{
    return kv_block_header__is_allocated(&kv_block->header);
}

bool kv_block__validate(const struct kv_block *kv_block)
{
    return kv_block_header__validate(&kv_block->header);
}
