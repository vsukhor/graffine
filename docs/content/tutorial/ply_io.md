# PLY Input/Output

Graphs can be written to and read from files in the
[PLY format](https://paulbourke.net/dataformats/ply/), using a modified
version of [tinyply](https://github.com/ddiakopoulos/tinyply) bundled with
graffine.

## Enabling the implementation

Define `TINYPLY_AS_LIBRARY` before including the PLY header in **exactly one**
source file of your program:

```cpp title="ply_impl.cpp"
#define TINYPLY_AS_LIBRARY
#include "graffine/structure/io/ply.h"
```

Other files include `graffine/structure/io/ply.h` normally.

## Writing and reading

```cpp
#include "graffine/structure/io/ply.h"

st::io::Ply<G> ply;

ply.write(gr, "graph.ply", /*asBinary=*/ false);

G restored = ply.read("graph.ply", /*preloadIntoMemory=*/ true);
```

`write` and `read` take an optional template flag `beLoud` for verbose
reporting: `ply.template write<true>(...)`.

The file content is validated before the graph is built. Both functions
throw `graffine::io_error` if the file cannot be written or read, or does
not describe a valid graph.

!!! note
    Chain and component indexes are not preserved by a write followed by a
    read; the restored graph is topologically equivalent, not identical in
    numbering.
