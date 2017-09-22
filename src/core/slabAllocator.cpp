#include "slabAllocator.h"
#include <cassert>

namespace babycpp {
namespace memory {
SlabAllocator::SlabAllocator(int slabSizeInByte) : slabSize(slabSizeInByte) {
  assert(slabSize != 0);
  // allocating a slab
  Slab &createdSlab = allocateSlab();
  currentSlab = &createdSlab;
}

void *SlabAllocator::alloc(uint32_t byteSize) {

  // making sure we have enough space
  assert(byteSize <= slabSize);
  if (currentSlab->rsp + byteSize > currentSlab->endp) {
    Slab &createdSlab = allocateSlab();
    currentSlab = &createdSlab;
  }
  // shifting stack pointer to allocate the required amount
  char *old = currentSlab->rsp;
  currentSlab->rsp += byteSize;
  return old;
}

Slab &SlabAllocator::allocateSlab() {
  // need to allocate memory
  auto *data = new char[slabSize];
  slabs.emplace_back(Slab{data, data, data + slabSize});
  return slabs[slabs.size() - 1];
}

void SlabAllocator::clear() {

  //de-allocating the memory except the first one
  uint32_t currentSize = slabs.size();
  for (uint32_t i = 1; i < currentSize; ++i) {
    delete[] slabs[i].data;
  }
  // keeping just one slab
  slabs.resize(1);
  currentSlab = &slabs[0];
  currentSlab->rsp = currentSlab->data;
}
} // namespace memory
} // namespace babycpp
