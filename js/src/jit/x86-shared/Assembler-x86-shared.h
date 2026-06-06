/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_x86_shared_Assembler_x86_shared_h
#define jit_x86_shared_Assembler_x86_shared_h

#include <cstddef>
#include <cstdint>

#include "jit/JitCode.h"
#include "jit/shared/Assembler-shared.h"

#if defined(JS_CODEGEN_X86)
#  include "jit/x86/BaseAssembler-x86.h"
#elif defined(JS_CODEGEN_X64)
#  include "jit/x64/BaseAssembler-x64.h"
#else
#  error "Unknown architecture!"
#endif
#include "jit/CompactBuffer.h"
#include "wasm/WasmTypeDecls.h"

namespace js {
namespace jit {

// Do not reference ScratchFloat32Reg_ directly, use ScratchFloat32Scope
// instead.
struct ScratchFloat32Scope : public AutoFloatRegisterScope {
  explicit ScratchFloat32Scope(MacroAssembler& masm)
      : AutoFloatRegisterScope(masm, ScratchFloat32Reg_) {}
};

// Do not reference ScratchDoubleReg_ directly, use ScratchDoubleScope instead.
struct ScratchDoubleScope : public AutoFloatRegisterScope {
  explicit ScratchDoubleScope(MacroAssembler& masm)
      : AutoFloatRegisterScope(masm, ScratchDoubleReg_) {}
};

struct ScratchSimd128Scope : public AutoFloatRegisterScope {
  explicit ScratchSimd128Scope(MacroAssembler& masm)
      : AutoFloatRegisterScope(masm, ScratchSimd128Reg) {}
};

class AssemblerX86Shared;

class AutoBundleInstructionScope {
 private:
#ifdef JS_SANDBOX_BUNDLE
  AssemblerX86Shared& masm;

  // Track whether this instance is nested within another.
  // This happens when instructions have lock prefixes etc.
  bool nested_;

  // Track whether the instruction bundle has been ended using end().
  // Used to decide whether the destructor needs to actually mark the end of the
  // instruction.
  bool active_;
#endif
 public:
  AutoBundleInstructionScope(AssemblerX86Shared& masm);
  void end();
  ~AutoBundleInstructionScope();
};

class AutoBundleGroupScope {
 private:
  AssemblerX86Shared& masm;

#ifdef JS_SANDBOX_BUNDLE
  // Track whether this instance is nested within another.
  // This happens when instructions have lock prefixes etc.
  bool nested_;

  // Track whether the instruction bundle has been ended using end().
  // Used to decide whether the destructor needs to actually mark the end of the
  // instruction.
  bool active_;
#endif
 public:
  AutoBundleGroupScope(AssemblerX86Shared& masm);
  void ensureSpace(size_t space);
  void nopToEnd(size_t space);
  void nopAndEnd();
  void end();
  void freeze();
  size_t offset();
  ~AutoBundleGroupScope();
};

class Operand {
 public:
  enum Kind { REG, MEM_REG_DISP, FPREG, MEM_SCALE, MEM_ADDRESS32 };

 private:
  Kind kind_ : 4;
  // Used as a Register::Encoding and a FloatRegister::Encoding.
  uint32_t base_ : 5;
  Scale scale_ : 3;
  // We don't use all 8 bits, of course, but GCC complains if the size of
  // this field is smaller than the size of Register::Encoding.
  Register::Encoding index_ : 8;
  int32_t disp_;
#ifdef JS_SANDBOX
  bool sandboxed_ = false;
  bool clobberScratch_ = false;
#endif

 public:
  explicit Operand(Register reg)
      : kind_(REG),
        base_(reg.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(0) {}
  explicit Operand(FloatRegister reg)
      : kind_(FPREG),
        base_(reg.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(0) {}
#ifdef JS_SANDBOX
  explicit Operand(const Address& address)
      : kind_(MEM_REG_DISP),
        base_(address.base.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(address.offset),
        sandboxed_(address.sandboxed),
        clobberScratch_(address.clobberScratch) {
    MOZ_ASSERT_IF(clobberScratch_, base_ == SandboxScratchReg.encoding());
  }
  explicit Operand(const BaseIndex& address)
      : kind_(MEM_SCALE),
        base_(address.base.encoding()),
        scale_(address.scale),
        index_(address.index.encoding()),
        disp_(address.offset),
        sandboxed_(address.sandboxed),
        clobberScratch_(address.clobberScratch) {
    MOZ_ASSERT_IF(clobberScratch_, base_ == SandboxScratchReg.encoding() ||
                                       index_ == SandboxScratchReg.encoding());
  }
  Operand(Register base, Register index, Scale scale, int32_t disp = 0,
          bool sandboxed = false, bool clobberScratch = false)
      : kind_(MEM_SCALE),
        base_(base.encoding()),
        scale_(scale),
        index_(index.encoding()),
        disp_(disp),
        sandboxed_(sandboxed),
        clobberScratch_(clobberScratch) {
    MOZ_ASSERT_IF(clobberScratch_, base_ == SandboxScratchReg.encoding() ||
                                       index_ == SandboxScratchReg.encoding());
  }
  Operand(Register reg, int32_t disp, bool sandboxed = false,
          bool clobberScratch = false)
      : kind_(MEM_REG_DISP),
        base_(reg.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(disp),
        sandboxed_(sandboxed),
        clobberScratch_(clobberScratch) {
    MOZ_ASSERT_IF(clobberScratch_, base_ == SandboxScratchReg.encoding());
  }
#else
  explicit Operand(const Address& address)
      : kind_(MEM_REG_DISP),
        base_(address.base.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(address.offset) {}
  explicit Operand(const BaseIndex& address)
      : kind_(MEM_SCALE),
        base_(address.base.encoding()),
        scale_(address.scale),
        index_(address.index.encoding()),
        disp_(address.offset) {}
  Operand(Register base, Register index, Scale scale, int32_t disp = 0)
      : kind_(MEM_SCALE),
        base_(base.encoding()),
        scale_(scale),
        index_(index.encoding()),
        disp_(disp) {}
  Operand(Register reg, int32_t disp)
      : kind_(MEM_REG_DISP),
        base_(reg.encoding()),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(disp) {}
#endif
  explicit Operand(AbsoluteAddress address)
      : kind_(MEM_ADDRESS32),
        base_(Registers::Invalid),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(X86Encoding::AddressImmediate(address.addr)) {}
  explicit Operand(PatchedAbsoluteAddress address)
      : kind_(MEM_ADDRESS32),
        base_(Registers::Invalid),
        scale_(TimesOne),
        index_(Registers::Invalid),
        disp_(X86Encoding::AddressImmediate(address.addr)) {}

  Address toAddress() const {
    MOZ_ASSERT(kind() == MEM_REG_DISP);
#ifdef JS_SANDBOX
    return Address(Register::FromCode(base()), disp(), clobberScratch_, sandboxed_);
#else
    return Address(Register::FromCode(base()), disp());
#endif
  }

  BaseIndex toBaseIndex() const {
    MOZ_ASSERT(kind() == MEM_SCALE);
#ifdef JS_SANDBOX
    return BaseIndex(Register::FromCode(base()), Register::FromCode(index()),
                     scale(), disp(), clobberScratch_, sandboxed_);
#else
    return BaseIndex(Register::FromCode(base()), Register::FromCode(index()),
                     scale(), disp());
#endif
  }

  Kind kind() const { return kind_; }
  Register::Encoding reg() const {
    MOZ_ASSERT(kind() == REG);
    return Register::Encoding(base_);
  }
  Register::Encoding base() const {
    MOZ_ASSERT(kind() == MEM_REG_DISP || kind() == MEM_SCALE);
    return Register::Encoding(base_);
  }
  Register::Encoding index() const {
    MOZ_ASSERT(kind() == MEM_SCALE);
    return index_;
  }
  Scale scale() const {
    MOZ_ASSERT(kind() == MEM_SCALE);
    return scale_;
  }
  FloatRegister::Encoding fpu() const {
    MOZ_ASSERT(kind() == FPREG);
    return FloatRegister::Encoding(base_);
  }
  int32_t disp() const {
    MOZ_ASSERT(kind() == MEM_REG_DISP || kind() == MEM_SCALE);
    return disp_;
  }
  void* address() const {
    MOZ_ASSERT(kind() == MEM_ADDRESS32);
    return reinterpret_cast<void*>(disp_);
  }

  bool containsReg(Register r) const {
    switch (kind()) {
      case REG:
        return r.encoding() == reg();
      case MEM_REG_DISP:
        return r.encoding() == base();
      case MEM_SCALE:
        return r.encoding() == base() || r.encoding() == index();
      default:
        return false;
    }
  }

#ifdef JS_SANDBOX
  bool sandboxed() const { return sandboxed_; }
  bool clobberScratch() const { return clobberScratch_; }
#endif
};

inline Imm32 Imm64::firstHalf() const { return low(); }

inline Imm32 Imm64::secondHalf() const { return hi(); }

class CPUInfo {
 public:
  // As the SSE's were introduced in order, the presence of a later SSE implies
  // the presence of an earlier SSE. For example, SSE4_2 support implies SSE2
  // support.
  enum SSEVersion {
    UnknownSSE = 0,
    NoSSE = 1,
    SSE = 2,
    SSE2 = 3,
    SSE3 = 4,
    SSSE3 = 5,
    SSE4_1 = 6,
    SSE4_2 = 7
  };
  static const int AVX_PRESENT_BIT = 8;

  static SSEVersion GetSSEVersion() {
    MOZ_ASSERT(FlagsHaveBeenComputed());
    MOZ_ASSERT_IF(maxEnabledSSEVersion != UnknownSSE,
                  maxSSEVersion <= maxEnabledSSEVersion);
    return maxSSEVersion;
  }

  static bool IsAVXPresent() {
    MOZ_ASSERT(FlagsHaveBeenComputed());
    MOZ_ASSERT_IF(!avxEnabled, !avxPresent);
    return avxPresent;
  }

  static inline uint32_t GetFingerprint() {
    return GetSSEVersion() | (IsAVXPresent() ? AVX_PRESENT_BIT : 0);
  }

 private:
  static SSEVersion maxSSEVersion;
  static SSEVersion maxEnabledSSEVersion;
  static bool avxPresent;
  static bool avxEnabled;
  static bool popcntPresent;
  static bool bmi1Present;
  static bool bmi2Present;
  static bool lzcntPresent;
  static bool fmaPresent;
  static bool avx2Present;

  static void SetMaxEnabledSSEVersion(SSEVersion v) {
    if (maxEnabledSSEVersion == UnknownSSE) {
      maxEnabledSSEVersion = v;
    } else {
      maxEnabledSSEVersion = std::min(v, maxEnabledSSEVersion);
    }
  }

 public:
  static bool IsSSE2Present() {
#ifdef JS_CODEGEN_X64
    return true;
#else
    return GetSSEVersion() >= SSE2;
#endif
  }
  static bool IsSSE3Present() { return GetSSEVersion() >= SSE3; }
  static bool IsSSSE3Present() { return GetSSEVersion() >= SSSE3; }
  static bool IsSSE41Present() { return GetSSEVersion() >= SSE4_1; }
  static bool IsSSE42Present() { return GetSSEVersion() >= SSE4_2; }
  static bool IsPOPCNTPresent() { return popcntPresent; }
  static bool IsBMI1Present() { return bmi1Present; }
  static bool IsBMI2Present() { return bmi2Present; }
  static bool IsLZCNTPresent() { return lzcntPresent; }
  static bool IsFMAPresent() { return fmaPresent; }
  static bool IsAVX2Present() { return avx2Present; }

  static bool FlagsHaveBeenComputed() { return maxSSEVersion != UnknownSSE; }

  static void ComputeFlags();

  // The following should be called only before JS_Init (where the flags are
  // computed). If several are called, the most restrictive setting is kept.

  static void SetSSE3Disabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    SetMaxEnabledSSEVersion(SSE2);
    avxEnabled = false;
  }
  static void SetSSSE3Disabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    SetMaxEnabledSSEVersion(SSE3);
    avxEnabled = false;
  }
  static void SetSSE41Disabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    SetMaxEnabledSSEVersion(SSSE3);
    avxEnabled = false;
  }
  static void SetSSE42Disabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    SetMaxEnabledSSEVersion(SSE4_1);
    avxEnabled = false;
  }
  static void SetAVXDisabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    avxEnabled = false;
  }
  static void SetAVXEnabled() {
    MOZ_ASSERT(!FlagsHaveBeenComputed());
    avxEnabled = true;
  }
};

class AssemblerX86Shared : public AssemblerShared {
 protected:
  struct RelativePatch {
    int32_t offset;
    void* target;
    RelocationKind kind;

