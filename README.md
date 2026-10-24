
<p>
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/imgs/graffine-logo_1-dark.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/imgs/graffine-logo_1.png">
  <img alt="graffine logo" width="50%" src="docs/imgs/graffine-logo_1.png">
</picture>
</p>

![build](https://github.com/vsukhor/graffine/actions/workflows/cmake.yml/badge.svg)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Elementary transformations of graphs.

## Overview

***graffine*** is a header-only C++ library for constructing and modifying low-degree graphs (space-independent networks of edges and vertices).

While it operates on graphs of arbitrary geometry — provided that the max vertex degree does not exceed 4 — it is optimized for graphs containing abundant chains of linearly connected vertices.

## Core elements

Two major parts of ***graffine*** are **structure** which defines and holds graph elements, and a set of topological **transforms**, which modify that structure.

### Structure

***graffine*** stores a graph in layers of increasing complexity. The most elementary layer defines, as expected, a set of graph *edges* and *vertices*. Edges span between vertex pairs at the edge ends; the order of vertices within a pair defines edge directionality. Conversely, the outermost structural layer consists of disconnected *components*, each formed by mutually connected vertices and edges. Notably, ***graffine*** defines an intermediate layer of edge *chains*, which are linear runs of edges with no branches.

### Transformations

***graffine*** provides vertex degree-specific operators for modifying graphs at the elementary layer:

- Creation and deletion of edges and components
- Splitting and merging of edges
- Splitting and merging of vertices for each combination of degrees.

On top of these, ***graffine*** defines "pull" operations. These modify graph components by shifting an entire path of edges — spanned between two preselected vertices (a "driver" and a "source") — relative to the rest of the component.

Internally, all transformations are implemented as function object templates under [include/graffine/transforms/](include/graffine/transforms/).

### Additional features

- Adjacency and connectivity analyzers
- Path traversal
- PLY file I/O
- Graph integrity checks to verify internal consistency

## Usage

***graffine*** requires a C++23 compiler. Since the library is header-only, using it amounts to adding the [include/](include/) directory to your include path and including the relevant headers (e.g. `#include "graffine/structure/elements/graph.h"`).
If the IO capabilities are required, you should also add the [external/tinyply/](external/tinyply) directory, which contains a modified version of the popular [tinyply](https://github.com/ddiakopoulos/tinyply) library.

Note that the headers depend on `include/graffine/config_cmake.h`, which is not part of the repository: it is generated from [config_cmake.h.in](include/graffine/config_cmake.h.in) when the project is configured with CMake. Configure the project at least once (see below) before using the headers.

### Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The CMake options `BUILD_TESTS` (default `ON`), `BUILD_EXAMPLES` (default `ON`) and `BUILD_DOCS` (default `OFF`) control which targets are built. Setting `GRAFFINE_NATIVE_ARCH=ON` (default `OFF`) compiles with `-march=native`, which tunes the binaries for the build machine at the cost of portability.

Library configuration is passed through the following CMake variables, which are written into the generated `config_cmake.h`:

| Variable | Default | Meaning |
| --- | --- | --- |
| `GRAFFINE_FUNCTORS_VERBOSE` | `ON` | Transformation functors report verbosely |
| `GRAFFINE_PRINT_EDGES` | `ON` | Print detailed information on edges |
| `GRAFFINE_PRINT_VERTICES` | `ON` | Print detailed information on vertices |
| `GRAFFINE_DO_INTEGRAL_TESTS` | `ON` in Debug builds, otherwise `OFF` | Run graph integrity checks |
| `GRAFFINE_REAL_IS_DOUBLE` | `OFF` | Use `double` instead of `float` for real numbers |

These defaults apply only when ***graffine*** is the top-level CMake project.

### Examples

A working [example](examples/create_merge_components/main.cpp) of a driver function using ***graffine*** creates a graph consisting of four linear components and subsequently merges two of them into one via a chosen pair of end vertices.

You can find additional usage examples within the extensive set of unit [tests](tests/).

## License

Unless otherwise specified, the content of this repository is licensed under
the Apache License, Version 2.0.
Third-party code may have its own licensing,
please see [NOTICE](/NOTICE) for more details.
