/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_JitCode_h
#define jit_JitCode_h

#include "mozilla/MemoryReporting.h"  // MallocSizeOf

#include <stddef.h>  // size_t
#include <stdint.h>  // uint8_t, uint32_t

#include "jstypes.h"

#include "gc/Allocator.h"  // AllowGC
#include "gc/Cell.h"       // gc::TenuredCellWithNonGCPointer
#include "jit/ExecutableAllocator.h" // Executable
#include "js/TraceKind.h"  // JS::TraceKind
#include "js/UbiNode.h"    // ubi::{TracerConcrete, Size, CourseType}

#ifdef JS_SANDBOX_LFI
#  include <syscall.h>
#  include <unistd.h>
#define SYS_jitcode_create 442
#define SYS_jitcode_delete 443
#define SYS_jitcode_create2 447

static int sys_jitcode_create(void* addrp, void* bufp, size_t length) {
  return syscall(SYS_jitcode_create, addrp, bufp, length);
}

static int sys_jitcode_create2(void* addrp, void* bufp1, size_t length1, void* bufp2, size_t length2) {
  return syscall(SYS_jitcode_create2, addrp, bufp1, length1, bufp2, length2);
}

static int sys_jitcode_delete(void* addrp, size_t length) {
  return syscall(SYS_jitcode_delete, addrp, length);
}
#endif

namespace js {
namespace jit {

class JitCode;
class MacroAssembler;


// All JitCode allocations are prefixed by a tiny function which only goal is to
// return the JitCode pointer for the class which is referencing the JitCode.
// This is necessary to keep the generated code alive while there are any
// reference live on the stack.
using GetJitCode = JitCode* (*)();

// Size of the code which is generated at the beginning of the JitCode
// Executable pages.
#ifdef JS_SANDBOX
const size_t JitCodeHeaderSize = 32;
#else
const size_t JitCodeHeaderSize = 16;
#endif

class JitCode : public gc::TenuredCellWithNonGCPointer<uint8_t> {
  friend class gc::CellAllocator;
  // Access Executable class.
  friend class AutoWritableJitCodeFallible;

 public:
  // Entry point used in the generated code, it corresponds to the
  // aligned(Executable.xStart + sizeof(JitCodeHeader))
  uint8_t* raw() const { return headerPtr(); }

  // The executable is allocated and aligned, the headerSize can be larger than
  // JitCodeHeaderSize, and the header should be at an offset from the code
  // entry point. (see FromExecutable)
  uint8_t* header() const { return raw() - JitCodeHeaderSize; }

 protected:
  Executable executable_;
  uint32_t insnSize_;    // Instruction stream size.
  uint32_t dataSectionBytes_;     // Size of the data section.
  uint32_t constantsTableBytes_;   // Size of constants table.
  uint8_t headerSize_ : 6;        // Number of bytes allocated before codeStart.
  bool invalidated_ : 1;     // Whether the code object has been invalidated.
                             // This is necessary to prevent GC tracing.
  bool hasBytecodeMap_ : 1;  // Whether the code object has been registered with
                             // native=>bytecode mapping tables.

  JitCode() = delete;
  JitCode(Executable&& exec, uint32_t headerSize)
      : TenuredCellWithNonGCPointer((uint8_t*)exec.xStart + headerSize),
        executable_(std::move(exec)),
        insnSize_(0),
        dataSectionBytes_(0),
        constantsTableBytes_(0),
        headerSize_(headerSize),
        invalidated_(false),
        hasBytecodeMap_(false) {
    MOZ_ASSERT(headerSize_ == headerSize);
  }

  uint32_t dataOffset() const {
      return 0;
  }
  uint32_t dataSectionOffset() const {
    return dataOffset();
  }
  uint32_t constantsTableOffset() const {
    return dataSectionOffset() + dataSectionBytes_;
  }

  uint32_t dataSize() const {
    return constantsTableOffset() + constantsTableBytes_;
  }

