// ask2374

#ifndef SANDBOX_INTERFACE
#define SANDBOX_INTERFACE

typedef size_t arena_id_t;

namespace js {
namespace sandbox {

struct Interface {
	void* (*MapAlignedPages)(size_t length, size_t alignment);
	arena_id_t SandboxMallocArena;
};

};
};

#endif

extern js::sandbox::Interface* sandboxInterface;

// ask2374
