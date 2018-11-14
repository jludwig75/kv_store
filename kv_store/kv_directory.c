/*
 * Directory.cpp
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */
#include "kv_directory.h"

#include <errno.h>
#include <assert.h>
#include <search.h>
#include <stdio.h>
#include <stdbool.h>


struct directory_entry
{
    uint64_t key_id;
    uint64_t sequence;
    uint32_t data_block;
    uint16_t data_bytes;
};

void directory_entry__init(struct directory_entry *entry, uint64_t key_id, uint64_t sequence, uint32_t data_block, uint16_t data_bytes)
{
    entry->key_id = key_id;
    entry->sequence = sequence;
    entry->data_block = data_block;
    entry->data_bytes = data_bytes;
}

bool directory_entry__is_allocated(const struct directory_entry *entry)
{
    return entry->data_bytes > 0;
}


struct kv_directory
{
    // The root of the directory entry tree
    // directory uses the Linux balanced search
    // tree in search.h
    void *entries_root;
};

static int dir_entry_compare(const void *pa, const void *pb)
{
    const struct directory_entry *entry_a = (const struct directory_entry *)pa;
    const struct directory_entry *entry_b = (const struct directory_entry *)pb;

    if (entry_a->key_id < entry_b->key_id)
    {
        return -1;
    }
    if (entry_a->key_id > entry_b->key_id)
    {
        return 1;
    }
    return 0;
}

static void dir_entry_free(void *node)
{
    struct directory_entry *entry = (struct directory_entry *)node;
    free(entry);
}

int kv_directory__init(struct kv_directory **directory)
{
    *directory = (struct kv_directory *)malloc(sizeof(struct kv_directory));
    if (!*directory)
    {
        return -ENOMEM;
    }

    (*directory)->entries_root = NULL;

    return 0;
}

void kv_directory__cleanup(struct kv_directory **directory)
{
    tdestroy((*directory)->entries_root, dir_entry_free);
    free(*directory);
    *directory = NULL;
}

static struct directory_entry *kv_directory__find_entry_for_key(struct kv_directory *directory, uint64_t key)
{
    struct directory_entry entry_key;
    directory_entry__init(&entry_key, key, 0, 0, 0);

    struct directory_entry **node = (struct directory_entry **)tfind(&entry_key, &directory->entries_root, dir_entry_compare);
    if (!node)
    {
        return NULL;
    }

    return *node;
}

int kv_directory__store_key(struct kv_directory *directory, uint64_t key, uint32_t block, size_t bytes, uint64_t sequence, bool *set_as_current_key_entry, uint32_t *replaced_block)
{
    *set_as_current_key_entry = false;
    *replaced_block = UINT32_MAX;

    struct directory_entry *entry = kv_directory__find_entry_for_key(directory, key);
    if (entry)
    {
        // The key is already stored in the directory.
        // We don't need to add it.

        // This should never happen. A sequence number should never be used more than once.
        assert(sequence != entry->sequence);
        
        // Only update the stored key if the new sequence
        // is greater than the sequence of the version stored.
        if (sequence > entry->sequence)
        {
            // Replace the key.
            *replaced_block = entry->data_block;
            entry->sequence = sequence;
            entry->data_block = block;
            entry->data_bytes = (uint16_t)bytes;
            *set_as_current_key_entry = true;
        }
        // Drop out now.
        return 0;
    }

    // The key is not yet stored. Add it now.
    struct directory_entry *new_entry = (struct directory_entry *)malloc(sizeof(struct directory_entry));
    directory_entry__init(new_entry, key, sequence, block, (uint16_t)bytes);
    *set_as_current_key_entry = true;

    struct directory_entry **new_node = (struct directory_entry **)tsearch(new_entry, &directory->entries_root, dir_entry_compare);
    if (!new_node)
    {
        return -ENOMEM;
    }
    if (*new_node != new_entry)
    {
        // Our allocated entry was not stored.
        // Delete it here.
        free(new_entry);
    }
    return 0;
}

int kv_directory__lookup_key(struct kv_directory *directory, uint64_t key, uint32_t *block, size_t *bytes)
{
    struct directory_entry *entry = kv_directory__find_entry_for_key(directory, key);
    if (!entry)
    {
        return -ENOENT;
    }

    *block = entry->data_block;
    *bytes = entry->data_bytes;
    return 0;
}

int kv_directory__remove_key(struct kv_directory *directory, uint64_t key)
{
    struct directory_entry *entry = kv_directory__find_entry_for_key(directory, key);
    if (!entry || !directory_entry__is_allocated(entry))
    {
        return -ENOENT;
    }

    if (!tdelete(entry, &directory->entries_root, dir_entry_compare))
    {
        return -EFAULT;
    }
    free(entry);
    return 0;
}
