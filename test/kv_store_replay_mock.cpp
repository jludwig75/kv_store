extern "C" {
#include "kv_store_replay.h"
}

#include "CppUTestExt/MockSupport.h"


int kv_store__replay_log(struct kvstor *store)
{
	return mock().actualCall("kv_store__replay_log").
		returnIntValue();
}