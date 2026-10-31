include(FetchContent)
set(FETCHCONTENT_QUIET off)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
    URL_HASH SHA256=63b9c77751a5b8f492486005f67533fdc58682b67476fcb91650be5958d5195a
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)  # keep gtest out of graffine install
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

list(APPEND cpps
    adjacency.cpp
    chain.cpp
    edge.cpp
    errors.cpp
    component.cpp
    connectivity.cpp
    graph.cpp
    hook_context.cpp
    io.cpp
    paths.cpp
    plan.cpp
    preconditions.cpp
    property_processor.cpp
    vertex_merger_core.cpp
    vertex_merger_11.cpp
    vertex_merger_12.cpp
    vertex_merger_13.cpp
    vertex_merger_22.cpp
    vertex_split_11.cpp
    vertex_split_12.cpp
    vertex_split_13.cpp
    vertex_split_22.cpp
    vertex_split_functor.cpp
    edge_creation.cpp
    edge_deletion.cpp
    edge_split_merger.cpp
    component_creation_deletion.cpp
    pulling_1.cpp
    pulling_2.cpp
    pulling_3.cpp
    pulling_hooks.cpp
    pulling_bulk_source.cpp
)

set(bindir ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(alltests "unittests")
add_executable(${alltests} ${cpps})

# tinyply implementation is compiled in io.cpp, the only file including it.
set_source_files_properties(io.cpp PROPERTIES
    COMPILE_DEFINITIONS TINYPLY_AS_LIBRARY
)
target_compile_definitions(${alltests} PRIVATE
    # If the debug configuration pass the DEBUG define to the compiler
    $<$<CONFIG:Debug>: -DDEBUG>
)
target_link_libraries(${alltests} PRIVATE graffine::graffine gtest_main)
gtest_discover_tests(${alltests})

add_custom_command(
    TARGET ${alltests}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${alltests}> ${bindir}
)

# Opt-in compile check of the hooks dispatch under nvcc (see
# nvcc/hook_dispatch.cu); set CMAKE_CUDA_COMPILER, and CMAKE_CUDA_HOST_COMPILER
# if the default host compiler lacks C++23.
option(GRAFFINE_TEST_NVCC "Compile check of the hooks dispatch under nvcc" OFF)
if(GRAFFINE_TEST_NVCC)
    enable_language(CUDA)
    add_executable(hook_dispatch_nvcc nvcc/hook_dispatch.cu)
    target_link_libraries(hook_dispatch_nvcc PRIVATE graffine::graffine)
    set_target_properties(hook_dispatch_nvcc PROPERTIES
        CUDA_STANDARD 23
        CUDA_STANDARD_REQUIRED ON
        # only nvcc options: those of the directory may target the C++ compiler
        COMPILE_OPTIONS "--expt-relaxed-constexpr"
    )
    add_test(NAME hook_dispatch_nvcc COMMAND hook_dispatch_nvcc)
endif()
