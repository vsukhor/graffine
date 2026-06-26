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
//#include "graffine/properties/processors.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"

//#include <algorithm>
#include <array>
//#include <filesystem>
//#include <memory>
//#include <numeric>  // accumulate, iota
//#include <string>


namespace graffine::tests::adjacency {

namespace elements = structure;
namespace trs = transforms;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

/// Subclass to make protected members accessible for testing:
struct VertexMergerCore
    : public trs::vertex_merger::Core<G> {

    using Base = trs::vertex_merger::Core<G>;

    using Base::antiparallel;
    using Base::parallel;
    using Base::to_cycle;
    using Base::cn;

    explicit VertexMergerCore(G& gr)
        : Base {gr, nullptr, "vm_core"}
    {}
};


/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G>
struct VertexMerger
    : public trs::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : trs::vertex_merger::From<D1, D2, G> {graph}
    {}
};


class AdjacencyTest
    : public Test {

protected:

    constexpr ESlot sA(const ChIdG w)
    {
        return ESlot{w, End::A};
    }

    constexpr ESlot sB(const ChIdG w)
    {
        return ESlot{w, End::B};
    }
};

// =============================================================================


/// Tests vertex adjacency.
TEST_F(AdjacencyTest, VerticesAdjacent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex adjacency");

    constexpr std::array<size_t, 9> len {4, 3, 3, 5, 2, 2, 5, 4, 1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // Chain indexes:
    auto u = ChIdG::undefined;
    auto v = ChIdG::undefined;
    auto w = ChIdG::undefined;
    auto x = ChIdG::undefined;

    // Different components: ---------------------------------------------------
    u = 0;
    v = 2;

    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), sA(v)));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(u, 1, v, 1));

    // Linear chain of size 4: -------------------------------------------------
    u = 0;
    v = 0;

    // ee:
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), v, 2));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), v, 3));
    EXPECT_FALSE(gr.adj.are_adjacent(sB(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(sB(u), v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 3));
    // aa:
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 2));
    EXPECT_FALSE(gr.adj.are_adjacent(u, 1, v, 3));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 3));
    EXPECT_FALSE(gr.adj.are_adjacent(u, 3, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 3, v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 3, v, 3));

    // Linear chain of size 3: -------------------------------------------------
    u = 1;
    v = 1;

    // ee:
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), v, 2));
    EXPECT_FALSE(gr.adj.are_adjacent(sB(u), v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 2));
    // aa:
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 2));

    // Linear chain of size 2: -------------------------------------------------
    u = 4;
    v = 4;

    // ee:
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 1));
    // aa:
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 1));

    // Linear chain of size 1: -------------------------------------------------
    u = 8;
    v = 8;

    // ee:
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), sA(v)));

    VertexMergerCore fc {gr};
    // Cycles:
    fc.to_cycle(2);    // length 3
    fc.to_cycle(4);    // length 2
    fc.to_cycle(7);    // length 4

    // cn[2].length() == 3 and it is a separate cycle: -------------------------
    u = 2;
    v = 2;

    // ee:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 2));
    // aa:
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 2));

    // cn[4].length() == 2 and it is a separate cycle: -------------------------
    u = 4;
    v = 4;

    // ee:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 1));

    // cn[7].length() == 4 and it is a separate cycle: -------------------------
    u = 7;
    v = 7;

    // ee:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), v, 3));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 1));
    EXPECT_FALSE(gr.adj.are_adjacent(sB(u), v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(sB(u), v, 3));
    // aa:
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 1, v, 2));
    EXPECT_FALSE(gr.adj.are_adjacent(u, 1, v, 3));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 2, v, 3));
    EXPECT_FALSE(gr.adj.are_adjacent(u, 3, v, 1));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 3, v, 2));
    EXPECT_TRUE(gr.adj.are_adjacent(u, 3, v, 3));

    // Three chains with a 3-way junction --------------------------------------
    u = 0;
    v = 1;
    x = 9;

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{u, End::A}, BSlot{v, 1});  // produce chain(x)

    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sB(v)));  // 3-w junction
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sA(x)));  // 3-w junction
    EXPECT_TRUE(gr.adj.are_adjacent(sA(v), sA(x))); // cn[v].length() == 1
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sA(v))); // cn[v].length() == 1
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), sB(x))); // chain(x).length() == 2

    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), x, 1)); // chain(x).length() == 2
    EXPECT_TRUE(gr.adj.are_adjacent(sA(x), u, 1)); // chain(x).length() == 2

    EXPECT_FALSE(gr.adj.are_adjacent(u, 3, x, 1)); // cn[v].length() == 1

    // Merge a degree 1 vertex to a boundary vertex belonging to a cycle. ------
    // Utilize the three connected chains and the cycle chain produced above:
    u = 0;
    v = 1;
    w = 4;
    x = 9;

    merge12(sA(v), w);

    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sA(w)));
    EXPECT_TRUE(gr.adj.are_adjacent(sA(u), sB(w)));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(u), w, 1));

    EXPECT_TRUE(gr.adj.are_adjacent(sA(w), sA(x)));
    EXPECT_FALSE(gr.adj.are_adjacent(sA(w), x, 1));
}


}  // namespace graffine::tests::adjacency