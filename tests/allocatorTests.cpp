#include "catch.hpp"
#include "slabAllocator.h"
#include <iostream>
#include <type_traits>

using babycpp::memory::Slab;
using babycpp::memory::SlabAllocator;

TEST_CASE("Testing allocator not movable", "[memory]") {
  REQUIRE(std::is_copy_constructible<SlabAllocator>::value == false);
  REQUIRE(std::is_copy_assignable<SlabAllocator>::value == false);
}

TEST_CASE("Testing allocator instantiation", "[memory]") {
  SlabAllocator slab;
  REQUIRE(slab.currentSlab != nullptr);

  //making sure there is at least one slab when istantiated
  Slab &currSlab = *slab.currentSlab;
  REQUIRE(slab.currentSlab != nullptr);
  REQUIRE(slab.currentSlab == &slab.slabs[0]);
  REQUIRE((currSlab.endp - currSlab.data) == babycpp::memory::SLAB_SIZE);
}
TEST_CASE("Testing allocator instantiation, not default slab size", "[memory]") {
  SlabAllocator slab(200);
  REQUIRE(slab.currentSlab != nullptr);

  //making sure there is at least one slab when istantiated
  Slab &currSlab = *slab.currentSlab;
  REQUIRE(slab.currentSlab != nullptr);
  REQUIRE(slab.currentSlab == &slab.slabs[0]);
  REQUIRE((currSlab.endp - currSlab.data) == 200);
}
TEST_CASE("Testing allocation of memory", "[memory]")
{
  SlabAllocator slab;

