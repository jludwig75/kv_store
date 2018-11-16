#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock-global/gmock-global.h>


extern "C" {
#include "kv_store.h"
#include "kv_store_replay.h"
#include "kv_store_write_block.h"
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


// kv_store (kv_store_replay)
MOCK_GLOBAL_FUNC1(kv_store__replay_log, int(struct kvstor *store));

// kv_store (kv_store_write_block)
MOCK_GLOBAL_FUNC4(kv_write_key_block, int(struct kvstor *store, const struct key *k, size_t value_bytes, const char *value_buffer));

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

class kv_op_Test : public GlobalMockTest
{
    virtual void SetUp()
    {
        store = &store_instance;
        store->block_array = _block_array_ptr = (struct kv_block_array *)0x345346;
        store->block_allocator = _block_allocator_ptr = (struct kv_block_allocator *)0x24534543;
        store->directory = _directory_ptr = (struct kv_directory *)0x534567;
        store->append_point = _append_point_ptr = (struct kv_append_point *)0x42356;
        store->current_sequence_number = 0;
	    testSetUp();
    }
    virtual void testSetUp() = 0;
    virtual void TearDown()
    {
    }
protected:    
    struct kvstor *store;
    struct kvstor store_instance;
    struct kv_block_array *_block_array_ptr;
    struct kv_block_allocator *_block_allocator_ptr;
    struct kv_directory *_directory_ptr;
    struct kv_append_point *_append_point_ptr;
};

class kv_set_Test : public kv_op_Test
{
protected:
    void testSetUp()
    {
		// Setup mocks to default behavior for all tests:
        ON_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                        WillByDefault(testing::Return(0));
    }
};

TEST_F(kv_set_Test, returns_EINVAL_if_value_to_large)
{
    struct test_value v1;
    v1.size = MAXBLOB + 1;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                        Times(0);

    ASSERT_EQ(-EINVAL, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, returns_EINVAL_if_value_size_is_0)
{
    struct test_value v1;
    v1.size = 0;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                        Times(0);

    ASSERT_EQ(-EINVAL, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, calls_kv_write_key_block_if_value_size_is_1)
{
    struct test_value v1;
    v1.size = 1;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, &k, v1.value.size, v1.value.data)).
                        WillOnce(testing::Return(0));

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, calls_kv_write_key_block_if_value_size_is_MAXBLOB)
{
    struct test_value v1;
    v1.size = MAXBLOB;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, &k, v1.value.size, v1.value.data)).
                        WillOnce(testing::Return(0));

    kv_set(store, &k, &v1.value);
}

TEST_F(kv_set_Test, calls_kv_write_key_block)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, &k, v1.value.size, v1.value.data)).
                        WillOnce(testing::Return(0));

    ASSERT_EQ(0, kv_set(store, &k, &v1.value));
}

TEST_F(kv_set_Test, fails_if_kv_write_key_block_fails)
{
    struct test_value v1;
    v1.size = 4;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, &k, v1.value.size, v1.value.data)).
                        WillOnce(testing::Return(-EIO));

    ASSERT_EQ(-EIO, kv_set(store, &k, &v1.value));
}

class kv_get_Test : public kv_op_Test
{
protected:
    void testSetUp()
    {
		// Setup mocks to default behavior for all tests:
        ON_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                        WillByDefault([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                        {
                                            *block = 77;
                                            *bytes = 89;
                                            return 0;
                                        });
        ON_GLOBAL_CALL(kv_block__init_empty, kv_block__init_empty(testing::_)).
                        WillByDefault([](struct kv_block *bv_block){});
        ON_GLOBAL_CALL(kv_block_array__read_block, kv_block_array__read_block(testing::_, testing::_, testing::_)).
                        WillByDefault(testing::Return(0));
        ON_GLOBAL_CALL(kv_block__validate, kv_block__validate(testing::_)).WillByDefault(testing::Return(true));
    }
};

TEST_F(kv_get_Test, looks_up_key_in_directory)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = 77;
                                        *bytes = 89;
                                        return 0;
                                    });

    kv_get(store, &k, &v1.value);
}

TEST_F(kv_get_Test, fails_if_key_not_found_in_directory)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce(testing::Return(-EFAULT));

    ASSERT_EQ(-EFAULT, kv_get(store, &k, &v1.value));
}

TEST_F(kv_get_Test, returns_ENOENT_if_key_found_but_is_deleted)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = 77;
                                        *bytes = 0;
                                        return 0;
                                    });

    ASSERT_EQ(-ENOENT, kv_get(store, &k, &v1.value));
}

