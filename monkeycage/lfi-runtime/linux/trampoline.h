#pragma once

#include "linux.h"

#ifdef __STDC_NO_THREADS__
#include "threads.h"
#else
#include <threads.h>
#endif

extern thread_local struct LFIContext *new_ctx;
