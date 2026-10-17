include(FetchContent)
set(FETCHCONTENT_QUIET off)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.18.0.zip
    URL_HASH SHA256=63b9c77751a5b8f492486005f67533fdc58682b67476fcb91650be5958d5195a
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

list(APPEND cpps
    adjacency.cpp
    edge.cpp
    component.cpp
    graph.cpp
    io.cpp
    paths.cpp
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
)

set(bindir ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(alltests "unittests")
add_executable(${alltests} ${cpps})
add_compile_definitions(-DTINYPLY_AS_LIBRARY)

target_compile_definitions(${alltests} PRIVATE
    # If the debug configuration pass the DEBUG define to the compiler
    $<$<CONFIG:Debug>: -DDEBUG>
)
target_include_directories(${alltests} PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/tinyply
)
target_link_libraries(${alltests} PRIVATE gtest_main)
gtest_discover_tests(${alltests})

add_custom_command(
    TARGET ${alltests}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${alltests}> ${bindir}
)
