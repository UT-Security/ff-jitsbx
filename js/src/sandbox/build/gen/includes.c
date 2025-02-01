#include <stddef.h>
#include <stdint.h>
#define INCBIN_SECTION ".rodata"

#define STR2(x) #x
#define STR(x) STR2(x)

#define INCBIN(name, file) \
__asm__(".section " INCBIN_SECTION "\n" \
        ".global __lfidata_" STR(name) "_start\n" \
        ".balign 16\n" \
        "__lfidata_" STR(name) "_start:\n" \
        ".incbin \"" file "\"\n" \
        \
        ".global __lfidata_" STR(name) "_end\n" \
        ".balign 1\n" \
        "__lfidata_" STR(name) "_end:\n" \
        ".byte 0\n" \
); \
extern __attribute__((aligned(16))) uint8_t __lfidata_ ## name ## _start[]; \
extern                              uint8_t __lfidata_ ## name ## _end[]

char* sbx_filenames[] = {
    "/lib/libc++.so.1",
    "/lib/libc++abi.so.1",
    "/lib/libc.so",
    "/lib/libmozjs-115.so",
    "/lib/libunwind.so.1",
    "stub",
};
INCBIN(__lib__libc______so__1, "/home/abhishekcs/workspace/lfi-toolchains/lfi-llvm-toolchain/x86_64-lfi-syscalls-clang/sysroot/usr/lib/libc++.so.1");
INCBIN(__lib__libc____abi__so__1, "/home/abhishekcs/workspace/lfi-toolchains/lfi-llvm-toolchain/x86_64-lfi-syscalls-clang/sysroot/usr/lib/libc++abi.so.1");
INCBIN(__lib__libc__so, "/home/abhishekcs/workspace/lfi-toolchains/lfi-llvm-toolchain/x86_64-lfi-syscalls-clang/sysroot/usr/lib/libc.so");
INCBIN(__lib__libmozjs__115__so, "/home/abhishekcs/workspace/sobox/libmozjs-115.so");
INCBIN(__lib__libunwind__so__1, "/home/abhishekcs/workspace/lfi-toolchains/lfi-llvm-toolchain/x86_64-lfi-syscalls-clang/sysroot/usr/lib/libunwind.so.1");
INCBIN(stub, "gen/stub.elf");

struct File {
    uint8_t* start;
    uint8_t* end;
};

struct File sbx_filedata[] = {
    (struct File){&__lfidata___lib__libc______so__1_start[0], &__lfidata___lib__libc______so__1_end[0]},
    (struct File){&__lfidata___lib__libc____abi__so__1_start[0], &__lfidata___lib__libc____abi__so__1_end[0]},
    (struct File){&__lfidata___lib__libc__so_start[0], &__lfidata___lib__libc__so_end[0]},
    (struct File){&__lfidata___lib__libmozjs__115__so_start[0], &__lfidata___lib__libmozjs__115__so_end[0]},
    (struct File){&__lfidata___lib__libunwind__so__1_start[0], &__lfidata___lib__libunwind__so__1_end[0]},
    (struct File){&__lfidata_stub_start[0], &__lfidata_stub_end[0]},
};

size_t sbx_nfiles = 6;
