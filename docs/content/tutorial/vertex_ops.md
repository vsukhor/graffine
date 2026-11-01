# Vertex Merger & Split

Vertex merger and split are inverse operations. Each is implemented for
every pair of degrees whose sum does not exceed the maximum degree 4.

## Vertex merger

Merges two vertices into one whose degree is the sum of theirs.

| Template | Inputs | Output |
| --- | --- | --- |
| `vertex_merger::From<Deg1, Deg1, G>` | two leaves | degree 2 |
| `vertex_merger::From<Deg1, Deg2, G>` | leaf + bulk | degree 3 |
| `vertex_merger::From<Deg1, Deg3, G>` | leaf + junction | degree 4 |
| `vertex_merger::From<Deg2, Deg2, G>` | bulk + bulk | degree 4 |
| `vertex_merger::Functor<G>` | any of the above, in either order | |

Header: `graffine/transforms/vertex_merger/functor.h`.

```cpp
gr.add_single_chain_component(1);
gr.add_single_chain_component(3);

trs::vertex_merger::From<Deg1, Deg1, G> merge {gr};

Chain::EndSlot s0B {ChIdG {0}, End::B};
Chain::EndSlot s1A {ChIdG {1}, End::A};

merge(s0B, s1A);          // one chain of 4 edges
```

Merging the two ends of the **same** chain closes it into a cycle (provided
the chain is long enough).

!!! note "Antiparallel merger"
    Merging equal ends (A with A, or B with B) reverses one of the chains, to
    keep edge directions consistent along the result.

## Vertex split

Splits a vertex in two, detaching some of its edges onto the new vertex.

| Template | Input | Outputs |
| --- | --- | --- |
| `vertex_split::To<Deg1, Deg1, G>` | degree 2 | two leaves (cuts a chain) |
| `vertex_split::To<Deg1, Deg2, G>` | degree 3 | leaf + bulk |
| `vertex_split::To<Deg1, Deg3, G>` | degree 4 | leaf + junction |
| `vertex_split::To<Deg2, Deg2, G>` | degree 4 | two bulk vertices |
| `vertex_split::Functor<G>` | any of the above | |

Header: `graffine/transforms/vertex_split/functor.h`.

The wrapper takes the vertex and the edges to detach from it:

```cpp
using Slots = std::vector<G::Vertex::EgDescr>;
trs::vertex_split::Functor<G> split {gr};

auto& v = *gr.chain(ChIdG {0}).vertex_at_bulk(EgIdA {2});
auto res = split.try_apply(v, Slots {});   // degree 2: the list is ignored
```

For degree 3 the list holds one edge descriptor; for degree 4, one or two.

A split may disconnect a component, in which case a new component is created.
