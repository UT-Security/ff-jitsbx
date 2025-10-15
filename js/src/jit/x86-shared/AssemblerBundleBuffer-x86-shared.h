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
    MOZ_ASSERT_IF(mode == BundleMode::Group, !frozen_bundle);
    ensureBundleSpace(size);
    MOZ_ASSERT(oom() || bundle_length + size <= sandbox::BUNDLE_SIZE, "Expected enough space in bundle");
    bundle_length += size;
  }

 public:
  AssemblerBundleBuffer()
      : mode(BundleMode::Instruction),
        bundle_length(0),
        in_bundle(false)
#ifdef DEBUG
        ,
        frozen_bundle(false) 
#endif
        {}

  void ensureSpace(size_t space) {
    m_inner_buffer.ensureSpace(space);
  }

  void ensureBundleSpace(size_t space) {
    MOZ_ASSERT(in_bundle, "Expected to be within a bundle");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected bundle overflow");
    if (MOZ_UNLIKELY(oom()))
      return;
    
    size_t bundle_space = js::sandbox::BUNDLE_SIZE - bundle_length; 
    if (MOZ_UNLIKELY(bundle_space < space)) {
        MOZ_ASSERT(bundle_length > 0, "Expected to be in midst of emission");

        // All but the currently emitted instruction/group belong to the current
        // bundle.

        // NOP padding required to pad out the partial instruction/group
        // being emitted + the remainder of the bundle.
        size_t nop_size = js::sandbox::BUNDLE_SIZE - bundle_start;
        MOZ_ASSERT(nop_size > 0 && nop_size < js::sandbox::BUNDLE_SIZE,
                   "Unexpected nop padding size");

        // we expect this to not fail since bundle_space < space and
        // the caller should already ensured that the buffer has space
        // available.
        m_inner_buffer.infallibleGrowByUninitialized(bundle_space);

        m_inner_buffer.ensureSpace(nop_size - bundle_space + space);
        unsigned char* current_start = m_inner_buffer.data() + m_inner_buffer.size() - nop_size;

        bundle_length -= bundle_start;
        m_inner_buffer.infallibleAppend(current_start, bundle_length);

        memcpy(current_start, nops[nop_size], nop_size);
        
        // the current instruction starts the bundle.
        bundle_start = 0;
    }
  }
  
  void ensureExactBundleSpace(size_t space) {
    ensureBundleSpace(space);

    // need to fill up the current bundle with NOPs until only "space" bytes are left.
    size_t nop_size = js::sandbox::BUNDLE_SIZE - bundle_length - space;
    if (!oom() && nop_size > 0) {
      m_inner_buffer.ensureSpace(nop_size);
      m_inner_buffer.infallibleAppend(nops[nop_size], nop_size);
      bundle_length += nop_size;
    }
  }

  void makeBundleSpace(size_t space) {
    MOZ_ASSERT(!in_bundle, "Unexpected to be within a bundle");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected bundle overflow");
    if (MOZ_UNLIKELY(oom())) return;
    size_t bundle_space = js::sandbox::BUNDLE_SIZE - bundle_length;
    if (MOZ_UNLIKELY(bundle_space < space)) {
        // NOP padding required to pad out the remainder of the bundle.
        if (bundle_space > 0) {
          m_inner_buffer.ensureSpace(bundle_space);
          m_inner_buffer.infallibleAppend(nops[bundle_space], bundle_space);
        }

        // we are at a new bundle now.
        bundle_length = 0;
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
    MOZ_ASSERT(bundle_length == 0 || bundle_length == js::sandbox::BUNDLE_SIZE,
               "Unexpected pending bundle");
    return m_inner_buffer.append(values, size);
  }

  size_t size() const {
    return m_inner_buffer.size();
  }

  bool oom() const { return m_inner_buffer.oom(); }

  bool reserve(size_t size) { return m_inner_buffer.reserve(size); }

  bool swap(Vector<uint8_t, 0, SystemAllocPolicy>& bytes) {
    MOZ_ASSERT(!in_bundle, "Unexpected to be in bundle");
    MOZ_ASSERT(oom() || bundle_length == 0 ||
                   bundle_length == js::sandbox::BUNDLE_SIZE,
               "Unexpected pending bundle");
    return m_inner_buffer.swap(bytes);
  }

  const unsigned char* buffer() const {
    MOZ_ASSERT(!in_bundle, "Unexpected to be in bundle");
    return m_inner_buffer.buffer();
  }

  unsigned char* data() {
    return m_inner_buffer.data();
  }

  MOZ_ALWAYS_INLINE bool beginBundleInstruction() {
    if (in_bundle) return false;
    MOZ_ASSERT(mode == BundleMode::Instruction,
               "Expected instruction bundling");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected unprocessed full bundle");

    bundle_length %= js::sandbox::BUNDLE_SIZE;
    bundle_start = bundle_length;
    in_bundle = true;
    return true;
  }

  MOZ_ALWAYS_INLINE void endBundleInstruction() {
    MOZ_ASSERT(in_bundle, "Unexpected instruction end outside instruction");
    MOZ_ASSERT(mode == BundleMode::Instruction,
               "Expected instruction bundling mode");
    MOZ_ASSERT(oom() || bundle_length - bundle_start > 0,
               "Unexpected 0 length instruction");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected oversized bundle");

    in_bundle = false;
    MOZ_ASSERT_IF(!oom() && bundle_length == js::sandbox::BUNDLE_SIZE,
                  m_inner_buffer.size() % js::sandbox::BUNDLE_SIZE == 0);
  }

  MOZ_ALWAYS_INLINE bool beginBundleGroup() {
    if (in_bundle) return false;
    MOZ_ASSERT(mode == BundleMode::Instruction,
               "Expected instruction bundling");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected unprocessed full bundle");

    in_bundle = true;
    mode = BundleMode::Group;
    bundle_length %= js::sandbox::BUNDLE_SIZE;
    bundle_start = bundle_length;
    return true;
  }

  MOZ_ALWAYS_INLINE bool inBundleGroup() {
    return in_bundle && mode == BundleMode::Group;
  }

  MOZ_ALWAYS_INLINE void endBundleGroup() {
    MOZ_ASSERT(in_bundle, "Unexpected bundle group end outside bundle");
    MOZ_ASSERT(mode == BundleMode::Group, "Expected group bundling mode");
    MOZ_ASSERT(oom() || bundle_length - bundle_start > 0,
               "Unexpected 0 length group");
    MOZ_ASSERT(oom() || bundle_length <= js::sandbox::BUNDLE_SIZE,
               "Unexpected oversized bundle");

    in_bundle = false;
#ifdef DEBUG
    frozen_bundle = false;
#endif
    mode = BundleMode::Instruction;
    MOZ_ASSERT_IF(!oom() && bundle_length == js::sandbox::BUNDLE_SIZE,
                  m_inner_buffer.size() % js::sandbox::BUNDLE_SIZE == 0);
  }

  MOZ_ALWAYS_INLINE void pauseBundleGroup() {
    MOZ_ASSERT(in_bundle, "Unexpected bundle group pause outside bundle");
    MOZ_ASSERT(mode == BundleMode::Group, "Expected group bundling mode");
    MOZ_ASSERT(oom() || bundle_length - bundle_start == 0,
               "Unexpected non-0 length bundle group pause");

    in_bundle = false;
    mode = BundleMode::Instruction;
  }

#ifdef DEBUG
  MOZ_ALWAYS_INLINE void freezeBundleGroup() {
    MOZ_ASSERT(in_bundle, "Unexpected bundle group freeze outside bundle");
    if (mode == BundleMode::Group)
      frozen_bundle = true;
  }
#endif

  MOZ_ALWAYS_INLINE size_t bundleOffset() {
    MOZ_ASSERT(in_bundle, "Expected to be within a bundle");
    return bundle_length - bundle_start;
  }

 protected:

  unsigned char bundle_buffer[js::sandbox::BUNDLE_SIZE];
  
  BundleMode mode;
  size_t bundle_length;
  bool in_bundle;
  bool frozen_bundle;
  size_t bundle_start;

  static constexpr unsigned char nops[32][32] = {
      // 0
      {},
      // 1
      {0x90},
      // 2
      {0x66, 0x90},
      // 3
      {0x0f, 0x1f, 0x00},
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
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x66,
       0x90},
      // 14
      {0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
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
       0x00, 0x00, 0x00, 0x00, 0x0f,  0x1f, 0x00},
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
