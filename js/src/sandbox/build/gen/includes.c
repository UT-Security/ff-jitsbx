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
    "/lib/libgcc_s.so.1",
    "/lib/libmozjs-115.so",
    "/lib/libstdc++.so.6",
    "ld-musl-x86_64.so.1",
    "stub",
};
INCBIN(__lib__libgcc_s__so__1, "/home/abhishekcs/workspace/lfi-toolchains/lfi-amd64-syscalls/x86_64-lfi-linux-musl/lib64/libgcc_s.so.1");
INCBIN(__lib__libmozjs__115__so, "/home/abhishekcs/workspace/sobox/libmozjs-115.so");
INCBIN(__lib__libstdc______so__6, "/home/abhishekcs/workspace/lfi-toolchains/lfi-amd64-syscalls/x86_64-lfi-linux-musl/lib64/libstdc++.so.6");
INCBIN(ld__musl__x86_64__so__1, "/home/abhishekcs/workspace/lfi-toolchains/lfi-amd64-syscalls/x86_64-lfi-linux-musl/lib/ld-musl-x86_64.so.1");
INCBIN(stub, "gen/stub.elf");

struct File {
    uint8_t* start;
    uint8_t* end;
};

struct File sbx_filedata[] = {
    (struct File){&__lfidata___lib__libgcc_s__so__1_start[0], &__lfidata___lib__libgcc_s__so__1_end[0]},
    (struct File){&__lfidata___lib__libmozjs__115__so_start[0], &__lfidata___lib__libmozjs__115__so_end[0]},
    (struct File){&__lfidata___lib__libstdc______so__6_start[0], &__lfidata___lib__libstdc______so__6_end[0]},
    (struct File){&__lfidata_ld__musl__x86_64__so__1_start[0], &__lfidata_ld__musl__x86_64__so__1_end[0]},
    (struct File){&__lfidata_stub_start[0], &__lfidata_stub_end[0]},
};

size_t sbx_nfiles = 5;
