#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "kv_store.h"
#include "kv_block_array.h"
}

#include <errno.h>
#include <string.h>


TEST_GROUP(kv_open)
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

TEST(kv_open, calls_block_array_open_with_file_name_and_create_flag)
{
	char *test_file_name = "test_file_name";
	char *argv[] = { "dummy", test_file_name };

	mock().expectOneCall("kv_block_array__open").
		withStringParameter("file_name", test_file_name).
		withBoolParameter("create", true).
		andReturnValue(0);

	struct kvstor *store;
	CHECK_EQUAL(0, kv_open(&store, true, 2, argv));
	kv_close(store);
}

TEST(kv_open, returns_failue_if_block_array_open_fails)
{
	char *test_file_name = "test_file_name";
	char *argv[] = { "dummy", test_file_name };

	mock().expectOneCall("kv_block_array__open").
		withStringParameter("file_name", test_file_name).
		withBoolParameter("create", false).
		andReturnValue(-77);

	struct kvstor *store;
	CHECK_EQUAL(-77, kv_open(&store, false, 2, argv));
	kv_close(store);
}

TEST(kv_open, scans_the_log_for_existing_store)
{
	char *test_file_name = "test_file_name";
	char *argv[] = { "dummy", test_file_name };

	mock().expectOneCall("kv_block_array__open").
		withStringParameter("file_name", test_file_name).
		withBoolParameter("create", false).
		andReturnValue(0);

	mock().expectOneCall("kv_store__replay_log").
		andReturnValue(0);

	struct kvstor *store;
	CHECK_EQUAL(0, kv_open(&store, false, 2, argv));
	kv_close(store);
}


int main(int argc, char *argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
