cmake_minimum_required(VERSION 3.6)
SET(PROJECT_NAME "ALLTest")
project(${PROJECT_NAME})

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    file(GLOB CORE_SOURCE_FILES "core/*.cpp" "core/*.h")
    file(GLOB JIT_SOURCE_FILES "jit/*.cpp" "jit/*.h")
    file(GLOB REPL_SOURCE_FILES "repl/*.cpp" "repl/*.h")

    include_directories(${CMAKE_SOURCE_DIR}/include/core
						${CMAKE_SOURCE_DIR}/include/jit
						${CMAKE_SOURCE_DIR}/include/repl
                        ${LLVM_INCLUDE_DIRS} 
                        ${CMAKE_CURRENT_SOURCE_DIR}) #used for catch

    add_definitions(${LLVM_DEFINITIONS})


	#defining standard compiling flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324 /wd4146 /wd4458 /wd4267 /wd4100 /wd4244 /wd4141 /wd4291 /wd4624 ")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	endif()
	
	add_definitions(-DCUMULATIVE_TESTS)

	#adding the executable
    add_executable(${PROJECT_NAME} ${CORE_SOURCE_FILES} ${JIT_SOURCE_FILES} ${REPL_SOURCE_FILES})

    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core irreader)

    target_link_libraries(${PROJECT_NAME} ${MAIN_LIB_NAME} babycppjit  babycppreplbase ${llvm_libs})


    file(GLOB files "${CMAKE_SOURCE_DIR}/tests/testdata/*.ll")
    foreach(file ${files})
        file(COPY ${file} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    endforeach()


	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	   set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()
