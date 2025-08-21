/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
*/

#ifndef jit_x86_shared_AssemblerBundleBuffer_x86_shared_h
#define jit_x86_shared_AssemblerBundleBuffer_x86_shared_h

#include "mozilla/Assertions.h"
#include "mozilla/Attributes.h"
#include "mozilla/Likely.h"
#include "mozilla/Vector.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "jit/x86-shared/AssemblerBuffer-x86-shared.h"
#include "sandbox/Bundle.h"

namespace js {
namespace jit {

enum class BundleMode {
  Instruction,
  Group,
};

class AssemblerBundleBuffer {
  template <size_t size, typename T>
  MOZ_ALWAYS_INLINE void sizedAppend(T value) {
    if(oom()) return;
    ensureBundleSpace(size);
    MOZ_ASSERT(bundle_length + size <= sandbox::BUNDLE_SIZE, "Expected enough space in bundle");
    memcpy(bundle_buffer + bundle_length, reinterpret_cast<unsigned char*>(&value), size);
    bundle_length += size;
  }

 public:
  AssemblerBundleBuffer()
      : mode(BundleMode::Instruction),
        bundle_length(0),
        in_bundle(false) {}

  void ensureSpace(size_t space) {
    m_inner_buffer.ensureSpace(space);
  }

  inline void ensureBundleSpace(size_t space) {
    MOZ_ASSERT(in_bundle, "Expected to be within a bundle");
    MOZ_ASSERT(bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected bundle overflow");
    if(oom()) return;
    if (js::sandbox::BUNDLE_SIZE - bundle_length < space) {
      // TODO(JS_SANDBOX_BUNDLE): process and prepare for next bundle.

      // All but the currently emitted instruction/group belong to the current bundle.

      // NOP pad out the bundle to complete it.
      size_t nop_size = js::sandbox::BUNDLE_SIZE - bundle_start;

      MOZ_ASSERT(nop_size < js::sandbox::BUNDLE_SIZE, "Unexpected NOP padding size");

      // Our new bundle now has only the bytes corresponding to the current
      // instruction/group.
      bundle_length = bundle_length - bundle_start;

      if (nop_size) {
        // undo the write of the current instruction/group.
        m_inner_buffer.shrinkBy(bundle_length);

        // ensure that the underlying buffer has enough space.
        m_inner_buffer.ensureSpace(nop_size + space);
        
        // insert nops to align to end of bundle.
        if(!m_inner_buffer.append(nops[nop_size], nop_size)) {
          return;
        }

        MOZ_ASSERT(m_inner_buffer.size() % js::sandbox::BUNDLE_SIZE == 0, "Expected bundle aligned buffer");

        if (bundle_length &&
            m_inner_buffer.append(bundle_buffer + bundle_start,
                                  bundle_length)) {
          memcpy(bundle_buffer, bundle_buffer + bundle_start,
                 bundle_length);
        }
      }

      // the current instruction starts the bundle.
      bundle_start = 0;
    }
  }

  bool isAligned(size_t alignment) const {
    return m_inner_buffer.isAligned(alignment);
  }

  MOZ_ALWAYS_INLINE void putByteUnchecked(int value) {
    sizedAppend<1>(value);
    m_inner_buffer.putByteUnchecked(value);
  }
  MOZ_ALWAYS_INLINE void putShortUnchecked(int value) {
    sizedAppend<2>(value);
    m_inner_buffer.putShortUnchecked(value);
  }
  MOZ_ALWAYS_INLINE void putIntUnchecked(int value) {
    sizedAppend<4>(value);
    m_inner_buffer.putIntUnchecked(value);
  }
  MOZ_ALWAYS_INLINE void putInt64Unchecked(int64_t value) {
    sizedAppend<8>(value);
    m_inner_buffer.putInt64Unchecked(value);
  }

  MOZ_ALWAYS_INLINE void putByte(int value) {
    sizedAppend<1>(value);
    m_inner_buffer.putByte(value);
  }

  MOZ_ALWAYS_INLINE void putShort(int value) {
    sizedAppend<2>(value);
    m_inner_buffer.putShort(value);
  }

  MOZ_ALWAYS_INLINE void putInt(int value) {
    sizedAppend<4>(value);
    m_inner_buffer.putInt(value);
  }

  MOZ_ALWAYS_INLINE void putInt64(int64_t value) {
    sizedAppend<8>(value);
    m_inner_buffer.putInt64(value);
  }

  [[nodiscard]] bool append(const unsigned char* values, size_t size) {
    MOZ_ASSERT(!in_bundle, "Unexpected to be in bundle");
    MOZ_ASSERT(bundle_length == 0, "Unexpected pending bundle");
    return m_inner_buffer.append(values, size);
  }

  size_t size() const {
    return m_inner_buffer.size();
  }

  bool oom() const { return m_inner_buffer.oom(); }

  bool reserve(size_t size) { return m_inner_buffer.reserve(size); }

  bool swap(Vector<uint8_t, 0, SystemAllocPolicy>& bytes) {
    MOZ_ASSERT(!in_bundle, "Unexpected to be in bundle");
    MOZ_ASSERT(oom() || bundle_length == 0, "Unexpected pending bundle");
    return m_inner_buffer.swap(bytes);
  }

  const unsigned char* buffer() const {
    MOZ_ASSERT(!in_bundle, "Unexpected to be in bundle");
    return m_inner_buffer.buffer();
  }

  unsigned char* data() {
    return m_inner_buffer.data();
  }

  bool beginBundleInstruction() {
    if (in_bundle) return false;
    MOZ_ASSERT(mode == BundleMode::Instruction, "Expected instruction bundling");
    MOZ_ASSERT(bundle_length < js::sandbox::BUNDLE_SIZE, "Unexpected unprocessed full bundle");

    bundle_start = bundle_length;
    in_bundle = true;
    return true;
  }

  void endBundleInstruction() {
    MOZ_ASSERT(in_bundle, "Unexpected instruction end outside instruction");
    MOZ_ASSERT(mode == BundleMode::Instruction, "Expected instruction bundling mode");
    MOZ_ASSERT(oom() || bundle_length - bundle_start > 0, "Unexpected 0 length instruction");
    MOZ_ASSERT(bundle_length <= js::sandbox::BUNDLE_SIZE, "Unexpected oversized bundle");

    in_bundle = false;
    if (bundle_length == js::sandbox::BUNDLE_SIZE) {
      MOZ_ASSERT(m_inner_buffer.size() % js::sandbox::BUNDLE_SIZE == 0,
                 "Expected bundle aligned buffer");
      // we are in a new empty bundle
      bundle_length = 0;
    }
  }

  void beginBundleGroup() {
    MOZ_ASSERT(!in_bundle, "Unexpected nested bundle group");
    MOZ_ASSERT(mode == BundleMode::Instruction, "Expected instruction bundling");
    MOZ_ASSERT(bundle_length < js::sandbox::BUNDLE_SIZE, "Unexpected unprocessed full bundle");

    in_bundle = true;
    mode = BundleMode::Group;
    bundle_start = bundle_length;
  }

  void endBundleGroup() {
    MOZ_ASSERT(in_bundle, "Unexpected bundle group end outside bundle");
    MOZ_ASSERT(mode == BundleMode::Group, "Expected group bundling mode");
    MOZ_ASSERT(oom() || bundle_length - bundle_start > 0, "Unexpected 0 length group");
    MOZ_ASSERT(bundle_length <= js::sandbox::BUNDLE_SIZE, "Unexpected oversized bundle");
    
    in_bundle = false;
    mode = BundleMode::Instruction;
    if (bundle_length == js::sandbox::BUNDLE_SIZE) {
      MOZ_ASSERT(m_inner_buffer.size() % js::sandbox::BUNDLE_SIZE == 0,
                 "Expected bundle aligned buffer");
      // we are in a new empty bundle
      bundle_length = 0;
    }
  }

 protected:

  unsigned char bundle_buffer[js::sandbox::BUNDLE_SIZE];
  
  BundleMode mode;
  size_t bundle_length;
  bool in_bundle;
  size_t bundle_start;

  static constexpr unsigned char nops[32][32] = {
      // 0
      {},
      // 1
      {0x90},
      // 2
      {0x66, 0x90},
      // 3
      {0xf, 0x1f, 0x00},
      // 4
      {0x0f, 0x1f, 0x40, 0x00},
      // 5
      {0x0f, 0x1f, 0x44, 0x00, 0x00},
      // 6
      {0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00},
      // 7
      {0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00},
      // 8
      {0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 9
      {0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 10
      {0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 11
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 12
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90},
      // 13
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
       0x90},
      // 14
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf,
       0x1f, 0x00},
      // 15
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
       0x1f, 0x40, 0x00},
      // 16
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
       0x1f, 0x44, 0x00, 0x00},
      // 17
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
       0x0f, 0x1f, 0x44, 0x00, 0x00},
      // 18
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
       0x1f, 0x80, 0x00, 0x00, 0x00, 0x00},
      // 19
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
       0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 20
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 21
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 22
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 23
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
       0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90},
      // 24
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
       0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x90},
      // 25
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00,
       0x00, 0x00, 0x00, 0x00, 0xf,  0x1f, 0x00},
      // 26
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0x40, 0x00},
      // 27
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0x44, 0x00, 0x00},
      // 28
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00},
      // 29
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00},
      // 30
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
      // 31
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00},
  };

  AssemblerBuffer m_inner_buffer;
};

}  // namespace jit
}  // namespace js

#endif
