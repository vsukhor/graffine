# Errors & Integrity

graffine **never terminates the program on its own** in Release builds.
Errors are reported by exceptions derived from `graffine::error` (itself a
`std::runtime_error`), declared in `graffine/utils/errors.h`.

| Exception | Meaning |
| --- | --- |
| `graffine::io_error` | A graph file cannot be read or written, or does not describe a valid graph. |
| `graffine::precondition_error` | An operation was requested for graph elements that do not meet its requirements. |
| `graffine::invariant_error` | The internal bookkeeping of a graph was found inconsistent. The graph is left in an unspecified state and can only be destroyed. |

## Rejections without exceptions

Transformations report unmet preconditions without throwing through
`try_apply` and `is_applicable`; see
[Applying Transformations](transformations.md).

## Integrity checks

The consistency of a graph can be checked with
`graffine::structure::IntegralTests` (`graffine/structure/analyzers/integral_tests.h`):

```cpp
#include "graffine/structure/analyzers/integral_tests.h"

st::IntegralTests<G> checks {gr};

if (auto res = checks.validate(); !res)      // first violation, if any
    std::cerr << res.error().message << '\n';

checks();                                    // throws invariant_error instead
```

With `GRAFFINE_DO_INTEGRAL_TESTS` enabled (the default in Debug builds), the
checks also run inside the library. See
[Configuration Options](../build_install/configuration.md).

## Debug assertions

Debug builds (those defining `DEBUG`) also check many internal assumptions on
hot paths. A failed check of this kind reveals a **defect**: it prints a
diagnostic and calls `std::abort()`. Please
[report it](https://github.com/vsukhor/graffine/issues).
