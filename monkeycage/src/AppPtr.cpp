#include <monkeycage/AppPtr.h>

namespace MC {
namespace detail {

TaintTable allExternalPtr (32);
mozilla::RWLock externalPtrLock = mozilla::RWLock("App pointer lock");

}
}
