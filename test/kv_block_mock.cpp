extern "C" {
#include "kv_block.h"
}

#include "CppUTestExt/MockSupport.h"


void kv_block__init_empty(struct kv_block *bv_block)
{

}

void kv_block__init(struct kv_block *bv_block, uint64_t key_id, uint32_t data_bytes, const char *value_data, uint64_t sequence)
{
}

bool kv_block__is_allocated(const struct kv_block *kv_block)
{
	return mock().actualCall("kv_block__is_allocated").returnBoolValue();
}

bool kv_block__validate(const struct kv_block *kv_block)
{
	return mock().actualCall("kv_block__validate").returnBoolValue();
}

