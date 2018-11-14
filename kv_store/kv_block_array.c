/*
 * BlockArray.cpp
 *
 *  Created on: Nov 5, 2018
 *      Author: jludwig
 */

#include "kv_block_array.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>


struct kv_block_array
{
    size_t raw_block_bytes;
    int fd;
};

int kv_block_array__init(struct kv_block_array **block_array, size_t raw_block_bytes)
{
    *block_array = (struct kv_block_array *)malloc(sizeof(struct kv_block_array));
    if (!*block_array)
    {
        return -ENOMEM;
    }

    (*block_array)->raw_block_bytes = raw_block_bytes;
    (*block_array)->fd = -1;

    return 0;
}

static void kv_block_array__close(struct kv_block_array *block_array)
{
    if (block_array->fd != -1)
    {
        close(block_array->fd);
        block_array->fd = -1;
    }
}

void kv_block_array__cleanup(struct kv_block_array **block_array)
{
    kv_block_array__close(*block_array);
    free(*block_array);
    *block_array = NULL;
}

int kv_block_array__open(struct kv_block_array *block_array, const char *file_name, bool create)
{
    block_array->fd = open(file_name, O_RDWR | (create ? (O_CREAT | O_TRUNC) : 0), S_IRWXU);
    if (block_array->fd == -1)
    {
        return errno;
    }
    return 0;
}


int kv_block_array__get_file_block_count(const struct kv_block_array *block_array, uint32_t *total_blocks)
{
    if (block_array->fd == -1)
    {
        return -EBADF;
    }

    off_t off = lseek(block_array->fd, 0, SEEK_END);

    if (off == (off_t)-1)
    {
        return errno;
    }

    *total_blocks = (uint32_t)(off / block_array->raw_block_bytes);
    return 0;
}


int kv_block_array__read_block(const struct kv_block_array *block_array, uint32_t block, uint8_t *block_data)
{
    if (block_array->fd == -1)
    {
        return -EBADF;
    }

    if (lseek(block_array->fd, block * block_array->raw_block_bytes, SEEK_SET) == -1)
    {
        return errno;
    }

    int ret = read(block_array->fd, block_data, block_array->raw_block_bytes);
    if (ret == -1)
    {
        return errno;
    }
    if ((size_t)ret != block_array->raw_block_bytes)
    {
        return -EIO;
    }

    return 0;
}

int kv_block_array__write_block(struct kv_block_array *block_array, uint32_t destination_block, const uint8_t *block_data)
{
    if (block_array->fd == -1)
    {
        return -EBADF;
    }

    if (lseek(block_array->fd, destination_block * block_array->raw_block_bytes, SEEK_SET) == -1)
    {
        return errno;
    }

    int ret = write(block_array->fd, block_data, block_array->raw_block_bytes);
    if (ret == -1)
    {
        return errno;
    }
    if ((size_t)ret != block_array->raw_block_bytes)
    {
        return -EIO;
    }

    return 0;
}