TEST_F(kv_get_Test, returns_EINVAL_if_value_buffer_is_too_small)
{
    struct test_value v1;
    v1.size = 1;

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = 77;
                                        *bytes = 97;
                                        return 0;
                                    });

    ASSERT_EQ(-EINVAL, kv_get(store, &k, &v1.value));
}

TEST_F(kv_get_Test, initializes_en_empty_kv_block)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_block__init_empty, kv_block__init_empty(testing::_)).
                    Times(1);

    kv_get(store, &k, &v1.value);
}

TEST_F(kv_get_Test, reads_from_block_returned_by_directroy)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    const uint32_t value_block = 113;
    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = value_block;
                                        *bytes = 97;
                                        return 0;
                                    });

    EXPECT_GLOBAL_CALL(kv_block_array__read_block, kv_block_array__read_block(_block_array_ptr, value_block, testing::_)).
                    WillOnce(testing::Return(0));

    kv_get(store, &k, &v1.value);
}

TEST_F(kv_get_Test, fails_if_block_read_fails)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_block_array__read_block, kv_block_array__read_block(_block_array_ptr, testing::_, testing::_)).
                    WillOnce(testing::Return(-EIO));

    ASSERT_EQ(-EIO, kv_get(store, &k, &v1.value));
}

TEST_F(kv_get_Test, will_not_validate_block_if_read_fails)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_block_array__read_block, kv_block_array__read_block(_block_array_ptr, testing::_, testing::_)).
                    WillOnce(testing::Return(-EIO));

    // We should not validate the block if we could not read it.
    EXPECT_GLOBAL_CALL(kv_block__validate, kv_block__validate(testing::_)).
                    Times(0);   // i.e. never

    kv_get(store, &k, &v1.value);
}

TEST_F(kv_get_Test, validates_block_if_read_succeeds)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_block_array__read_block, kv_block_array__read_block(_block_array_ptr, testing::_, testing::_)).
                    WillOnce(testing::Return(0));

    EXPECT_GLOBAL_CALL(kv_block__validate, kv_block__validate(testing::_)).
                    WillOnce(testing::Return(true));

    kv_get(store, &k, &v1.value);
}

TEST_F(kv_get_Test, returns_EFAULT_if_block_fails_to_validate)
{
    struct test_value v1;
    v1.size = sizeof(v1.data);

    struct key k;
    k.id = 97;

    EXPECT_GLOBAL_CALL(kv_block__validate, kv_block__validate(testing::_)).
                    WillOnce(testing::Return(false));

    ASSERT_EQ(-EFAULT, kv_get(store, &k, &v1.value));
}

class kv_del_Test : public kv_op_Test
{
protected:
    void testSetUp()
    {
		// Setup mocks to default behavior for all tests:
        ON_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                        WillByDefault([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                        {
                                            *block = 77;
                                            *bytes = 89;
                                            return 0;
                                        });
        ON_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                        WillByDefault(testing::Return(0));
    }
};

TEST_F(kv_del_Test, calls_kv_directory_lookup_key)
{
    struct key k;
    k.id = 917;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = 77;
                                        *bytes = 89;
                                        return 0;
                                    });

    kv_del(store, &k);
}

TEST_F(kv_del_Test, fails_if_kv_directory_lookup_key_fails)
{
    struct key k;
    k.id = 917;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce(testing::Return(-EFAULT));

    ASSERT_EQ(-EFAULT, kv_del(store, &k));
}

// 0 bytes is a delete entry. We must fail, because the entry does not actually exist;
// it has already been deleted. (The last operation on that key was to delete it from the store)
TEST_F(kv_del_Test, returns_ENOENT_if_kv_directory_lookup_key_finds_value_of_0_byte_size)
{
    struct key k;
    k.id = 917;

    EXPECT_GLOBAL_CALL(kv_directory__lookup_key, kv_directory__lookup_key(_directory_ptr, testing::_, testing::_, testing::_)).
                    WillOnce([](struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
                                    {
                                        *block = 77;
                                        *bytes = 0;
                                        return 0;
                                    });

    ASSERT_EQ(-ENOENT, kv_del(store, &k));
}

TEST_F(kv_del_Test, calls_kv_write_key_block)
{
    struct key k;
    k.id = 917;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                    WillOnce(testing::Return(0));

    // kv_write_key_block is the last call that can fail in kv_del.
    // If kv_write_key_block returns 0, kv_del should as well.
    ASSERT_EQ(0, kv_del(store, &k));
}

TEST_F(kv_del_Test, fails_if_kv_write_key_block_fails)
{
    struct key k;
    k.id = 917;

    EXPECT_GLOBAL_CALL(kv_write_key_block, kv_write_key_block(store, testing::_, testing::_, testing::_)).
                    WillOnce(testing::Return(-ENOMEM));

    ASSERT_EQ(-ENOMEM, kv_del(store, &k));
}