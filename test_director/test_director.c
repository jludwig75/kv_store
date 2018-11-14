#include "test_director.h"

#include "kv_store.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


struct test_director
{
    struct kvstor *stor;
    struct kv_checker *checker;
    size_t number_of_operations_run;
    size_t number_of_failed_operations;
    size_t number_of_miscompares;
    size_t number_of_unexpected_missing_keys;
};

int test_director__init(struct test_director **director, unsigned random_seed, struct kv_checker *checker, int argc, char **argv)
{
    *director = (struct test_director *)malloc(sizeof(struct test_director));
    if (!director)
    {
        return -ENOMEM;
    }

    (*director)->checker = checker;
    (*director)->number_of_operations_run = 0;
    (*director)->number_of_failed_operations = 0;
    (*director)->number_of_miscompares = 0;
    (*director)->number_of_unexpected_missing_keys = 0;

    int ret = kv_open(&(*director)->stor, true, argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    srand(random_seed);

    return 0;
}

void test_director__cleanup(struct test_director **director)
{
    kv_close((*director)->stor);
    free(*director);
    *director = NULL;
}

size_t test_director__get_number_of_operations_run(const struct test_director *director)
{
    return director->number_of_operations_run;
}

size_t test_director__get_number_of_failed_operations(const struct test_director *director)
{
    return director->number_of_failed_operations;
}

size_t test_director__select_data_size()
{
    return (rand() % MAXBLOB) + 1;
}

void test_director__fill_random_data(struct value *v)
{
    size_t i;

    for(i = 0; i < v->size; i++)
    {
        v->data[i] = (char)(rand() % UINT8_MAX);
    }
}

void test_director__generate_random_value(struct value *v)
{
    v->size = test_director__select_data_size();
    test_director__fill_random_data(v);
}

bool test_director__compare_values(struct value *a, struct value *b)
{
    // First check the size. We can't memcmp
    // the whole structure until we know they are
    // the same size. We might overrun one of the buffers.
    if (a->size != b->size)
    {
        return false;
    }

    return memcmp(a->data, b->data, a->size) == 0;
}

//uint64_t hash_value(const struct value *v)
//{
//    uint64_t chk_sum = 0;
//
//    size_t bytes_processed = 0;
//    while (bytes_processed < v->size)
//    {
//        size_t bytes_to_process = min(sizeof(uint64_t), v->size - bytes_processed);
//        uint64_t t = 0;
//        memcpy(&t, &v->data[bytes_processed], bytes_to_process);
//        chk_sum += t;
//        bytes_processed += bytes_to_process;
//    }
//
//    return chk_sum;
//}

// We want a small range to force overlaps
const size_t max_unique_keys = 5;
uint64_t test_director__select_key()
{
    return rand() % max_unique_keys;
}


struct test_value
{
    union {
        struct
        {
            size_t size;
            char data[MAXBLOB];
        };
        struct value value;
    };
};

// These callbacks only return non-zero if there
// was a failure fatal to running the test suite.
typedef int (*test_director__op)(struct test_director *director);

int test_director__do_set(struct test_director *director)
{
    struct key key;
    key.id = test_director__select_key();
    struct test_value value;

    test_director__generate_random_value(&value.value);

    //printf("set(%llu, %u, %llu)\n", key.id, value.size, hash_value(&value.value));

    int ret = kv_set(director->stor, &key, &value.value);
    if (ret != 0 && ret != -ENOSPC)
    {
        fprintf(stderr, "kv_set failed.\n");
        director->number_of_failed_operations++;
        return 0;
    }

    return kv_checker__store_value(director->checker, &key, &value.value);
}

int test_director__do_get(struct test_director *director)
{
    struct key key;
    key.id = test_director__select_key();
    struct test_value value;
    value.value.size = sizeof(value.data);

    //printf("get(%llu) -> ", key.id);

    int ret = kv_get(director->stor, &key, &value.value);
    if (ret != 0 && ret != -ENOENT)
    {
        fprintf(stderr, "kv_get failed.\n");
        director->number_of_failed_operations++;
        return 0;
    }

    //if (ret == -ENOENT)
    //{
    //    printf("ENOENT\n");
    //}
    //else
    //{
    //    printf("%u, %llu\n", value.size, hash_value(&value.value));
    //}

    bool values_match;
    int checker_ret = kv_checker__check_value(director->checker, &key, &value.value, &values_match);
    if (checker_ret != 0 && checker_ret != -ENOENT)
    {
        // -ENOENT is not fatal to the test suite.
        return checker_ret;
    }

    if (ret != checker_ret)
    {
        fprintf(stderr, "kv_get failed because of key mismatch.\n");
        // If it wasn't found in the KV store under test, it shouldn't be found by the checker.
        // If it was found in the KV store under test, it should be found by the checker.
        director->number_of_unexpected_missing_keys++;
        director->number_of_failed_operations++;
        return 0;
    }

    if (ret != -ENOENT && !values_match)
    {
        fprintf(stderr, "kv_get failed because of value miscompare for key %" PRIu64 ".\n", key.id);
        // The value was found, but it does not match.
        director->number_of_miscompares++;
        director->number_of_failed_operations++;
    }

    return 0;
}

int test_director__do_del(struct test_director *director)
{
    struct key key;
    key.id = test_director__select_key();

    //printf("del(%llu)\n", key.id);

    int ret = kv_del(director->stor, &key);
    if (ret != 0 && ret != -ENOENT)
    {
        fprintf(stderr, "kv_del failed.\n");
        director->number_of_failed_operations++;
        return 0;
    }

    int checker_ret = kv_checker__delete_value(director->checker, &key);
    if (checker_ret != 0 && checker_ret != -ENOENT)
    {
        // -ENOENT is not fatal to the test suite.
        return checker_ret;
    }

    if (ret != checker_ret)
    {
        fprintf(stderr, "kv_del failed because of key mismatch.\n");
        // If it wasn't found in the KV store under test, it shouldn't be found by the checker.
        // If it was found in the KV store under test, it should be found by the checker.
        director->number_of_unexpected_missing_keys++;
        director->number_of_failed_operations++;
    }

    return 0;
}


test_director__op test_director__select_operation(struct test_director *director)
{
    switch(rand() % 6)
    {
    case 0:    // Make this workload set heavy so we're sure to fill up and get failures.
    case 1:
    case 2:
        return test_director__do_set;
    case 3:
    case 4:
        return test_director__do_get;
    default:
        return test_director__do_del;
    }
}

int test_director__run_stress(struct test_director *director, size_t iterations)
{
    size_t i;
    for(i = 0; i < iterations; i++)
    {
        test_director__op op = test_director__select_operation(director);

        int ret = op(director);
        if (ret != 0)
        {
            return ret;
        }
        director->number_of_operations_run++;
    }

    return 0;
}
