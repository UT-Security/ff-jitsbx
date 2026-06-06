#pragma once

// Some code to make the wasm sandboxed code easily compatible when compiling LFI
// This header is automatically injected in the C/C++ files compiled to LFI
#include <stdlib.h>

#define mozalloc_handle_oom(size) abort();