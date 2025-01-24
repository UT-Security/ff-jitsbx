/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mozilla_dom_JSTainted_h__
#define mozilla_dom_JSTainted_h__

#include <type_traits>
#include "js/RootingAPI.h"
#include "js/Value.h"

namespace mozilla {

namespace dom {

template<typename T>
class TaintObj {
	public:
	static inline mozilla::HashSet<void *> PtrTable = mozilla::HashSet<void *>(1);
	TaintObj() {
		if(PtrTable.put((void *)this))
		;
	}
	~TaintObj() {
		PtrTable.remove((void *)this);
	}
};

template<typename T>
class JSTainted;

template<typename T>
class JSTaintedVolatile;

template <typename T, typename Enable = void>
class JSTaintedOperations {};

class JSAppPtr;

class JSAppPtr {
  public:
  explicit JSAppPtr(void * ptr) : app_ptr(ptr) {}
  
  template <typename T>
  T* verify(const mozilla::HashSet<void*> & PtrTable) {
    for (auto iter = PtrTable.iter(); !iter.done(); iter.next()) {
      void * cur = iter.get();
      if (app_ptr == cur) {
        return static_cast<T*>(cur);
      }
    }
    return nullptr;
  }

  private:
  void * app_ptr;
};

template <template <typename> typename T_Wrap, typename T>
class JSTaintedBase {
public:
  inline auto& impl() { return *static_cast<T_Wrap<T>*>(this); }
  inline auto& impl() const { return *static_cast<const T_Wrap<T>*>(this); }

  inline auto& UNSAFE_unverified_ref() const { return impl().get_raw_value_ref(); }
  inline auto& UNSAFE_unverified_ref() { return impl().get_raw_value_ref(); }

private:
  using T_OpDerefRet = JSTaintedVolatile<std::remove_pointer_t<T>>;

public:
};

template<typename T>
class JSTainted : public JSTaintedBase<JSTainted, T>, public JSTaintedOperations<T> {
public:
  JSTainted() = default;
  JSTainted(const JSTainted<T>&) = default;

  // constructor for JSTainted<JS::Rooted>
  template <typename RootingContext>
  explicit JSTainted(const RootingContext& cx) : data(cx) {}

  JSTainted(T val, const void* /* internal tag */) : data(val) {}

  inline auto& get_raw_value_ref() noexcept { return data; }
  inline auto& get_raw_value_ref() const noexcept { return data; }

public:
  template<typename T_Rhs>
  void assign_raw_value(T_Rhs val) {
    // TODO: somehow check that it's a type for which it makes sense to check
    // if the value is within the sandbox (is pointer-like).
    static_assert(std::is_assignable_v<T&, T_Rhs>, "Should assign compatible type");

    data = val;
  }
  
private:
  T data;
};

template<typename T>
class JSTaintedVolatile : JSTaintedBase<JSTaintedVolatile, T> {
public:
  JSTaintedVolatile() = default;

private:
  T data;
};

template <>
class JSTaintedOperations<JS::Value> {
protected:
  const JS::Value& value() const {
    return static_cast<const JSTainted<JS::Value>*>(this)->UNSAFE_unverified_ref();
  }

  void set(const JS::Value& v) {
    static_cast<JSTainted<JS::Value>*>(this)->assign_raw_value(v);
  }
  
public:
  template <typename T>
  void setNumber(T t) {
    set(JS::NumberValue(t));
  }
  
};

template <typename T>
class JSTaintedOperations<JS::Rooted<T>> {
protected:
  JS::Rooted<T>& rooted() {
    return static_cast<JSTainted<JS::Rooted<T>>*>(this)->UNSAFE_unverified_ref();
  }

  const JS::Rooted<T>& rooted() const {
    return static_cast<const JSTainted<JS::Rooted<T>>*>(this)->UNSAFE_unverified_ref();
  }
public:
  JSTainted<T>* address() {
    return reinterpret_cast<JSTainted<T>*>(rooted().address());
  }
  const JSTainted<T>* address() const { 
    return reinterpret_cast<const JSTainted<T>*>(rooted().address());
  }

