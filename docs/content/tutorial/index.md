---
icon: lucide/graduation-cap
---

# Tutorial

This tutorial walks through graffine from its data model to each family of
transformations. Pages build on one another, but each can be read on its own
once [Concepts](concepts.md) is familiar.

| Page | You will learn |
| --- | --- |
| [Concepts](concepts.md) | The layered structure: edges, vertices, chains, components, graph |
| [Identifiers](identifiers.md) | The typed ids, their scopes, and which survive transformations |
| [First Graph](first_graph.md) | Building a graph and merging two components, step by step |
| [Applying Transformations](transformations.md) | The common interface of every transformation functor |
| [Vertex Merger & Split](vertex_ops.md) | Joining and separating vertices of each degree |
| [Edges & Components](edge_component_ops.md) | Creating, deleting, splitting and merging edges and components |
| [Pulling](pulling.md) | Moving a path of edges relative to its component |
| [Property Processors](property_processors.md) | Keeping your own per-element data in sync with topology |
| [PLY Input/Output](ply_io.md) | Saving and loading graphs |
| [Errors & Integrity](errors.md) | Exceptions, rejections and consistency checks |

All examples assume the graph type and namespace aliases below:

```cpp
#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"

namespace st  = graffine::structure;
namespace trs = graffine::transforms;
using namespace graffine::definitions;   // ids, degrees (Deg1...Deg4), real

using G     = st::Graph<st::Component<st::Chain<st::Edge<st::Vertex>>>>;
using Chain = G::Chain;
using End   = G::End;
```
