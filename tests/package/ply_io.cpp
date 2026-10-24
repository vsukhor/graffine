// The tinyply implementation is compiled in exactly one translation unit.
#define TINYPLY_AS_LIBRARY

#include "graph_type.h"

#include "graffine/structure/io/ply.h"

#include <filesystem>

auto write_and_read(const G& gr,
                    const std::filesystem::path& file) -> G
{
    graffine::structure::io::Ply<G> ply;
    ply.write<false>(gr, file, false);

    return ply.read<false>(file, false);
}
