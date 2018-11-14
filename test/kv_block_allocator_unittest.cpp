#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"

extern "C" {
#include "kv_block_allocator.h"
}

TEST_GROUP(kv_block_allocator)
{
};

TEST(kv_block_allocator, all_blocks_start_free)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 15));

	CHECK_EQUAL(0, kv_block_allocator__find_next_free_block(allocator, 0));
	CHECK_EQUAL(1, kv_block_allocator__find_next_free_block(allocator, 1));
	CHECK_EQUAL(13, kv_block_allocator__find_next_free_block(allocator, 13));
	CHECK_EQUAL(14, kv_block_allocator__find_next_free_block(allocator, 14));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_allocated_block)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	CHECK_EQUAL(1, kv_block_allocator__find_next_free_block(allocator, 0));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_allocated_block_at_end_and_wraps_to_beginning_block)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	CHECK_EQUAL(0, kv_block_allocator__find_next_free_block(allocator, 14));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_range_of_allocated_blocks_that_wrap_to_block_0)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 11);
	kv_block_allocator__mark_block_as_allocated(allocator, 12);
	kv_block_allocator__mark_block_as_allocated(allocator, 13);
	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	CHECK_EQUAL(0, kv_block_allocator__find_next_free_block(allocator, 11));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__skips_range_of_allocated_blocks_that_wrap_to_block_2)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 15));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 13);
	kv_block_allocator__mark_block_as_allocated(allocator, 14);
	CHECK_EQUAL(2, kv_block_allocator__find_next_free_block(allocator, 13));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__returns_UINT32_MAX_when_all_blocks_are_allocated)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	CHECK_EQUAL(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 0));
	CHECK_EQUAL(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__block_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	CHECK_EQUAL(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 2);
	CHECK_EQUAL(2, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__block_0_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	CHECK_EQUAL(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 0);
	CHECK_EQUAL(0, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_block_allocator, find_next_free_block__last_block_can_be_allocated_after_it_is_freed)
{
	kv_block_allocator *allocator = NULL;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 5));

	kv_block_allocator__mark_block_as_allocated(allocator, 0);
	kv_block_allocator__mark_block_as_allocated(allocator, 1);
	kv_block_allocator__mark_block_as_allocated(allocator, 2);
	kv_block_allocator__mark_block_as_allocated(allocator, 3);
	kv_block_allocator__mark_block_as_allocated(allocator, 4);
	CHECK_EQUAL(UINT32_MAX, kv_block_allocator__find_next_free_block(allocator, 3));
	kv_block_allocator__free_block(allocator, 4);
	CHECK_EQUAL(4, kv_block_allocator__find_next_free_block(allocator, 3));

	kv_block_allocator__cleanup(&allocator);
}


int main(int argc, char *argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
