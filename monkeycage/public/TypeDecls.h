/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// This file contains public type declarations that are used *frequently*.  If
// it doesn't occur at least 10 times in Gecko, it probably shouldn't be in
// here.
//
// It includes only:
// - forward declarations of structs and classes;
// - typedefs;
// - enums (maybe).
// It does *not* contain any struct or class definitions.

#ifndef mc_TypeDecls_h
#define mc_TypeDecls_h

#include "js/TypeDecls.h"

namespace MC {

#ifdef JS_SANDBOX
template <typename T>
class MutableHandle;
template <typename T>
class Rooted;
template <typename T>
class PersistentRooted;
template <typename T>
class RootedVector;

template <typename T, typename AllocPolicy = js::TempAllocPolicy>
using StackGCVector = JS::StackGCVector<T, AllocPolicy>;
#else
template <typename T>
using Rooted = JS::Rooted<T>;

template <typename T>
using MutableHandle = JS::MutableHandle<T>;

template <typename T>
using PersistentRooted = JS::PersistentRooted<T>;

template <typename T, typename AllocPolicy = js::TempAllocPolicy>
using StackGCVector = JS::StackGCVector<T, AllocPolicy>;
#endif

typedef MutableHandle<JSFunction*> MutableHandleFunction;
typedef MutableHandle<JS::PropertyKey> MutableHandleId;
typedef MutableHandle<JSObject*> MutableHandleObject;
typedef MutableHandle<JSScript*> MutableHandleScript;
typedef MutableHandle<JSString*> MutableHandleString;
typedef MutableHandle<JS::Symbol*> MutableHandleSymbol;
typedef MutableHandle<JS::BigInt*> MutableHandleBigInt;
typedef MutableHandle<JS::Value> MutableHandleValue;

typedef Rooted<JSObject*> RootedObject;
typedef Rooted<JSFunction*> RootedFunction;
typedef Rooted<JSScript*> RootedScript;
typedef Rooted<JSString*> RootedString;
typedef Rooted<JS::Symbol*> RootedSymbol;
typedef Rooted<JS::BigInt*> RootedBigInt;
typedef Rooted<JS::PropertyKey> RootedId;
typedef Rooted<JS::Value> RootedValue;
}

#endif
