#include "mozilla/dom/JSTainted.h"

namespace mozilla {

namespace dom {
mozilla::HashSet<const char16_t *> TaintedExternalStringBacking (1);

TaintTable allExternalPtr (32);

mozilla::RWLock externalPtrLock = mozilla::RWLock("ExternalPtrLock");

}

}
