# Configuration Options

Library configuration is passed through CMake variables, which are written
into the generated header `graffine/config_cmake.h`.

| Variable | Default | Meaning |
| --- | --- | --- |
| `GRAFFINE_FUNCTORS_VERBOSE` | `ON` | Transformation functors report verbosely |
| `GRAFFINE_PRINT_EDGES` | `ON` | Print detailed information on edges |
| `GRAFFINE_PRINT_VERTICES` | `ON` | Print detailed information on vertices |
| `GRAFFINE_DO_INTEGRAL_TESTS` | `ON` in Debug builds, otherwise `OFF` | Run graph integrity checks |
| `GRAFFINE_REAL_IS_DOUBLE` | `OFF` | Use `double` instead of `float` for `graffine::real` |

```sh
cmake -S . -B build -DGRAFFINE_REAL_IS_DOUBLE=ON -DGRAFFINE_FUNCTORS_VERBOSE=OFF
```

## Where the defaults apply

- **Top-level project:** the defaults in the table above.
- **Subproject** (`add_subdirectory`, `FetchContent`): every variable defaults
  to `OFF`; set it before adding graffine to enable it.
- **Installed package:** keeps the values it was configured with.
- **`BUILD_BENCHMARKS=ON`:** the three reporting options default to `OFF`, so
  that benchmarks time the transformations rather than their console output.

## Console output

Reports go through a message processor held in the thread-local pointer
`graffine::msgr`. Create one before using verbose functors or printing
functions, and delete it at the end:

```cpp
graffine::msgr = new graffine::Msgr {&std::cout, nullptr, 6};
// ...
delete graffine::msgr;
```
