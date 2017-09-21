#pragma once

#include "allocator.h"
#include <vector>

namespace babycpp {
namespace memory {

/**
 * Simple struct representing an area of allocation
 * @param data the start of the alloated memory
 * @param rsp short for stack pointer, represent where
 * the currently allocated user memory ends
 * @param endp represent the end of the slab, aka system
 * allocated memory
 */
struct Slab {
  char *data;
  char *rsp;
  char *endp;
};

/**
 * Default size for slabs, if not provided this will be used
 */
const uint32_t SLAB_SIZE = 1 << 21;
// Allocator interface
struct SlabAllocator : Allocator {
  /**
   * Constructor
   * @param slabSizeInByte how biges are the slabs in bites,
   *        each new slab will have this size
   */
  explicit SlabAllocator(int slabSizeInByte = SLAB_SIZE);
  /**
   * Allocates the requested amount of memory
   * @param byteSize the size of the requested allocation in bytes
   * @return pointer to the start of allocated memory
   */
  void *alloc(uint32_t byteSize) override;
  /**
   * @brief clear the allocation memory
   * Clears the internal memory and all the slabs, only
   * leaves a single slab allocated and empty
   * WARNING: the allocator does not call any kind of free
   * on the content of the memory, but frees only the whole
   */
  void clear() override;
  /**
   * @brief Allocates a new slabs an returns a ref to it
   * @return  newly allocated slab
   */
  Slab &allocateSlab();
  virtual ~SlabAllocator()
  {
	  //freeing all slab except first one
	  clear();
	  //freeing last surviving slab
	  delete[] currentSlab->data;
  };

  /**
   * Automatically computes the offet between the base
   * and the stack pointer, this offset represent the amount
   * of allocation in bytes
   * @return offets in byte
   */
  inline uint64_t getStackPtrOffset() const {
    if (currentSlab != nullptr) {
      return (currentSlab->rsp - currentSlab->data);
    }
    return 0;
  }

  // data
  /**
   * All the allocated slabs
   */
  std::vector<Slab> slabs;
  /**
   * Currently in used slabs, when one is full, a new
   * one gets allocated and this pointer changed to the new one
   */
  Slab *currentSlab = nullptr;
  uint32_t slabSize;

  // making the allocator not copyable,movable etc
  SlabAllocator(const SlabAllocator &other) = delete;
  SlabAllocator &operator=(const SlabAllocator &other) = delete;
};
} // namespace memory
} // namespace babycpp
