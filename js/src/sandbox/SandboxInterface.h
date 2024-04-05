// ask2374

#ifndef SANDBOX_INTERFACE
#define SANDBOX_INTERFACE

namespace js {
namespace sandbox {

struct Interface {
	void* (*MapAlignedPages)(size_t length, size_t alignment);
};

};
};

#endif

extern js::sandbox::Interface* sandboxInterface;

// ask2374
