#ifndef mc_GCPolicyAPI_h
#define mc_GCPolicyAPI_h

#include "js/GCPolicyAPI.h"

#ifdef JS_SANDBOX

#include "monkeycage/RootingAPI.h"

namespace JS {

template <typename T>
struct GCPolicy<MC::Heap<T>> {
  static void trace(JSTracer* trc, MC::Heap<T>* thingp, const char* name) {
    TraceEdge(trc, thingp->INTERNAL_unverified_safe(), name);
  }
  static bool traceWeak(JSTracer* trc, MC::Heap<T>* thingp) {
    return !*thingp ||
           js::gc::TraceWeakEdge(trc, thingp->INTERNAL_unverified_safe());
  }
};
}

#endif

#endif
