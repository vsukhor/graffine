# Benchmarks

Benchmarks of graph construction, transformations and lookups use
[Google Benchmark](https://github.com/google/benchmark), fetched at
configuration time.

```sh
cmake -S . -B build-bench -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_BENCHMARKS=ON -DBUILD_TESTS=OFF
cmake --build build-bench
build-bench/benchmarks/benchmarks
```

With `BUILD_BENCHMARKS=ON`, the reporting options `GRAFFINE_FUNCTORS_VERBOSE`,
`GRAFFINE_PRINT_EDGES` and `GRAFFINE_PRINT_VERTICES` default to `OFF`, so that
the benchmarks time the transformations rather than their console output.

The benchmarks of single transformations keep the graph size constant, and
report how their cost scales with it.

!!! tip
    Add `-DGRAFFINE_NATIVE_ARCH=ON` to tune for the build machine. Results
    are then not comparable across machines.
