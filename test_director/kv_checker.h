#pragma once

#include "kv_store.h"

#include <stdbool.h>


struct kv_checker;


int kv_checker__init(struct kv_checker **checker);

void kv_checker__cleanup(struct kv_checker **checker);

int kv_checker__store_value(struct kv_checker *checker, const struct key *k, const struct value *v);

int kv_checker__check_value(const struct kv_checker *checker, const struct key *k, const struct value *v, bool *values_match);

int kv_checker__delete_value(struct kv_checker *checker, const struct key *k);
