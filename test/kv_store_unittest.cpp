#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"
#include "opmock.h"

#include "kv_append_point_stub.h"

extern "C" {
#include "kv_store.h"
#include "kv_block_array.h"
}

#include <errno.h>
#include <string.h>


struct test_value
{
	union
	{
		struct
		{
			size_t size;
			char data[MAXBLOB];
		};
		struct value value;
	};
};


TEST_GROUP(kv_open)
{
    void setup()
    {
    }
	void teardown()
    {
        mock().checkExpectations();
        mock().clear();
		OP_VERIFY();
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


TEST_GROUP(kv_set)
{
    void setup()
    {
		char *test_file_name = "test_file_name";
		char *argv[] = { "dummy", test_file_name };

		mock().expectOneCall("kv_block_array__open").
			withStringParameter("file_name", test_file_name).
			withBoolParameter("create", true).
			andReturnValue(0);

		kv_append_point__init_IgnoreAndReturn(0);

		CHECK_EQUAL(0, kv_open(&store, true, 2, argv));
    }
	void teardown()
    {
		kv_append_point__cleanup_IgnoreAndReturn();
		kv_close(store);

        mock().checkExpectations();
        mock().clear();
    }
	struct kvstor *store;
};

TEST(kv_set, initializes_kv_block)
{
	struct test_value v1;
	v1.size = 4;

	struct key k;
	k.id = 97;

	mock().expectOneCall("kv_block__init").
			withParameter("key_id", (unsigned long int)k.id).
			withParameter("data_bytes", v1.value.size).
			withParameter("value_data", v1.value.data).
			withUnsignedLongIntParameter("sequence", 1);

	kv_append_point__get_append_point_ExpectAndReturn(NULL, UINT32_MAX, NULL);

	CHECK_EQUAL(-ENOSPC, kv_set(store, &k, &v1.value));
}

int main(int argc, char *argv[])
{
	return CommandLineTestRunner::RunAllTests(argc, argv);
}
