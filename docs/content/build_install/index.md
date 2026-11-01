---
icon: lucide/hammer
---

# Build & Install

graffine is **header-only**: there is no library binary to compile. "Building"
graffine means either

- **consuming** it from your own CMake project, as an installed package or as
  a subproject, or
- **building its own targets**: unit tests, examples, benchmarks and
  documentation.

| I want to... | Go to |
| --- | --- |
| Check compiler and tool versions | [Requirements](requirements.md) |
| Use graffine in my project | [Using in a CMake Project](cmake.md) |
| Build and run the tests, examples or benchmarks | [Building from Source](building.md) |
| Change floating-point precision, verbosity, integrity checks | [Configuration Options](configuration.md) |
| Build this documentation site locally | [Building the Docs](docs.md) |

## Quick start

```sh
git clone https://github.com/vsukhor/graffine.git
cd graffine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```
