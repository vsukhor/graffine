
<p>
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/imgs/graffine-logo_1-dark.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/imgs/graffine-logo_1.png">
  <img style="width: 50%; height: 50%" src="docs/imgs/graffine-logo_1.png">
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

On top of these, ***graffine*** defines "pull" operations. These modify graph components by shifting an entire path of edges—spanned between two preselected vertices (a "driver" and a "source") — relative to the rest of the component.

Internally, all transformations are implemented as function object templates under [include/graffine/transforms/](include/graffine/transforms/).

### Additional features

- Adjacency and connectivity analyzers
- Path traversal
- PLY file I/O
- Graph integrity checks to verify internal consistency

## Usage

***graffine*** is a header-only C++ library, so it is sufficient to add the [include/graffine/](include/graffine/) directory to your project and include the relevant headers.
If the IO capabilities are required, you should also include the [external/tinyply/](external/tinyply) directory, which contains a modified version of the popular [tinyply](https://github.com/ddiakopoulos/tinyply.git) library.

A working [example](examples/create_merge_components/main.cpp) of a driver function using ***graffine*** creates a graph consisting of two components and subsequently merges them into one via a chosen pair of vertices.

You can find additional usage examples within the extensive set of unit [tests](tests/).

Build options are set in CMake and written into a generated config_cmake.h. These include verbose logging, how much detail gets printed about edges vertices and chains, whether the integrity checks are run, and whether  float or double precision is used.

## License

Unless otherwise specified, content of this repository is licensed under
the Apache License, Version 2.0.
Third-party code may have its own licensing,
please see [NOTICE](/NOTICE) for more details.
