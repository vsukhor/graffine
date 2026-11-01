# Pulling

Pulling modifies a component by **shifting an entire path of edges**,
spanned between two preselected vertices, a **driver** and a **source**,
relative to the rest of the component, from the source towards the driver.

A pull by one step shifts each chain junction along the path by one edge
towards the source: the chain at the driver gains an edge, the chain at the
source loses one, and the chains in between keep their lengths. **Vertex
degrees and the number of edges are preserved.** A source chain made of a
single edge is dissolved by the step, its edge joining the adjacent chain of
the path. If the driver and the source are on the same chain, a step leaves
the topology unchanged. A pull by `n` steps repeats the step `n` times.

## Path

A path, `pulling::Path<Component>`, is a contiguous sequence of edges inside
a component:

- the **driver** is the chain end at the vertex initiating the pull, given by
  the first path edge and its end at that vertex;
- the **source** is where the path ends: a free chain end, an end of a cycle
  chain, or a degree 2 vertex inside a chain.

```cpp
#include "graffine/transforms/pulling/functor.h"

using Path   = trs::pulling::Path<G::Compt>;
using Driver = Path::Driver;
using Source = Path::Source;

gr.add_single_chain_component(5);
const ChIdG w0 {0};

Path path {&gr.compt(w0),
           Driver {&gr.chain(w0).tail(), End::A},
           Source {w0, End::B}};
```

## Pulling functors

| Functor | Driver vertex degree |
| --- | --- |
| `pulling::On<Deg1, G>` | 1 |
| `pulling::On<Deg2, G>` | 2 |
| `pulling::On<Deg3, G>` | 3 |
| `pulling::Functor<G>` | any of the above, dispatched at run time |

They take the path and the number of steps `n`, and return the index of the
component hosting the path after the pull:

```cpp
trs::pulling::Functor<G> pull {gr};

if (auto res = pull.try_apply(path, 2); !res)
    std::cerr << res.error().reason << '\n';
```

The path is updated in place as the pull moves edges between chains.

Rejected arguments include a non-positive `n`, a path too short for `n`
steps, a path over another graph's component, and a driver of degree 4.

!!! note
    The precondition checks of pulling from a degree 2 vertex copy the path,
    so their cost is proportional to its length.
