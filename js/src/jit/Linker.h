/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_Linker_h
#define jit_Linker_h

#include "JitCode.h"
#include "mozilla/Maybe.h"

#include <stdint.h>

#include "jstypes.h"

#include "jit/AutoWritableJitCode.h"
#include "jit/MacroAssembler.h"
#include "vm/Runtime.h"

struct JS_PUBLIC_API JSContext;

namespace js {
namespace jit {

class JitCode;

enum class CodeKind : uint8_t;

class Linker {
  MacroAssembler& masm;
  mozilla::Maybe<AutoWritableJitCodeFallible> awjcf;

  JitCode* fail(JSContext* cx) {
    ReportOutOfMemory(cx);
    return nullptr;
  }

 public:
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
  mozilla::Maybe<JS::Rooted<JitCode*>> code;
#endif

  // Construct a linker with a rooted macro assembler.
  explicit Linker(MacroAssembler& masm) : masm(masm) { masm.finish(); }

  ~Linker() {
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
    if (code) {
#if defined(JS_SANDBOX_CFI_LABEL) && defined(JS_SANDBOX_SW_SHSTK)
      uint8_t headerContent[JitCodeHeaderSize] = {
          0xF3, 0x0F, 0x1E, 0xFA,        // endbr64
          0x48, 0xB8, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00,  // mov $<imm64>, %rax
          0x49, 0x89, 0x67, 0x18,        // mov %rsp, 24(%r15)
          0x49, 0x8b, 0x67, 0x10,        // mov 16(%r15), %rsp
          0xC3,                          // ret
          0xE5, 0xE5, 0xE5, 0xE5, 0xE5,  // in (Illegal x9)
          0xE5, 0xE5, 0xE5, 0xE5
        };
#elif defined(JS_SANDBOX_CFI_LABEL)
      uint8_t headerContent[JitCodeHeaderSize] = {
          0xF3, 0x0F, 0x1E, 0xFA,        // endbr64
          0x48, 0xB8, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00,  // mov $<imm64>, %rax
          0xC3,                          // ret
          0xE5                           // in (Illegal)
      };
#else
      uint8_t headerContent[JitCodeHeaderSize] = {
          0x48, 0xB8, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00,  // mov $<imm64>, %rax
          0xC3,                          // ret
          0xE5, 0xE5, 0xE5, 0xE5, 0xE5   // in (Illegal x5)
      };
#endif

      JitCode* self = code->get();
#ifdef JS_SANDBOX_CFI_LABEL
      memcpy(&headerContent[6], reinterpret_cast<uint8_t*>(&self), 8);
#else
      memcpy(&headerContent[2], reinterpret_cast<uint8_t*>(&self), 8);
#endif
      sys_jitcode_create2(code->get()->header(), headerContent,
                      js::jit::JitCodeHeaderSize, masm.buffer(),
                      masm.execSize() + js::jit::JitCodeHeaderSize);
    }
#endif
  }

  // Create a new JitCode object and populate it with the contents of the
  // macro assember buffer.
  //
  // This method cannot GC. Errors are reported to the context.
  JitCode* newCode(JSContext* cx, CodeKind kind);
};

}  // namespace jit
}  // namespace js

#endif /* jit_Linker_h */
