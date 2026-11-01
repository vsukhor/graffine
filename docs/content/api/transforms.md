# Transformation Catalogue

All functors live in `graffine::transforms::<family>`, are constructed as
`F {graph, std::shared_ptr<PP> pp = nullptr}`, and share the interface of
[Applying Transformations](../tutorial/transformations.md). Arguments below
are those of `apply`, the call operator, `is_applicable` and `try_apply`.

## Vertex merger

| Functor | Arguments | Result |
| --- | --- | --- |
| `From<Deg1, Deg1, G>` | `Vertex&, Vertex&` or `EndSlot, EndSlot` | Merged degree 2 vertex |
| `From<Deg1, Deg2, G>` | leaf + bulk vertex or slots | Merged degree 3 vertex |
| `From<Deg1, Deg3, G>` | leaf + degree 3 vertex or slots | Merged degree 4 vertex |
| `From<Deg2, Deg2, G>` | two bulk vertices or slots | Merged degree 4 vertex |
| `Functor<G>` | `Vertex&, Vertex&` | Dispatches on degrees |
| `Core<G>` | e.g. `antiparallel(End::Id, ChIdG, ChIdG)` | Unchecked low-level merger |

## Vertex split

| Functor | Arguments | Result |
| --- | --- | --- |
| `To<Deg1, Deg1, G>` | `Vertex&` of degree 2 | Two leaves |
| `To<Deg1, Deg2, G>` | degree 3 vertex + edge to detach | Leaf + bulk vertex |
| `To<Deg1, Deg3, G>` | degree 4 vertex + edge to detach | Leaf + degree 3 vertex |
| `To<Deg2, Deg2, G>` | degree 4 vertex + edges to detach | Two bulk vertices |
| `Functor<G>` | `Vertex&, const std::vector<Vertex::EgDescr>&` | `std::array<Vertex*, 2>` |

## Edges

| Functor | Arguments | Result |
| --- | --- | --- |
| `edge_creation::InExistingChain<G>` | `Vertex&` or a slot | New edge |
| `edge_creation::InNewChain<D, G>` | `Vertex&` or a slot / chain id | New edge in a new chain |
| `edge_deletion::Functor<G>` | `EgIdG` | |
| `edge_split::Functor<G>` | `EgIdG` | New edge |
| `edge_merger::Functor<G>` | `EgIdG, EgIdG` (adjacent) | Merged edge |

## Components

| Functor | Arguments | Result |
| --- | --- | --- |
| `component_creation::Functor<G>` | `std::size_t` edges (default 1) | New `CmpId` |
| `component_deletion::Functor<G>` | `CmpId` | `CmpId` |

## Pulling

| Functor | Arguments | Result |
| --- | --- | --- |
| `pulling::On<Deg1, G>` | `Path<Compt>&, int n` | Hosting `CmpId` |
| `pulling::On<Deg2, G>` | `Path<Compt>&, int n` | Hosting `CmpId` |
| `pulling::On<Deg3, G>` | `Path<Compt>&, int n` | Hosting `CmpId` |
| `pulling::Functor<G>` | `Path<Compt>&, int n` | Dispatches on driver degree |

For exact signatures and overloads, see the
[Full Reference](reference/index.html).
