#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock-global/gmock-global.h>


extern "C" {
#include "kv_store.h"
#include "kv_store_replay.h"
#include "kv_block_array.h"
#include "kv_block_allocator.h"
#include "kv_block.h"
#include "kv_append_point.h"
#include "kv_directory.h"
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


//kv_store (kv_store_replay)
MOCK_GLOBAL_FUNC1(kv_store__replay_log, int(struct kvstor *store));

// kv_block_allocator
MOCK_GLOBAL_FUNC2(kv_block_allocator__init, int(struct kv_block_allocator **block_allocator, uint32_t number_of_blocks));
MOCK_GLOBAL_FUNC1(kv_block_allocator__cleanup, void(struct kv_block_allocator **block_allocator));
MOCK_GLOBAL_FUNC2(kv_block_allocator__find_next_free_block, uint32_t(const struct kv_block_allocator *block_allocator, uint32_t starting_block));
MOCK_GLOBAL_FUNC2(kv_block_allocator__free_block, void(struct kv_block_allocator *block_allocator, uint32_t block));
MOCK_GLOBAL_FUNC2(kv_block_allocator__mark_block_as_allocated, void(struct kv_block_allocator *block_allocator, uint32_t block));

// kv_block_array
MOCK_GLOBAL_FUNC2(kv_block_array__init, int(struct kv_block_array **block_array, size_t raw_block_bytes));
MOCK_GLOBAL_FUNC1(kv_block_array__cleanup, void(struct kv_block_array **block_array));
MOCK_GLOBAL_FUNC3(kv_block_array__open, int(struct kv_block_array *block_array, const char *file_name, bool create));
MOCK_GLOBAL_FUNC2(kv_block_array__get_file_block_count, int(const struct kv_block_array *block_array, uint32_t *total_blocks));
MOCK_GLOBAL_FUNC3(kv_block_array__read_block, int(const struct kv_block_array *block_array, uint32_t block, uint8_t *block_data));
MOCK_GLOBAL_FUNC3(kv_block_array__write_block, int(struct kv_block_array *block_array, uint32_t destination_block, const uint8_t *block_data));

// kv_block
MOCK_GLOBAL_FUNC1(kv_block__init_empty, void(struct kv_block *bv_block));
MOCK_GLOBAL_FUNC5(kv_block__init, void(struct kv_block *bv_block, uint64_t key_id, uint32_t data_bytes, const char *value_data, uint64_t sequence));
MOCK_GLOBAL_FUNC1(kv_block__is_allocated, bool(const struct kv_block *kv_block));
MOCK_GLOBAL_FUNC1(kv_block__validate, bool(const struct kv_block *kv_block));

// kv_append_point
MOCK_GLOBAL_FUNC2(kv_append_point__init, int(struct kv_append_point **append_point, struct kv_block_allocator *block_allocator));
MOCK_GLOBAL_FUNC1(kv_append_point__cleanup, void(struct kv_append_point **append_point));
MOCK_GLOBAL_FUNC1(kv_append_point__get_append_point, uint32_t(struct kv_append_point *append_point));
MOCK_GLOBAL_FUNC3(kv_append_point__update_append_point, void (struct kv_append_point *append_point, uint32_t block, uint64_t sequence));

MOCK_GLOBAL_FUNC1(kv_directory__init, int(struct kv_directory **directory));
MOCK_GLOBAL_FUNC1(kv_directory__cleanup, void(struct kv_directory **directory));
MOCK_GLOBAL_FUNC7(kv_directory__store_key, int(struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block));
MOCK_GLOBAL_FUNC4(kv_directory__lookup_key, int(struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes));
MOCK_GLOBAL_FUNC2(kv_directory__remove_key, int(struct kv_directory *directory, uint64_t key));


TEST(kv_open_Test, calls_block_array_open_with_file_name_and_create_flag)
{
    char *test_file_name = "test_file_name";
    char *argv[] = { "dummy", test_file_name };
    struct kvstor *store;

    EXPECT_GLOBAL_CALL(kv_block_allocator__init, kv_block_allocator__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__init, kv_block_array__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_directory__init, kv_directory__init(testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_append_point__init, kv_append_point__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__open, kv_block_array__open(testing::_, test_file_name, true)).WillOnce(testing::Return(0));

    ASSERT_EQ(0, kv_open(&store, true, 2, argv));


    EXPECT_GLOBAL_CALL(kv_block_allocator__cleanup, kv_block_allocator__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_block_array__cleanup, kv_block_array__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_append_point__cleanup, kv_append_point__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_directory__cleanup, kv_directory__cleanup(testing::_));
    kv_close(store);
}

TEST(kv_open_Test, returns_failue_if_block_array_open_fails)
{
    char *test_file_name = "test_file_name";
    char *argv[] = { "dummy", test_file_name };
    struct kvstor *store;

    EXPECT_GLOBAL_CALL(kv_block_allocator__init, kv_block_allocator__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__init, kv_block_array__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_directory__init, kv_directory__init(testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_append_point__init, kv_append_point__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__open, kv_block_array__open(testing::_, test_file_name, false)).WillOnce(testing::Return(-77));

    ASSERT_EQ(-77, kv_open(&store, false, 2, argv));


    EXPECT_GLOBAL_CALL(kv_block_allocator__cleanup, kv_block_allocator__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_block_array__cleanup, kv_block_array__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_directory__cleanup, kv_directory__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_append_point__cleanup, kv_append_point__cleanup(testing::_));
    kv_close(store);
}

TEST(kv_open_Test, scans_the_log_for_existing_store)
{
    char *test_file_name = "test_file_name";
    char *argv[] = { "dummy", test_file_name };
    struct kvstor *store;

    EXPECT_GLOBAL_CALL(kv_block_allocator__init, kv_block_allocator__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__init, kv_block_array__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_directory__init, kv_directory__init(testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_append_point__init, kv_append_point__init(testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_GLOBAL_CALL(kv_block_array__open, kv_block_array__open(testing::_, test_file_name, false)).WillOnce(testing::Return(0));

    EXPECT_GLOBAL_CALL(kv_store__replay_log, kv_store__replay_log(testing::_)).WillOnce(testing::Return(0));

    ASSERT_EQ(0, kv_open(&store, false, 2, argv));


    EXPECT_GLOBAL_CALL(kv_block_allocator__cleanup, kv_block_allocator__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_block_array__cleanup, kv_block_array__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_directory__cleanup, kv_directory__cleanup(testing::_));
    EXPECT_GLOBAL_CALL(kv_append_point__cleanup, kv_append_point__cleanup(testing::_));
    kv_close(store);
}

class kv_set_Test : public GlobalMockTest
{
protected:
    virtual void SetUp()
    {
        char *test_file_name = "test_file_name";
        char *argv[] = { "dummy", test_file_name };
        
        _block_array_ptr = (struct kv_block_array *)0x345346;
        _block_allocator_ptr = (struct kv_block_allocator *)0x24534543;
        _directory_ptr = (struct kv_directory *)0x534567;
        _append_point_ptr = (struct kv_append_point *)0x42356;
        EXPECT_GLOBAL_CALL(kv_block_allocator__init, kv_block_allocator__init(testing::_, testing::_)).WillOnce([this](struct kv_block_allocator **block_allocator, uint32_t number_of_blocks) { *block_allocator = _block_allocator_ptr; return 0; });
        EXPECT_GLOBAL_CALL(kv_block_array__init, kv_block_array__init(testing::_, testing::_)).WillOnce([this](struct kv_block_array **block_array, size_t raw_block_bytes){ *block_array = _block_array_ptr; return 0; });
        EXPECT_GLOBAL_CALL(kv_directory__init, kv_directory__init(testing::_)).WillOnce([this](struct kv_directory **directory) { *directory = _directory_ptr; return 0; } );
        EXPECT_GLOBAL_CALL(kv_append_point__init, kv_append_point__init(testing::_, testing::_)).WillOnce([this](struct kv_append_point **append_point, struct kv_block_allocator *block_allocator){ *append_point = _append_point_ptr; return 0;});
        EXPECT_GLOBAL_CALL(kv_block_array__open, kv_block_array__open(testing::_, test_file_name, true)).WillOnce(testing::Return(0));

        ASSERT_EQ(0, kv_open(&store, true, 2, argv));

	    testSetUp();
    }
    void testSetUp()
    {
		// Setup mocks to default behavior for all tests:
		ON_GLOBAL_CALL(kv_block__init, kv_block__init(testing::_, testing::_, testing::_, testing::_, testing::_)).
							WillByDefault([](struct kv_block *bv_block, uint64_t key_id, uint32_t data_bytes, const char *value_data, uint64_t sequence){});
		ON_GLOBAL_CALL(kv_append_point__get_append_point, kv_append_point__get_append_point(_append_point_ptr)).
							WillByDefault(testing::Return(17));
		ON_GLOBAL_CALL(kv_block_array__write_block, kv_block_array__write_block(_block_array_ptr, testing::_, testing::_)).
							WillByDefault(testing::Return(0));
		ON_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).
							WillByDefault([](struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
											{
												*set_as_current_key_entry = true;
												*replaced_block = UINT32_MAX;
												return 0;
											});
		ON_GLOBAL_CALL(kv_block_allocator__mark_block_as_allocated, kv_block_allocator__mark_block_as_allocated(testing::_, testing::_)).WillByDefault([](struct kv_block_allocator *block_allocator, uint32_t block){});
    }
    virtual void TearDown()
    {
        EXPECT_GLOBAL_CALL(kv_block_allocator__cleanup, kv_block_allocator__cleanup(testing::_));
        EXPECT_GLOBAL_CALL(kv_block_array__cleanup, kv_block_array__cleanup(testing::_));
        EXPECT_GLOBAL_CALL(kv_directory__cleanup, kv_directory__cleanup(testing::_));
        EXPECT_GLOBAL_CALL(kv_append_point__cleanup, kv_append_point__cleanup(testing::_));
        kv_close(store);
    }
    struct kvstor *store;
    struct kv_block_array *_block_array_ptr;
    struct kv_block_allocator *_block_allocator_ptr;
    struct kv_directory *_directory_ptr;
    struct kv_append_point *_append_point_ptr;
};

TEST_F(kv_set_Test, initializes_kv_block_with_correct_data)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    const uint64_t exepected_sequence_number = 1;
    EXPECT_GLOBAL_CALL(kv_block__init, kv_block__init(testing::_, k.id, v1.value.size, v1.value.data, exepected_sequence_number));

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, returns_error_when_append_point_cannot_be_allocated)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_append_point__get_append_point, kv_append_point__get_append_point(_append_point_ptr)).WillOnce(testing::Return(UINT32_MAX));

	// We should not attempt to write a block if we could not allocate one.
    EXPECT_GLOBAL_CALL(kv_block_array__write_block, kv_block_array__write_block(_block_array_ptr, testing::_, testing::_)).Times(0);

    ASSERT_EQ(-ENOSPC, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, writes_kv_block_to_allocated_file_block)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    const uint32_t allocated_block = 17;
    EXPECT_GLOBAL_CALL(kv_append_point__get_append_point, kv_append_point__get_append_point(_append_point_ptr)).WillOnce(testing::Return(allocated_block));

    // Return error here to exit kv_set at this point.
    EXPECT_GLOBAL_CALL(kv_block_array__write_block, kv_block_array__write_block(_block_array_ptr, allocated_block, testing::_)).WillOnce(testing::Return(0));

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, returns_error_when_block_write_failes)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    // Return error here to exit kv_set at this point.
    EXPECT_GLOBAL_CALL(kv_block_array__write_block, kv_block_array__write_block(_block_array_ptr, testing::_, testing::_)).WillOnce(testing::Return(-EIO));

	// We should not write a directory entry if we fail to write a block.
    EXPECT_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).Times(0);

    ASSERT_EQ(-EIO, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, stores_written_block_to_directroy)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    const uint64_t exepected_sequence_number = 1;

    const uint32_t allocated_block = 97;
    EXPECT_GLOBAL_CALL(kv_append_point__get_append_point, kv_append_point__get_append_point(_append_point_ptr)).WillOnce(testing::Return(allocated_block));

    // Return error here to exit kv_set at this point.
    EXPECT_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, k.id, allocated_block, v1.value.size, exepected_sequence_number, testing::_, testing::_)).
						WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
									{
										*set_as_current_key_entry = true;
										*replaced_block = UINT32_MAX;
										return 0;
									});

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, returns_error_on_failure_to_write_block_to_directroy)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    // Return error here to exit kv_set at this point.
    EXPECT_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(-ENOMEM));

    ASSERT_EQ(-ENOMEM, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, marks_new_block_allocated)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    const uint32_t allocated_block = 97;
    EXPECT_GLOBAL_CALL(kv_append_point__get_append_point, kv_append_point__get_append_point(_append_point_ptr)).WillOnce(testing::Return(allocated_block));

	EXPECT_GLOBAL_CALL(kv_block_allocator__mark_block_as_allocated, kv_block_allocator__mark_block_as_allocated(_block_allocator_ptr, allocated_block));

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, marks_replaced_block_as_free_if_there_was_one)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    const uint32_t previous_block = 97;
    EXPECT_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).
						WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
									{
										*set_as_current_key_entry = true;
										*replaced_block = previous_block;
										return 0;
									});

	EXPECT_GLOBAL_CALL(kv_block_allocator__free_block, kv_block_allocator__free_block(_block_allocator_ptr, previous_block));

    // kv_directory__store_key is the last call that can fail in kv_set.
	// If if succeeds, kv_set will return 0.
	ASSERT_EQ(0, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, does_not_mark_replaced_block_as_free_if_there_was_not_one)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_directory__store_key, kv_directory__store_key(_directory_ptr, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_)).
						WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
									{
										*set_as_current_key_entry = true;
										*replaced_block = UINT32_MAX;	// There was no replaced block.
										return 0;
									});

	// Should not be called.
	EXPECT_GLOBAL_CALL(kv_block_allocator__free_block, kv_block_allocator__free_block(_block_allocator_ptr, testing::_)).
						Times(0);	// i.e. never

    // kv_directory__store_key is the last call that can fail in kv_set.
	// If if succeeds, kv_set will return 0.
	ASSERT_EQ(0, kv_set(store, &k, &v1.value));
}