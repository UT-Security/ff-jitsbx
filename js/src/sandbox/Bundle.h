#ifndef sandbox_Bundle_h
#define sandbox_Bundle_h

#include <cstddef>
#include <cstdint>

namespace js {

namespace sandbox {

constexpr size_t BUNDLE_SIZE = 32;
constexpr int32_t BUNDLE_MASK = 0xffffffe0; 

static bool isSameBundle(size_t x, size_t y) {
  return (x / BUNDLE_SIZE) == (y / BUNDLE_SIZE);
} 
  
}
  
}

#endif 
