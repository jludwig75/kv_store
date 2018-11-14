/*
 * AppendPoint.h
 *
 *  Created on: Nov 9, 2018
 *      Author: jludwig
 */
/// @file kv_append_point.h
/// @brief  The append point tracks a the current address for writing into a log.
///         It works with a kv_block_allocator instance to advance to the next free
///         block when it is incremented. It will wrap back to the beginning of the
///         log when it reaches the end.
#pragma once

#include <stdint.h>

struct kv_block_allocator;

struct kv_append_point;

/// @brief Initializes and append point instance
/// @param  append_point    Receives a pointer to the newly created append point instance.
/// @param  block_allocator block allocator instance to use for the append point.
/// @return 0 if the append point instance was successfully created.
///         an error (< 0) if there was an error creating the append point.
int kv_append_point__init(struct kv_append_point **append_point, struct kv_block_allocator *block_allocator);

/// @brief Destroys an append point instance.
/// @param  append_point    Pointer to pointer of append point instance.
///                         The pointer will be set to NULL when the function returns
void kv_append_point__cleanup(struct kv_append_point **append_point);

/// @brief Gets the append point or next block to write to in the log
/// @param  append_point    The append point instance
/// @return On success the next block to write to in the log. This block will be free.
///         On failure UINT32_MAX. This means the block allocator is full.
uint32_t kv_append_point__get_append_point(struct kv_append_point *append_point);

/// @brief Updates the append point to the latest block of a
///        block being read while replyaing the log
///        This function is only called as part of a log replay
/// @param  append_point    An append point instance.
/// @param  block           The valid block that was found while replaying the log.
/// @param  sequence        The sequence of the valid block being replayed
void kv_append_point__update_append_point(struct kv_append_point *append_point, uint32_t block, uint64_t sequence);
