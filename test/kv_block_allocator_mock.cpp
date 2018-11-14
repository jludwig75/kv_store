extern "C" {
#include "kv_block_allocator.h"
}

#include "CppUTestExt/MockSupport.h"


struct kv_block_allocator
{
	int dummy;
};

int kv_block_allocator__init(struct kv_block_allocator **block_allocator, uint32_t number_of_blocks)
{
	*block_allocator = new kv_block_allocator;

	return 0;
}

void kv_block_allocator__cleanup(struct kv_block_allocator **block_allocator)
{
	delete *block_allocator;
	*block_allocator = NULL;
}

uint32_t kv_block_allocator__find_next_free_block(const struct kv_block_allocator *block_allocator, uint32_t starting_block)
{
	return (uint32_t)mock().actualCall("kv_block_allocator__find_next_free_block").
							withParameter("starting_block", starting_block).
							returnUnsignedIntValue();
}

void kv_block_allocator__free_block(struct kv_block_allocator *block_allocator, uint32_t block)
{
	mock().actualCall("kv_block_allocator__free_block").
			withParameter("block", block);
}

void kv_block_allocator__mark_block_as_allocated(struct kv_block_allocator *block_allocator, uint32_t block)
{
	mock().actualCall("kv_block_allocator__mark_block_as_allocated").
			withParameter("block", block);
}

