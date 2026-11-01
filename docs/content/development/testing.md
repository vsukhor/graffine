# Testing

The unit tests use [Google Test](https://github.com/google/googletest),
fetched by CMake at configuration time. They are built by default
(`BUILD_TESTS=ON`).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

## What is covered

The suite in `tests/` has one or more files per area:

| Area | Files |
| --- | --- |
| Structure | `graph.cpp`, `component.cpp`, `chain.cpp`, `edge.cpp` |
| Analyzers and paths | `adjacency.cpp`, `connectivity.cpp`, `paths.cpp` |
| Vertex merger / split | `vertex_merger_*.cpp`, `vertex_split_*.cpp` |
| Edges and components | `edge_creation.cpp`, `edge_deletion.cpp`, `edge_split_merger.cpp`, `component_creation_deletion.cpp` |
| Pulling | `pulling_1.cpp`, `pulling_2.cpp`, `pulling_3.cpp`, `pulling_bulk_source.cpp`, `pulling_hooks.cpp` |
| Preconditions | `preconditions.cpp` |
| Property processors | `property_processor.cpp` |
| Errors and I/O | `errors.cpp`, `io.cpp` |

Most tests validate the graph with `IntegralTests` after each
transformation, and the precondition tests check that every rejection leaves
the graph unchanged.

The tests also serve as **usage examples** for every transformation.

## Coverage

Coverage is reported to
[Codecov](https://codecov.io/gh/vsukhor/graffine) by the CI workflow.
