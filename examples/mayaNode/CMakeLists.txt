cmake_minimum_required(VERSION 3.6.0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmakeutils/modules")

project(babyMaya)   

    find_package(LLVM REQUIRED CONFIG)
    find_package(Maya REQUIRED)


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
    include_directories(${LLVM_INCLUDE_DIRS} 
	                    ${MAYA_INCLUDE_DIR} 
						${CMAKE_SOURCE_DIR}/include/core
                        ${CMAKE_SOURCE_DIR}/include/jit)
    add_definitions(${LLVM_DEFINITIONS})

    include_directories(${CMAKE_SOURCE_DIR}/include/core ${LLVM_INCLUDE_DIRS})
	link_directories(${MAYA_LIBRARY_DIR})
    #defining standard compiling flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} -fno-exceptions ")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324 /wd4146 /wd4458 /wd4267 /wd4100 /wd4244 /wd4141 /wd4291 /wd4624 ")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	endif()

    #adding the executable
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${INCLUDES_FILES})


    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core irreader)

    # Link against LLVM libraries
    target_link_libraries(${PROJECT_NAME} ${llvm_libs} ${MAYA_LIBRARIES} ${MAIN_LIB_NAME} babycppjit )

    #enabling clang tidy
    enable_clang_tidy_for_project()

	MAYA_PLUGIN(${PROJECT_NAME})
