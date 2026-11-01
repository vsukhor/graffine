---
icon: lucide/house
---

# graffine

![graffine logo](assets/graffine-logo-light.png#only-light){ .graffine-logo }
![graffine logo](assets/graffine-logo-dark.png#only-dark){ .graffine-logo }

[![build](https://github.com/vsukhor/graffine/actions/workflows/cmake.yml/badge.svg)](https://github.com/vsukhor/graffine/actions/workflows/cmake.yml)
[![codecov](https://codecov.io/gh/vsukhor/graffine/graph/badge.svg)](https://codecov.io/gh/vsukhor/graffine)
[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

**Elementary transformations of graphs.**

## Introduction

***graffine*** is a header-only C++23 library for constructing and modifying
low-degree graphs: space-independent networks of edges and vertices whose
vertex degree does not exceed 4.

While it handles graphs of arbitrary geometry within that degree limit, it is
optimized for graphs containing abundant *chains*: linear runs of edges with
no branches.

## Why graffine

- **Local topological moves as first-class operations.** Splitting and
  merging of vertices, creation and deletion of edges and components, and
  "pulls" of whole paths are provided as ready functors for every supported
  combination of vertex degrees.
- **Safe to propose, cheap to reject.** Every transformation checks its
  preconditions at a cost independent of the graph size, and can report a
  rejection without throwing or allocating: suited to simulations that
  propose many candidate moves, most of which may be rejected.
- **Chain-aware storage.** The intermediate chain layer keeps the common case
  of long unbranched runs compact and fast to traverse.
- **Your data stays in sync.** Optional property processors receive hooks
  before and after each transformation, so positions, weights or any other
  per-element data can follow topology changes.
- **Header-only, CMake-native.** Link to `graffine::graffine` and include the
  headers; there is no compiled library to build.

## Key Features

<div class="grid cards" markdown>

-   :lucide-layers: **Layered structure**

    ---

    Edges and vertices, grouped into chains, grouped into connected
    components, all held by a graph.

    [:lucide-arrow-right: Concepts](tutorial/concepts.md)

-   :lucide-git-merge: **Elementary transformations**

    ---

    Degree-specific vertex merger and split; edge creation, deletion, split
    and merger; component creation and deletion.

    [:lucide-arrow-right: Applying Transformations](tutorial/transformations.md)

-   :lucide-move-right: **Pulling**

    ---

    Shift an entire path of edges, spanned between a driver and a source
    vertex, relative to the rest of its component.

    [:lucide-arrow-right: Pulling](tutorial/pulling.md)

-   :lucide-search-check: **Analysis & I/O**

    ---

    Adjacency and connectivity analyzers, path traversal, integrity checks
    and PLY file input/output.

    [:lucide-arrow-right: PLY Input/Output](tutorial/ply_io.md)

</div>

## A taste

```cpp
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/functor.h"

namespace st = graffine::structure;
using G = st::Graph<st::Component<st::Chain<st::Edge<st::Vertex>>>>;

G gr;
gr.add_single_chain_component(4);   // two linear components
gr.add_single_chain_component(2);

graffine::transforms::vertex_merger::Functor<G> merge {gr};

auto& v1 = *gr.chain(graffine::ChIdG {0}).vertex_at_end(G::End::B);
auto& v2 = *gr.chain(graffine::ChIdG {1}).vertex_at_end(G::End::A);

if (auto res = merge.try_apply(v1, v2); !res)
    std::cerr << res.error().reason << '\n';   // the graph is left unchanged
```

## Document Guidance

- [**Build & Install**](build_install/index.md): requirements, consuming
  graffine from CMake, building tests, examples and benchmarks, and
  configuration options.
- [**Tutorial**](tutorial/index.md): a guided tour from the structural
  concepts to every family of transformations.
- [**API Documentation**](api/index.md): namespaces, header map, the
  transformation catalogue, and the full Doxygen-generated reference.
- [**Development**](development/index.md): running the tests and benchmarks,
  and contributing.

## License

Unless otherwise specified, graffine is licensed under the
[Apache License, Version 2.0](https://github.com/vsukhor/graffine/blob/main/LICENSE).
Third-party code may have its own licensing; see
[Third-Party Libraries](third_party.md).
