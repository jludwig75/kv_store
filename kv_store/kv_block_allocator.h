/*
 * BlockAllocator.h
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */
/// @file   kv_block_allocator.h
/// @brief  A block allocator instance is a bitmap used to track allocate blocks in a log.
#pragma once

#include <stdint.h>
#include <stdlib.h>


struct kv_block_allocator;

/// @brief Initializes a block allocator instnace
/// @param  block_allocator Receives the pointer to the newly created instnace
/// @param  The maximum number of blocks to track.
/// @return 0 if the block allocator instance was successfully created.
///         an error (< 0) if there was an error creating the block allocator.
int kv_block_allocator__init(struct kv_block_allocator **block_allocator, uint32_t number_of_blocks);

/// @brief Destroys a block allocator instance
/// @param  block_allocator Pointer to pointer of block allocator instance.
///                         The pointer will be set to NULL when the function returns
void kv_block_allocator__cleanup(struct kv_block_allocator **block_allocator);


/// @brief Finds the next free block in the bitmap
/// @param  block_allocator A block allocator instance
/// @param  starting_block  The block at which to start searching for a free block.
///                         This allows the append point to always move forward in the log
///                         until it reaches the end.
///                         If there are no more free blocks from starting_block until the
///                         end of the log the search will restart at the beginning of the log
///                         up to the block right before starting_block.
/// @return The next free block starting at starting_block.
///         If not blocks are free it returns UINT32_MAX
uint32_t kv_block_allocator__find_next_free_block(const struct kv_block_allocator *block_allocator, uint32_t starting_block);

/// @brief Marks a block as free
/// @param  block_allocator A block allocator instance
/// @param  block   The block to mark free
void kv_block_allocator__free_block(struct kv_block_allocator *block_allocator, uint32_t block);

/// @brief Marks a block as allocated
/// @param  block_allocator A block allocator instance
/// @param  block   The block to mark allocated
void kv_block_allocator__mark_block_as_allocated(struct kv_block_allocator *block_allocator, uint32_t block);
