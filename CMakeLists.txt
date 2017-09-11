cmake_minimum_required(VERSION 3.6.0)

#preventing in source build
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
        message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
    endif()

#compile database
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

#including utils function, this will cascade to all subsequent included cmakes
include (cmakeutils/code_gen.txt)

#setupping clang tidy
init_clang_tidy()

SET(MAIN_LIB_NAME "babycpp")

#enable c++ 17
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#check the compiler
MESSAGE(STATUS "compiler id ${CMAKE_CXX_COMPILER_ID} ${MSVC}")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}  -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math ")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}   -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}  /std:c++14 /W4 /arch:AVX2 /fp:fast")
endif()

#adding core 
add_subdirectory(src)

#adding test
add_subdirectory(tests)
