/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mc_Vector_h
#define mc_Vector_h

#include "Tainted.h"
#include "js/Vector.h"
#include "monkeycage/Tainted.h"

namespace MC {
namespace detail {

template <typename T, typename MC_Sbx, size_t MinInlineCapacity,
          class AllocPolicy>
class TaintedVolatile<js::Vector<T, MinInlineCapacity, AllocPolicy>, MC_Sbx> {
private:
  js::Vector<T, MinInlineCapacity, AllocPolicy> data;

public:
  Tainted<size_t, MC_Sbx> length() const {
    return Tainted<size_t, MC_Sbx>(data.length());
  }

  Tainted<const T*, MC_Sbx> begin() const {
    Tainted<const T*, MC_Sbx> ret(nullptr);
    ret.assign_raw_pointer(data.begin());
    return ret;
  }

  template <typename U>
  Tainted<bool, MC_Sbx> append(const U* aBegin, size_t aLength) {
    return Tainted<bool, MC_Sbx>(data.append(aBegin, aLength));
  }

  T* copyRawBuffer() {
    return data.copyRawBuffer();
  }

  size_t sizeOfIncludingThis(mozilla::MallocSizeOf aMallocSizeOf) const {
    return data.sizeOfIncludingThis(aMallocSizeOf);
  }
  
  size_t sizeOfExcludingThis(mozilla::MallocSizeOf aMallocSizeOf) const {
    return data.sizeOfExcludingThis(aMallocSizeOf);
  }
};
          
}
}

#endif
