cmake_minimum_required(VERSION 3.28)

project(graffine
    VERSION 1.84.0
    DESCRIPTION "Elementary Transformations on Graphs"
    LANGUAGES CXX)

add_compile_options("-std=c++23" "-Wall" "-Wextra" "-fexceptions")
if(CMAKE_BUILD_TYPE==Debug)
    add_compile_options("-O0;-g3;-ggdb")
elseif(CMAKE_BUILD_TYPE==Release)
    add_compile_options("-O3; -march=native")
endif()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

message("CXX Compiler: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Let's ensure -std=++cxx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_VERBOSE_MAKEFILE OFF)

    # Add the cmake folder so the FindSphinx module is found
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    option(BUILD_DOCS "Build documentation" OFF)
    if(BUILD_DOCS)
        add_subdirectory(docs)
    endif()

endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    option(BUILD_TESTS "Build test files" ON)
    if(BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()

    option(BUILD_EXAMPLES "Build examples illustrating basic library usage" ON)
    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

endif()
