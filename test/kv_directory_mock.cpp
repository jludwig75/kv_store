extern "C" {
#include "kv_directory.h"
}

#include "CppUTestExt/MockSupport.h"

struct kv_directory
{
	int dummy;
};

int kv_directory__init(struct kv_directory **directory)
{
	*directory = new kv_directory;

	return 0;
}

void kv_directory__cleanup(struct kv_directory **directory)
{
	delete *directory;
	*directory = NULL;
}

int kv_directory__store_key(struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
{
	return mock().actualCall("kv_directory__store_key").
		withUnsignedLongIntParameter("key", (unsigned long)key).
		withParameter("block", block).
		withUnsignedIntParameter("bytes", (unsigned)bytes).
		withUnsignedLongIntParameter("sequence", (unsigned long)sequence).
		returnIntValue();
}

int kv_directory__lookup_key(struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
{
	return mock().actualCall("kv_directory__lookup_key").
		withUnsignedLongIntParameter("key", (unsigned long)key).
		returnIntValue();
}

int kv_directory__remove_key(struct kv_directory *directory, uint64_t key)
{
	return mock().actualCall("kv_directory__remove_key").
		withUnsignedLongIntParameter("key", (unsigned long)key).
		returnIntValue();
}
