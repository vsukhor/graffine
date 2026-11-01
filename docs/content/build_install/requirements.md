# Requirements

## Toolchain

| Tool | Version |
| --- | --- |
| C++ compiler | C++23 support, including `std::expected` |
| CMake | 3.28 or newer |

graffine is developed and tested on Linux with GCC (14.x, 15.x and 16.x). Any
compiler with sufficient C++23 support should work.

## Optional dependencies

These are needed only for graffine's own targets, never by code consuming the
library. Google Test and Google Benchmark are fetched automatically by CMake
at configuration time.

| Dependency | Needed for |
| --- | --- |
| [Google Test](https://github.com/google/googletest) 1.18 | Unit tests (`BUILD_TESTS`) |
| [Google Benchmark](https://github.com/google/benchmark) 1.9 | Benchmarks (`BUILD_BENCHMARKS`) |
| [Doxygen](https://www.doxygen.nl) | API reference |
| [Zensical](https://zensical.org) | This documentation site |

## Bundled

A modified copy of [tinyply](https://github.com/ddiakopoulos/tinyply), used for
PLY file input/output, ships in `external/tinyply/` and is made available
through the `graffine::graffine` target. See [PLY Input/Output](../tutorial/ply_io.md).
