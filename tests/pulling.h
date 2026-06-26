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

#ifndef GRAFFINE_TESTS_PULLING_H
#define GRAFFINE_TESTS_PULLING_H

#include "common.h"
//#include "graffine/properties/processors.h"
//#include "graffine/properties/trivial.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/pulling/functor.h"
#include "graffine/transforms/pulling/on_1.h"
#include "graffine/transforms/pulling/on_2.h"
#include "graffine/transforms/pulling/on_3.h"
#include "graffine/transforms/pulling/path.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <string>

namespace graffine::tests::pulling {

namespace elements = structure;
namespace trs = graffine::transforms;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using Compt = typename G::Compt;
using Source = trs::pulling::Path<Compt>::Source;
using Driver = trs::pulling::Path<Compt>::Driver;
using Path = trs::pulling::Path<Compt>;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

static constexpr const char* tagPathBefore {"TEST_BEFORE: PATH"};
static constexpr auto minCycleLength = Chain::minCycleLength;
static constexpr auto eA = End::A;
static constexpr auto eB = End::B;

template<Degree D1,
         Degree D2,
         typename G> requires (isImplementedDegree<D1> &&
                               isImplementedDegree<D2>)
struct VertexMerger
    : public trs::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : trs::vertex_merger::From<D1, D2, G> {graph}
    {}
};

/// Subclass to make protected members accessible for testing:
template<Degree D,
         Orientation Dir,
         typename G>
struct Pulling
    : public trs::pulling::On<D, Dir, G> {

    explicit Pulling(G& graph)
        : trs::pulling::On<D, Dir, G> {graph}
    {}
};


template<Degree D>
class PullingTest
    : public Test {

protected:

    static constexpr bool profuse {true};
    static constexpr const char* tagBefore {"TEST_BEFORE"};
    static constexpr const char* tagAfter {"TEST_AFTER"};

    auto create_graph() const -> G
    {
        constexpr std::array<size_t, 5> len {9, 7, 7, 3, 5};
        constexpr std::size_t nchains {13};
        constexpr std::array<ChIdG, nchains> w {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
        };

        G gr;

        for (const auto u : len)
            gr.add_single_chain_component(u);

        VertexMerger<2, 2, G> merge22 {gr};

        // w[0], w[5], w[6] : length (9) -> (1, 2, 6)
        merge22(BSlot{w[0], 1},
                BSlot{w[0], 7});     // creates w[5], w[6]

        VertexMerger<1, 2, G> merge12 {gr};
        // w[6], w[7] : length (6) -> (1, 5)
        merge12(ESlot{w[1], eA},
                BSlot{w[6], 1});     // creates w[7]

        // w[1], w[8] : length (7) -> (4, 3)
        merge12(ESlot{w[1], eB},
                BSlot{w[1], 4});     // creates w[8]: cycle connected to w[1]B

        // w[2], w[9], w[10] : length (7) -> (2, 1, 4)
        merge22(BSlot{w[2], 2},
                BSlot{w[2], 6});     // creates w[9], w[10]

        // w[10], w[11] : length (4) -> (2, 2)
        merge12(ESlot{w[3], eA},
                BSlot{w[10], 2});     // creates w[11]

        // w[1], w[12] : length (4) -> (3, 1)
        merge12(ESlot{w[3], eB},
                BSlot{w[1], 3});     // creates w[12]

        gr.print_chains("");

        return gr;
    }
};


}  // namespace graffine::tests::pulling

#endif  // GRAFFINE_TESTS_PULLING_H