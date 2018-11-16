#pragma once

#include "kv_store_private.h"

int kv_write_key_block(struct kvstor *store, const struct key *k, size_t value_bytes, const char *value_buffer);
