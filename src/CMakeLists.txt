cmake_minimum_required(VERSION 3.6.0)

project(${MAIN_LIB_NAME})   

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
    #options
    option(BUILD_TESTS "Wheter or not to build the tests" ON)

    #just an overal log of the passed options
    MESSAGE( STATUS "LLVM PATH :                   " ${LLVM_ROOT})
    MESSAGE( STATUS "Building with the following options")
    MESSAGE( STATUS "BUILD TESTS:                  " ${BUILD_TESTS})

    #looking for  files
    file(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.h")

    message (STATUS "found CPU files")
    foreach(dir ${SOURCE_FILES})
          message(STATUS "dir='${dir}'")
    endforeach()
    #looking for the header files
    file(GLOB_RECURSE INCLUDES_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

    message (STATUS "found includes files")
    foreach(dir ${INCLUDES_FILES})
         message(STATUS "dir='${dir}'")
    endforeach()

    #setupping llvm 
    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    include_directories(${CMAKE_SOURCE_DIR}/include ${LLVM_ROOT}/include)
    #defining standard compiling flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} -fno-exceptions")

    #adding the executable
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${INCLUDES_FILES})

    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core irreader)

    # Link against LLVM libraries
    target_link_libraries(${PROJECT_NAME} ${llvm_libs})

    #enabling clang tidy
    enable_clang_tidy_for_project()
