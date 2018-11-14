#include "kv_store.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>

/* simple standalone test framework */
static unsigned int testnum;
static unsigned int testfail;

static int
__vtest(const char *file, int line, int test, const char *fmt, va_list args)
{
    int sz;
    va_list atmp;

    testnum += 1;
    printf("%sok %d", !test ? "not " : "", testnum);

    va_copy(atmp, args);
    sz = vsnprintf(NULL, 0, fmt ? fmt : "", atmp);
    if (sz != 0) {
        printf(" - ");
        printf(fmt ? fmt : "", atmp);
    }
    va_end(atmp);
    printf("\n");

    if (!test) {
        printf("#   Failed test ");
        va_copy(atmp, args);
        sz = vsnprintf(NULL, 0, fmt ? fmt : "", atmp);
        if (sz != 0) {
            printf("'");
            printf(fmt ? fmt : "", atmp);
            printf("'\n#   ");
        }
        va_end(atmp);
        printf("at %s line %d.\n", file, line);

        testfail += 1;
    }

    return test;
}

static int
__test(const char *file, int line, int test, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    __vtest(file, line, test, fmt, args);
    va_end(args);
    return test;
}

#define test(...)   __test(__FILE__, __LINE__, __VA_ARGS__, NULL)

struct value_buffer
{
    size_t  size;
    char    data[MAXBLOB];
};

int kv_set_get_cmp(struct kvstor *stor)
{
    struct key k = { 54689 };

    struct value_buffer input_buffer;
    struct value *input_value = (struct value *)&input_buffer;
    strcpy(input_value->data, "This is a test value.");
    input_value->size = strlen(input_value->data);

    int ret = kv_set(stor, &k, input_value);
    if (ret != 0)
    {
        return ret;
    }

    struct value_buffer output_value_buffer;
    struct value *output_value = (struct value *)&output_value_buffer;
    output_value->size = MAXBLOB;
    ret = kv_get(stor, &k, output_value);
    if (ret != 0)
    {
        return ret;
    }

    if (input_value->size != output_value->size || memcmp(input_value->data, output_value->data, input_value->size) != 0)
    {
        return -1;
    }

    return kv_del(stor, &k);
}

