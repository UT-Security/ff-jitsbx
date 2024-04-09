// ask2374

#include "js/HeapAPI.h"
#include "mozilla/Assertions.h"
#include "mozilla/TaggedAnonymousMemory.h"
#include "js/JitSandbox.h"

void js::sandbox::checkJitMask(void* ptr) {
  __asm__ __volatile__(
      "push %rax\n"
      "push %rcx\n"
      "push %rdx\n"
      "push %rbx\n"
      "push %rsi\n"
      "push %rdi\n"
      "push %r8\n"
      "push %r9\n"
      "push %r10\n"
      "push %r11\n"
      "push %r12\n"
      "push %r13\n"
      "push %r14\n"
      "push %r15\n");

  void* gsbase;
  __asm__ __volatile__("rdgsbase %0" : "=r"(gsbase));
  void* masked_ptr = (void*)(((uint64_t)ptr) & 0xFFFFFFFF00000000);

  u_int64_t rsp;
  __asm__ __volatile__("movq %%rsp,%0" : "=r"(rsp));
  int set = rsp & 0xf;
  if (set) {
    __asm__ __volatile__("push %rax\n");
  }
  if (masked_ptr == gsbase) {
    SANDBOX_LOG("checkJitMask,pass,%p\n", ptr);
  } else {
    SANDBOX_LOG("checkJitMask,fail,%p,%p\n", gsbase, ptr);
  }
  if (set) {
    __asm__ __volatile__("pop %rax\n");
  }

  __asm__ __volatile__(
      "pop %r15\n"
      "pop %r14\n"
      "pop %r13\n"
      "pop %r12\n"
      "pop %r11\n"
      "pop %r10\n"
      "pop %r9\n"
      "pop %r8\n"
      "pop %rdi\n"
      "pop %rsi\n"
      "pop %rbx\n"
      "pop %rdx\n"
      "pop %rcx\n"
      "pop %rax\n");
}

static size_t pageSize = 0;

#ifdef XP_WIN
enum class PageAccess : DWORD {
  None = PAGE_NOACCESS,
  Read = PAGE_READONLY,
  ReadWrite = PAGE_READWRITE,
  Execute = PAGE_EXECUTE,
  ReadExecute = PAGE_EXECUTE_READ,
  ReadWriteExecute = PAGE_EXECUTE_READWRITE,
};
#elif defined(__wasi__)
enum class PageAccess : int {
  None = 0,
  Read = 0,
  ReadWrite = 0,
  Execute = 0,
  ReadExecute = 0,
  ReadWriteExecute = 0,
};
#else
enum class PageAccess : int {
  None = PROT_NONE,
  Read = PROT_READ,
  ReadWrite = PROT_READ | PROT_WRITE,
  Execute = PROT_EXEC,
  ReadExecute = PROT_READ | PROT_EXEC,
  ReadWriteExecute = PROT_READ | PROT_WRITE | PROT_EXEC,
};
#endif

enum class Commit : bool {
  No = false,
  Yes = true,
};

std::atomic<uint64_t> heap_bump_ptr;

static inline size_t OffsetFromAligned(void* region, size_t alignment) {
  return uintptr_t(region) % alignment;
}

static inline void ProtectMemory(void* region, size_t length, PageAccess prot) {
  MOZ_RELEASE_ASSERT(region && OffsetFromAligned(region, pageSize) == 0);
  MOZ_RELEASE_ASSERT(length > 0 && length % pageSize == 0);
#ifdef XP_WIN
  DWORD oldProtect;
  MOZ_RELEASE_ASSERT(VirtualProtect(region, length, DWORD(prot), &oldProtect) !=
                     0);
#elif defined(__wasi__)
  /* nothing */
#else
  MOZ_RELEASE_ASSERT(mprotect(region, length, int(prot)) == 0);
#endif
}

static void UnprotectPages(void* region, size_t length) {
  ProtectMemory(region, length, PageAccess::ReadWrite);
}

template <Commit commit, PageAccess prot>
static inline void* MapInternal(void* desired, size_t length) {
  void* region = nullptr;
#ifdef XP_WIN
  DWORD flags =
      (commit == Commit::Yes ? MEM_RESERVE | MEM_COMMIT : MEM_RESERVE);
  region = VirtualAlloc(desired, length, flags, DWORD(prot));
#elif defined(__wasi__)
  if (int err = posix_memalign(&region, gc::SystemPageSize(), length)) {
    MOZ_RELEASE_ASSERT(err == ENOMEM);
    return nullptr;
  }
  if (region) {
    memset(region, 0, length);
  }
#else
  int flags = MAP_PRIVATE | MAP_ANON;
  region = MozTaggedAnonymousMmap(desired, length, int(prot), flags, -1, 0,
                                  "js-gc-heap");
  if (region == MAP_FAILED) {
    return nullptr;
  }
#endif
  return region;
}

void* MapInternal(void* region, size_t length, bool rw) {
  MOZ_ASSERT(length > 0);

  if (rw)
    return MapInternal<Commit::Yes, PageAccess::ReadWrite>(region, length);

  return MapInternal<Commit::No, PageAccess::None>(region, length);
}

void js::sandbox::InitMemory() {
  if (pageSize == 0) {
#ifdef XP_WIN
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    pageSize = sysinfo.dwPageSize;
    allocGranularity = sysinfo.dwAllocationGranularity;
#else
    pageSize = size_t(sysconf(_SC_PAGESIZE));
#endif
  }

  heap_bump_ptr =
      (uint64_t)MapInternal((void*)((uint64_t)1 << 32), (size_t)1 << 32, false);
}

/*
 * This allocator maps pages in a contiguous 4GB region. Contiguous allocation
 * of memory is essential for efficiently masking accesses in the JIT comppiler.
 */
void* js::sandbox::MapAlignedPages(size_t length, size_t alignment) {
  MOZ_ASSERT(length % alignment == 0);
  MOZ_ASSERT(alignment % js::gc::ChunkSize == 0);
  MOZ_ASSERT(heap_bump_ptr.load() != 0);
	
	UnprotectPages((void*)heap_bump_ptr.load(), length);
	void* current_ptr = (void*)heap_bump_ptr.load();
	heap_bump_ptr += length;
	// Check for sandbox overflow
	MOZ_ASSERT(((uint64_t)current_ptr >> 32) == (heap_bump_ptr >> 32));
  return current_ptr;
}
// ask2374
