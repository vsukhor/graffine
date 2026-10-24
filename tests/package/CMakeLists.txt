# Consumer of an installed graffine, verifying the exported package:
# cmake -S tests/package -B <dir> -DCMAKE_PREFIX_PATH=<graffine install prefix>
cmake_minimum_required(VERSION 3.28)

project(graffine_package_test LANGUAGES CXX)

find_package(graffine 1 REQUIRED CONFIG)

add_executable(package_test main.cpp ply_io.cpp)
target_link_libraries(package_test PRIVATE graffine::graffine)

enable_testing()
add_test(NAME package_test COMMAND package_test)