int kv_set__can_set_max_values(struct kvstor *stor)
{
    uint64_t i;
    for (i = 0; i < MAXKEYS; i++)
    {
        struct key k;
        k.id = i;

        struct value_buffer value_buffer;
        struct value *value = (struct value *)&value_buffer;
        memset(value->data, (int)i, MAXBLOB);
        value->size = MAXBLOB;

        int ret = kv_set(stor, &k, value);
        if (ret != 0)
        {
            return ret;
        }
    }

    for (i = 0; i < MAXKEYS; i++)
    {
        struct key k;
        k.id = i;

        int ret = kv_del(stor, &k);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

int kv_set__can_replace_max_values(struct kvstor *stor)
{
    uint64_t i;
    size_t r;
    const size_t test_repetitions = 5;
    for (r = 0; r < test_repetitions; r++)
    {
        int jrl = 43;
        (void)jrl;
        for (i = 0; i < MAXKEYS; i++)
        {
            struct key k;
            k.id = i;

            struct value_buffer value_buffer;
            struct value *value = (struct value *)&value_buffer;
            memset(value->data, (int)i, MAXBLOB);
            value->size = MAXBLOB;

            int ret = kv_set(stor, &k, value);
            if (ret != 0)
            {
                printf("Failed to set key %u on repetition %lu\n", (unsigned)i, r);
                return ret;
            }
        }
    }

    for (i = 0; i < MAXKEYS; i++)
    {
        struct key k;
        k.id = i;

        int ret = kv_del(stor, &k);
        if (ret != 0)
        {
            return ret;
        }
    }
    return 0;
}

int kv_store_can_persist(int argc, char **argv)
{
    struct kvstor *stor;

    int ret = kv_open(&stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t keys_to_store = 100;
    size_t i;

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = i;

        struct value_buffer value_buffer;
        struct value *value = (struct value *)&value_buffer;
        memset(value->data, (int)i, MAXBLOB);
        value->size = MAXBLOB;

        ret = kv_set(stor, &k, value);
        if (ret != 0)
        {
            return ret;
        }
    }

    kv_close(stor);

    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = i;

        struct value_buffer expected_value_buffer;
        struct value *expected_value = (struct value *)&expected_value_buffer;
        memset(expected_value->data, (int)i, MAXBLOB);
        expected_value->size = MAXBLOB;

        struct value_buffer actual_value_buffer;
        struct value *actual_value = (struct value *)&actual_value_buffer;
        actual_value->size = MAXBLOB;

        int ret = kv_get(stor, &k, actual_value);
        if (ret != 0)
        {
            return ret;
        }

        if (expected_value->size != actual_value->size || memcmp(expected_value->data, actual_value->data, expected_value->size) != 0)
        {
            return -1;
        }
    }

    kv_close(stor);

    return 0;
}

struct test_value_data
{
    uint64_t key;
    uint64_t version;
};

struct test_value_data_buffer
{
    size_t  size;
    struct test_value_data data;
};

int kv_store_retrieves_latest_persisted_keys_after_overwrites(int argc, char **argv)
{
    struct kvstor *stor;

    int ret = kv_open(&stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t keys_to_store = 100;
    const size_t versions_to_write = 100;
    size_t i;
    uint32_t v;

    for (v = 0; v < versions_to_write; v++)
    {
        for (i = 0; i < keys_to_store; i++)
        {
            struct key k;
            k.id = i;

            struct test_value_data_buffer data_to_store;
            data_to_store.size = sizeof(data_to_store.data);
            data_to_store.data.key = k.id;
            data_to_store.data.version = v;

            ret = kv_set(stor, &k, (struct value*)&data_to_store);
            if (ret != 0)
            {
                return ret;
            }
        }
    }

    kv_close(stor);

    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t expected_last_version = versions_to_write - 1;

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = i;


        struct test_value_data_buffer retrieved_value;

        int ret = kv_get(stor, &k, (struct value*)&retrieved_value);
        if (ret != 0)
        {
            fprintf(stderr, "Could not retrieve value for key\n");
            return ret;
        }

        if (retrieved_value.size != sizeof(struct test_value_data) || retrieved_value.data.key != k.id || retrieved_value.data.version != expected_last_version)
        {
            fprintf(stderr, "Found KV mismatch %"PRIu64" exepcted %"PRIu64" retrieved\n", expected_last_version, retrieved_value.data.version);
            return -1;
        }
    }

    kv_close(stor);

    return 0;
}

int kv_store_retrieves_latest_persisted_keys_after_deletes(int argc, char **argv)
{
    struct kvstor *stor;

    int ret = kv_open(&stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t keys_to_store = 100;
    const size_t versions_to_write = 100;
    size_t i;
    uint32_t v;

    for (v = 0; v < versions_to_write; v++)
    {
        for (i = 0; i < keys_to_store; i++)
        {
            struct key k;
            k.id = 0x1000000 | i;

            if (v > 0)
            {
                kv_del(stor, &k);
            }

            struct test_value_data_buffer data_to_store;
            data_to_store.size = sizeof(data_to_store.data);
            data_to_store.data.key = k.id;
            data_to_store.data.version = v;

            ret = kv_set(stor, &k, (struct value*)&data_to_store);
            if (ret != 0)
            {
                return ret;
            }
        }
    }

    kv_close(stor);

    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t expected_last_version = versions_to_write - 1;

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = 0x1000000 | i;


        struct test_value_data_buffer retrieved_value;

        int ret = kv_get(stor, &k, (struct value*)&retrieved_value);
        if (ret != 0)
        {
            fprintf(stderr, "Could not retrieve value for key\n");
            return ret;
        }

        if (retrieved_value.size != sizeof(struct test_value_data) || retrieved_value.data.key != k.id || retrieved_value.data.version != expected_last_version)
        {
            fprintf(stderr, "Found KV mismatch %"PRIu64" exepcted %"PRIu64" retrieved\n", expected_last_version, retrieved_value.data.version);
            return -1;
        }
    }

    kv_close(stor);

    return 0;
}

int kv_store_retrieves_latest_persisted_keys_twice(int argc, char **argv)
{
    struct kvstor *stor;

    int ret = kv_open(&stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    const size_t keys_to_store = 100;
    const size_t versions_to_write = 100;
    size_t i;
    uint32_t v;

    for (v = 0; v < versions_to_write; v++)
    {
        for (i = 0; i < keys_to_store; i++)
        {
            struct key k;
            k.id = i;

            struct test_value_data_buffer data_to_store;
            data_to_store.size = sizeof(data_to_store.data);
            data_to_store.data.key = k.id;
            data_to_store.data.version = v;

            ret = kv_set(stor, &k, (struct value*)&data_to_store);
            if (ret != 0)
            {
                return ret;
            }
        }
    }

    kv_close(stor);

    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    size_t expected_last_version = versions_to_write - 1;

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = i;


        struct test_value_data_buffer retrieved_value;

        int ret = kv_get(stor, &k, (struct value*)&retrieved_value);
        if (ret != 0)
        {
            fprintf(stderr, "Could not retrieve value for key\n");
            return ret;
        }

        if (retrieved_value.size != sizeof(struct test_value_data) || retrieved_value.data.key != k.id || retrieved_value.data.version != expected_last_version)
        {
            fprintf(stderr, "Found KV mismatch %"PRIu64" exepcted %"PRIu64" retrieved\n", expected_last_version, retrieved_value.data.version);
            return -1;
        }
    }

    const size_t new_version_start = 10000;

    for (v = new_version_start; v < new_version_start + versions_to_write; v++)
    {
        for (i = 0; i < keys_to_store; i++)
        {
            struct key k;
            k.id = i;

            struct test_value_data_buffer data_to_store;
            data_to_store.size = sizeof(data_to_store.data);
            data_to_store.data.key = k.id;
            data_to_store.data.version = v;

            ret = kv_set(stor, &k, (struct value*)&data_to_store);
            if (ret != 0)
            {
                return ret;
            }
        }
    }

    kv_close(stor);


    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    expected_last_version = new_version_start + versions_to_write - 1;

    for (i = 0; i < keys_to_store; i++)
    {
        struct key k;
        k.id = i;


        struct test_value_data_buffer retrieved_value;

        int ret = kv_get(stor, &k, (struct value*)&retrieved_value);
        if (ret != 0)
        {
            fprintf(stderr, "Could not retrieve value for key\n");
            return ret;
        }

        if (retrieved_value.size != sizeof(struct test_value_data) || retrieved_value.data.key != k.id || retrieved_value.data.version != expected_last_version)
        {
            fprintf(stderr, "Found KV mismatch %"PRIu64" exepcted %"PRIu64" retrieved\n", expected_last_version, retrieved_value.data.version);
            return -1;
        }
    }

    kv_close(stor);
    return 0;
}

int kv_store_cannot_get_deleted_values_after_replay(int argc, char **argv)
{
    struct key k;
    k.id = 43;

    struct kvstor *stor;

    int ret = kv_open(&stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    struct test_value_data_buffer data_to_store;
    data_to_store.size = sizeof(data_to_store.data);
    data_to_store.data.key = k.id;
    data_to_store.data.version = 197;

    ret = kv_set(stor, &k, (struct value*)&data_to_store);
    if (ret != 0)
    {
        return ret;
    }

    ret = kv_del(stor, &k);
    if (ret != 0)
    {
        return ret;
    }

    kv_close(stor);


    ret = kv_open(&stor, false, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    struct test_value_data_buffer retrieved_value;

    ret = kv_get(stor, &k, (struct value*)&retrieved_value);
    if (ret != 0 && ret != -ENOENT)
    {
        fprintf(stderr, "Unexpected error trying to retrieve value for key\n");
        return ret;
    }
    if (ret == 0)
    {
        fprintf(stderr, "Deleted key was found.\n");
        return -1;
    }

    kv_close(stor);

    return 0;
}

int
main(int argc, char **argv)
{
    struct kvstor *stor;

    /* run the test vectors */
    test(kv_open(&stor, true, argc, argv) == 0, "kv_open");

    struct key k = { 123 };

    struct value_buffer input_buffer;
    struct value *input_value = (struct value *)&input_buffer;
    strcpy(input_value->data, "This is a test value.");
    input_value->size = strlen(input_value->data);

    test(kv_set(stor, &k, input_value) == 0, "kv_set");

    struct value_buffer output_value_buffer;
    struct value *output_value = (struct value *)&output_value_buffer;
    output_value->size = MAXBLOB;
    test(kv_get(stor, &k, output_value) == 0, "kv_get");

    test(kv_del(stor, &k) == 0, "kv_del");

    test(kv_set_get_cmp(stor) == 0, "kv_set_get_cmp");

    test(kv_set__can_set_max_values(stor) == 0, "kv_set__can_set_max_values");

    test(kv_set__can_replace_max_values(stor) == 0, "kv_set__can_replace_max_values");

    kv_close(stor);

    test(kv_store_can_persist(argc, argv) == 0, "kv_store_can_persist");

    test(kv_store_retrieves_latest_persisted_keys_after_overwrites(argc, argv) == 0, "kv_store_retrieves_latest_persisted_keys_after_overwrites");

    test(kv_store_retrieves_latest_persisted_keys_after_deletes(argc, argv) == 0, "kv_store_retrieves_latest_persisted_keys_after_deletes");

    test(kv_store_retrieves_latest_persisted_keys_twice(argc, argv) == 0, "kv_store_retrieves_latest_persisted_keys_twice");

    test(kv_store_cannot_get_deleted_values_after_replay(argc, argv) == 0, "kv_store_cannot_get_deleted_values_after_replay");

    printf("%u failed out of %u run\n", testfail, testnum);
    return 0;
}