    RelativePatch(int32_t offset, void* target, RelocationKind kind)
        : offset(offset), target(target), kind(kind) {}
  };

  Vector<std::pair<CodeOffset, ImmGCPtr>, 8, SystemAllocPolicy> dataGCSection_;
  Vector<JitCode*, 8, SystemAllocPolicy> dataJitSection_;
  Vector<std::pair<CodeOffset, Value>, 8, SystemAllocPolicy> dataValueSection_;

  void writeDataSection(CodeOffset offset, ImmGCPtr ptr) {
    if (gc::IsInsideNursery(ptr.value)) {
      embedsNurseryPointers_ = true;
    }
    if (!dataGCSection_.append(std::pair(offset, ptr))) {
      enoughMemory_ = false;
    }
  }

  void writeDataSection(JitCode* code) {
    if (!dataJitSection_.append(code)) {
      enoughMemory_ = false;
    }
  }

 protected:
  X86Encoding::BaseAssemblerSpecific masm;

  using JmpSrc = X86Encoding::JmpSrc;
  using JmpDst = X86Encoding::JmpDst;

  friend class AutoBundleInstructionScope;
  friend class AutoBundleGroupScope;

#ifdef JS_SANDBOX_BUNDLE
  inline bool beginBundleInstruction() { return masm.beginBundleInstruction(); }

  inline void endBundleInstruction() { masm.endBundleInstruction(); }

  inline bool endAndBeginBundleInstruction() {
    masm.endBundleInstruction();
    return masm.beginBundleInstruction();
  }

  inline bool beginBundleGroup() { return masm.beginBundleGroup(); }

  inline bool inBundleGroup() { return masm.inBundleGroup(); }

  inline void endBundleGroup() { masm.endBundleGroup(); }
  
  inline void nopAndEndBundleGroup() { masm.nopAndEndBundleGroup(); }

  inline void pauseBundleGroup() { masm.pauseBundleGroup(); }

  inline void freezeBundleGroup() { masm.freezeBundleGroup(); }

  inline void ensureBundleSpace(size_t space) { masm.ensureBundleSpace(space); }

  inline void ensureExactBundleSpace(size_t space) {
    masm.ensureExactBundleSpace(space);
  }

  inline size_t bundleOffset() { return masm.bundleOffset(); }
#else
  inline bool beginBundleInstruction() { return false; }
  inline void endBundleInstruction() {}
  inline bool endAndBeginBundleInstruction() { return false; }
  inline bool beginBundleGroup() { return false; }
  inline void pauseBundleGroup() {}
  inline void freezeBundleGroup() {}
#endif

  Operand sandboxMemoryWrite(const Operand& op) {
#ifdef JS_SANDBOX_HEAP
    if (op.sandboxed()) {
      return op;
    }
#ifdef JS_SANDBOX_BUNDLE
    MOZ_ASSERT(inBundleGroup(), "Expected to be in bundle when masking stores");
    MOZ_ASSERT(bundleOffset() == 0, "Expected to be at the beginning of the bundle");
#endif
#ifdef DEBUG
    Label sandboxed;
#endif
    switch (op.kind()) {
      case Operand::REG:
      case Operand::FPREG:
        return op;
      case Operand::MEM_SCALE:
        MOZ_ASSERT(!op.containsReg(SandboxScratchReg) || op.clobberScratch(),
                   "Must allow ScratchReg to be clobbered");
        pauseBundleGroup();
        beginBundleInstruction();
        masm.leaq_mr(op.disp(), op.base(), op.index(), op.scale(),
                     SandboxScratchReg.encoding());
        endBundleInstruction();
#ifdef DEBUG
        beginBundleInstruction();
        masm.push_r(SandboxScratchReg.encoding());
        endAndBeginBundleInstruction();
        masm.andq_rr(SandboxMaskReg.encoding(), SandboxScratchReg.encoding());
        endAndBeginBundleInstruction();
        masm.orq_rr(SandboxBaseReg.encoding(), SandboxScratchReg.encoding());
        endAndBeginBundleInstruction();
        masm.cmpq_rm(SandboxScratchReg.encoding(), 0, StackPointer.encoding());
        endAndBeginBundleInstruction();
        j(Condition::Equal, &sandboxed);
        endBundleInstruction();
        breakpoint();
        bind(&sandboxed);
        beginBundleInstruction();
        masm.pop_r(SandboxScratchReg.encoding());
        endBundleInstruction();
#endif
        beginBundleGroup();
        masm.andq_rr(SandboxMaskReg.encoding(), SandboxScratchReg.encoding());
        return Operand(SandboxBaseReg, SandboxScratchReg, TimesOne, 0, true);
      case Operand::MEM_REG_DISP:
        if (op.containsReg(StackPointer)) {
          return op;
        }
#ifdef DEBUG
        pauseBundleGroup();
        beginBundleInstruction();
        masm.push_r(op.base());
        endAndBeginBundleInstruction();
        masm.andq_rr(SandboxMaskReg.encoding(), op.base());
        endAndBeginBundleInstruction();
        masm.orq_rr(SandboxBaseReg.encoding(), op.base());
        endAndBeginBundleInstruction();
        masm.cmpq_rm(op.base(), 0, StackPointer.encoding());
        endAndBeginBundleInstruction();
        j(Condition::Equal, &sandboxed);
        endBundleInstruction();
        breakpoint();
        bind(&sandboxed);
        beginBundleInstruction();
        masm.pop_r(op.base());
        endBundleInstruction();
        beginBundleGroup();
#endif
        masm.andq_rr(SandboxMaskReg.encoding(), op.base());
        masm.orq_rr(SandboxBaseReg.encoding(), op.base());
        return Operand(Register(op.base()), op.disp(), true);
      default:
        MOZ_CRASH("unexpected operand kind");
    }
#else
    return op;
#endif
  }

  void sandboxStackPointer(Register dest) {
#ifdef JS_SANDBOX_BUNDLE
      MOZ_ASSERT(
          inBundleGroup(),
          "Expected to be in bundle when masking stack pointer modifications");
#endif
#ifdef JS_SANDBOX_HEAP
    if (dest == StackPointer) {
      masm.andq_rr(SandboxMaskReg.encoding(), StackPointer.encoding());
      masm.orq_rr(SandboxBaseReg.encoding(), StackPointer.encoding());
    }
#endif
  }

 public:
  AssemblerX86Shared() {
    if (!HasAVX()) {
      masm.disableVEX();
    }
  }

  enum Condition {
    Equal = X86Encoding::ConditionE,
    NotEqual = X86Encoding::ConditionNE,
    Above = X86Encoding::ConditionA,
    AboveOrEqual = X86Encoding::ConditionAE,
    Below = X86Encoding::ConditionB,
    BelowOrEqual = X86Encoding::ConditionBE,
    GreaterThan = X86Encoding::ConditionG,
    GreaterThanOrEqual = X86Encoding::ConditionGE,
    LessThan = X86Encoding::ConditionL,
    LessThanOrEqual = X86Encoding::ConditionLE,
    Overflow = X86Encoding::ConditionO,
    NoOverflow = X86Encoding::ConditionNO,
    CarrySet = X86Encoding::ConditionC,
    CarryClear = X86Encoding::ConditionNC,
    Signed = X86Encoding::ConditionS,
    NotSigned = X86Encoding::ConditionNS,
    Zero = X86Encoding::ConditionE,
    NonZero = X86Encoding::ConditionNE,
    Parity = X86Encoding::ConditionP,
    NoParity = X86Encoding::ConditionNP
  };

  enum class SSERoundingMode {
    Nearest = int(X86Encoding::SSERoundingMode::RoundToNearest),
    Floor = int(X86Encoding::SSERoundingMode::RoundDown),
    Ceil = int(X86Encoding::SSERoundingMode::RoundUp),
    Trunc = int(X86Encoding::SSERoundingMode::RoundToZero)
  };

  // If this bit is set, the vucomisd operands have to be inverted.
  static const int DoubleConditionBitInvert = 0x10;

  // Bit set when a DoubleCondition does not map to a single x86 condition.
  // The macro assembler has to special-case these conditions.
  static const int DoubleConditionBitSpecial = 0x20;
  static const int DoubleConditionBits =
      DoubleConditionBitInvert | DoubleConditionBitSpecial;

  enum DoubleCondition {
    // These conditions will only evaluate to true if the comparison is ordered
    // - i.e. neither operand is NaN.
    DoubleOrdered = NoParity,
    DoubleEqual = Equal | DoubleConditionBitSpecial,
    DoubleNotEqual = NotEqual,
    DoubleGreaterThan = Above,
    DoubleGreaterThanOrEqual = AboveOrEqual,
    DoubleLessThan = Above | DoubleConditionBitInvert,
    DoubleLessThanOrEqual = AboveOrEqual | DoubleConditionBitInvert,
    // If either operand is NaN, these conditions always evaluate to true.
    DoubleUnordered = Parity,
    DoubleEqualOrUnordered = Equal,
    DoubleNotEqualOrUnordered = NotEqual | DoubleConditionBitSpecial,
    DoubleGreaterThanOrUnordered = Below | DoubleConditionBitInvert,
    DoubleGreaterThanOrEqualOrUnordered =
        BelowOrEqual | DoubleConditionBitInvert,
    DoubleLessThanOrUnordered = Below,
    DoubleLessThanOrEqualOrUnordered = BelowOrEqual
  };

  enum NaNCond { NaN_HandledByCond, NaN_IsTrue, NaN_IsFalse };

  // If the primary condition returned by ConditionFromDoubleCondition doesn't
  // handle NaNs properly, return NaN_IsFalse if the comparison should be
  // overridden to return false on NaN, NaN_IsTrue if it should be overridden
  // to return true on NaN, or NaN_HandledByCond if no secondary check is
  // needed.
  static inline NaNCond NaNCondFromDoubleCondition(DoubleCondition cond) {
    switch (cond) {
      case DoubleOrdered:
      case DoubleNotEqual:
      case DoubleGreaterThan:
      case DoubleGreaterThanOrEqual:
      case DoubleLessThan:
      case DoubleLessThanOrEqual:
      case DoubleUnordered:
      case DoubleEqualOrUnordered:
      case DoubleGreaterThanOrUnordered:
      case DoubleGreaterThanOrEqualOrUnordered:
      case DoubleLessThanOrUnordered:
      case DoubleLessThanOrEqualOrUnordered:
        return NaN_HandledByCond;
      case DoubleEqual:
        return NaN_IsFalse;
      case DoubleNotEqualOrUnordered:
        return NaN_IsTrue;
    }

    MOZ_CRASH("Unknown double condition");
  }

  static void StaticAsserts() {
    // DoubleConditionBits should not interfere with x86 condition codes.
    static_assert(!((Equal | NotEqual | Above | AboveOrEqual | Below |
                     BelowOrEqual | Parity | NoParity) &
                    DoubleConditionBits));
  }

