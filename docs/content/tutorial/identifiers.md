# Identifiers

Elements are referred to by **typed identifiers**. An id's type encodes both
the *kind* of element and its *scope*, the range over which it is unique, so
an edge id cannot be passed where a chain id is expected, nor a
component-local index where a graph-wide one is.

## Naming

The alias names combine an element prefix with a scope suffix:

| Prefix | Element |
| --- | --- |
| `EgId` | edge |
| `ChId` | chain |
| `VId` | vertex |
| `CmpId` | component |

| Suffix | Scope | Meaning |
| --- | --- | --- |
| `H` | program run | Stable handle, assigned once when the element is created |
| `G` | graph | Position in a container of the graph |
| `C` | component | Position in a container of the component |
| `A` | chain | Position of an edge along its chain |

So `EgIdG` is an edge's graph-wide index, `ChIdC` a chain's index within its
component, and `VIdH` a vertex's stable handle.

## Which ids survive transformations

The `G`, `C` and `A` ids are **positions**, and change when the graph does:

- Removing an element moves the last element of the same container into the
  vacated position. After `Graph::remove_edge`, the formerly last edge of
  the graph takes over the removed `EgIdG`, and the last edge of the
  component the removed `EgIdC`. Chains and components are renumbered alike.
- Merging and splitting components renumbers the chains and edges they
  contain (`ChIdC`, `EgIdC`) and may change `CmpId`.
- Inserting or removing an edge in a chain shifts the `EgIdA` of the edges
  following it.

!!! warning
    Only the `H` ids remain valid over transformations. Do not hold on to
    positional ids, slots or element references across a transformation;
    re-derive them, or keep `H` ids and look the elements up again.

## Where ids live

| Element | Members |
| --- | --- |
| Edge | `id` (`EgIdH`), `ind` (`EgIdG`), `indc` (`EgIdC`), `indw` (`EgIdA`), `w` (host chain `ChIdG`), `c` (host `CmpId`) |
| Chain | `idw` (`ChIdG`), `idc` (`ChIdC`), `c` (`CmpId`) |
| Component | `ind` (`CmpId`), `ww` (the `ChIdG`s of its chains) |
| Vertex | `id` (`VIdH`), `c` (`CmpId`) |
| Slots, edge descriptors | `w` (chain), `a` (`EgIdA`), `e` (end) |
