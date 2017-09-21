#pragma once

#include "allocator.h"
#include <vector>

namespace babycpp {
namespace memory {

struct Slab {
  char *data;
  char *rsp;
  char *endp;
};

// default slab size two MB for the use case should be more than enough
const uint32_t SLAB_SIZE = 1 << 21;
// Allocator interface
struct SlabAllocator : Allocator {
  explicit SlabAllocator(int slabSizeInByte = SLAB_SIZE);
  void *alloc(uint32_t byteSize) override;
  /*!
   * @brief clear the allocation memory
   * Clears the internal memory and all the slabs, only
   * leaves a single slab allocated and empty
   * WARNING: the allocator does not call any kind of free
   * on the content of the memory, but frees only the whole
   * h
   */
  void clear() override;
  Slab &allocateSlab();
  virtual ~SlabAllocator() = default;

  inline uint64_t getStackPtrOffset() const {
    if (currentSlab != nullptr) {
      return (currentSlab->rsp - currentSlab->data);
    }
    return 0;
  }

  // data
  std::vector<Slab> slabs;
  Slab *currentSlab = nullptr;
  uint32_t slabSize;

  // making the allocator not copyable,movable etc
  SlabAllocator(const SlabAllocator &other) = delete;
  SlabAllocator &operator=(const SlabAllocator &other) = delete;
};
} // namespace memory
} // namespace babycpp
