#pragma once

#include <cstdint>

namespace babycpp {
namespace memory {

    /**
     * @brief abstract allocator interface
     * All the allocator will need to derive from this,
     * the interface is minimal, later on will be expanded if needed
     * for example a free, dealloc method is not required since some
     * allocators might not need or want to deallocate
     */
struct Allocator {
  //interface
   /**
    * @brief alloc the specific amount of memory
    * @param byteSize
    * @return the starting pointer of the allocated memory
    */
  virtual void *alloc(uint32_t byteSize) = 0;
    /**
     * @brif release all the memory af the allocator
     */
  virtual void clear() = 0;

  Allocator() = default;
  virtual ~Allocator() = default;

  //making the allocator not copyable,movable etc
  Allocator(const Allocator& other) = delete;
  Allocator& operator = (const Allocator& other) = delete;

};
} // namespace memory
} // namespace babycpp