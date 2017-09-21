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

//default slab size two MB for the use case should be more than enough
const uint32_t SLAB_SIZE = 1 << 21;
// Allocator interface
struct SlabAllocator : Allocator {
  explicit SlabAllocator(int slabSizeInByte = SLAB_SIZE);
  void *alloc(uint32_t byteSize) override { return nullptr; };
  void clear() override{};
  Slab& allocateSlab();

  virtual ~SlabAllocator() = default;

  // data
  std::vector<Slab> slabs;
  Slab *currentSlab = nullptr;
  uint32_t slabSize;

  //making the allocator not copiable,movable etc
  SlabAllocator(const SlabAllocator& other) = delete;
  SlabAllocator& operator = (const SlabAllocator& other) = delete;
};
} // namespace memory
} // namespace babycpp
