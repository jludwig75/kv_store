#include <gtest/gtest.h>

extern "C" {
#include "kv_directory.h"
}


#include <errno.h>
#include <stdint.h>


TEST(kv_directory, store_key__adds_new_keys)
{
	uint64_t sequence = 0;
	bool set_as_current_key_entry;
	uint32_t replaced_block;
	
	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 0, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 10, 20, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 20, 21, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 30, 22, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 40, 23, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, store_key__adds_updated_keys)
{
	uint64_t sequence = 0;
	bool set_as_current_key_entry;
	uint32_t replaced_block;

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	ASSERT_TRUE(0 == kv_directory__store_key(dir,  0, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 10, 20, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 0, 21, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(19, replaced_block);

	ASSERT_TRUE(0 == kv_directory__store_key(dir, 10, 22, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(20, replaced_block);

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, store_key__retains_old_value_when_new_value_with_older_sequence_added)
{
	bool set_as_current_key_entry;
	uint32_t replaced_block;

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	uint64_t key = 234534;

	uint64_t first_added_sequence = 5;
	uint32_t first_added_block = 19;
	size_t first_added_bytes = 159;
	ASSERT_TRUE(0 == kv_directory__store_key(dir, key, first_added_block, first_added_bytes, first_added_sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(set_as_current_key_entry);
	ASSERT_EQ(UINT32_MAX, replaced_block);

	uint64_t second_added_sequence = 3;	// Lower sequence than first add.
	uint32_t second_added_block = 14;
	size_t second_added_bytes = 357;
	ASSERT_TRUE(0 == kv_directory__store_key(dir, key, second_added_block, second_added_bytes, second_added_sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_FALSE(set_as_current_key_entry);

	uint32_t block;
	size_t bytes;
	ASSERT_TRUE(0 == kv_directory__lookup_key(dir, key, &block, &bytes));
	ASSERT_EQ(first_added_block, block);
	ASSERT_EQ(first_added_bytes, bytes);

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, stored_keys_can_be_retrieved_with_correct_values)
{
	uint64_t sequence = 0;
	bool set_as_current_key_entry;
	uint32_t replaced_block;
	const size_t test_max_dir_entries = 5;

	uint32_t dir_entry_blocks[] = {19, 111, 219, 319, 419};
	size_t dir_entry_byte_counts[] = {100, 111, 222, 333, 444};

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	for(size_t i = 0; i < test_max_dir_entries; i++)
	{
		uint64_t key = i * 10;
		ASSERT_TRUE(0 == kv_directory__store_key(dir, key, dir_entry_blocks[i], dir_entry_byte_counts[i], ++sequence, &set_as_current_key_entry, &replaced_block));
	}

	for(uint64_t i = 0; i < test_max_dir_entries; i++)
	{
		uint32_t block;
		size_t bytes;
		uint64_t key = i * 10;
		ASSERT_EQ(0, kv_directory__lookup_key(dir, key, &block, &bytes));
		ASSERT_EQ(dir_entry_blocks[i], block);
		ASSERT_EQ(dir_entry_byte_counts[i], bytes);
	}

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, returns_ENOENT_for_key_not_stored_when_directory_is_empty)
{

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	uint32_t block;
	size_t bytes;
	ASSERT_EQ(-ENOENT, kv_directory__lookup_key(dir, 99, &block, &bytes));

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, returns_ENOENT_for_key_not_stored_when_directory_is_full)
{
	uint64_t sequence = 0;
	bool set_as_current_key_entry;
	uint32_t replaced_block;

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	ASSERT_TRUE(0 == kv_directory__store_key(dir,  0, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(0 == kv_directory__store_key(dir, 10, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(0 == kv_directory__store_key(dir, 20, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(0 == kv_directory__store_key(dir, 30, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));
	ASSERT_TRUE(0 == kv_directory__store_key(dir, 40, 19, 334, ++sequence, &set_as_current_key_entry, &replaced_block));

	uint32_t block;
	size_t bytes;
	ASSERT_EQ(-ENOENT, kv_directory__lookup_key(dir, 99, &block, &bytes));

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, cannot_find_key_after_it_is_removed)
{
	uint64_t sequence = 0;
	bool set_as_current_key_entry;
	uint32_t replaced_block;

	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	const uint64_t test_key = 20345;
	const uint32_t test_block = 15439;
	const size_t test_bytes = 43;
	// Store the key
	ASSERT_TRUE(0 == kv_directory__store_key(dir, test_key, test_block, test_bytes, ++sequence, &set_as_current_key_entry, &replaced_block));

	uint32_t block;
	size_t bytes;
	// We can look it up
	ASSERT_EQ(0, kv_directory__lookup_key(dir, test_key, &block, &bytes));
	ASSERT_EQ(test_block, block);
	ASSERT_EQ(test_bytes, bytes);

	// Remove the key from the dictionary
	ASSERT_EQ(0, kv_directory__remove_key(dir, test_key));

	// Now we can't look it up.
	ASSERT_EQ(-ENOENT, kv_directory__lookup_key(dir, test_key, &block, &bytes));

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, lookup_key__returns_ENOENT_if_key_not_stored)
{
	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	uint32_t block;
	size_t bytes;
	ASSERT_EQ(-ENOENT, kv_directory__lookup_key(dir, 546, &block, &bytes));

	kv_directory__cleanup(&dir);
}

TEST(kv_directory, remove_key__returns_ENOENT_if_key_not_stored)
{
	kv_directory *dir;
	ASSERT_EQ(0, kv_directory__init(&dir));

	ASSERT_EQ(-ENOENT, kv_directory__remove_key(dir, 546));

	kv_directory__cleanup(&dir);
}
