#include "catch.hpp"
#include "slabAllocator.h"
#include <iostream>
#include <type_traits>

using babycpp::memory::SlabAllocator;
using babycpp::memory::Slab;

TEST_CASE("Testing allocator not movable", "[memory]") 
{
	REQUIRE(std::is_copy_constructible<SlabAllocator>::value == false);
	REQUIRE(std::is_copy_assignable<SlabAllocator>::value == false);
}

TEST_CASE("Testing allocator instantiation", "[memory]") 
{
	SlabAllocator slab;
	REQUIRE(slab.currentSlab != nullptr);

	Slab& currSlab = *slab.currentSlab;
	REQUIRE(slab.currentSlab != nullptr);
	REQUIRE(slab.currentSlab == &slab.slabs[0]);
}

