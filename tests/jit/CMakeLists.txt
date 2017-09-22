cmake_minimum_required(VERSION 3.6)
SET(PROJECT_NAME "jitTest")
project(${PROJECT_NAME})

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    file(GLOB SOURCE_FILES "*.cpp" "*.h")
    include_directories(${CMAKE_SOURCE_DIR}/include/core
                        ${CMAKE_SOURCE_DIR}/include/jit
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
    MESSAGE(STATUS "____________________________ ${llvm_libs}")

    target_link_libraries(${PROJECT_NAME} ${MAIN_LIB_NAME} babycppjit ${llvm_libs})


    file(GLOB files "testdata/*.ll")
    foreach(file ${files})
        file(COPY ${file} DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME} )
    endforeach()


	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	   set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

    ##check if to run the test or not
    #if(${RUN_TESTS} STREQUAL "ON")
    #    if (NOT WIN32)


    #        #now the problem is that if i am in build, because I did cmake ../, the test executable
    #        #wont be in the build directory but in a sub folder tests, going into the sub folder
    #        #run getting out is a pain, so I am adding a custom command to trigger the test after
    #        #build and setting the working directo as the current output dir, and expect
    #        #the test file to be picked as tests/myTestFile.txt

    #        add_custom_command(
    #            DEPENDS tests
    #            COMMAND ./tests/tests
    #            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    #            OUTPUT run_my_tests)

    #        # Custom target will always cause its dependencies to be evaluated and is
    #        # run by default
    #        add_custom_target(dummy_target ALL
    #            DEPENDS
    #              run_my_tests
    #              create_dataset_soft_link
    #            )
    #        MESSAGE(STATUS "Creating soft link for dataset: \n  ${CMAKE_SOURCE_DIR}/tests/test_data/datasets ->  ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/datasets")
    #        #command to generate a soft link to the dataset folder
    #            add_custom_command(
    #                DEPENDS tests
    #                COMMAND ln -f -s ${CMAKE_SOURCE_DIR}  ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/datasets
    #                OUTPUT create_dataset_soft_link
    #                )
    #    endif()
    #
    #endif()
