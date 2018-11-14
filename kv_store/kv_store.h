#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum {
    //! Maximum number of key-value pairs that can be stored
    //!     This may not always be true in a delete heavy workload if keys are not recycled.
    MAXKEYS = 100000,
    //! Maximum size of any key stored in the key-value store
    MAXBLOB = 4 * 1024,
};

struct kvstor;

struct key {
    uint64_t id;
};

struct value {
    //! The size of the value
    //! set by the caller to the size of the data portion of the value buffer when calling kv_set
    //! set by the caller to the size of the data portion of the value buffer when calling kv_get
    //!     to let the store now how big the read buffer is.
    size_t  size;
    // The actual data stored
    char    data[];
};

// @brief Opens a key value store
int    kv_open(struct kvstor **storpp, bool create,
    int argc, char **argv);

// @brief closes and frees a key value store
void    kv_close(struct kvstor *stor);

/// @brief Gets a value from the key value store
/// @param  stor    a kvstor instance
/// @param  k The key of the value to retrieve
/// @param  v   Receives the value stored in the store
///             v->size must be set to the size of the data portion of this buffer
///             The data will only be retrieved if this is >= the size of the value.
///             Otherwise -EINVAL will be returned.
/// @return 0 if the value was successfully retrieved
///         -ENOENT if the value is not stored
///         -EINVAL if the buffer is to small (size set in v->size)
///         Some other error < 0 if there was an error retrieving the value
int    kv_get(struct kvstor *stor,
    const struct key *k, struct value *v);

/// @brief Sets a value in the store
/// @param  stor    a kvstor instance
/// @param  k       The key to be stored
/// @param  v       The value to be stored for the key
/// @return 0 if the value was successfully stored
///         -ENOSPC if the log is full
///         Some other error < 0 if there was any other error storing the value.
int    kv_set(struct kvstor *stor,
    const struct key *k, const struct value *v);

/// @brief Deletes a value in the store
/// @param  stor    a kvstor instance
/// @param  k       The key to be deleted
/// @return 0 if the value was successfully stored
///         -ENOENT if the key was not stored in the log
///         -ENOSPC if the log is full (the log must have free space to be able to delete a key)
///         Some other error < 0 if there was any other error storing the value.
int    kv_del(struct kvstor *stor, const struct key *k);
