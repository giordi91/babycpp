cmake_minimum_required(VERSION 3.6)
SET(PROJECT_NAME "replTest")
project(${PROJECT_NAME})

    find_package(LLVM REQUIRED CONFIG)

    file(GLOB SOURCE_FILES "*.cpp" "*.h")
    include_directories(${CMAKE_SOURCE_DIR}/include/core
                        ${CMAKE_SOURCE_DIR}/include/jit
                        ${CMAKE_SOURCE_DIR}/include/repl
                        ${LLVM_INCLUDE_DIRS}
                        ${CMAKE_CURRENT_SOURCE_DIR}/..)
    add_definitions(${LLVM_DEFINITIONS})


	#defining standard compiling flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}  -fno-rtti")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4324 /wd4146 /wd4458 /wd4267 /wd4100 /wd4244 /wd4141 /wd4291 /wd4624 ")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
	endif()

	#adding the executable
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} )

    # Find the libraries that correspond to the LLVM components
    # that we wish to use
    llvm_map_components_to_libnames(llvm_libs support core codegen irreader orcjit native)

    target_link_libraries(${PROJECT_NAME} ${MAIN_LIB_NAME} babycppjit babycppreplbase ${llvm_libs})


    file(GLOB files "testdata/*.ll")
    foreach(file ${files})
        file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME} )
    endforeach()


	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	   set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

