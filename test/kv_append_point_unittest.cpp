#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "kv_append_point.h"
#include "kv_block_allocator.h"
}

TEST_GROUP(kv_append_point)
{
    void setup()
    {
    }
	void teardown()
    {
        mock().checkExpectations();
        mock().clear();
    }
};

TEST(kv_append_point, get_append_point__checks_append_point_with_allocator)
{
	kv_block_allocator *allocator;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 1000));

	kv_append_point *append_point;
	CHECK_EQUAL(0, kv_append_point__init(&append_point, allocator));

	mock().expectOneCall("kv_block_allocator__find_next_free_block").
		withParameter("starting_block", 0).
		andReturnValue(97);

	CHECK_EQUAL(97, kv_append_point__get_append_point(append_point));

	kv_append_point__cleanup(&append_point);
	kv_block_allocator__cleanup(&allocator);
}

TEST(kv_append_point, update_append_point__retains_highest_sequence_append_point)
{
	kv_block_allocator *allocator;
	CHECK_EQUAL(0, kv_block_allocator__init(&allocator, 1000));

	kv_append_point *append_point;
	CHECK_EQUAL(0, kv_append_point__init(&append_point, allocator));

	kv_append_point__update_append_point(append_point, 5, 6);
	kv_append_point__update_append_point(append_point, 15, 8);
	kv_append_point__update_append_point(append_point, 4, 4);
	kv_append_point__update_append_point(append_point, 555, 3);

	mock().expectOneCall("kv_block_allocator__find_next_free_block").
		withParameter("starting_block", 15).
		andReturnValue(15);

	CHECK_EQUAL(15, kv_append_point__get_append_point(append_point));

	kv_append_point__cleanup(&append_point);
	kv_block_allocator__cleanup(&allocator);
}

int main(int argc, char *argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
