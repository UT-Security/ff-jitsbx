#ifndef mc_GCPolicyAPI_h
#define mc_GCPolicyAPI_h

#include "js/GCPolicyAPI.h"

#ifdef JS_SANDBOX

#include "monkeycage/RootingAPI.h"
#include "monkeycage/TracingAPI.h"

namespace JS {

template <typename T>
struct GCPolicy<MC::Heap<T>> {
  static void trace(JSTracer* trc, MC::Heap<T>* thingp, const char* name) {
    TraceEdge(trc, thingp, name);
  }
  static bool traceWeak(JSTracer* trc, MC::Heap<T>* thingp) {
    return !*thingp ||
           js::gc::TraceWeakEdge(trc, thingp->INTERNAL_unverified_safe());
  }
};
}

#endif

#endif
