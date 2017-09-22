#pragma once

#include <cstdint>

namespace babycpp {
namespace memory {

// Allocator interface
struct Allocator {
  virtual void *alloc(uint32_t byteSize) = 0;
  virtual void clear() = 0;

  Allocator() = default;
  virtual ~Allocator() = default;

  //making the allocator not copiable,movable etc
  Allocator(const Allocator& other) = delete;
  Allocator& operator = (const Allocator& other) = delete;

};
} // namespace memory
} // namespace babycpp