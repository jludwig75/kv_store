#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock-global/gmock-global.h>

extern "C" {
#include "kv_append_point.h"
#include "kv_block_allocator.h"
}

MOCK_GLOBAL_FUNC2(kv_block_allocator__init, int(struct kv_block_allocator **block_allocator, uint32_t number_of_blocks));
MOCK_GLOBAL_FUNC1(kv_block_allocator__cleanup, void(struct kv_block_allocator **block_allocator));
MOCK_GLOBAL_FUNC2(kv_block_allocator__find_next_free_block, uint32_t(const struct kv_block_allocator *block_allocator, uint32_t starting_block));
MOCK_GLOBAL_FUNC2(kv_block_allocator__free_block, void(struct kv_block_allocator *block_allocator, uint32_t block));
MOCK_GLOBAL_FUNC2(kv_block_allocator__mark_block_as_allocated, void(struct kv_block_allocator *block_allocator, uint32_t block));


TEST(kv_append_point, get_append_point__checks_append_point_with_allocator)
{
	kv_block_allocator *allocator = (kv_block_allocator *)0x534547;

	kv_append_point *append_point;
	ASSERT_EQ(0, kv_append_point__init(&append_point, allocator));

	EXPECT_GLOBAL_CALL(kv_block_allocator__find_next_free_block, kv_block_allocator__find_next_free_block(allocator, 0)).WillOnce(testing::Return(97));

	ASSERT_EQ(97, kv_append_point__get_append_point(append_point));

	kv_append_point__cleanup(&append_point);
}

TEST(kv_append_point, update_append_point__retains_highest_sequence_append_point)
{
	kv_block_allocator *allocator = (kv_block_allocator *)0x534547;

	kv_append_point *append_point;
	ASSERT_EQ(0, kv_append_point__init(&append_point, allocator));

	kv_append_point__update_append_point(append_point, 5, 6);
	kv_append_point__update_append_point(append_point, 15, 8);
	kv_append_point__update_append_point(append_point, 4, 4);
	kv_append_point__update_append_point(append_point, 555, 3);

	EXPECT_GLOBAL_CALL(kv_block_allocator__find_next_free_block, kv_block_allocator__find_next_free_block(allocator, 15)).WillOnce(testing::Return(15));

	ASSERT_EQ(15, kv_append_point__get_append_point(append_point));

	kv_append_point__cleanup(&append_point);
}
