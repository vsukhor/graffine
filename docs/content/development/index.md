---
icon: lucide/code
---

# Development

This section is for contributors working on graffine itself.

- [Testing](testing.md): the unit test suite and coverage.
- [Benchmarks](benchmarks.md): measuring the cost of transformations.

## Repository layout

| Path | Contents |
| --- | --- |
| `include/graffine/` | The library headers |
| `external/tinyply/` | Modified tinyply, for PLY I/O |
| `tests/` | Google Test unit tests |
| `benchmarks/` | Google Benchmark benchmarks |
| `examples/` | Example programs |
| `docs/` | Sources of this documentation site |
| `cmake/` | CMake package configuration |

## Contributing

Bug reports and pull requests are welcome on
[GitHub](https://github.com/vsukhor/graffine). Before submitting a change:

- [ ] build in Debug, so internal assertions and integrity checks are active;
- [ ] run the full test suite;
- [ ] add tests covering the change;
- [ ] document new public names in the headers (they feed the
      [API reference](../api/index.md)).