 public:
  size_t dataSectionEntries() const {
    return dataSectionBytes_ / sizeof(gc::Cell*);  
  }
  
  uint8_t* dataRaw() const {
    if (executable_.desc.rwSize) {
      return (uint8_t*)executable_.rwStart;
    }
    return rawEnd();
  }
  uint8_t* dataSection() const { return &dataRaw()[dataSectionOffset()]; }
  uint8_t* constantsTable() const { return &dataRaw()[constantsTableOffset()]; }
  uint8_t* dataRawEnd() const { return dataRaw() + dataSize(); }

 public:
  uint8_t* rawEnd() const { return raw() + insnSize_; }
  bool containsNativePC(const void* addr) const {
    const uint8_t* addr_u8 = (const uint8_t*)addr;
    return raw() <= addr_u8 && addr_u8 < rawEnd();
  }
  bool containsDataPtr(const void* addr) const {
    const uint8_t* addr_u8 = (const uint8_t*)addr;
    return dataRaw() <= addr_u8 && addr_u8 < dataRawEnd();
  }
  size_t instructionsSize() const { return insnSize_; }
  size_t bufferSize() const { return executable_.desc.xSize - headerSize_; }
  size_t headerSize() const { return headerSize_; }

  void traceChildren(JSTracer* trc);
  void finalize(JS::GCContext* gcx);
  void setInvalidated() { invalidated_ = true; }

  void setHasBytecodeMap() { hasBytecodeMap_ = true; }

  // If this JitCode object has been, effectively, corrupted due to
  // invalidation patching, then we have to remember this so we don't try and
  // trace relocation entries that may now be corrupt.
  bool invalidated() const { return !!invalidated_; }

  template <typename T>
  T as() const {
    return JS_DATA_TO_FUNC_PTR(T, raw());
  }

  void copyFrom(MacroAssembler& masm);

  static JitCode* FromExecutable(uint8_t* entry) {
    // The JitCode pointer associated with this entry point is stored in the
    // early bits of the executable code. At an offset ahead which let us encode
    // enough instruction to encode the code pointer, and which can be returned
    // once called.
    uint8_t* fetchJitCode = entry - JitCodeHeaderSize;
    GetJitCode fetch = reinterpret_cast<GetJitCode>(fetchJitCode);
    JitCode* code = fetch();
    MOZ_RELEASE_ASSERT(code->raw() == entry);
    return code;
  }

  static size_t offsetOfCode() { return offsetOfHeaderPtr(); }

  // Allocates a new JitCode object which will be managed by the GC. If no
  // object can be allocated, nullptr is returned. On failure, |pool| is
  // automatically released, so the code may be freed.
  template <AllowGC allowGC>
  static JitCode* New(JSContext* cx, Executable&& exec, uint32_t headerSize);

 public:
  static const JS::TraceKind TraceKind = JS::TraceKind::JitCode;
};

}  // namespace jit
}  // namespace js

// JS::ubi::Nodes can point to js::jit::JitCode instances; they're js::gc::Cell
// instances with no associated compartment.
namespace JS {
namespace ubi {
template <>
class Concrete<js::jit::JitCode> : TracerConcrete<js::jit::JitCode> {
 protected:
  explicit Concrete(js::jit::JitCode* ptr)
      : TracerConcrete<js::jit::JitCode>(ptr) {}

 public:
  static void construct(void* storage, js::jit::JitCode* ptr) {
    new (storage) Concrete(ptr);
  }

  CoarseType coarseType() const final { return CoarseType::Script; }

  Size size(mozilla::MallocSizeOf mallocSizeOf) const override {
    Size size = js::gc::Arena::thingSize(get().asTenured().getAllocKind());
    size += get().bufferSize();
    size += get().headerSize();
    return size;
  }

  const char16_t* typeName() const override { return concreteTypeName; }
  static const char16_t concreteTypeName[];
};

}  // namespace ubi
}  // namespace JS

#endif /* jit_JitCode_h */
