# Header Map

All headers live under `include/graffine/`.

## Top level

| Header | Contents |
| --- | --- |
| `definitions.h` | Project-wide aliases: ids, degrees, `real`, `Msgr`, `Orientation` |
| `config_cmake.h` | Generated from `config_cmake.h.in`: [configuration](../build_install/configuration.md) values |
| `property.h` | Property support |

## Structure: `structure/`

| Path | Contents |
| --- | --- |
| `elements/graph.h` (+ `graph/`) | `Graph`: owns components; graph-wide access to chains, edges, vertices |
| `elements/component.h` (+ `component/`) | `Component`: a connected set of chains |
| `elements/chain.h` (+ `chain/`) | `Chain`: a linear run of edges, its ends and slots |
| `elements/edge.h` | `Edge` |
| `elements/vertex.h` | `Vertex` |
| `descriptors/` | `id.h` (typed ids), `vertex_degrees.h`, `slot.h`, `end.h`, `bulk.h`, edge/vertex descriptors |
| `containers/` | Element containers: `edges.h`, `chains.h`, `components.h`, `vertices.h`, ... |
| `analyzers/adjacency.h` | `Adjacency`: adjacency lists of edges and chains |
| `analyzers/connectivity.h` | `Connectivity`: chains connected to a slot or chain |
| `analyzers/integral_tests.h` | `IntegralTests`: graph consistency checks |
| `paths/over_edges/` | Paths and distances over edges |
| `paths/over_endslots/` | Shortest paths over chain end slots, weighted by chain weights |
| `modifiers/` | `GraphFactory`, `JunctionShifter` |
| `io/ply.h` | `Ply`: PLY file input/output |

## Transforms: `transforms/`

| Path | Contents |
| --- | --- |
| `common.h` | `Applicable` (call operator, `is_applicable`, `try_apply`), shared checks |
| `property_processor.h` | `PropertyProcessor` concept |
| `vertex_merger/` | `From<D1, D2, G>`, `Functor<G>`, `Core<G>` |
| `vertex_split/` | `To<D1, D2, G>`, `Functor<G>` |
| `edge_creation/` | `InExistingChain<G>`, `InNewChain<D, G>` |
| `edge_deletion/` | `Functor<G>` and configuration-specific deleters |
| `edge_split/functor.h` | `Functor<G>` |
| `edge_merger/functor.h` | `Functor<G>` |
| `component_creation/functor.h` | `Functor<G>` |
| `component_deletion/functor.h` | `Functor<G>` |
| `pulling/` | `Path<C>`, `On<D, G>`, `Functor<G>` |

## Utilities: `utils/`

| Header | Contents |
| --- | --- |
| `errors.h` | `error`, `io_error`, `precondition_error`, `invariant_error`, `Violation`, `Validation` |
| `msgr.h`, `logger.h` | Message processor `Msgr` and the thread-local `graffine::msgr` |
| `constants.h`, `misc.h`, `io.h`, `colorcodes.h` | Helpers |
