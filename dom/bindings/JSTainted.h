/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
 
#ifndef mozilla_dom_JSTainted_h__
#define mozilla_dom_JSTainted_h__

#include <type_traits>
#include <functional>
#include <uchar.h>
#include <mozilla/Tainting.h>
#include <mozilla/AlreadyAddRefed.h>
#include "js/RootingAPI.h"
#include "js/Value.h"
#include "js/experimental/JitInfo.h"

namespace mozilla {

namespace dom {

extern mozilla::HashSet<const char16_t *> TaintedExternalStringBacking;

template<typename T>
class TaintObj {
	public:
	static inline mozilla::HashSet<void*> PtrTable = mozilla::HashSet<void*>(1);
	TaintObj() {
		if(!PtrTable.put(this)) {
			MOZ_CRASH("Couldn't add new pointer to pointer table");
        }
	}
	~TaintObj() {
		PtrTable.remove(this);
	}
};

template<typename T>
class JSTainted;

template<typename T>
class JSTaintedVolatile;

template <typename T, typename Enable = void>
class JSTaintedOperations {};

template<typename T>
class JSAppPtr;

template<typename T>
class JSAppPtr {
  public:
  JSAppPtr(void * ptr) : app_ptr(ptr) {}
  
  template <typename O>
  O* verify(const mozilla::HashSet<void*> & PtrTable) {
    if(PtrTable.has(static_cast<void*>(app_ptr))) {
      return static_cast<O*>(app_ptr);
    } else {
      MOZ_CRASH("Invalid AppPtr Detected.");
    }
  }

  //TODO: replace w/ macros that allow branching
  //see Tainted types under mfbt
  T* verify(std::function<bool(T*)> f) {
    if(f(static_cast<T*>(app_ptr))) {
        return static_cast<T*>(app_ptr);
    } else {
        return nullptr;
    }
  }

  T* UNVERIFIED_ref(void) {
    return static_cast<T*>(app_ptr);
  }

