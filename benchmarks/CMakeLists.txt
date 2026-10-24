include(FetchContent)
set(FETCHCONTENT_QUIET off)
FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.zip
    URL_HASH SHA256=7a273667fbc23480df1306f82bdb960672811dd29a0342bb34e14040307cf820
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING "Benchmarks are meaningful in Release builds only")
endif()
if(GRAFFINE_FUNCTORS_VERBOSE OR GRAFFINE_PRINT_EDGES OR GRAFFINE_PRINT_VERTICES)
    message(WARNING "Benchmarks include the reports printed by the "
                    "transformations: set GRAFFINE_FUNCTORS_VERBOSE, "
                    "GRAFFINE_PRINT_EDGES and GRAFFINE_PRINT_VERTICES to OFF")
endif()

add_executable(benchmarks transforms.cpp)
target_link_libraries(benchmarks PRIVATE graffine::graffine benchmark::benchmark)
