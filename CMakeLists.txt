cmake_minimum_required(VERSION 3.28)

project(graffine
    VERSION 1.85.0
    DESCRIPTION "Elementary Transformations on Graphs"
    LANGUAGES CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0135 NEW)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

message("CXX Compiler: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})

# LIBRARY TARGET ---------------------------------------------------------------

# Header-only: consumers link to graffine::graffine to get the include paths
# and the C++ standard requirement.
add_library(graffine INTERFACE)
add_library(graffine::graffine ALIAS graffine)

target_compile_features(graffine INTERFACE cxx_std_23)

# Location of the generated config_cmake.h.
set(GRAFFINE_GENERATED_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")

# The modified tinyply is installed under graffine/ to keep its generic
# header names (reader.h, writer.h, ...) out of the top-level include dir.
set(GRAFFINE_INSTALL_TINYPLY_DIR
    "${CMAKE_INSTALL_INCLUDEDIR}/graffine/external/tinyply")

target_include_directories(graffine INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${GRAFFINE_GENERATED_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/tinyply>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<INSTALL_INTERFACE:${GRAFFINE_INSTALL_TINYPLY_DIR}>
)

# CONFIGURATION HEADER ---------------------------------------------------------

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/graffine")

# Whether functors should report verbosely.
if(PROJECT_IS_TOP_LEVEL)
    if(NOT DEFINED GRAFFINE_FUNCTORS_VERBOSE)
        set(GRAFFINE_FUNCTORS_VERBOSE ON)  # ON/OFF
    endif()
    if(NOT DEFINED GRAFFINE_PRINT_EDGES)
        set(GRAFFINE_PRINT_EDGES ON)  # ON/OFF
    endif()
    if(NOT DEFINED GRAFFINE_PRINT_VERTICES)
        set(GRAFFINE_PRINT_VERTICES ON)  # ON/OFF
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        if(NOT DEFINED GRAFFINE_DO_INTEGRAL_TESTS)
            set(GRAFFINE_DO_INTEGRAL_TESTS ON)  # ON/OFF
        endif()
    endif()
    if(NOT DEFINED GRAFFINE_REAL_IS_DOUBLE)
        set(GRAFFINE_REAL_IS_DOUBLE OFF)  # ON/OFF
    endif()
endif()

# Earlier versions generated the header into the source tree, where it would
# shadow the one generated below.
if(EXISTS "${INCLUDE_DIR}/config_cmake.h")
    message(STATUS "Removing stale in-source ${INCLUDE_DIR}/config_cmake.h")
    file(REMOVE "${INCLUDE_DIR}/config_cmake.h")
endif()

# Fill in configuration options
configure_file(
  "${INCLUDE_DIR}/config_cmake.h.in"
  "${GRAFFINE_GENERATED_INCLUDE_DIR}/graffine/config_cmake.h")

# INSTALLATION -----------------------------------------------------------------

option(GRAFFINE_INSTALL "Generate the install target" ${PROJECT_IS_TOP_LEVEL})

if(GRAFFINE_INSTALL)
    set(GRAFFINE_CMAKE_DIR "${CMAKE_INSTALL_DATADIR}/cmake/graffine")

    install(TARGETS graffine EXPORT graffineTargets)

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/graffine"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
            PATTERN "config_cmake.h*" EXCLUDE)
    install(FILES "${GRAFFINE_GENERATED_INCLUDE_DIR}/graffine/config_cmake.h"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/graffine")
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/external/tinyply/"
            DESTINATION "${GRAFFINE_INSTALL_TINYPLY_DIR}")
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" "${PROJECT_SOURCE_DIR}/NOTICE"
            DESTINATION "${CMAKE_INSTALL_DOCDIR}")

    install(EXPORT graffineTargets
            NAMESPACE graffine::
            DESTINATION "${GRAFFINE_CMAKE_DIR}")

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/graffineConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/graffineConfig.cmake"
        INSTALL_DESTINATION "${GRAFFINE_CMAKE_DIR}")
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/graffineConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT)
    install(FILES "${PROJECT_BINARY_DIR}/graffineConfig.cmake"
                  "${PROJECT_BINARY_DIR}/graffineConfigVersion.cmake"
            DESTINATION "${GRAFFINE_CMAKE_DIR}")
endif()

# DEVELOPMENT ------------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL)

    # Let's ensure -std=++cxx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_VERBOSE_MAKEFILE OFF)

    add_compile_options("-Wall" "-Wextra" "-fexceptions")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options("-O0;-g3;-ggdb")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options("-O3")
    endif()

    # Opt-in for build machine architecture.
    option(GRAFFINE_NATIVE_ARCH
           "Optimize for the CPU of the build machine (-march=native)" OFF)
    if(GRAFFINE_NATIVE_ARCH)
        add_compile_options("-march=native")
    endif()

    # Add the cmake folder so the FindSphinx module is found
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

    # Support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    option(BUILD_DOCS "Build documentation" OFF)
    if(BUILD_DOCS)
        add_subdirectory(docs)
    endif()

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
