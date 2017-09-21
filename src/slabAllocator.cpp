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
  //shifting stack pointer to allocate the required amount
  char *old = currentSlab->rsp;
  currentSlab->rsp += byteSize;
  return old;
}

Slab &SlabAllocator::allocateSlab() {
  // need to allocate memory
  char *data = new char[slabSize];
  slabs.emplace_back(Slab{data, data, data + slabSize});
  return slabs[slabs.size() - 1];
}
} // namespace memory
} // namespace babycpp
