/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "jit/Linker.h"

#include "mozilla/Casting.h"
#include "jit/JitZone.h"
#include "util/Memory.h"

#include "gc/StoreBuffer-inl.h"

namespace js {
namespace jit {

JitCode* Linker::newCode(JSContext* cx, CodeKind kind) {
  JS::AutoAssertNoGC nogc(cx);
  if (masm.oom()) {
    return fail(cx);
  }

  static const size_t ExecutableAllocatorAlignment = sizeof(void*);
  static_assert(CodeAlignment >= ExecutableAllocatorAlignment,
                "Unexpected alignment requirements");

  static const size_t ReadWriteAllocatorAlignment = sizeof(void*);

  // Query the MacroAssembler to know if data should be allocated separately,
  // and size the sections accordingly.
  size_t execNeeded = masm.execSize();
  size_t dataNeeded = masm.dataSize();

  // ExecutableAllocator requires execNeeded to be aligned.
  execNeeded += JitCodeHeaderSize;
  execNeeded += (CodeAlignment - ExecutableAllocatorAlignment);
  if (execNeeded >= MAX_BUFFER_SIZE || dataNeeded >= MAX_BUFFER_SIZE) {
    return fail(cx);
  }

  dataNeeded += ReadWriteAllocatorAlignment;

  execNeeded = AlignBytes(execNeeded, ExecutableAllocatorAlignment);
  dataNeeded = AlignBytes(dataNeeded, ReadWriteAllocatorAlignment);

  JitZone* jitZone = cx->zone()->getJitZone(cx);
  if (!jitZone) {
    // Note: don't call fail(cx) here, getJitZone reports OOM.
    return nullptr;
  }

  using mozilla::AssertedCast;
  ExecutableDesc desc{AssertedCast<uint32_t>(execNeeded), AssertedCast<uint32_t>(dataNeeded), kind};
  Executable result(jitZone->execAlloc().alloc(cx, desc));
  if (!result) {
    return fail(cx);
  }

  // The JitCodeHeader will be stored right before the code buffer.
  uint8_t* execStart = (uint8_t*) result.xStart;
  // Bump the code up to a nice alignment.
  uint8_t* codeStart = (uint8_t*)AlignBytes((uintptr_t)execStart, CodeAlignment);
  codeStart += JitCodeHeaderSize;
  
  MOZ_ASSERT(codeStart + masm.execSize() <= execStart + execNeeded);
  uint32_t headerSize = codeStart - execStart;

  uint8_t* dataStart = (uint8_t*)AlignBytes((uintptr_t)execStart, ReadWriteAllocatorAlignment);
  MOZ_ASSERT(dataStart + masm.dataSize() <= dataStart + dataNeeded);
  
  JitCode* code =
      JitCode::New<NoGC>(cx, std::move(result), headerSize);
  if (!code) {
    return fail(cx);
  }
  if (masm.oom()) {
    code->finalize(nullptr);
    return fail(cx);
  }
  awjcf.emplace(code);
  if (!awjcf->makeWritable()) {
    code->finalize(nullptr);
    return fail(cx);
  }
  code->copyFrom(masm);
  masm.link(code);
  if (masm.embedsNurseryPointers()) {
    cx->runtime()->gc.storeBuffer().putWholeCell(code);
  }
  return code;
}

}  // namespace jit
}  // namespace js
