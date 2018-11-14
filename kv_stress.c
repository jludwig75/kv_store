#include "test_director.h"
#include "kv_checker.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>


int main(int argc, char **argv)
{
    unsigned seed = (unsigned)time(NULL);
    printf("Running with seed %u\n", seed);

    struct kv_checker *checker;
    int ret = kv_checker__init(&checker);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to run test suite: %d.\n", ret);
        return ret;
    }

    struct test_director *director;
    ret = test_director__init(&director, seed, checker, argc, argv);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to run test suite: %d.\n", ret);
        return ret;
    }

    ret = test_director__run_stress(director, 100000);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to run test suite: %d.\n", ret);
        return ret;
    }

    printf("Ran %lu operations with %lu failures\n", test_director__get_number_of_operations_run(director), test_director__get_number_of_failed_operations(director));

    test_director__cleanup(&director);
    kv_checker__cleanup(&checker);

    return 0;
}
