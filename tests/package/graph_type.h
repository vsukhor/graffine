#ifndef GRAFFINE_PACKAGE_TEST_GRAPH_TYPE_H
#define GRAFFINE_PACKAGE_TEST_GRAPH_TYPE_H

#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/elements/vertex.h"

#include <filesystem>

namespace elements = graffine::structure;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;

/// Writes \p gr to \p file in PLY format and reads it back.
auto write_and_read(const G& gr,
                    const std::filesystem::path& file) -> G;

#endif  // GRAFFINE_PACKAGE_TEST_GRAPH_TYPE_H