  static Condition InvertCondition(Condition cond);
  static Condition UnsignedCondition(Condition cond);
  static Condition ConditionWithoutEqual(Condition cond);

  static DoubleCondition InvertCondition(DoubleCondition cond);

  // Return the primary condition to test. Some primary conditions may not
  // handle NaNs properly and may therefore require a secondary condition.
  // Use NaNCondFromDoubleCondition to determine what else is needed.
  static inline Condition ConditionFromDoubleCondition(DoubleCondition cond) {
    return static_cast<Condition>(cond & ~DoubleConditionBits);
  }

  static void TraceDataRelocations(JSTracer* trc, JitCode* code,
                                   CompactBufferReader& reader);
  static void TraceDataSection(JSTracer* trc, JitCode* code);

  inline void makeBundleSpace(size_t space) {
#ifdef JS_SANDBOX_BUNDLE
    masm.makeBundleSpace(space);
#endif
  }

  void setUnlimitedBuffer() {
    // No-op on this platform
  }
  bool oom() const {
    return AssemblerShared::oom() || masm.oom();
  }
  bool reserve(size_t size) { return masm.reserve(size); }
  bool swapBuffer(wasm::Bytes& other) { return masm.swapBuffer(other); }

  void setPrinter(Sprinter* sp) { masm.setPrinter(sp); }

  Register getStackPointer() const { return StackPointer; }

  void executableCopy(void* buffer);
  void processCodeLabels(uint8_t* rawCode);
  void processDataLabels(uint8_t* rawCode, JitCode* code);
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
  void processCodeLabelsInPlace(uint8_t* rawCode);
  void processDataLabelsInPlace(uint8_t* rawCode, JitCode* code);
#endif
  void copyJumpRelocationTable(uint8_t* dest);
  void copyDataRelocationTable(uint8_t* dest);
  void copyDataSection(uint8_t* dest);

  // Size of the instruction stream, in bytes.
  size_t size() const { return masm.size(); }
  // Size of the jump relocation table, in bytes.
  size_t dataSectionBytes() const {
    return dataGCSection_.length() * sizeof(void*) +
           dataValueSection_.length() * sizeof(Value) +
           dataJitSection_.length() * sizeof(JitCode*);
  }

  // Size of executable code, in bytes.
  size_t execSize() const { return size(); }

  // Size of the data table, in bytes.
  size_t dataSize() const {
    return dataSectionBytes();
  }

