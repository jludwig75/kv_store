/*
 * BlockArray.h
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */
/// @file   kv_block_array.h
/// @brief  A block array instance abstracts a file as an array of fixe-sized blocks.
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct kv_block_array;


/// @brief Initializes a block array instnace
/// @param  block_array Receives the pointer to the newly created instnace
/// @param  The maximum number of blocks to track.
/// @return 0 if the block array instance was successfully created.
///         an error (< 0) if there was an error creating the block array.
int kv_block_array__init(struct kv_block_array **block_array, size_t raw_block_bytes);

/// @brief Destroys a block array instance
/// @param  block_array Pointer to pointer of block array instance.
///                         The pointer will be set to NULL when the function returns
void kv_block_array__cleanup(struct kv_block_array **block_array);


/// @brief  Opens a block array file.
/// @param  block_array A block array instance
/// @param  file_name   The file name of the block array file to open
/// @param  create      If true a new zero-length file will be created overwritting any existing file.
///                     If false an existing file will be opened. It the file does not exist, an error will be returned.
/// @return 0 if the file was successfully opened, non-zero ( < 0) on error
int kv_block_array__open(struct kv_block_array *block_array, const char *file_name, bool create);

/// @brief  Gets the numer of blocks currently stored in the file.
/// @param  block_array A block array instance
/// @param  total_blocks    Receives the total number of blocks currently stored in the file.
/// @return 0 on success, non-zero ( < 0) on error
int kv_block_array__get_file_block_count(const struct kv_block_array *block_array, uint32_t *total_blocks);

/// @brief  Reads a block from the block array
/// @param  block_array A block array instance
/// @param  block
/// @param  block_data
/// @return 0 if the block was successfully read, non-zero ( < 0) on error
int kv_block_array__read_block(const struct kv_block_array *block_array, uint32_t block, uint8_t *block_data);

/// @brief  Writes a block to the block array
/// @param  block_array A block array instance
/// @param  block
/// @param  block_data
/// @return 0 if the block was successfully written, non-zero ( < 0) on error
int kv_block_array__write_block(struct kv_block_array *block_array, uint32_t destination_block, const uint8_t *block_data);
