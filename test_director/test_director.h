#pragma once

#include "kv_store.h"

#include "kv_checker.h"


#include <stdlib.h>


struct test_director;


int test_director__init(struct test_director **director, unsigned random_seed, struct kv_checker *checker, int argc, char **argv);

void test_director__cleanup(struct test_director **director);

size_t test_director__get_number_of_operations_run(const struct test_director *director);

size_t test_director__get_number_of_failed_operations(const struct test_director *director);

int test_director__run_stress(struct test_director *director, size_t iterations);
