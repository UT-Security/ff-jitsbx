#include <monkeycage/AppPtr.h>

namespace MC {
namespace detail {

#ifdef MC_APP_PTR
TaintTable allExternalPtr (32);
mozilla::RWLock externalPtrLock = mozilla::RWLock("App pointer lock");
#endif

}
}
