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
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS} -g3 -O0 -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math -fno-exception")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}   -Wall -pedantic -Wextra -m64 -mavx2 -mfma -ffast-math")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(COMMON_CXX_FLAGS "${COMMON_CXX_FLAGS}  /std:c++14 /W4 /arch:AVX2 /fp:fast")
endif()

find_package(LLVM REQUIRED CONFIG)
#options
option(BUILD_TESTS "Whether or not to build the tests" ON)
option(BUILD_JIT   "Whether or not to build the jit engine" ON)
option(BUILD_REPL  "Whether or not to build the interpreter" ON)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

#just an overal log of the passed options
MESSAGE( STATUS "LLVM PATH :                   " ${LLVM_DIR})
MESSAGE( STATUS "Building with the following options")
MESSAGE( STATUS "BUILD JIT:                    " ${BUILD_JIT})
MESSAGE( STATUS "BUILD REPL:                   " ${BUILD_REPL})
MESSAGE( STATUS "BUILD TESTS:                  " ${BUILD_TESTS})
#adding core
add_subdirectory(src/core)
if(${BUILD_JIT} STREQUAL "ON")
add_subdirectory(src/jit)
endif()
if(${BUILD_REPL} STREQUAL "ON")
    add_subdirectory(src/repl)
endif()

#adding test
if(${BUILD_TESTS} STREQUAL "ON")
    add_subdirectory(tests/core)

    if(${BUILD_JIT} STREQUAL "ON")
        add_subdirectory(tests/jit)
    endif()

    if(${BUILD_REPL} STREQUAL "ON")
        add_subdirectory(tests/repl)
    endif()

    if(${BUILD_JIT} STREQUAL "ON" AND  ${BUILD_REPL} STREQUAL "ON"  )
        add_subdirectory(tests)
	endif()
endif()


#adding examples
add_subdirectory(examples/mayaNode)