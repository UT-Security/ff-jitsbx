// ask2374
#ifndef JITSANDBOX
#  define JITSANDBOX

#  include <atomic>
#  include <cstdint>
#  include <iostream>  // remove when finished
#  include <mutex>
#  include <stdlib.h>
#  include <string.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <unordered_map>

// #include "jstypes.h"

#  define PAGE_BASE(ptr) ((void*)((uint64_t)ptr & (uint64_t)(~(PAGE_SIZE - 1))))

#  define SANDBOX_BASE(ptr) \
    ((void*)((uint64_t)ptr & (uint64_t)(~(SANDBOX_SIZE - 1))))

#  define SANDBOX_LOG(format, ...)                             \
    if (LOG_OPT) {                                             \
      log_mutex.lock();                                        \
      fprintf(sandbox_log, format __VA_OPT__(, ) __VA_ARGS__); \
      fflush(sandbox_log);                                     \
      log_mutex.unlock();                                      \
    }

#  define PAGE_SIZE 4096

#  define SANDBOX_SIZE (((uint64_t)1) << 32)

namespace js {
namespace sandbox {

void checkJitMask(void* ptr);
void InitMemory();
void* MapAlignedPages(size_t length, size_t alignment);

}
}  // namespace js
#endif

extern bool LOG_OPT;
extern bool SANDBOX_OPT;
extern FILE* sandbox_log;
extern std::mutex log_mutex;
extern void js::sandbox::checkJitMask(void* ptr);
extern std::atomic<uint64_t> heap_bump_ptr;
// ask2374
