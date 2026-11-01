# Using in a CMake Project

The library is consumed through the CMake target `graffine::graffine`, which
provides the include paths and the C++23 requirement. Pick whichever way of
obtaining it suits your project.

=== "Installed package"

    Install graffine once:

    ```sh
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
    cmake --install build --prefix <prefix>
    ```

    This installs the headers, the tinyply headers (under
    `include/graffine/external/tinyply`) and the CMake package files (under
    `share/cmake/graffine`). Then, in your project:

    ```cmake
    find_package(graffine 1 REQUIRED CONFIG)
    target_link_libraries(my_app PRIVATE graffine::graffine)
    ```

    and configure it with `-DCMAKE_PREFIX_PATH=<prefix>`.

=== "FetchContent"

    ```cmake
    include(FetchContent)
    FetchContent_Declare(
        graffine
        GIT_REPOSITORY https://github.com/vsukhor/graffine.git
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(graffine)

    target_link_libraries(my_app PRIVATE graffine::graffine)
    ```

=== "add_subdirectory"

    ```cmake
    add_subdirectory(external/graffine)
    target_link_libraries(my_app PRIVATE graffine::graffine)
    ```

!!! note "Subproject defaults"

    The [configuration variables](configuration.md) default to `OFF` when
    graffine is a subproject. Set them before adding graffine to enable them.
    The install rules are generated only when graffine is the top-level
    project; set `GRAFFINE_INSTALL=ON` to enable them for a subproject.

## Including headers

Include the headers of the elements and transformations you use, e.g.

```cpp
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/functor.h"
```

The headers depend on `graffine/config_cmake.h`, which is generated from
`config_cmake.h.in` into the build directory when the project is configured,
and is installed alongside the other headers. The `graffine::graffine` target
puts it on the include path.

## PLY support

The tinyply implementation is compiled in by defining `TINYPLY_AS_LIBRARY`
before including `graffine/structure/io/ply.h` in **exactly one** source file
of your program:

```cpp title="ply_impl.cpp"
#define TINYPLY_AS_LIBRARY
#include "graffine/structure/io/ply.h"
```
