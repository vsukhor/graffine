/* =============================================================================

Copyright (c) 2021-2026 Valerii Sukhorukov <vsukhorukov@yahoo.com>
All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

================================================================================
*/
#include "common.h"
#include "sample_graph.h"

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/io/ply.h"
#include "graffine/structure/analyzers/integral_tests.h"

#include <array>
#include <iostream>
#include <filesystem>


namespace graffine::tests::io {

namespace elements = structure;
using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using IOTest = Test;

using namespace elements::io;

/// Creation of single-edge and multi-edge components.
TEST_F(IOTest, PlyCircularWriteRead)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests circular write and read of a multicomponent graph using ",
            ".PLY file format"
        );

    // Create initial graph (24 chains over 11 connected components).
    auto gr = create_sample_graph<G>();

    const auto workingDir = std::filesystem::current_path(); ///"test1.ply";

    if constexpr (profuse)
        gr.print_components(tagBefore);

    Ply<G> ply;
    ply.template write<profuse>(gr, "testPlyCircularWriteRead", false);

    auto grNew = ply.template read<profuse>("testPlyCircularWriteRead", true);

    elements::IntegralTests<G, profuse> integral_test {grNew};
    integral_test();

    if constexpr (profuse)
        grNew.print_components(tagAfter);
}

}  // namespace graffine::tests::io