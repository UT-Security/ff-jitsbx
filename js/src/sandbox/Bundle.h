#ifndef sandbox_Bundle_h
#define sandbox_Bundle_h

#include <cstddef>

namespace js {

namespace sandbox {

constexpr size_t BUNDLE_SIZE = 32;

static bool isSameBundle(size_t x, size_t y) {
  return (x / BUNDLE_SIZE) == (y / BUNDLE_SIZE);
} 
  
}
  
}

#endif 
