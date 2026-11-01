# First Graph

This page walks through the example shipped with graffine,
[`examples/create_merge_components/main.cpp`](https://github.com/vsukhor/graffine/blob/main/examples/create_merge_components/main.cpp).
It creates a graph of four linear components and merges two of them into one
through a pair of end vertices.

## 1. Set up output

Printing functions report through the thread-local message processor
`graffine::msgr`:

```cpp
graffine::msgr = new graffine::Msgr {&std::cout, nullptr, 6};
```

## 2. Define the graph type

```cpp
namespace elements = graffine::structure;
namespace trs = graffine::transforms;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using End = Chain::End;  // Chain end descriptors.
```

## 3. Populate it

`add_single_chain_component(n)` adds a new component made of one linear
chain of `n` edges:

```cpp
constexpr std::array len {4UL, 2UL, 1UL, 3UL};

G gr;
for (const auto u : len)
    gr.add_single_chain_component(u);

gr.print_components("BEFORE");
```

The graph now holds four components, with chains 0 to 3 of lengths 4, 2, 1
and 3 edges.

## 4. Merge two components

`vertex_merger::Core` joins two chain ends. Here the tails (end A) of chains 0
and 1 are merged *antiparallel*, i.e. at equal ends, so one of the chains is
reversed to keep a consistent direction:

```cpp
using Core = trs::vertex_merger::Core<G>;
Core core {gr, nullptr, "vm_core"};

constexpr graffine::ChIdG w1 {0};
constexpr graffine::ChIdG w2 {1};

core.antiparallel(End::A, w1, w2);

gr.print_components("AFTER");
```

Because both merged vertices are leaves and both input chains are linear, the
result is a single linear chain of 6 edges, and the graph has three
components.

## 5. Clean up

```cpp
delete graffine::msgr;
```

## Build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target create_merge_components
./examples/bin/create_merge_components
```

## Next

`Core` is a low-level building block. For general use, the checked functors
described in [Applying Transformations](transformations.md) are preferred:
the same merger with precondition checks is
`vertex_merger::From<Deg1, Deg1, G>`, or the degree-independent
`vertex_merger::Functor<G>`.
