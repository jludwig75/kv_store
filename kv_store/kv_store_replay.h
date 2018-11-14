#pragma once
#include "kv_store_private.h"


/// @brief  Replays the log file after opening a new file
/// @param  A kvstor instance
/// @return 0 if the log was successfully replayed
///         A value < 0 if there was an error. The log is corrupted
int kv_store__replay_log(struct kvstor *store);