  operator bool() {
    return app_ptr != nullptr;
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
  JSTainted(T val) : data(val) {}

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

template <typename T>
class JSTainted<already_AddRefed<T>> {
    public:
        JSTainted() : data (nullptr) {}
        MOZ_IMPLICIT JSTainted(decltype(nullptr)) : data(nullptr) {}
        explicit JSTainted(T* rawptr) : data (rawptr) {}

        JSTainted(JSTainted<already_AddRefed<T>>&& other) 
        : data (other.data) {}

        JSTainted(already_AddRefed<T>&& other)
        : data (other.take()) {}
        
        JSTainted<already_AddRefed<T>>&
        operator=(already_AddRefed<T>&& other) {
            data = other.data;
            return *this;
        } 

        inline auto& UNVERIFIED_ref() noexcept { return data; }

    private:
    already_AddRefed<T> data;
};

template <>
class JSTaintedOperations<const char16_t**> {
  public:
    const char16_t ** verify(std::function<bool(const char16_t **)> f) {
      const char16_t ** data = static_cast<JSTainted<const char16_t**>*>(this)->get_raw_value_ref();
      if(f(data)) {
        return data;
      } else {
        MOZ_CRASH("Invalid JSExternalString backing store.");
      }
    }
};

template <>
class JSTaintedOperations<uint32_t> {
    public:
    JSTainted<uint32_t> operator & (uint32_t other) {
         uint32_t data = static_cast<JSTainted<uint32_t>*>(this)->get_raw_value_ref();
         JSTainted<uint32_t> ret (data & other);
         return ret;
    }
    
    operator JSTainted<bool>() {
        uint32_t data = static_cast<JSTainted<uint32_t>*>(this)->get_raw_value_ref();
        JSTainted<bool> ret (data != 0);
        return ret;
    }
};


template<typename T>
class JSTaintedVolatile : JSTaintedBase<JSTaintedVolatile, T> {
public:
  JSTaintedVolatile() = default;

private:
  T data;
};

template <typename T>
class JSTaintedOperations<T*> {
    public:
    operator bool() {
        return 
            static_cast<JSTainted<T*>*>(this)->UNSAFE_unverified_ref() 
                == nullptr;
    }
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

  template <typename T>
  void setString(T t) {
    set(JS::StringValue(t));
  }

  void setUndefined(void) {
    set(JS::UndefinedValue());
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

  void set(const JSTainted<T>& value) {
    get().assign_raw_value(value.get_raw_value_ref());
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

template <typename T>
class JSTaintedMutableWrapperOperations<JSTaintedRooted<T*>, T*> {
  public:
  operator bool() {
    return (reinterpret_cast<JSTaintedRooted<T*>*>(this))->get().UNSAFE_unverified_ref() != nullptr;
  }
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

  operator bool() {
    return ptr != nullptr;
  }

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

  operator bool() {
    return ptr != nullptr;
  }

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

  template<typename T>
  void setString(T t) {
    static_cast<Wrapper*>(this)->get().setString(t);
  }

  template<typename T>
  void setUndefined(T t) {
    static_cast<Wrapper*>(this)->get().setUndefined();
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

class JSTaintedJitSetterCallArgs : protected JSTaintedMutableHandle<JS::Value> {
    public:
        explicit JSTaintedJitSetterCallArgs(JSTaintedMutableHandle<JS::Value> rooted) :
        JSTaintedMutableHandle(rooted) {}

        JSTaintedMutableHandle<JS::Value> operator[](unsigned i) {
            MOZ_ASSERT(i == 0);
            return *this;
        }

        unsigned length() const { return 1; }
};

class MOZ_STACK_CLASS TaintedGlobalObject {
    public:
    TaintedGlobalObject(JSContext* aCx, JSTainted<JSObject*> aObject);
   
     JSTainted<JSObject*> Get() const { return mGlobalJSObject; }
      
     // The context that this returns is not guaranteed to be in the compartment of
     // the object returned from Get(), in fact it's generally in the caller's
     // compartment.
     JSContext* Context() const { return mCx; }
   
     bool Failed() const { return !Get().UNSAFE_unverified_ref(); }

    nsISupports* GetAsSupports() const;

	JSAppPtr<nsISupports> GetAsTaintedSupports(void) const;
      
    protected:
     JSTaintedRooted<JSObject*> mGlobalJSObject;
     JSContext* mCx;
     mutable nsISupports* MOZ_UNSAFE_REF(
         "Valid because GlobalObject is a stack "
         "class, and mGlobalObject points to the "
         "global, so it won't be destroyed as long "
         "as GlobalObject lives on the stack") mGlobalObject;
};
   
//TODO:
// create reflection of https://searchfox.org/mozilla-esr115/source/js/public/experimental/JitInfo.h#75
// 
template <>
class JSTaintedOperations<JSJitMethodCallArgs> {
  public:

    mozilla::Tainted<bool> requireAtLeast(JSContext* cx, const char* fnname,
						unsigned required) {
	//We should be fine just forwarding this request.
    //worst-case is that some nonsense sandbox data gets accessed,
    //which we won't treat as trusted anyway
    
    JSJitMethodCallArgs& data = 
        static_cast<JSTainted<JSJitMethodCallArgs>*>(this)->
        get_raw_value_ref();
    mozilla::Tainted<bool> result (data.requireAtLeast(cx, fnname, required));
    return result;
  }

  mozilla::Tainted<unsigned> length(void) {
    JSJitMethodCallArgs& data = 
        static_cast<JSTainted<JSJitMethodCallArgs>*>(this)->
        get_raw_value_ref();
    mozilla::Tainted<unsigned> result (data.length());
    return result;
  }

  /*
  JSTaintedMutableHandle<JS::Value> rval(void) {
    JS::MutableHandle<JS::Value> untaint_rval = 
        static_cast<JSTainted<JSJitMethodCallArgs>*>(this)->get_raw_value_ref();
        
  }
  */

};
  
} //namespace DOM

} //namespace mozilla

#endif
