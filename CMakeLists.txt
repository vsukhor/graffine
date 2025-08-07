cmake_minimum_required(VERSION 3.28)

project(graph_mutator
    VERSION 0.31.0
    DESCRIPTION "Graph Elementary Transformations"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-ggdb3 -Wall) #(-O3 -msse4 -Wall)

if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
        set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Let's ensure -std=++cxx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_VERBOSE_MAKEFILE OFF)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

   option(BUILD_DOC "Build documentation" ON)
    if(BUILD_DOC)
#        add_subdirectory(docs)
    endif()
endif()

# The source code is here
include_directories(graph_mutator_lib PUBLIC
                    include
                    external/utils)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(BUILD_TESTS "Build test files" ON)
    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()
