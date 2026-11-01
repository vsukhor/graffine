# Property Processors

Topology is only part of most models: vertices may carry positions, edges
weights, chains labels. A **property processor** keeps such non-topological
data in sync with the graph.

## Wiring a processor

Transformations take the processor type as their last template parameter,
`PP`, and a `std::shared_ptr<PP>` at construction. `PP` defaults to `void`,
for no processing.

```cpp
auto pp = std::make_shared<MyProcessor>();
trs::vertex_merger::Functor<G, MyProcessor> merge {gr, pp};
```

## Hooks

Each transformation calls hooks of the processor **before** (`on_start`) and
**after** (`on_end`) it changes the graph. Hooks are grouped by
transformation family in members of the processor:

| Member | Family |
| --- | --- |
| `vertexMerger` | Vertex merger |
| `vertexSplit` | Vertex split |
| `edgeCreation` | Edge creation |
| `edgeDeletion` | Edge deletion |
| `edgeMerger` | Edge merger |
| `componentCreation` | Component creation |
| `componentDeletion` | Component deletion |
| `pulling` | Pulling |

A transformation built from others also requires their hooks: edge split,
for instance, uses those of `edgeCreation` and `vertexMerger`.

For example, the vertex merger requires:

```cpp
struct MyProcessor {
    struct {
        void on_start(const G::EndSlot&,  const G::EndSlot&) { /* ... */ }
        void on_start(const G::BulkSlot&, const G::EndSlot&) { /* ... */ }
        void on_start(const G::BulkSlot&, const G::BulkSlot&) { /* ... */ }
        void on_end(G::Vertex& merged) { /* ... */ }
    } vertexMerger;
    // ...
};
```

## Concepts

Each family specifies the hooks it requires, with their arguments, as a
concept `Processor` in its namespace, e.g.
`graffine::transforms::vertex_merger::Processor<PP, G>`.
A transformation instantiated with an unsuitable processor **fails to
compile** with a report of the unsatisfied requirement.

`graffine/transforms/property_processor.h` defines the concept
`graffine::transforms::PropertyProcessor<PP, G>` of a processor usable by
**all** transformations.

!!! tip
    A processor implementing every hook is in the library's test suite:
    [`tests/property_processor.cpp`](https://github.com/vsukhor/graffine/blob/main/tests/property_processor.cpp).
    It is a good starting template.
