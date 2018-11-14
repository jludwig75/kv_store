extern "C" {
#include "kv_block_array.h"
}

#include "CppUTestExt/MockSupport.h"

struct kv_block_array
{
	int dummy;
};

int kv_block_array__init(struct kv_block_array **block_array, size_t raw_block_bytes)
{
	*block_array = new kv_block_array;
	return 0;
}

void kv_block_array__cleanup(struct kv_block_array **block_array)
{
	delete *block_array;
	*block_array = NULL;
}

int kv_block_array__open(struct kv_block_array *block_array, const char *file_name, bool create)
{
	return mock().actualCall("kv_block_array__open").
			withStringParameter("file_name", file_name).
			withBoolParameter("create", create).
			returnIntValue();
}

void kv_block_array__close(struct kv_block_array *block_array)
{
	mock().actualCall("kv_block_array__close");
}

int kv_block_array__get_file_block_count(const struct kv_block_array *block_array, uint32_t *total_blocks)
{
	return mock().actualCall("kv_block_array__get_file_block_count").
					withOutputParameter("total_blocks", total_blocks).
					returnIntValue();
}

int kv_block_array__read_block(const struct kv_block_array *block_array, uint32_t block, uint8_t *block_data)
{
	return mock().actualCall("kv_block_array__read_block").
			withParameter("block", block).
			withPointerParameter("block_data", block_data).
			returnIntValue();
}

int kv_block_array__write_block(struct kv_block_array *block_array, uint32_t destination_block, const uint8_t *block_data)
{
	return mock().actualCall("kv_block_array__write_block").
			withParameter("destination_block", destination_block).
			withConstPointerParameter("block_data", block_data).
			returnIntValue();
}
