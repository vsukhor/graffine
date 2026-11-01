# Edges & Components

## Edge creation

| Functor | Effect | Header |
| --- | --- | --- |
| `edge_creation::InExistingChain<G>` | Adds an edge to an existing chain, at a free end or inside it | `edge_creation/in_existing_chain.h` |
| `edge_creation::InNewChain<D, G>` | Creates a new single-edge chain attached at a vertex | `edge_creation/in_new_chain.h` |

```cpp
gr.add_single_chain_component(3);

trs::edge_creation::InNewChain<Deg2, G> branch {gr};
trs::edge_creation::InExistingChain<G>  extend {gr};

branch(Chain::BulkSlot {ChIdG {0}, 1});    // branch off an internal vertex
extend(Chain::EndSlot  {ChIdG {0}, End::A}); // grow the chain at its tail
```

## Edge deletion

`edge_deletion::Functor<G>` deletes the edge with a given graph-wide index,
choosing the right procedure for its configuration (single- or multi-edge
chain, host chain kept or deleted).

```cpp
trs::edge_deletion::Functor<G> del {gr};
del.try_apply(EgIdG {3});
```

Header: `graffine/transforms/edge_deletion/functor.h`.

## Edge split and merger

| Functor | Effect | Header |
| --- | --- | --- |
| `edge_split::Functor<G>` | Splits an edge in two, inserting a degree 2 vertex | `edge_split/functor.h` |
| `edge_merger::Functor<G>` | Merges two adjacent edges of a chain into one | `edge_merger/functor.h` |

```cpp
trs::edge_split::Functor<G>  split {gr};
trs::edge_merger::Functor<G> merge {gr};

const auto& m = gr.chain(ChIdG {0});
merge.try_apply(m.g[0].ind, m.g[1].ind);   // adjacent edges only
split.try_apply(EgIdG {0});
```

## Component creation and deletion

| Functor | Effect | Header |
| --- | --- | --- |
| `component_creation::Functor<G>` | Adds a new linear component of `s` edges (default 1) | `component_creation/functor.h` |
| `component_deletion::Functor<G>` | Deletes a component by `CmpId` | `component_deletion/functor.h` |

```cpp
trs::component_creation::Functor<G> create {gr};
trs::component_deletion::Functor<G> remove {gr};

create.try_apply(5);          // new component of 5 edges
remove.try_apply(CmpId {0});
```

Unlike `Graph::add_single_chain_component`, these functors run
[property processor](property_processors.md) hooks.
