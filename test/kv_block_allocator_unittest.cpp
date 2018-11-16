#include <gtest/gtest.h>

extern "C" {
#include "kv_block_allocator.h"
}

TEST(kv_block_allocator, all_blocks_start_free)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 15));

	ASSERT_EQ(0, kv_block_allocator__find_next_free_block(allocator, 0));
	ASSERT_EQ(1, kv_block_allocator__find_next_free_block(allocator, 1));
	ASSERT_EQ(13, kv_block_allocator__find_next_free_block(allocator, 13));
	ASSERT_EQ(14, kv_block_allocator__find_next_free_block(allocator, 14));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_allocated_block)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	ASSERT_EQ(1, kv_block_allocator__find_next_free_block(allocator, 0));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_allocated_block_at_end_and_wraps_to_beginning_block)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	ASSERT_EQ(0, kv_block_allocator__find_next_free_block(allocator, 14));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_range_of_allocated_blocks_that_wrap_to_block_0)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 11);
	kv_block_allocator__mark_block_as_allocated(allocator, 12);
	kv_block_allocator__mark_block_as_allocated(allocator, 13);
	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	ASSERT_EQ(0, kv_block_allocator__find_next_free_block(allocator, 11));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_range_of_allocated_blocks_that_wrap_to_block_2)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 13);
	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	ASSERT_EQ(2, kv_block_allocator__find_next_free_block(allocator, 13));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__returns_UINT32_MAX_when_all_blocks_are_allocated)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	ASSERT_EQ(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 0));
	ASSERT_EQ(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__block_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	ASSERT_EQ(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 2);
	ASSERT_EQ(2, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__block_0_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	ASSERT_EQ(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 0);
	ASSERT_EQ(0, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__last_block_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	ASSERT_EQ(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	ASSERT_EQ(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 4);
	ASSERT_EQ(4, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}
