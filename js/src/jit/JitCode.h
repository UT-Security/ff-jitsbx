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

namespace js {
namespace jit {

class JitCode;
class MacroAssembler;

// All JitCode executable allocations are prefixed by a tiny function which
// returns the JitCode pointer for the class which is referencing it.
// This is necessary to keep the generated code alive while there are any
// reference live on the stack.
using GetJitCode = JitCode* (*)();

const size_t JitCodeHeaderSize = 16;

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
  uint32_t gcDataBytes_;  // Size of the GC data.
  uint32_t immDataBytes_;  // Size of the immediate data.
  uint8_t headerSize_ : 5;        // Number of bytes allocated before codeStart.
  bool invalidated_ : 1;     // Whether the code object has been invalidated.
                             // This is necessary to prevent GC tracing.
  bool hasBytecodeMap_ : 1;  // Whether the code object has been registered with
                             // native=>bytecode mapping tables.

  JitCode() = delete;
  JitCode(Executable&& exec, uint32_t headerSize)
      : TenuredCellWithNonGCPointer((uint8_t*)exec.xStart + headerSize),
        executable_(std::move(exec)),
        insnSize_(0),
        gcDataBytes_(0),
        immDataBytes_(0),
        headerSize_(headerSize),
        invalidated_(false),
        hasBytecodeMap_(false) {
    MOZ_ASSERT(headerSize_ == headerSize);
  }

  uint32_t dataOffset() const {
    return reinterpret_cast<uintptr_t>(dataRaw()) % sizeof(void*);
  }
  uint32_t gcDataOffset() const { return dataOffset(); }
  
  uint32_t immDataOffset() const {
    return gcDataOffset() + gcDataBytes_;
  }

  uint32_t dataSize() const {
    return immDataOffset() + immDataBytes_;
  }

 public:
  uint8_t* dataRaw() const {
    if (executable_.desc.rwSize) {
      return (uint8_t*)executable_.rwStart;
    }
    return rawEnd();
  }

  size_t gcDataEntries() const {
    return gcDataBytes_ / sizeof(gc::Cell*); 
  }

  uint8_t* gcDataSection() const { return &dataRaw()[gcDataOffset()]; }
  uint8_t* immDataSection() const { return &dataRaw()[immDataOffset()]; }
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

  static JitCode* FromExecutable(uint8_t* entry);

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