  JSTainted<T>& get() {
    return *address();
  }
  const JSTainted<T>& get() const {
    return *address();
  }
};

template <typename Wrapper, typename T, typename Enable = void>
class JSTaintedWrapperOperations {};

template <typename Wrapper, typename T, typename Enable = void>
class JSTaintedMutableWrapperOperations
  : public JSTaintedWrapperOperations<Wrapper, T> {};

template<typename T>
class JSTaintedRooted : public JSTaintedMutableWrapperOperations<JSTaintedRooted<T>, T> {
public:
  template <typename RootingContext,
            typename = std::enable_if_t<std::is_copy_constructible_v<T>,
                                        RootingContext>>
  explicit JSTaintedRooted(const RootingContext& cx) : ptr(cx) {}

  void set(const T& value) {
    get().assign_raw_value(value);  
  }

  operator const JSTainted<T>&() const { return get(); }
  const JSTainted<T>& operator->() const { return get(); }

  JSTainted<T>& get() { return ptr.get(); }
  const JSTainted<T>& get() const { return ptr.get(); }

  JSTainted<T>* address() { return ptr.address(); }
  const JSTainted<T>* address() const { return ptr.address(); }

private:
  JSTainted<JS::Rooted<T>> ptr;
};

template<typename T>
class JSTaintedHandle : public JSTaintedWrapperOperations<JSTaintedHandle<T>, T> {
public:
  JSTaintedHandle(const JSTaintedHandle<T>&) = default;

  inline JSTaintedHandle(const JSTaintedRooted<T>* root) { ptr = root->address(); }

  const JSTainted<T>* address() const { return ptr; }
  const JSTainted<T>& get() const { return *ptr; }
  
  operator const JSTainted<T>&() const { get(); }
  const JSTainted<T>& operator->() const { get(); } 
private:
  JSTaintedHandle() = default;
  
  template <typename S>
  JSTaintedHandle<T>& operator=(S) = delete;
  JSTaintedHandle<T>& operator=(const JSTaintedHandle<T>&) = delete;
  
  const JSTainted<T>* ptr;
};

template<typename T>
class JSTaintedMutableHandle : public JSTaintedMutableWrapperOperations<JSTaintedMutableHandle<T>, T> {
public:
  inline JSTaintedMutableHandle(JSTaintedRooted<T>* root) {
    static_assert(sizeof(JSTaintedMutableHandle<T>) == sizeof(T*),
                  "JSTaintedMutableHandle must be binary compatible with T*.");
    ptr = root->address();
  }

private:
  JSTaintedMutableHandle(decltype(nullptr)) = delete;

public:
  JSTaintedMutableHandle(const JSTaintedMutableHandle<T>&) = default;

  const JSTainted<T>* address() const { return ptr; }
  JSTainted<T>* address() { return ptr; }
  
  const JSTainted<T>& get() const { return *ptr; }
  JSTainted<T>& get() { return *ptr; }
    
  operator const JSTainted<T>&() const { get(); }
  const JSTainted<T>& operator->() const { get(); } 
  void set(const T& v) { ptr->assign_raw_value(v); }

    //TODO: verify p is within the sandbox
    //also this is very unsafe.....
  static JSTaintedMutableHandle<T> fromMarkedLocation(JSTainted<T>* p) {
    JSTaintedMutableHandle<T> h;
    h.ptr = p;
    return h;
  }
  
private:
  JSTaintedMutableHandle() = default;

  template <typename S>
  JSTaintedMutableHandle<T>& operator=(S) = delete;
  JSTaintedMutableHandle<T>& operator=(const JSTaintedMutableHandle<T>&) = delete;
  
  JSTainted<T>* ptr;
};

template<typename Wrapper>
class JSTaintedWrapperOperations<Wrapper, JS::Value> {};

template<typename Wrapper>
class JSTaintedMutableWrapperOperations<Wrapper, JS::Value> {
public:
  template<typename T>
  void setNumber(T t) {
    static_cast<Wrapper*>(this)->get().setNumber(t);
  }
};

template<typename T>
class JSTaintedVolatileHandle {
private:
  JSTainted<T*> ptr;
};

class JSTaintedJitGetterCallArgs : protected JSTaintedMutableHandle<JS::Value> {
    public : 
    explicit JSTaintedJitGetterCallArgs(JSTaintedMutableHandle handle) 
    : JSTaintedMutableHandle(handle) {}
    JSTaintedMutableHandle<JS::Value> rval() { return *static_cast<JSTaintedMutableHandle<JS::Value>*> (this); }
};
  
}

}

#endif
