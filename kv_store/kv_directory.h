/*
 * Directory.h
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */
/// @file   kv_directory.h
/// @brief  A directory as an in-memory structure that tracks all valid blocks stored in a log.
///         Valid means the latest version for the key in the block header. This includes all
///         entries used to mark keys as deleted. All previous versions of the key (lower sequence
///         number) are not tracked in the directory.
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct kv_directory;


/// @brief Initializes a directory instnace
/// @param  block_array Receives the pointer to the newly created instnace
/// @param  The maximum number of blocks to track.
/// @return 0 if the directory instance was successfully created.
///         an error (< 0) if there was an error creating the directory.
int kv_directory__init(struct kv_directory **directory);

/// @brief Destroys a directory instance
/// @param  block_array Pointer to pointer of directory instance.
///                         The pointer will be set to NULL when the function returns
void kv_directory__cleanup(struct kv_directory **directory);


/// @brief  Stores the block, size and sequence for a key if the sequence of the key
///         is greater than the currently stored sequence for the key or if the key is
///         not currently stored.
/// @param  directory   A directory instance
/// @param  key         The key tio store
/// @param  block       The block where the value for this key is stored
/// @param  bytes       The size of the value in bytes
/// @param  sequence    The sequence of this version of the key. At runtime, this should
///                     be guaranteed to be greater than any value for any key previously
///                     stored in the directory.
/// @param  set_as_current_key_entry    Set to true if this key is new or has a higher
///                                     sequence than the version of the key currently
///                                     stored. If this set to false, the key data in the
///                                     the directory was not changed and the new data was
///                                     ignored. For runtime operations, this should always
///                                     be true or there is a bug with the sequence number.
///                                     At replay only one version of the key (the one with
///                                     the highest sequence) will be stored. All others will
///                                     be discarded.
/// @param  replaced_block              If this version of the key replaces another, this is
///                                     set to the block of the old version so it can be marked
///                                     as free. If there was no prior version it will be set to
///                                     UINT32_MAX and no block needs to be marked as free
/// @return 0 if the key was successfully stored or ignored, because it was older than the current version stored.
///         non-zero ( < 0) on error
int kv_directory__store_key(struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block);

/// @brief Looks up a key in the directory.
/// @param  directory   A directory instance
/// @param  key         The key to look up
/// @param  block       Receives the block of the value for the latest instance of key,
///                     if it is stored in the directory.
/// @param  bytes       Receives the size in bytes of the value for the latest instance of key,
///                     if it is stored in the directory.
/// @return 0 if the key was found in the directory
///         -ENOENT if the key was not found
///         non-zero ( < 0) if there was an error looking for the key
int kv_directory__lookup_key(struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes);

/// @brief Removes a key from the directory if it is stored.
/// @param  directory   A directory instance
/// @param  key         The key to remove
/// @return 0 if the key was found in the directory and deleted
///         -ENOENT if the key was not found
///         non-zero ( < 0) if there was an error looking for the key
int kv_directory__remove_key(struct kv_directory *directory, uint64_t key);
