// ask2374
#ifndef JITSANDBOX
#define JITSANDBOX

#include <atomic>
#include <cstdint>
#include <iostream> // remove when finished
#include <mutex>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

#define PAGE_BASE(ptr) ((void*)((uint64_t)ptr & (uint64_t)(~(PAGE_SIZE-1))))

#define SANDBOX_BASE(ptr) ((void*)((uint64_t)ptr & (uint64_t)(~(SANDBOX_SIZE-1))))

#define SANDBOX_LOG(format, ...) \
	if (LOG_OPT) { \
		log_mutex.lock(); \
		fprintf(sandbox_log, format __VA_OPT__(,) __VA_ARGS__); \
		fflush(sandbox_log); \
		log_mutex.unlock(); \
	}

#define PAGE_SIZE 4096

#define SANDBOX_SIZE (((uint64_t)1) << 32)

namespace js {
namespace sandbox {

void checkJitMask(void* ptr);

}
}
#endif

extern bool LOG_OPT;
extern bool SANDBOX_OPT;
extern FILE* sandbox_log;
extern std::atomic<uint32_t> max_zone_id;
extern thread_local uint32_t zone_id;
extern std::mutex log_mutex;
extern std::mutex map_mutex;
extern std::unordered_map<uint32_t, void*> zone_map;
extern void js::sandbox::checkJitMask(void* ptr);
// ask2374
