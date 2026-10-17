
<p style="width: 50%; height: 50%">
<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/imgs/graffine-logo_1-dark.png">
  <source media="(prefers-color-scheme: light)" srcset="docs/imgs/graffine-logo_1.png">
  <img src="docs/imgs/graffine-logo_1.png">
</picture>
</p>

![build](https://github.com/vsukhor/graffine/actions/workflows/cmake.yml/badge.svg)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Elementary transformations of graphs.

## Overview

***graffine*** is a header-only C++ library for constructing and modifying low-degree graphs (space-independent networks of edges and vertices).

While it operates on graphs of arbitrary geometry, provided that the max vertex degree does not exceed 4, it is optimized with emphasis on the graphs containing abundant chains of linearly connected vertices.

## Core elements

Two major parts of ***graffine*** are **structure** that defines and holds graph elements, and a set of topological **transforms** that modify its structure.

### Structure

***graffine*** stores a graph in layers of increasing complexity. The most elementary layer defines, as espected, a set of graph *edges* and *vertices*. The edges are spanned between vertex pairs at the edge ends; vertex order inside the pair defines the edge directionality. On the other hand, the graph outer structural layer is a set of disconnected *components*, each formed by mutually connected vertices and edges. Notably, ***graffine*** defines an intermediate layer of edge *chains* which are linear runs of edges with no branches.

### Transformations

***graffine*** provides vertex degree-specific operators for modifying graphs on elementary layer:

- creation and deletion of edges and components
- split and merger of edges
- split and merger of vertices for each combination of degrees.

On top of these, ***graffine*** defines "pull" operations. These are modifications of the graph components whereis a path of edges spanned between two preselected vertices (a "driver" and a "source") is shifted as a whole relative to the rest of the component.

Internally, all transformations are implemented as function object templates under [include/graffine/transforms/](include/graffine/transforms/).

### Additional features

- adjacency and connectivity analyzers
- path traversal
- PLY file I/O
- graph's integrity checks to verify internal consistency

## Usage

***graffine*** is a header-only C++ library, soit is sufficient to plug the [include/graffine/](include/graffine/) directory into your project and provide the corresponding #include's.
If the IO capability is required, also [external/tinyply/](external/tinyply) directory should be inserted. It contains a modification of the popular [tinyply](https://github.com/ddiakopoulos/tinyply.git) library.

A working [example](examples/create_merge_components/main.cpp) of a driver function using ***graffine*** creates a graph consisting of two components and subsequently merges the components into one on a chosen pair of vertices.

A multitude of various examples on how the library can be used in practice can be found among the extensive set of unit [tests](tests/).


## License

Unless otherwise specified, content of this repository is licensed under
the Apache License, Version 2.0.
Third-party code may have its own licensing,
please see [NOTICE](/NOTICE) for more details.
