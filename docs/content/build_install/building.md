# Building from Source

## Configure and build

```sh
git clone https://github.com/vsukhor/graffine.git
cd graffine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Targets

| CMake option | Default | Builds |
| --- | --- | --- |
| `BUILD_TESTS` | `ON` | Unit tests, see [Testing](../development/testing.md) |
| `BUILD_EXAMPLES` | `ON` | Example programs in `examples/` |
| `BUILD_BENCHMARKS` | `OFF` | Benchmarks, see [Benchmarks](../development/benchmarks.md) |
| `BUILD_DOCS` | `OFF` | This documentation site, see [Building the Docs](docs.md) |
| `GRAFFINE_NATIVE_ARCH` | `OFF` | Compiles with `-march=native` (tuned to the build machine, not portable) |

For example, a lean Release build of only the examples:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build
```

## Build types

`Debug` builds enable the [integrity checks](../tutorial/errors.md) by
default and check many internal assumptions on hot paths. Use `Release` for
production and timing.

## Running the example

```sh
cmake --build build --target create_merge_components
./examples/bin/create_merge_components
```

The example is walked through in [First Graph](../tutorial/first_graph.md).