 public:
  void haltingAlign(int alignment) {
    MOZ_ASSERT(hasCreator());
    masm.haltingAlign(alignment);
  }
  void haltingAlignOne(int alignment) {
    MOZ_ASSERT(hasCreator());
    masm.haltingAlignOne(alignment);
  }
  void nopAlign(int alignment) {
    MOZ_ASSERT(hasCreator());
    masm.nopAlign(alignment);
  }
  void writeCodePointer(CodeLabel* label) {
    MOZ_ASSERT(hasCreator());
    // Use -1 as dummy value. This will be patched after codegen.
    AutoBundleInstructionScope bundle(*this);
    masm.jumpTablePointer(-1);
    bundle.end();
    label->patchAt()->bind(masm.size());
  }
  void cmovCCl(Condition cond, const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    X86Encoding::Condition cc = static_cast<X86Encoding::Condition>(cond);
    switch (src.kind()) {
      case Operand::REG:
        masm.cmovCCl_rr(cc, src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmovCCl_mr(cc, src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.cmovCCl_mr(cc, src.disp(), src.base(), src.index(), src.scale(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void cmovCCl(Condition cond, Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    X86Encoding::Condition cc = static_cast<X86Encoding::Condition>(cond);
    masm.cmovCCl_rr(cc, src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void cmovzl(const Operand& src, Register dest) {
    cmovCCl(Condition::Zero, src, dest);
  }
  void cmovnzl(const Operand& src, Register dest) {
    cmovCCl(Condition::NonZero, src, dest);
  }
  void movl(Imm32 imm32, Register dest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    masm.movl_i32r(imm32.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void movl(Register src, Register dest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    masm.movl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  CodeOffset movl(const Operand& src, Register dest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    size_t preOffset = bundle.offset();
    switch (src.kind()) {
      case Operand::REG:
        masm.movl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.movl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movl_mr(src.disp(), src.base(), src.index(), src.scale(),
                     dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.movl_mr(src.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(dest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset movl(Register src, const Operand& unsafeDest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::REG:
        masm.movl_rr(src.encoding(), dest.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.movl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.movl_rm(src.encoding(), dest.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    if (dest.kind() ==  Operand::REG) {
      sandboxStackPointer(Register(dest.reg()));
    }
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset movl(Imm32 imm32, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::REG:
        masm.movl_i32r(imm32.value, dest.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.movl_i32m(imm32.value, dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movl_i32m(imm32.value, dest.disp(), dest.base(), dest.index(),
                       dest.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.movl_i32m(imm32.value, dest.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    if (dest.kind() == Operand::REG) {
      sandboxStackPointer(Register(dest.reg()));
    }
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  void xchgl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.xchgl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(src);
    sandboxStackPointer(dest);
  }

  void vmovapd(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovapd_rr(src.encoding(), dest.encoding());
  }
  // Eventually vmovapd should be overloaded to support loads and
  // stores too.
  void vmovapd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vmovapd_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vmovaps(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovaps_rr(src.encoding(), dest.encoding());
  }
  void vmovaps(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovaps_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovaps_mr(src.disp(), src.base(), src.index(), src.scale(),
                        dest.encoding());
        break;
      case Operand::FPREG:
        masm.vmovaps_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovaps(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovaps_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovaps_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovups(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovups_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovups_mr(src.disp(), src.base(), src.index(), src.scale(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  CodeOffset vmovups(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovups_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovups_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  void vmovsd(const Address& src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovsd_mr(src.offset, src.base.encoding(), dest.encoding());
  }
  void vmovsd(const BaseIndex& src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovsd_mr(src.offset, src.base.encoding(), src.index.encoding(),
                   src.scale, dest.encoding());
  }
  void vmovsd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        vmovsd(src.toAddress(), dest);
        break;
      case Operand::MEM_SCALE:
        vmovsd(src.toBaseIndex(), dest);
        break;
      default:
        MOZ_CRASH("Unknown operand for vmovsd");
    }
  }
  CodeOffset vmovsd(FloatRegister src, const Address& unsafeDest) {
#ifdef JS_SANDBOX_HEAP
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(Operand(unsafeDest));
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovsd_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovsd_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                       dest.scale());
        break;
      default:
        MOZ_CRASH("Unknown operand for vmovsd");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
#else
    AutoBundleGroupScope bundle(*this);
    size_t preOffset = bundle.offset();
    masm.vmovsd_rm(src.encoding(), unsafeDest.offset,
                   unsafeDest.base.encoding());
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
#endif
  }
  CodeOffset vmovsd(FloatRegister src, const BaseIndex& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(Operand(unsafeDest));
    size_t preOffset = bundle.offset();
    MOZ_ASSERT(dest.kind() == Operand::MEM_SCALE);
    masm.vmovsd_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                   dest.scale());
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  // Note special semantics of this - does not clobber high bits of destination.
  void vmovsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmovss(const Address& src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovss_mr(src.offset, src.base.encoding(), dest.encoding());
  }
  void vmovss(const BaseIndex& src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovss_mr(src.offset, src.base.encoding(), src.index.encoding(),
                   src.scale, dest.encoding());
  }
  void vmovss(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        vmovss(src.toAddress(), dest);
        break;
      case Operand::MEM_SCALE:
        vmovss(src.toBaseIndex(), dest);
        break;
      default:
        MOZ_CRASH("Unknown operand for vmovss");
    }
  }
  CodeOffset vmovss(FloatRegister src, const Address& unsafeDest) {
#ifdef JS_SANDBOX_HEAP
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(Operand(unsafeDest));
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovss_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovss_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                       dest.scale());
        break;
      default:
        MOZ_CRASH("Unknown operand for vmovss");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
#else
    AutoBundleGroupScope bundle(*this);
    size_t preOffset = bundle.offset();
    masm.vmovss_rm(src.encoding(), unsafeDest.offset,
                   unsafeDest.base.encoding());
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
#endif
  }
  CodeOffset vmovss(FloatRegister src, const BaseIndex& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(Operand(unsafeDest));
    size_t preOffset = bundle.offset();
    MOZ_ASSERT(dest.kind() == Operand::MEM_SCALE);
    masm.vmovss_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                   dest.scale());
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void vmovss(FloatRegister src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        vmovss(src, dest.toAddress());
        break;
      case Operand::MEM_SCALE:
        vmovss(src, dest.toBaseIndex());
        break;
      default:
        MOZ_CRASH("Unknown operand for vmovss");
    }
  }
  // Note special semantics of this - does not clobber high bits of destination.
  void vmovss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vmovss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmovdqu(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovdqu_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovdqu_mr(src.disp(), src.base(), src.index(), src.scale(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovdqu(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovdqu_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovdqu_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovdqa(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vmovdqa_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmovdqa_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovdqa_mr(src.disp(), src.base(), src.index(), src.scale(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovdqa(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovdqa_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovdqa_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovdqa(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovdqa_rr(src.encoding(), dest.encoding());
  }
  void vcvtss2sd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtss2sd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vcvtsd2ss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtsd2ss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void movzbl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movzbl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movzbl_mr(src.disp(), src.base(), src.index(), src.scale(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void movsbl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.movsbl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void movsbl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movsbl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movsbl_mr(src.disp(), src.base(), src.index(), src.scale(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  CodeOffset movb(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    size_t preOffset = bundle.offset();
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movb_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movb_mr(src.disp(), src.base(), src.index(), src.scale(),
                     dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(dest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void movb(Imm32 src, Register dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.movb_ir(src.value & 255, dest.encoding());
  }
  CodeOffset movb(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movb_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movb_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset movb(Imm32 src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movb_im(src.value, dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movb_im(src.value, dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void movzwl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.movzwl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.movzwl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movzwl_mr(src.disp(), src.base(), src.index(), src.scale(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void movzwl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.movzwl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  CodeOffset movw(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    size_t preOffset = bundle.offset();
    masm.prefix_16_for_32();
    movl(src, dest);
    size_t postOffset = bundle.offset();
    sandboxStackPointer(dest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void movw(Imm32 src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.prefix_16_for_32();
    movl(src, dest);
    sandboxStackPointer(dest);
  }
  CodeOffset movw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset movw(Imm32 src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    size_t preOffset = bundle.offset();
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movw_im(src.value, dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.movw_im(src.value, dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void movswl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.movswl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void movswl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.movswl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.movswl_mr(src.disp(), src.base(), src.index(), src.scale(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void leal(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.leal_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.leal_mr(src.disp(), src.base(), src.index(), src.scale(),
                     dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }

 protected:
  void jSrc(Condition cond, Label* label) {
    if (label->bound()) {
      AutoBundleInstructionScope bundle(*this);
      // The jump can be immediately encoded to the correct destination.
      masm.jCC_i(static_cast<X86Encoding::Condition>(cond),
                 JmpDst(label->offset()));
      bundle.end();
    } else {
      // Thread the jump list through the unpatched jump targets.
      AutoBundleInstructionScope bundle(*this);
      JmpSrc j = masm.jCC(static_cast<X86Encoding::Condition>(cond));
      bundle.end();
      JmpSrc prev;
      if (label->used()) {
        prev = JmpSrc(label->offset());
      }
      label->use(j.offset());
      masm.setNextJump(j, prev);
    }
  }
  void jmpSrc(Label* label) {
    if (label->bound()) {
      // The jump can be immediately encoded to the correct destination.
      AutoBundleInstructionScope bundle(*this);
      masm.jmp_i(JmpDst(label->offset()));
      bundle.end();
    } else {
      // Thread the jump list through the unpatched jump targets.
      AutoBundleInstructionScope bundle(*this);
      JmpSrc j = masm.jmp();
      bundle.end();
      JmpSrc prev;
      if (label->used()) {
        prev = JmpSrc(label->offset());
      }
      label->use(j.offset());
      masm.setNextJump(j, prev);
    }
  }

  // Comparison of EAX against the address given by a Label.
  JmpSrc cmpSrc(Label* label) {
    AutoBundleInstructionScope bundle(*this);
    JmpSrc j = masm.cmp_eax();
    bundle.end();
    if (label->bound()) {
      // The jump can be immediately patched to the correct destination.
      masm.linkJump(j, JmpDst(label->offset()));
    } else {
      // Thread the jump list through the unpatched jump targets.
      JmpSrc prev;
      if (label->used()) {
        prev = JmpSrc(label->offset());
      }
      label->use(j.offset());
      masm.setNextJump(j, prev);
    }
    return j;
  }

 public:
  void nop() {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    masm.nop();
  }
  void nop(size_t n) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    masm.insert_nop(n);
  }
  void j(Condition cond, Label* label) {
    MOZ_ASSERT(hasCreator());
    jSrc(cond, label);
  }
  void jmp(Label* label) {
    MOZ_ASSERT(hasCreator());
    jmpSrc(label);
  }

  void jmp(const Operand& op) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.jmp_m(op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.jmp_m(op.disp(), op.base(), op.index(), op.scale());
        break;
      case Operand::REG:
        masm.jmp_r(op.reg());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  static size_t JmpSize(Register reg) {
    return X86Encoding::BaseAssembler::jmp_r_size(reg.encoding());
  }
  
  void cmpEAX(Label* label) { cmpSrc(label); }
  void bind(Label* label) {
    JmpDst dst(masm.label());
    if (label->used()) {
      bool more;
      JmpSrc jmp(label->offset());
      do {
        JmpSrc next;
        more = masm.nextJump(jmp, &next);
        masm.linkJump(jmp, dst);
        jmp = next;
      } while (more);
    }
    label->bind(dst.offset());
  }
  void bind(CodeLabel* label) { label->target()->bind(masm.label().offset()); }
  uint32_t currentOffset() { return masm.currentOffset(); }

  // Re-routes pending jumps to a new label.
  void retarget(Label* label, Label* target) {
    if (!label->used()) {
      return;
    }
    bool more;
    JmpSrc jmp(label->offset());
    do {
      JmpSrc next;
      more = masm.nextJump(jmp, &next);
      if (target->bound()) {
        // The jump can be immediately patched to the correct destination.
        masm.linkJump(jmp, JmpDst(target->offset()));
      } else {
        // Thread the jump list through the unpatched jump targets.
        JmpSrc prev;
        if (target->used()) {
          prev = JmpSrc(target->offset());
        }
        target->use(jmp.offset());
        masm.setNextJump(jmp, prev);
      }
      jmp = JmpSrc(next.offset());
    } while (more);
    label->reset();
  }

  static void Bind(uint8_t* raw, const CodeLabel& label) {
    if (label.patchAt().bound()) {
      intptr_t offset = label.patchAt().offset();
      intptr_t target = label.target().offset();
      X86Encoding::SetPointer(raw + offset, raw + target);
    }
  }

#ifdef JS_SANDBOX_LFI_JIT_MEMORY
  static void BindInPlace(uint8_t* raw_in_place, uint8_t* raw, const CodeLabel& label) {
    if (label.patchAt().bound()) {
      intptr_t offset = label.patchAt().offset();
      intptr_t target = label.target().offset();
      X86Encoding::SetPointer(raw_in_place + offset, raw + target);
    }
  }
#endif

  void ret() {
    MOZ_ASSERT(hasCreator());
#if defined(JS_SANDBOX) && !defined(JS_SANDBOX_USE_RET)
    MOZ_ASSERT(false, "Unexpected return instruction");
#endif
    AutoBundleInstructionScope bundle(*this);
    masm.ret();
  }
  void retn(Imm32 n) {
    MOZ_ASSERT(hasCreator());
#if defined(JS_SANDBOX) && !defined(JS_SANDBOX_USE_RET)
    MOZ_ASSERT(false, "Unexpected return instruction");
#endif
    AutoBundleInstructionScope bundle(*this);
    // Remove the size of the return address which is included in the frame.
    masm.ret_i(n.value - sizeof(void*));
  }
  void call(Label* label) {
#if defined(JS_SANDBOX) && !defined(JS_SANDBOX_USE_CALL)
    MOZ_ASSERT(false, "Unexpected call instruction");
#endif
    AutoBundleInstructionScope bundle(*this);
    JmpSrc j = masm.call();
    bundle.end();
    if (label->bound()) {
      masm.linkJump(j, JmpDst(label->offset()));
    } else {
      JmpSrc prev;
      if (label->used()) {
        prev = JmpSrc(label->offset());
      }
      label->use(j.offset());
      masm.setNextJump(j, prev);
    }
  }

  size_t CallSize(Label* label) {
    return X86Encoding::BaseAssembler::call_size();
  }

  void call(Register reg) {
#if defined(JS_SANDBOX_CFI) && !defined(JS_SANDBOX_USE_CALL)
    MOZ_ASSERT(false, "Unexpected call instruction");
#endif
    AutoBundleInstructionScope bundle(*this);
    masm.call_r(reg.encoding());
  }
  static size_t CallSize(Register reg) {
    return X86Encoding::BaseAssembler::call_r_size(reg.encoding());
  }
  static size_t CallSize(const Operand& op) {
    return X86Encoding::BaseAssembler::call_m_size(op.disp(), op.base());
  }
  void call(const Operand& op) {
#if defined(JS_SANDBOX) && !defined(JS_SANDBOX_USE_CALL)
    MOZ_ASSERT(false, "Unexpected call instruction");
#endif
    AutoBundleInstructionScope bundle(*this);
    switch (op.kind()) {
      case Operand::REG:
        masm.call_r(op.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.call_m(op.disp(), op.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  CodeOffset callWithPatch() {
    AutoBundleInstructionScope bundle(*this);
    return CodeOffset(masm.call().offset());
  }
  static size_t CallWithPatchSize() {
    return X86Encoding::BaseAssembler::call_size();
  }

  CodeOffset jmpWithPatch() {
    AutoBundleInstructionScope bundle(*this);
    return CodeOffset(masm.jmp().offset());
  }
  static size_t JmpWithPatchSize() {
    return X86Encoding::BaseAssembler::jmp_size();
  }

  void patchCall(uint32_t callerOffset, uint32_t calleeOffset) {
    unsigned char* code = masm.data();
    X86Encoding::SetRel32(code + callerOffset, code + calleeOffset);
  }

#if defined(JS_SANDBOX) && !defined(JS_SANDBOX_USE_CALL)
  void patchRetAddr(CodeOffset callOffset, CodeOffset retOffset) {
    unsigned char* code = masm.data();
    CodeLocationLabel patchAt(code + callOffset.offset());
    CodeLocationLabel target(code + retOffset.offset());

    ptrdiff_t off = target - patchAt;
    MOZ_ASSERT(off > ptrdiff_t(INT32_MIN));
    MOZ_ASSERT(off < ptrdiff_t(INT32_MAX));
    PatchWrite_Imm32(patchAt, Imm32(off));
  }
#endif

  CodeOffset farJumpWithPatch() {
    AutoBundleInstructionScope bundle(*this);
    return CodeOffset(masm.jmp().offset());
  }
  void patchFarJump(CodeOffset farJump, uint32_t targetOffset) {
    unsigned char* code = masm.data();
    X86Encoding::SetRel32(code + farJump.offset(), code + targetOffset);
  }

  // This is for patching during code generation, not after.
  void patchAddl(CodeOffset offset, int32_t n) {
    unsigned char* code = masm.data();
    X86Encoding::SetInt32(code + offset.offset(), n);
  }

  static void patchFiveByteNopToCall(uint8_t* callsite, uint8_t* target) {
    X86Encoding::BaseAssembler::patchFiveByteNopToCall(callsite, target);
  }
  static void patchCallToFiveByteNop(uint8_t* callsite) {
    X86Encoding::BaseAssembler::patchCallToFiveByteNop(callsite);
  }

#ifdef JS_SANDBOX_CET
  void readShadowStack(Register reg) {
    AutoBundleInstructionScope bundle(*this);
    masm.rdssp(reg.encoding());
  }
  void writeShadowStack(Register src, const Operand& dest) {
    AutoBundleInstructionScope bundle(*this);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.wrss(dest.disp(), dest.base(), src.encoding());
        break;
      default:
        MOZ_CRASH("unimplemented operand kind");
    }
  }
  void incShadowStack(Register reg) {
    AutoBundleInstructionScope bundle(*this);
    masm.incssp(reg.encoding());
  }
#endif
  void breakpoint() {
    AutoBundleInstructionScope bundle(*this);
    masm.int3();
  }
  CodeOffset ud2() {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    masm.ud2();
    freezeBundleGroup();
    bundle.end();
    CodeOffset off(masm.currentOffset() - 2);
    return off;
  }

  static bool HasSSE2() { return CPUInfo::IsSSE2Present(); }
  static bool HasSSE3() { return CPUInfo::IsSSE3Present(); }
  static bool HasSSSE3() { return CPUInfo::IsSSSE3Present(); }
  static bool HasSSE41() { return CPUInfo::IsSSE41Present(); }
  static bool HasSSE42() { return CPUInfo::IsSSE42Present(); }
  static bool HasPOPCNT() { return CPUInfo::IsPOPCNTPresent(); }
  static bool HasBMI1() { return CPUInfo::IsBMI1Present(); }
  static bool HasBMI2() { return CPUInfo::IsBMI2Present(); }
  static bool HasLZCNT() { return CPUInfo::IsLZCNTPresent(); }
  static bool SupportsFloatingPoint() { return CPUInfo::IsSSE2Present(); }
  static bool SupportsUnalignedAccesses() { return true; }
  static bool SupportsFastUnalignedFPAccesses() { return true; }
  static bool SupportsWasmSimd() { return CPUInfo::IsSSE41Present(); }
  static bool HasAVX() { return CPUInfo::IsAVXPresent(); }
  static bool HasAVX2() { return CPUInfo::IsAVX2Present(); }
  static bool HasFMA() { return CPUInfo::IsFMAPresent(); }

  static bool HasRoundInstruction(RoundingMode mode) {
    switch (mode) {
      case RoundingMode::Up:
      case RoundingMode::Down:
      case RoundingMode::NearestTiesToEven:
      case RoundingMode::TowardsZero:
        return CPUInfo::IsSSE41Present();
    }
    MOZ_CRASH("unexpected mode");
  }

  void cmpl(Register rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.cmpl_rr(rhs.encoding(), lhs.encoding());
  }
  void cmpl(const Operand& rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::REG:
        masm.cmpl_rr(rhs.reg(), lhs.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpl_mr(rhs.disp(), rhs.base(), lhs.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpl_mr(rhs.address(), lhs.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void cmpl(Register rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.cmpl_rr(rhs.encoding(), lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpl_rm(rhs.encoding(), lhs.disp(), lhs.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpl_rm(rhs.encoding(), lhs.disp(), lhs.base(), lhs.index(),
                     lhs.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpl_rm(rhs.encoding(), lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void cmpl(Imm32 rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.cmpl_ir(rhs.value, lhs.encoding());
  }
  void cmpl(Imm32 rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.cmpl_ir(rhs.value, lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpl_im(rhs.value, lhs.disp(), lhs.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpl_im(rhs.value, lhs.disp(), lhs.base(), lhs.index(),
                     lhs.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpl_im(rhs.value, lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void cmpw(Register rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.cmpw_rr(rhs.encoding(), lhs.encoding());
  }
  void cmpw(Imm32 rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.cmpw_ir(rhs.value, lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpw_im(rhs.value, lhs.disp(), lhs.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpw_im(rhs.value, lhs.disp(), lhs.base(), lhs.index(),
                     lhs.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpw_im(rhs.value, lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void cmpb(Register rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.cmpb_rr(rhs.encoding(), lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpb_rm(rhs.encoding(), lhs.disp(), lhs.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpb_rm(rhs.encoding(), lhs.disp(), lhs.base(), lhs.index(),
                     lhs.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpb_rm(rhs.encoding(), lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void cmpb(Imm32 rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.cmpb_ir(rhs.value, lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.cmpb_im(rhs.value, lhs.disp(), lhs.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpb_im(rhs.value, lhs.disp(), lhs.base(), lhs.index(),
                     lhs.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.cmpb_im(rhs.value, lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void setCC(Condition cond, Register r) {
    AutoBundleInstructionScope bundle(*this);
    masm.setCC_r(static_cast<X86Encoding::Condition>(cond), r.encoding());
  }
  void testb(Register rhs, Register lhs) {
    MOZ_ASSERT(
        AllocatableGeneralRegisterSet(Registers::SingleByteRegs).has(rhs));
    MOZ_ASSERT(
        AllocatableGeneralRegisterSet(Registers::SingleByteRegs).has(lhs));
    AutoBundleInstructionScope bundle(*this);
    masm.testb_rr(rhs.encoding(), lhs.encoding());
  }
  void testw(Register rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.testw_rr(lhs.encoding(), rhs.encoding());
  }
  void testl(Register rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.testl_rr(lhs.encoding(), rhs.encoding());
  }
  void testl(Imm32 rhs, Register lhs) {
    AutoBundleInstructionScope bundle(*this);
    masm.testl_ir(rhs.value, lhs.encoding());
  }
  void testl(Imm32 rhs, const Operand& lhs) {
    AutoBundleInstructionScope bundle(*this);
    switch (lhs.kind()) {
      case Operand::REG:
        masm.testl_ir(rhs.value, lhs.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.testl_i32m(rhs.value, lhs.disp(), lhs.base());
        break;
      case Operand::MEM_ADDRESS32:
        masm.testl_i32m(rhs.value, lhs.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  void addl(Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.addl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  CodeOffset addlWithPatch(Imm32 imm, Register dest) {
    MOZ_ASSERT(dest != StackPointer, "Unsupported instruction");
    AutoBundleInstructionScope bundle(*this);
    masm.addl_i32r(imm.value, dest.encoding());
    bundle.end();
    return CodeOffset(masm.currentOffset());
  }
  void addl(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.addl_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.addl_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_ADDRESS32:
        masm.addl_im(imm.value, op.address());
        break;
      case Operand::MEM_SCALE:
        masm.addl_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void addw(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.addw_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.addw_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_ADDRESS32:
        masm.addw_im(imm.value, op.address());
        break;
      case Operand::MEM_SCALE:
        masm.addw_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void subl(Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.subl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void subl(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.subl_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.subl_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.subl_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void subw(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.subw_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.subw_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.subw_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void addl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.addl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void addl(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.addl_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.addl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.addl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void addw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.addw_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.addw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.addw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void sbbl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.sbbl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void subl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.subl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void subl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.subl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.subl_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void subl(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.subl_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.subl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.subl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void subw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.subw_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.subw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.subw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void orl(Register reg, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.orl_rr(reg.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void orl(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.orl_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.orl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.orl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                    dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void orw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.orw_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.orw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.orw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                    dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void orl(Imm32 imm, Register reg) {
    AutoBundleGroupScope bundle(*this);
    masm.orl_ir(imm.value, reg.encoding());
    sandboxStackPointer(reg);
  }
  void orl(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.orl_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.orl_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.orl_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void orw(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.orw_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.orw_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.orw_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void xorl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.xorl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void xorl(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.xorl_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.xorl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void xorw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.xorw_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.xorw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void xorl(Imm32 imm, Register reg) {
    AutoBundleGroupScope bundle(*this);
    masm.xorl_ir(imm.value, reg.encoding());
    sandboxStackPointer(reg);
  }
  void xorl(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.xorl_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.xorl_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorl_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void xorw(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.xorw_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.xorw_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorw_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void andl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.andl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void andl(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.andl_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.andl_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.andl_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void andw(Register src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.andw_rr(src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.andw_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.andw_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                     dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void andl(Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.andl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void andl(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.andl_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.andl_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.andl_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void andw(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::REG:
        masm.andw_ir(imm.value, op.reg());
        sandboxStackPointer(Register(op.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.andw_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.andw_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void addl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.addl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.addl_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void orl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.orl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.orl_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void xorl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.xorl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.xorl_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void andl(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.andl_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.andl_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.andl_mr(src.disp(), src.base(), src.index(), src.scale(),
                     dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void bsrl(const Register& src, const Register& dest) {
    AutoBundleGroupScope bundle(*this);
    masm.bsrl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void bsfl(const Register& src, const Register& dest) {
    AutoBundleGroupScope bundle(*this);
    masm.bsfl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void bswapl(Register reg) {
    AutoBundleGroupScope bundle(*this);
    masm.bswapl_r(reg.encoding());
    sandboxStackPointer(reg);
  }
  void lzcntl(const Register& src, const Register& dest) {
    AutoBundleGroupScope bundle(*this);
    masm.lzcntl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void tzcntl(const Register& src, const Register& dest) {
    AutoBundleGroupScope bundle(*this);
    masm.tzcntl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void popcntl(const Register& src, const Register& dest) {
    AutoBundleGroupScope bundle(*this);
    masm.popcntl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void imull(Register multiplier) {
    AutoBundleInstructionScope bundle(*this);
    // Consumes eax as the other argument
    // and clobbers edx, as result is in edx:eax
    masm.imull_r(multiplier.encoding());
  }
  void umull(Register multiplier) {
    AutoBundleInstructionScope bundle(*this);
    masm.mull_r(multiplier.encoding());
  }
  void imull(Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.imull_ir(imm.value, dest.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void imull(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.imull_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void imull(Imm32 imm, Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.imull_ir(imm.value, src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void imull(const Operand& src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.imull_rr(src.reg(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.imull_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    sandboxStackPointer(dest);
  }
  void negl(const Operand& unsafeSrc) {
    AutoBundleGroupScope bundle(*this);
    const Operand src = sandboxMemoryWrite(unsafeSrc);
    switch (src.kind()) {
      case Operand::REG:
        masm.negl_r(src.reg());
        sandboxStackPointer(Register(src.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.negl_m(src.disp(), src.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void negl(Register reg) {
    AutoBundleGroupScope bundle(*this);
    masm.negl_r(reg.encoding());
    sandboxStackPointer(reg);
  }
  void notl(const Operand& unsafeSrc) {
    AutoBundleGroupScope bundle(*this);
    const Operand src = sandboxMemoryWrite(unsafeSrc);
    switch (src.kind()) {
      case Operand::REG:
        masm.notl_r(src.reg());
        sandboxStackPointer(Register(src.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.notl_m(src.disp(), src.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void notl(Register reg) {
    AutoBundleGroupScope bundle(*this);
    masm.notl_r(reg.encoding());
    sandboxStackPointer(reg);
  }
  void shrl(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shrl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void shll(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shll_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void sarl(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.sarl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void shrl_cl(Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shrl_CLr(dest.encoding());
    sandboxStackPointer(dest);
  }
  void shll_cl(Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shll_CLr(dest.encoding());
    sandboxStackPointer(dest);
  }
  void sarl_cl(Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.sarl_CLr(dest.encoding());
    sandboxStackPointer(dest);
  }
  void shrdl_cl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shrdl_CLr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void shldl_cl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.shldl_CLr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }

  void sarxl(Register src, Register shift, Register dest) {
    MOZ_ASSERT(HasBMI2());
    AutoBundleGroupScope bundle(*this);
    masm.sarxl_rrr(src.encoding(), shift.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void shlxl(Register src, Register shift, Register dest) {
    MOZ_ASSERT(HasBMI2());
    AutoBundleGroupScope bundle(*this);
    masm.shlxl_rrr(src.encoding(), shift.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void shrxl(Register src, Register shift, Register dest) {
    MOZ_ASSERT(HasBMI2());
    AutoBundleGroupScope bundle(*this);
    masm.shrxl_rrr(src.encoding(), shift.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }

  void roll(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.roll_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void roll_cl(Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.roll_CLr(dest.encoding());
    sandboxStackPointer(dest);
  }
  void rolw(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.rolw_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void rorl(const Imm32 imm, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.rorl_ir(imm.value, dest.encoding());
    sandboxStackPointer(dest);
  }
  void rorl_cl(Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.rorl_CLr(dest.encoding());
    sandboxStackPointer(dest);
  }

  void incl(const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.incl_m32(op.disp(), op.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void lock_incl(const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    masm.prefix_lock();
    incl(op);
  }

  void decl(const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.decl_m32(op.disp(), op.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void lock_decl(const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    masm.prefix_lock();
    decl(op);
  }

  void addb(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.addb_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.addb_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }
  void addb(Register src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.addb_rm(src.encoding(), op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.addb_rm(src.encoding(), op.disp(), op.base(), op.index(),
                     op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  void subb(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.subb_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.subb_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }
  void subb(Register src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.subb_rm(src.encoding(), op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.subb_rm(src.encoding(), op.disp(), op.base(), op.index(),
                     op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  void andb(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.andb_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.andb_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }
  void andb(Register src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.andb_rm(src.encoding(), op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.andb_rm(src.encoding(), op.disp(), op.base(), op.index(),
                     op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  void orb(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.orb_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.orb_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }
  void orb(Register src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.orb_rm(src.encoding(), op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.orb_rm(src.encoding(), op.disp(), op.base(), op.index(),
                    op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  void xorb(Imm32 imm, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.xorb_im(imm.value, op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorb_im(imm.value, op.disp(), op.base(), op.index(), op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }
  void xorb(Register src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    switch (op.kind()) {
      case Operand::MEM_REG_DISP:
        masm.xorb_rm(src.encoding(), op.disp(), op.base());
        break;
      case Operand::MEM_SCALE:
        masm.xorb_rm(src.encoding(), op.disp(), op.base(), op.index(),
                     op.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
        break;
    }
  }

  template <typename T>
  CodeOffset lock_addb(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    addb(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_subb(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    subb(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_andb(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    andb(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_orb(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    orb(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_xorb(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    xorb(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  template <typename T>
  CodeOffset lock_addw(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    addw(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_subw(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    subw(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_andw(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    andw(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_orw(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    orw(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_xorw(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    xorw(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  // Note, lock_addl(imm, op) is used for a memory barrier on non-SSE2 systems,
  // among other things.  Do not optimize, replace by XADDL, or similar.
  template <typename T>
  CodeOffset lock_addl(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    addl(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_subl(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    subl(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_andl(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    andl(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_orl(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    orl(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  template <typename T>
  CodeOffset lock_xorl(T src, const Operand& unsafeOp) {
    AutoBundleGroupScope bundle(*this);
    const Operand op = sandboxMemoryWrite(unsafeOp);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    xorl(src, op);
    size_t postOffset = bundle.offset();
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  CodeOffset lock_cmpxchgb(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.cmpxchgb(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpxchgb(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset lock_cmpxchgw(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.cmpxchgw(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpxchgw(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset lock_cmpxchgl(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    masm.prefix_lock();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.cmpxchgl(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpxchgl(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  void lock_cmpxchg8b(Register srcHi, Register srcLo, Register newHi,
                      Register newLo, const Operand& unsafeMem) {
    //TODO(JS_SANDBOX_PERF): check if this is necessary.
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    masm.prefix_lock();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.cmpxchg8b(srcHi.encoding(), srcLo.encoding(), newHi.encoding(),
                       newLo.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.cmpxchg8b(srcHi.encoding(), srcLo.encoding(), newHi.encoding(),
                       newLo.encoding(), mem.disp(), mem.base(), mem.index(),
                       mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  CodeOffset xchgb(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.xchgb_rm(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.xchgb_rm(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset xchgw(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.xchgw_rm(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.xchgw_rm(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset xchgl(Register src, const Operand& unsafeMem) {
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.xchgl_rm(src.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.xchgl_rm(src.encoding(), mem.disp(), mem.base(), mem.index(),
                      mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(src);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  CodeOffset lock_xaddb(Register srcdest, const Operand& unsafeMem) {
    //TODO(JS_SANDBOX_PERF): check if this is necessary.
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.lock_xaddb_rm(srcdest.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.lock_xaddb_rm(srcdest.encoding(), mem.disp(), mem.base(),
                           mem.index(), mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(srcdest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset lock_xaddw(Register srcdest, const Operand& unsafeMem) {
    //TODO(JS_SANDBOX_PERF): check if this is necessary.
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    masm.prefix_16_for_32();
    lock_xaddl(srcdest, mem);
    size_t postOffset = bundle.offset();
    sandboxStackPointer(srcdest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }
  CodeOffset lock_xaddl(Register srcdest, const Operand& unsafeMem) {
    //TODO(JS_SANDBOX_PERF): check if this is necessary.
    AutoBundleGroupScope bundle(*this);
    const Operand mem = sandboxMemoryWrite(unsafeMem);
    size_t preOffset = bundle.offset();
    switch (mem.kind()) {
      case Operand::MEM_REG_DISP:
        masm.lock_xaddl_rm(srcdest.encoding(), mem.disp(), mem.base());
        break;
      case Operand::MEM_SCALE:
        masm.lock_xaddl_rm(srcdest.encoding(), mem.disp(), mem.base(),
                           mem.index(), mem.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
    size_t postOffset = bundle.offset();
    sandboxStackPointer(srcdest);
    freezeBundleGroup();
    return CodeOffset(size() - (postOffset - preOffset));
  }

  void push(const Imm32 imm) {
    AutoBundleInstructionScope bundle(*this);
    masm.push_i(imm.value);
  }

  void push(const Operand& src) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::REG:
        masm.push_r(src.reg());
        break;
      case Operand::MEM_REG_DISP:
        masm.push_m(src.disp(), src.base());
        break;
      case Operand::MEM_SCALE:
        masm.push_m(src.disp(), src.base(), src.index(), src.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void push(Register src) {
    MOZ_ASSERT(hasCreator());
    AutoBundleInstructionScope bundle(*this);
    masm.push_r(src.encoding());
  }

  static size_t PushSize(Register src) {
    return X86Encoding::BaseAssembler::push_size(src.encoding());
  }

  void push(const Address& src) {
    AutoBundleInstructionScope bundle(*this);
    masm.push_m(src.offset, src.base.encoding());
  }

  void pop(const Operand& unsafeSrc) {
    AutoBundleGroupScope bundle(*this);
    const Operand src = sandboxMemoryWrite(unsafeSrc);
    MOZ_ASSERT(hasCreator());
    switch (src.kind()) {
      case Operand::REG:
        MOZ_ASSERT(src.reg() != r15.encoding());
        masm.pop_r(src.reg());
        sandboxStackPointer(Register(src.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.pop_m(src.disp(), src.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void pop(Register src) {
    MOZ_ASSERT(src != r15);
    MOZ_ASSERT(hasCreator());
    AutoBundleGroupScope bundle(*this);
    masm.pop_r(src.encoding());
    sandboxStackPointer(src);
  }
  void pop(const Address& src) {
#ifdef JS_SANDBOX_HEAP
    pop(Operand(src));
#else
    AutoBundleInstructionScope bundle(*this);
    masm.pop_m(src.offset, src.base.encoding());
#endif
  }

  void pushFlags() {
    AutoBundleInstructionScope bundle(*this);
    masm.push_flags();
  }
  void popFlags() {
    AutoBundleInstructionScope bundle(*this);
    masm.pop_flags();
  }

#ifdef JS_CODEGEN_X86
  void pushAllRegs() { masm.pusha(); }
  void popAllRegs() { masm.popa(); }
#endif

  // Zero-extend byte to 32-bit integer.
  void movzbl(Register src, Register dest) {
    AutoBundleGroupScope bundle(*this);
    masm.movzbl_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }

  void cdq() {
    AutoBundleInstructionScope bundle(*this);
    masm.cdq();
  }
  void idiv(Register divisor) {
    AutoBundleInstructionScope bundle(*this);
    masm.idivl_r(divisor.encoding());
  }
  void udiv(Register divisor) {
    AutoBundleInstructionScope bundle(*this);
    masm.divl_r(divisor.encoding());
  }

  void vpblendw(uint32_t mask, FloatRegister src1, FloatRegister src0,
                FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vpblendw_irr(mask, src1.encoding(), src0.encoding(), dest.encoding());
  }

  void vpblendvb(FloatRegister mask, FloatRegister src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vpblendvb_rr(mask.encoding(), src1.encoding(), src0.encoding(),
                      dest.encoding());
  }

  void vpinsrb(unsigned lane, const Operand& src1, FloatRegister src0,
               FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::REG:
        masm.vpinsrb_irr(lane, src1.reg(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpinsrb_imr(lane, src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpinsrb_imr(lane, src1.disp(), src1.base(), src1.index(),
                         src1.scale(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpinsrw(unsigned lane, const Operand& src1, FloatRegister src0,
               FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::REG:
        masm.vpinsrw_irr(lane, src1.reg(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpinsrw_imr(lane, src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpinsrw_imr(lane, src1.disp(), src1.base(), src1.index(),
                         src1.scale(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpinsrd(unsigned lane, Register src1, FloatRegister src0,
               FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vpinsrd_irr(lane, src1.encoding(), src0.encoding(), dest.encoding());
  }

  void vpextrb(unsigned lane, FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.vpextrb_irr(lane, src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.vpextrb_irm(lane, src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vpextrb_irm(lane, src.encoding(), dest.disp(), dest.base(),
                         dest.index(), dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpextrw(unsigned lane, FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::REG:
        masm.vpextrw_irr(lane, src.encoding(), dest.reg());
        sandboxStackPointer(Register(dest.reg()));
        break;
      case Operand::MEM_REG_DISP:
        masm.vpextrw_irm(lane, src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vpextrw_irm(lane, src.encoding(), dest.disp(), dest.base(),
                         dest.index(), dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpextrd(unsigned lane, FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleGroupScope bundle(*this);
    masm.vpextrd_irr(lane, src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vpsrldq(Imm32 shift, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrldq_ir(shift.value, src0.encoding(), dest.encoding());
  }
  void vpslldq(Imm32 shift, FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpslldq_ir(shift.value, src.encoding(), dest.encoding());
  }
  void vpsllq(Imm32 shift, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsllq_ir(shift.value, src0.encoding(), dest.encoding());
  }
  void vpsllq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsllq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsrlq(Imm32 shift, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrlq_ir(shift.value, src0.encoding(), dest.encoding());
  }
  void vpsrlq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrlq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpslld(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpslld_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpslld(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpslld_ir(count.value, src0.encoding(), dest.encoding());
  }
  void vpsrad(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrad_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsrad(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrad_ir(count.value, src0.encoding(), dest.encoding());
  }
  void vpsrld(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrld_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsrld(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrld_ir(count.value, src0.encoding(), dest.encoding());
  }

  void vpsllw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsllw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsllw(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsllw_ir(count.value, src0.encoding(), dest.encoding());
  }
  void vpsraw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsraw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsraw(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsraw_ir(count.value, src0.encoding(), dest.encoding());
  }
  void vpsrlw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrlw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpsrlw(Imm32 count, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpsrlw_ir(count.value, src0.encoding(), dest.encoding());
  }

  void vcvtsi2sd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::REG:
        masm.vcvtsi2sd_rr(src1.reg(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vcvtsi2sd_mr(src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vcvtsi2sd_mr(src1.disp(), src1.base(), src1.index(), src1.scale(),
                          src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vcvttsd2si(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vcvttsd2si_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vcvttss2si(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vcvttss2si_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vcvtsi2ss(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::REG:
        masm.vcvtsi2ss_rr(src1.reg(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vcvtsi2ss_mr(src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vcvtsi2ss_mr(src1.disp(), src1.base(), src1.index(), src1.scale(),
                          src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vcvtsi2ss(Register src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtsi2ss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vcvtsi2sd(Register src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtsi2sd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vcvttps2dq(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvttps2dq_rr(src.encoding(), dest.encoding());
  }
  void vcvttpd2dq(FloatRegister src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vcvttpd2dq_rr(src.encoding(), dest.encoding());
  }
  void vcvtdq2ps(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtdq2ps_rr(src.encoding(), dest.encoding());
  }
  void vcvtdq2pd(FloatRegister src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtdq2pd_rr(src.encoding(), dest.encoding());
  }
  void vcvtps2pd(FloatRegister src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtps2pd_rr(src.encoding(), dest.encoding());
  }
  void vcvtpd2ps(FloatRegister src, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    masm.vcvtpd2ps_rr(src.encoding(), dest.encoding());
  }
  void vmovmskpd(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vmovmskpd_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vmovmskps(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vmovmskps_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vpmovmskb(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vpmovmskb_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vptest(FloatRegister rhs, FloatRegister lhs) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vptest_rr(rhs.encoding(), lhs.encoding());
  }
  void vucomisd(FloatRegister rhs, FloatRegister lhs) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vucomisd_rr(rhs.encoding(), lhs.encoding());
  }
  void vucomiss(FloatRegister rhs, FloatRegister lhs) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vucomiss_rr(rhs.encoding(), lhs.encoding());
  }

  void vpcmpeqb(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpeqb_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpeqb_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpeqb_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpcmpgtb(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpgtb_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpgtb_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpgtb_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpcmpeqw(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpeqw_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpeqw_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpeqw_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpcmpgtw(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpgtw_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpgtw_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpgtw_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpcmpeqd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpeqd_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpeqd_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpeqd_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpcmpgtd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpgtd_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpgtd_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpgtd_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpcmpgtq(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE42());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpgtq_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpcmpeqq(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vpcmpeqq_rr(rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpcmpeqq_mr(rhs.disp(), rhs.base(), lhs.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpcmpeqq_mr(rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vcmpps(uint8_t order, Operand rhs, FloatRegister lhs,
              FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vcmpps_rr(order, rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vcmpps_mr(order, rhs.disp(), rhs.base(), lhs.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vcmpps_mr(order, rhs.address(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vcmpeqps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_EQ, rhs, lhs, dest);
  }
  void vcmpltps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_LT, rhs, lhs, dest);
  }
  void vcmpleps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_LE, rhs, lhs, dest);
  }
  void vcmpunordps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_UNORD, rhs, lhs, dest);
  }
  void vcmpneqps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_NEQ, rhs, lhs, dest);
  }
  void vcmpordps(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmpps(X86Encoding::ConditionCmp_ORD, rhs, lhs, dest);
  }
  void vcmppd(uint8_t order, Operand rhs, FloatRegister lhs,
              FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    switch (rhs.kind()) {
      case Operand::FPREG:
        masm.vcmppd_rr(order, rhs.fpu(), lhs.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("NYI");
    }
  }
  void vcmpeqpd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_EQ, rhs, lhs, dest);
  }
  void vcmpltpd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_LT, rhs, lhs, dest);
  }
  void vcmplepd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_LE, rhs, lhs, dest);
  }
  void vcmpneqpd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_NEQ, rhs, lhs, dest);
  }
  void vcmpordpd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_ORD, rhs, lhs, dest);
  }
  void vcmpunordpd(const Operand& rhs, FloatRegister lhs, FloatRegister dest) {
    vcmppd(X86Encoding::ConditionCmp_UNORD, rhs, lhs, dest);
  }
  void vrcpps(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vrcpps_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vrcpps_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vrcpps_mr(src.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsqrtps(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vsqrtps_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vsqrtps_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vsqrtps_mr(src.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vrsqrtps(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vrsqrtps_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vrsqrtps_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vrsqrtps_mr(src.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsqrtpd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vsqrtpd_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovd(Register src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovd_rr(src.encoding(), dest.encoding());
  }
  void vmovd(FloatRegister src, Register dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    masm.vmovd_rr(src.encoding(), dest.encoding());
    sandboxStackPointer(dest);
  }
  void vmovd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovd_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovd_mr(src.disp(), src.base(), src.index(), src.scale(),
                      dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovd(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovd_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovd_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                      dest.scale());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vmovq_rm(src.encoding(), dest.address());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovq(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovq_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovq_mr(src.disp(), src.base(), src.index(), src.scale(),
                      dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vmovq_mr(src.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovq(FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovq_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovq_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                      dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaddubsw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSSE3());
    AutoBundleInstructionScope bundle(*this);
    masm.vpmaddubsw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpaddb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddb_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubb_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddsb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddsb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddsb_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddsb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddusb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddusb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddusb_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddusb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubsb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubsb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubsb_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubsb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubusb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubusb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubusb_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubusb_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddw_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubw_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddsw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddsw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddsw_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddsw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddusw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddusw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddusw_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddusw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubsw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubsw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubsw_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubsw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubusw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubusw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubusw_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubusw_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpaddd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpaddd_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpsubd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpsubd_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmuldq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vpmuldq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpmuludq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpmuludq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpmuludq(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmuludq_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmuludq_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmullw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmullw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmullw_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmulhw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmulhw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmulhw_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmulhuw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmulhuw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmulhuw_mr(src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmulhrsw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmulhrsw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmulhrsw_mr(src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmulld(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmulld_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmulld_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpmulld_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaddwd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaddwd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpaddq(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpaddq_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpsubq(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpsubq_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vaddps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vaddps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vaddps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vaddps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsubps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vsubps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vsubps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vsubps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmulps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmulps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmulps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vmulps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vdivps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vdivps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vdivps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vdivps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmaxps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmaxps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmaxps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vmaxps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vminps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vminps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vminps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vminps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vminpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vminpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmaxpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmaxpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vaddpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vaddpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsubpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vsubpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmulpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmulpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vdivpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vdivpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpavgb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpavgb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpavgw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpavgw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminsb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminsb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminub(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminub_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxsb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxsb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxub(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxub_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminsw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminsw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminuw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminuw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxsw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxsw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxuw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxuw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpminud(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpminud_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmaxud(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpmaxud_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpacksswb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpacksswb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpackuswb(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpackuswb_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpackssdw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpackssdw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpackusdw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpackusdw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpabsb(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpabsb_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpabsw(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpabsw_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpabsd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpabsd_rr(src.fpu(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovsxbw(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovsxbw_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovsxbw_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovsxbw_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovzxbw(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovzxbw_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovzxbw_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovzxbw_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovsxwd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovsxwd_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovsxwd_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovsxwd_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovzxwd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovzxwd_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovzxwd_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovzxwd_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovsxdq(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovsxdq_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovsxdq_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovsxdq_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpmovzxdq(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vpmovzxdq_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpmovzxdq_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vpmovzxdq_mr(src.disp(), src.base(), src.index(), src.scale(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vphaddd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vphaddd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpalignr(const Operand& src1, FloatRegister src0, FloatRegister dest,
                uint8_t shift) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpalignr_irr(shift, src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpunpcklbw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpcklbw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpckhbw(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpckhbw_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpcklbw(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpunpcklbw_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpunpckldq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpckldq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpckldq(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vpunpckldq_mr(src1.disp(), src1.base(), src0.encoding(),
                           dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpunpckldq_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpunpcklqdq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpcklqdq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpcklqdq(const Operand& src1, FloatRegister src0,
                   FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vpunpcklqdq_mr(src1.disp(), src1.base(), src0.encoding(),
                            dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpunpcklqdq_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpunpckhdq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpckhdq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpckhqdq(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpckhqdq_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpcklwd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpcklwd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpunpckhwd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    MOZ_ASSERT(src0.size() == 16);
    MOZ_ASSERT(src1.size() == 16);
    MOZ_ASSERT(dest.size() == 16);
    AutoBundleInstructionScope bundle(*this);
    masm.vpunpckhwd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }

  void vandps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vandps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vandps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vandps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vandnps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    // Negates bits of dest and then applies AND
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vandnps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vandnps_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vandnps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vorps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vorps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vorps_mr(src1.disp(), src1.base(), src0.encoding(),
                      dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vorps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vxorps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vxorps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vxorps_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vxorps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vandpd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vandpd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpand(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpand_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpand(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpand_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpand_mr(src1.disp(), src1.base(), src0.encoding(),
                      dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpand_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpor(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpor_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpor(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpor_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpor_mr(src1.disp(), src1.base(), src0.encoding(),
                     dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpor_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpxor(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpxor_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpxor(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpxor_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpxor_mr(src1.disp(), src1.base(), src0.encoding(),
                      dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpxor_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vpandn(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpandn_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vpandn(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpandn_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpandn_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpandn_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpshufd(uint32_t mask, FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpshufd_irr(mask, src.encoding(), dest.encoding());
  }
  void vpshufd(uint32_t mask, const Operand& src1, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vpshufd_irr(mask, src1.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vpshufd_imr(mask, src1.disp(), src1.base(), dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vpshufd_imr(mask, src1.address(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vpshuflw(uint32_t mask, FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpshuflw_irr(mask, src.encoding(), dest.encoding());
  }
  void vpshufhw(uint32_t mask, FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vpshufhw_irr(mask, src.encoding(), dest.encoding());
  }
  void vpshufb(FloatRegister mask, FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSSE3());
    AutoBundleInstructionScope bundle(*this);
    masm.vpshufb_rr(mask.encoding(), src.encoding(), dest.encoding());
  }
  void vmovddup(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vmovddup_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmovddup_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovddup_mr(src.disp(), src.base(), src.index(), src.scale(),
                         dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovhlps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovhlps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmovlhps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovlhps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vunpcklps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vunpcklps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vunpcklps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vunpcklps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vunpcklps_mr(src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vunpcklps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vunpckhps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vunpckhps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vunpckhps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vunpckhps_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vunpckhps_mr(src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vunpckhps_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vshufps(uint32_t mask, FloatRegister src1, FloatRegister src0,
               FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vshufps_irr(mask, src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vshufps(uint32_t mask, const Operand& src1, FloatRegister src0,
               FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vshufps_irr(mask, src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vshufps_imr(mask, src1.disp(), src1.base(), src0.encoding(),
                         dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vshufps_imr(mask, src1.address(), src0.encoding(),
                         dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vshufpd(uint32_t mask, FloatRegister src1, FloatRegister src0,
               FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vshufpd_irr(mask, src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vaddsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vaddsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vaddss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vaddss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vaddsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vaddsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vaddsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vaddsd_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vaddss(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vaddss_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vaddss_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      case Operand::MEM_ADDRESS32:
        masm.vaddss_mr(src1.address(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsubsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vsubsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vsubss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vsubss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vsubsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vsubsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vsubsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vsubss(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vsubss_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vsubss_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmulsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmulsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmulsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmulsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmulsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmulss(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmulss_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmulss_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmulss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmulss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vdivsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vdivsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vdivss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vdivss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vdivsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vdivsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vdivsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vdivss(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vdivss_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vdivss_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vxorpd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vxorpd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vxorps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vxorps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vorpd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vorpd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vorps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vorps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vandpd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vandpd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vandps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vandps_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vsqrtsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vsqrtsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vsqrtss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vsqrtss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vroundps(SSERoundingMode mode, const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vroundps_irr((X86Encoding::SSERoundingMode)mode, src.fpu(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vroundpd(SSERoundingMode mode, const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vroundpd_irr((X86Encoding::SSERoundingMode)mode, src.fpu(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  static X86Encoding::RoundingMode ToX86RoundingMode(RoundingMode mode) {
    switch (mode) {
      case RoundingMode::Up:
        return X86Encoding::RoundUp;
      case RoundingMode::Down:
        return X86Encoding::RoundDown;
      case RoundingMode::NearestTiesToEven:
        return X86Encoding::RoundToNearest;
      case RoundingMode::TowardsZero:
        return X86Encoding::RoundToZero;
    }
    MOZ_CRASH("unexpected mode");
  }
  void vroundsd(X86Encoding::RoundingMode mode, FloatRegister src,
                FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vroundsd_irr(mode, src.encoding(), dest.encoding());
  }
  void vroundss(X86Encoding::RoundingMode mode, FloatRegister src,
                FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vroundss_irr(mode, src.encoding(), dest.encoding());
  }

  unsigned vinsertpsMask(unsigned sourceLane, unsigned destLane,
                         unsigned zeroMask = 0) {
    // Note that the sourceLane bits are ignored in the case of a source
    // memory operand, and the source is the given 32-bits memory location.
    MOZ_ASSERT(zeroMask < 16);
    unsigned ret = zeroMask;
    ret |= destLane << 4;
    ret |= sourceLane << 6;
    MOZ_ASSERT(ret < 256);
    return ret;
  }
  void vinsertps(uint32_t mask, FloatRegister src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vinsertps_irr(mask, src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vinsertps(uint32_t mask, const Operand& src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vinsertps_irr(mask, src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vinsertps_imr(mask, src1.disp(), src1.base(), src0.encoding(),
                           dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vinsertps_imr(mask, src1.disp(), src1.base(), src1.index(),
                           src1.scale(), src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovlps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovlps_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovlps_mr(src1.disp(), src1.base(), src1.index(), src1.scale(),
                        src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovlps(FloatRegister src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovlps_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovlps_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovhps(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovhps_mr(src1.disp(), src1.base(), src0.encoding(),
                        dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vmovhps_mr(src1.disp(), src1.base(), src1.index(), src1.scale(),
                        src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovhps(FloatRegister src, const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vmovhps_rm(src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vmovhps_rm(src.encoding(), dest.disp(), dest.base(), dest.index(),
                        dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vextractps(unsigned lane, FloatRegister src, const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE41());
    MOZ_ASSERT(lane < 4);
    AutoBundleGroupScope bundle(*this);
    const Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.vextractps_rm(lane, src.encoding(), dest.disp(), dest.base());
        break;
      case Operand::MEM_SCALE:
        masm.vextractps_rm(lane, src.encoding(), dest.disp(), dest.base(),
                           dest.index(), dest.scale());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  unsigned blendpsMask(bool x, bool y, bool z, bool w) {
    return (x << 0) | (y << 1) | (z << 2) | (w << 3);
  }
  void vblendps(unsigned mask, FloatRegister src1, FloatRegister src0,
                FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vblendps_irr(mask, src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vblendps(unsigned mask, const Operand& src1, FloatRegister src0,
                FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vblendps_irr(mask, src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vblendps_imr(mask, src1.disp(), src1.base(), src0.encoding(),
                          dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vblendvps(FloatRegister mask, FloatRegister src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vblendvps_rr(mask.encoding(), src1.encoding(), src0.encoding(),
                      dest.encoding());
  }
  void vblendvps(FloatRegister mask, const Operand& src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vblendvps_rr(mask.encoding(), src1.fpu(), src0.encoding(),
                          dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vblendvps_mr(mask.encoding(), src1.disp(), src1.base(),
                          src0.encoding(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vblendvpd(FloatRegister mask, FloatRegister src1, FloatRegister src0,
                 FloatRegister dest) {
    MOZ_ASSERT(HasSSE41());
    AutoBundleInstructionScope bundle(*this);
    masm.vblendvpd_rr(mask.encoding(), src1.encoding(), src0.encoding(),
                      dest.encoding());
  }
  void vmovsldup(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovsldup_rr(src.encoding(), dest.encoding());
  }
  void vmovsldup(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vmovsldup_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmovsldup_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmovshdup(FloatRegister src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    masm.vmovshdup_rr(src.encoding(), dest.encoding());
  }
  void vmovshdup(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vmovshdup_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmovshdup_mr(src.disp(), src.base(), dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vminsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vminsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vminsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vminsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vminsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vminss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vminss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmaxsd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmaxsd_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vmaxsd(const Operand& src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    switch (src1.kind()) {
      case Operand::FPREG:
        masm.vmaxsd_rr(src1.fpu(), src0.encoding(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vmaxsd_mr(src1.disp(), src1.base(), src0.encoding(),
                       dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vmaxss(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasSSE2());
    AutoBundleInstructionScope bundle(*this);
    masm.vmaxss_rr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fisttp(const Operand& unsafeDest) {
    MOZ_ASSERT(HasSSE3());
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fisttp_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fistp(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fistp_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fnstcw(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fnstcw_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fldcw(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fldcw_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fnstsw(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fnstsw_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fld(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fld_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fld32(const Operand& unsafeDest) {
    AutoBundleGroupScope bundle(*this);
    Operand dest = sandboxMemoryWrite(unsafeDest);
    switch (dest.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fld32_m(dest.disp(), dest.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fstp(const Operand& unsafeSrc) {
    AutoBundleGroupScope bundle(*this);
    Operand src = sandboxMemoryWrite(unsafeSrc);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fstp_m(src.disp(), src.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  // TODO(JS_SANDBOX_HEAP): is this actually a destination ?
  void fstp32(const Operand& unsafeSrc) {
    AutoBundleGroupScope bundle(*this);
    Operand src = sandboxMemoryWrite(unsafeSrc);
    switch (src.kind()) {
      case Operand::MEM_REG_DISP:
        masm.fstp32_m(src.disp(), src.base());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }

  void vbroadcastb(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasAVX2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vbroadcastb_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vbroadcastb_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vbroadcastb_mr(src.disp(), src.base(), src.index(), src.scale(),
                            dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vbroadcastw(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasAVX2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vbroadcastw_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vbroadcastw_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vbroadcastw_mr(src.disp(), src.base(), src.index(), src.scale(),
                            dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vbroadcastd(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasAVX2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vbroadcastd_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vbroadcastd_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vbroadcastd_mr(src.disp(), src.base(), src.index(), src.scale(),
                            dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vbroadcastq(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasAVX2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vbroadcastq_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vbroadcastq_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vbroadcastq_mr(src.disp(), src.base(), src.index(), src.scale(),
                            dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vbroadcastss(const Operand& src, FloatRegister dest) {
    MOZ_ASSERT(HasAVX2());
    AutoBundleInstructionScope bundle(*this);
    switch (src.kind()) {
      case Operand::FPREG:
        masm.vbroadcastss_rr(src.fpu(), dest.encoding());
        break;
      case Operand::MEM_REG_DISP:
        masm.vbroadcastss_mr(src.disp(), src.base(), dest.encoding());
        break;
      case Operand::MEM_SCALE:
        masm.vbroadcastss_mr(src.disp(), src.base(), src.index(), src.scale(),
                             dest.encoding());
        break;
      default:
        MOZ_CRASH("unexpected operand kind");
    }
  }
  void vfmadd231ps(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasFMA());
    AutoBundleInstructionScope bundle(*this);
    masm.vfmadd231ps_rrr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vfnmadd231ps(FloatRegister src1, FloatRegister src0,
                    FloatRegister dest) {
    MOZ_ASSERT(HasFMA());
    AutoBundleInstructionScope bundle(*this);
    masm.vfnmadd231ps_rrr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vfmadd231pd(FloatRegister src1, FloatRegister src0, FloatRegister dest) {
    MOZ_ASSERT(HasFMA());
    AutoBundleInstructionScope bundle(*this);
    masm.vfmadd231pd_rrr(src1.encoding(), src0.encoding(), dest.encoding());
  }
  void vfnmadd231pd(FloatRegister src1, FloatRegister src0,
                    FloatRegister dest) {
    MOZ_ASSERT(HasFMA());
    AutoBundleInstructionScope bundle(*this);
    masm.vfnmadd231pd_rrr(src1.encoding(), src0.encoding(), dest.encoding());
  }

  void flushBuffer() {}

  // Patching.

  static size_t PatchWrite_NearCallSize() { return 5; }
  static uintptr_t GetPointer(uint8_t* instPtr) {
    uint8_t* ptr = instPtr - sizeof(uintptr_t);
    return mozilla::LittleEndian::readUintptr(ptr);
  }
  // Write a relative call at the start location |dataLabel|.
  // Note that this DOES NOT patch data that comes before |label|.
  static void PatchWrite_NearCall(CodeLocationLabel startLabel,
                                  CodeLocationLabel target) {
    uint8_t* start = startLabel.raw();
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
    size_t val = 0xE8;
    uint32_t dist = target - startLabel - PatchWrite_NearCallSize();
    val |= ((size_t)dist << 8);
    sys_jitcode_modify(start, val, 5, 0);
#else
    *start = 0xE8;  // <CALL> rel32
    ptrdiff_t offset = target - startLabel - PatchWrite_NearCallSize();
    MOZ_ASSERT(int32_t(offset) == offset);
    mozilla::LittleEndian::writeInt32(start + 1, offset);  // CALL <rel32>
#endif
  }

  static constexpr size_t PatchWrite_HltImm32_Size() {
    return 5;
  }

  static void PatchWrite_HltImm32(CodeLocationLabel dataLabel,
                                  CodeLocationLabel hltStartLabel,
                                  Imm32 toWrite) {
    // dataLabel is a code location which targets the end of an instruction
    // which has a 32 bits immediate. Thus writting a value requires shifting
    // back to the address of the 32 bits immediate within the instruction.
    uint8_t* ptr = dataLabel.raw();

    uint8_t* hlt_ptr = hltStartLabel.raw();
    MOZ_ASSERT((uintptr_t)ptr - (uintptr_t)hlt_ptr >= 1 + sizeof(int32_t));
    memset(hlt_ptr, 0xf4, ptr - hlt_ptr - sizeof(int32_t));

    mozilla::LittleEndian::writeInt32(ptr - sizeof(int32_t), toWrite.value);
  }

  static void PatchWrite_Imm32(CodeLocationLabel dataLabel, Imm32 toWrite) {
    // dataLabel is a code location which targets the end of an instruction
    // which has a 32 bits immediate. Thus writting a value requires shifting
    // back to the address of the 32 bits immediate within the instruction.
    uint8_t* ptr = dataLabel.raw();
    mozilla::LittleEndian::writeInt32(ptr - sizeof(int32_t), toWrite.value);
  }

#ifdef JS_SANDBOX_LFI_JIT_MEMORY
  static void PatchWrite_HltImm32_Runtime(CodeLocationLabel dataLabel,
                                  CodeLocationLabel hltStartLabel,
                                  Imm32 toWrite) {
    // dataLabel is a code location which targets the end of an instruction
    // which has a 32 bits immediate. Thus writting a value requires shifting
    // back to the address of the 32 bits immediate within the instruction.
    uint8_t* ptr = dataLabel.raw();
    sys_jitcode_modify(ptr - sizeof(int32_t), toWrite.value, sizeof(int32_t),
                       hltStartLabel.raw() - dataLabel.raw());
  }
#endif

  static void PatchDataWithValueCheck(CodeLocationLabel data,
                                      PatchedImmPtr newData,
                                      PatchedImmPtr expectedData) {
    // The pointer given is a pointer to *after* the data.
    uint8_t* ptr = data.raw() - sizeof(uintptr_t);
    MOZ_ASSERT(mozilla::LittleEndian::readUintptr(ptr) ==
               uintptr_t(expectedData.value));
    mozilla::LittleEndian::writeUintptr(ptr, uintptr_t(newData.value));
  }
  static void PatchDataWithValueCheck(CodeLocationLabel data, ImmPtr newData,
                                      ImmPtr expectedData) {
    PatchDataWithValueCheck(data, PatchedImmPtr(newData.value),
                            PatchedImmPtr(expectedData.value));
  }
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
  static void PatchDataWithValueCheckInPlace(uint8_t* data_in_place,
                                             CodeLocationLabel data,
                                             PatchedImmPtr newData,
                                             PatchedImmPtr expectedData) {
    // The pointer given is a pointer to *after* the data.
    uint8_t* ptr = data_in_place - sizeof(uintptr_t);
    mozilla::LittleEndian::writeUintptr(ptr, uintptr_t(newData.value));
  }
  static void PatchDataWithValueCheckInPlace(uint8_t* data_in_place,
                                             CodeLocationLabel data,
                                             ImmPtr newData,
                                             ImmPtr expectedData) {
    PatchDataWithValueCheckInPlace(data_in_place, data,
                                   PatchedImmPtr(newData.value),
                                   PatchedImmPtr(expectedData.value));
  }
#endif

  static uint32_t NopSize() { return 1; }
  static uint8_t* NextInstruction(uint8_t* cur, uint32_t* count) {
    MOZ_CRASH("nextInstruction NYI on x86");
  }

  // Toggle a jmp or cmp emitted by toggledJump().
  static void ToggleToJmp(CodeLocationLabel inst) {
    uint8_t* ptr = (uint8_t*)inst.raw();
    // TODO: why do these fail? sometimes code ptr is nulled out
    MOZ_ASSERT(*ptr == 0x3D);  // <CMP> eax, imm32
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
    sys_jitcode_modify(ptr, 0xE9, 1, 0);
#else
    *ptr = 0xE9;               // <JMP> rel32
#endif
  }
  static void ToggleToCmp(CodeLocationLabel inst) {
    uint8_t* ptr = (uint8_t*)inst.raw();
    MOZ_ASSERT(*ptr == 0xE9);  // <JMP> rel32
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
    sys_jitcode_modify(ptr, 0x3D, 1, 0);
#else
    *ptr = 0x3D;               // <CMP> eax, imm32
#endif
  }
  static void ToggleCall(CodeLocationLabel inst, bool enabled) {
    uint8_t* ptr = (uint8_t*)inst.raw();
    MOZ_ASSERT(*ptr == 0x3D ||  // <CMP> eax, imm32
               *ptr == 0xE8);   // <CALL> rel32
#ifdef JS_SANDBOX_LFI_JIT_MEMORY
    sys_jitcode_modify(ptr, enabled ? 0xE8 : 0x3D, 1, 0);
#else
    *ptr = enabled ? 0xE8 : 0x3D;
#endif
  }

  MOZ_COLD void verifyHeapAccessDisassembly(
      uint32_t begin, uint32_t end, const Disassembler::HeapAccess& heapAccess);
};

}  // namespace jit
}  // namespace js

#endif /* jit_x86_shared_Assembler_x86_shared_h */
