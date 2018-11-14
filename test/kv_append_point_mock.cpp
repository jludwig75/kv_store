extern "C" {
#include "kv_append_point.h"
}

#include "CppUTestExt/MockSupport.h"


struct kv_append_point
{
	int dummy;
};

int kv_append_point__init(struct kv_append_point **append_point, struct kv_block_allocator *block_allocator)
{
	*append_point = new kv_append_point;
	return 0;
}

void kv_append_point__cleanup(struct kv_append_point **append_point)
{
	delete *append_point;
	*append_point = NULL;
}

uint32_t kv_append_point__get_append_point(struct kv_append_point *append_point)
{
	return mock().actualCall("kv_append_point__get_append_point").
					returnUnsignedIntValue();
}

void kv_append_point__update_append_point(struct kv_append_point *append_point, uint32_t block, uint64_t sequence)
{
	mock().actualCall("kv_append_point__update_append_point").
		withParameter("block", block).
		withUnsignedLongIntParameter("sequence", (unsigned long)sequence);
}

