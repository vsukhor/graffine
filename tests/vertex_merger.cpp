/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "common.h"

#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/structure/basic/io.h"
#include "graph-mutator/structure/basic/world.h"
#include "graph-mutator/transforms/vertex_merger/from_00.h"
#include "graph-mutator/transforms/vertex_merger/from_10.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"
#include "graph-mutator/transforms/vertex_merger/from_13.h"
#include "graph-mutator/transforms/vertex_merger/from_20.h"
#include "graph-mutator/transforms/vertex_merger/from_22.h"


namespace graph_mutator::tests::vertex_merger {

using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using Ends = Chain::Ends;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;


/// Subclass to make protected members accessible for testing:
struct VertexMergerCore
    : public graph_mutator::vertex_merger::Core<G> {

    using Base = graph_mutator::vertex_merger::Core<G>;

    using Base::antiparallel;
    using Base::parallel;
    using Base::to_cycle;
    using Base::cn;

    explicit VertexMergerCore(G& gr)
        : Base {gr, "vm_core"}
    {}
};


/// Subclass to make protected members accessible for testing:
template<unsigned D1,
         unsigned D2,
         typename G>
struct VertexMerger
    : public graph_mutator::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : graph_mutator::vertex_merger::From<D1, D2, G> {graph}
    {}
};


class VertexMergerTest
    : public Test {

protected:

    constexpr ESlot sA(const ChId w)
    {
        return ESlot{w, Ends::A};
    }

    constexpr ESlot sB(const ChId w)
    {
        return ESlot{w, Ends::B};
    }
};

// =============================================================================


TEST_F(VertexMergerTest, Constructor)
{
    ++testCount;

    G gr;
    VertexMerger<1, 2, G> {gr};

    ASSERT_TRUE(gr.glm.empty());
    ASSERT_TRUE(gr.gla.empty());
    ASSERT_TRUE(gr.cn.empty());
    EXPECT_EQ(gr.chain_num(), 0);
    EXPECT_EQ(gr.cmpt_num(), 0);
    EXPECT_EQ(gr.edgenum, 0);
    ASSERT_TRUE(gr.ct.empty());
    ASSERT_TRUE(gr.chis.cn11.empty());
    ASSERT_TRUE(gr.chis.cn22.empty());
    ASSERT_TRUE(gr.chis.cn33.empty());
    ASSERT_TRUE(gr.chis.cn13.empty());
}


/// Tests vertex adjacency.
TEST_F(VertexMergerTest, VerticesAdjacent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex adjacency");

    constexpr std::array<EgId, 9> len {4, 3, 3, 5, 2, 2, 5, 4, 1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // Chain indexes:
    auto u = undefined<ChId>;
    auto v = undefined<ChId>;
    auto w = undefined<ChId>;
    auto x = undefined<ChId>;

    // Different components: ---------------------------------------------------
    u = 0;
    v = 2;

    EXPECT_FALSE(gr.vertices_are_adjacent_ee(sA(u), sA(v)));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 1, v, 1));

    // Linear chain of size 4: -------------------------------------------------
    u = 0;
    v = 0;

    // ee:
    EXPECT_FALSE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), v, 2));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), v, 3));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sB(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sB(u), v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 3));
    // aa:
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 2));
    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 1, v, 3));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 3));
    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 3, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 3, v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 3, v, 3));

    // Linear chain of size 3: -------------------------------------------------
    u = 1;
    v = 1;

    // ee:
    EXPECT_FALSE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), v, 2));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sB(u), v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 2));
    // aa:
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 2));

    // Linear chain of size 2: -------------------------------------------------
    u = 4;
    v = 4;

    // ee:
    EXPECT_FALSE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 1));
    // aa:
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 1));

    // Linear chain of size 1: -------------------------------------------------
    u = 8;
    v = 8;

    // ee:
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sB(u), sA(v)));

    VertexMergerCore fc {gr};
    // Cycles:
    fc.to_cycle(2);    // length 3
    fc.to_cycle(4);    // length 2
    fc.to_cycle(7);    // length 4

    // cn[2].length() == 3 and it is a separate cycle: -------------------------
    u = 2;
    v = 2;

    // ee:
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 2));
    // aa:
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 2));

    // cn[4].length() == 2 and it is a separate cycle: -------------------------
    u = 4;
    v = 4;

    // ee:
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 1));

    // cn[7].length() == 4 and it is a separate cycle: -------------------------
    u = 7;
    v = 7;

    // ee:
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));
    // ea:
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), v, 3));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 1));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sB(u), v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sB(u), v, 3));
    // aa:
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 1, v, 2));
    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 1, v, 3));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 2, v, 3));
    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 3, v, 1));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 3, v, 2));
    EXPECT_TRUE(gr.vertices_are_adjacent_aa(u, 3, v, 3));

    // Three chains with a 3-way junction --------------------------------------
    u = 0;
    v = 1;
    x = 9;

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{u, Ends::A}, BSlot{v, 1});  // produce cn[x]

    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sB(v)));  // 3-w junction
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sA(x)));  // 3-w junction
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(v), sA(x))); // cn[v].length() == 1
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sA(v))); // cn[v].length() == 1
    EXPECT_FALSE(gr.vertices_are_adjacent_ee(sA(u), sB(x))); // cn[x].length() == 2

    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(u), x, 1)); // cn[x].length() == 2
    EXPECT_TRUE(gr.vertices_are_adjacent_ea(sA(x), u, 1)); // cn[x].length() == 2

    EXPECT_FALSE(gr.vertices_are_adjacent_aa(u, 3, x, 1)); // cn[v].length() == 1

    // Merge a degree 1 vertex to a boundary vertex belonging to a cycle. ------
    // Utilize the three connected chains and the cycle chain produced above:
    u = 0;
    v = 1;
    w = 4;
    x = 9;

    VertexMerger<1, 0, G> merge10 {gr};

    merge10(sA(v), w);

    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sA(w)));
    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(u), sB(w)));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(u), w, 1));

    EXPECT_TRUE(gr.vertices_are_adjacent_ee(sA(w), sA(x)));
    EXPECT_FALSE(gr.vertices_are_adjacent_ea(sA(w), x, 1));

}


/// Tests antiparallel vertex merger 11 at ends B of chains belonging to distinct components.
TEST_F(VertexMergerTest, vm11a_eB_case_distinct_components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests antiparallel vertex merger 11 at ends B ",
                          "of chains belonging to distinct components");

    constexpr std::array<EgId, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = std::array<ChId, 6> {0, 1, 2, 3, 4, 5};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    // Original copies
    const auto g4 = gr.cn[w4].g;
    const auto g5 = gr.cn[w5].g;

    VertexMerger<1, 1, G> merge11 {gr};
    // Call core.antiparallel(eB, w4, w5)
    // w4(2> 3> 4>) + w5(7> 8> 9> 10>) ->  w4(2> 3> 4> 10< 9< 8< 7<)
    merge11(ESlot{w4, eB}, ESlot{w5, eB});

//    gr.ct[0].print_chains("CH");

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 5);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[w4].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w4].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w4].length(), g4.size() + g5.size());
    for (szt i {}; i<g5.size(); ++i) {
        ASSERT_EQ(gr.cn[w4].g[i+g4.size()].ind, g5[g5.size()-1-i].ind);
        ASSERT_EQ(gr.cn[w4].g[i+g4.size()].orientation(), -g5[g5.size()-1-i].orientation());
    }
    for (szt i {}; i<g4.size(); ++i) {
        ASSERT_EQ(gr.cn[w4].g[i].ind, g4[i].ind);
        ASSERT_EQ(gr.cn[w4].g[i].orientation(), g4[i].orientation());
    }
}


/// Tests antiparallel vertex merger 11 at ends B of chains belonging to the same component.
TEST_F(VertexMergerTest, vm11a_eB_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests antiparallel vertex merger 11 at ends B ",
                          "of chains belonging to the same component");

    constexpr std::array<EgId, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = std::array<ChId, 6> {0, 1, 2, 3, 4, 5};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    // Original copies
    const auto g3 = gr.cn[w3].g;
    const auto g4 = gr.cn[w4].g;

    // Call core.antiparallel(eB, w4, w3)
    // w4(2> 3> 4>) + w3(7> 8> 9> 10>) ->  w3(2> 3> 4> 10< 9< 8< 7<)
    merge11(ESlot{w4, eB}, ESlot{w3, eB});

//    gr.ct[0].print_chains("CH");

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[w3].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w3].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w3].length(), g3.size() + g4.size());
    for (szt i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.cn[w3].g[i+g4.size()].ind, g3[g3.size()-1-i].ind);
        ASSERT_EQ(gr.cn[w3].g[i+g4.size()].orientation(), -g3[g3.size()-1-i].orientation());
    }
    for (szt i {}; i<g4.size(); ++i) {
        ASSERT_EQ(gr.cn[w3].g[i].ind, g4[i].ind);
        ASSERT_EQ(gr.cn[w3].g[i].orientation(), g4[i].orientation());
    }
}


/// Tests parallel vertex merger 11 at w1 end A of chains belonging to the same component.
TEST_F(VertexMergerTest, vm11p_eA_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests parallel vertex merger 11 at w1 end A ",
                          "of chains belonging to the same component");

    constexpr std::array<EgId, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = std::array<ChId, 6> {0, 1, 2, 3, 4, 5};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    // Original copies
    const auto g0 = gr.cn[w0].g;
    const auto g3 = gr.cn[w3].g;

    // Call core.parallel(w0, w3)
    // w3(7> 8> 9> 10>) + w0(0> 1>)->  w0(7> 8> 9> 10> 0> 1>)
    merge11(ESlot{w0, eA}, ESlot{w3, eB});

//    gr.ct[0].print_chains("CH");

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w0].length(), g0.size() + g3.size());
    for (szt i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.cn[w0].g[i].ind, g3[i].ind);
        ASSERT_EQ(gr.cn[w0].g[i].orientation(), g3[i].orientation());
    }
    for (szt i {}; i<g0.size(); ++i) {
        ASSERT_EQ(gr.cn[w0].g[i+g3.size()].ind, g0[i].ind);
        ASSERT_EQ(gr.cn[w0].g[i+g3.size()].orientation(), g0[i].orientation());
    }
}


/// Tests parallel vertex merger 11 at w1 end B of chains belonging to the same component.
TEST_F(VertexMergerTest, vm11p_eB_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests parallel vertex merger 11 at w1 end B ",
                          "of chains belonging to the same component");

    constexpr std::array<EgId, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = std::array<ChId, 6> {0, 1, 2, 3, 4, 5};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    // Original copies
    const auto g0 = gr.cn[w0].g;
    const auto g3 = gr.cn[w3].g;

    // Call core.parallel(w0, w3)
    // w3(7> 8> 9> 10>) + w0(0> 1>)->  w0(7> 8> 9> 10> 0> 1>)
    merge11(ESlot{w3, eB}, ESlot{w0, eA});

//    gr.ct[0].print_chains("CH");

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w0].length(), g0.size() + g3.size());
    for (szt i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.cn[w0].g[i].ind, g3[i].ind);
        ASSERT_EQ(gr.cn[w0].g[i].orientation(), g3[i].orientation());
    }
    for (szt i {}; i<g0.size(); ++i) {
        ASSERT_EQ(gr.cn[w0].g[i+g3.size()].ind, g0[i].ind);
        ASSERT_EQ(gr.cn[w0].g[i+g3.size()].orientation(), g0[i].orientation());
    }
}


/// Tests vertex merger 12 case 1: a chain end A to an inner vertex of the same chain.
TEST_F(VertexMergerTest, vm12_case_1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 1: ",
                          "a chain end A to an inner vertex of the same chain");

    constexpr EgId len {10};

    constexpr ChId w {0};
    constexpr ChId v {1};    // chain resulting from the vertex merger.

    constexpr auto e = Ends::A;
    constexpr auto oe = Ends::opp(e);

    for (EgId a=1; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w, e}, BSlot{w, a});

        const auto clmass = len;
        ASSERT_EQ(gr.edgenum, clmass);
        ASSERT_EQ(gr.chain_num(), 2);
        ASSERT_EQ(gr.cmpt_num(), 1);

        ASSERT_EQ(gr.template num_vertices<0>(), 0);
        ASSERT_EQ(gr.template num_vertices<1>(), 1);
        ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
        ASSERT_EQ(gr.template num_vertices<3>(), 1);
        ASSERT_EQ(gr.template num_vertices<4>(), 0);

        for (ChId c {}, j {}; j<gr.chain_num(); ++j) {
            const auto& m = gr.cn[j];
            ASSERT_EQ(m.c, 0);
            ASSERT_EQ(m.idw, j);
            for (EgId i {}; i<m.length(); ++i, ++c) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, c);
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c);
            }
        }

        ASSERT_EQ(gr.cn[w].length(), a);
        ASSERT_EQ(gr.cn[v].length(), len - a);

        for (szt i {}, k {}, h {}; i<gr.ct.size(); ++i) {
            const auto& c = gr.ct[i];
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 2);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.gl.size(), c.num_edges());
            for (const auto w: c.ww) {
                ASSERT_EQ(w, k);
                const auto& m = gr.cn[w];
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc, k++);
                for (EgId j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.gl[h].w, m.g[j].w);
                    ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                    ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                }
            }
            ASSERT_EQ(c.chis.cn11, undefined<ChId>);
            ASSERT_EQ(c.chis.cn22, undefined<ChId>);
            ASSERT_EQ(c.chis.cn33.size(), 1);
            ASSERT_EQ(c.chis.cn44.size(), 0);
            ASSERT_EQ(c.chis.cn13.size(), 1);
            ASSERT_EQ(c.chis.cn14.size(), 0);
            ASSERT_EQ(c.chis.cn34.size(), 0);
        }

        ASSERT_EQ(gr.cn[w].ngs[oe].num(), 2);
        ASSERT_EQ(gr.cn[w].ngs[oe][0].w, w);
        ASSERT_EQ(gr.cn[w].ngs[oe][0].e, e);
        ASSERT_EQ(gr.cn[w].ngs[oe][1].w, v);
        ASSERT_EQ(gr.cn[w].ngs[oe][1].e, e);
        ASSERT_EQ(gr.cn[w].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[w].ngs[e][0].w, w);
        ASSERT_EQ(gr.cn[w].ngs[e][0].e, oe);
        ASSERT_EQ(gr.cn[w].ngs[e][1].w, v);
        ASSERT_EQ(gr.cn[w].ngs[e][1].e, e);

        ASSERT_EQ(gr.cn[v].ngs[oe].num(), 0);
        ASSERT_EQ(gr.cn[v].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[v].ngs[e][0].w, w);
        ASSERT_EQ(gr.cn[v].ngs[e][0].e, e);
        ASSERT_EQ(gr.cn[v].ngs[e][1].w, w);
        ASSERT_EQ(gr.cn[v].ngs[e][1].e, oe);
    }
}


/// Tests vertex merger 12 case 2: a chain end B to an inner vertex of the same chain.
TEST_F(VertexMergerTest, vm12_case_2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 2: ",
                          "a chain end B to an inner vertex of the same chain");

    constexpr EgId len {10};

    constexpr ChId w {0};
    constexpr ChId v {1};    // chain resulting from the vertex merger.

    constexpr auto e = Ends::B;
    constexpr auto oe = Ends::opp(e);

    for (EgId a=1; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w, e}, BSlot{w, a});

        const auto clmass = len;
        ASSERT_EQ(gr.edgenum, clmass);
        ASSERT_EQ(gr.chain_num(), 2);
        ASSERT_EQ(gr.cmpt_num(), 1);

        ASSERT_EQ(gr.template num_vertices<0>(), 0);
        ASSERT_EQ(gr.template num_vertices<1>(), 1);
        ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
        ASSERT_EQ(gr.template num_vertices<3>(), 1);
        ASSERT_EQ(gr.template num_vertices<4>(), 0);

        for (ChId c {}, j {}; j<gr.chain_num(); ++j) {
            const auto& m = gr.cn[j];
            ASSERT_EQ(m.c, 0);
            ASSERT_EQ(m.idw, j);
            for (EgId i {}; i<m.length(); ++i, ++c) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, c);
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c);
            }
        }

        ASSERT_EQ(gr.cn[w].length(), a);
        ASSERT_EQ(gr.cn[v].length(), len - a);

        for (szt i {}, k {}, h {}; i<gr.ct.size(); ++i) {
            const auto& c = gr.ct[i];
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 2);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.gl.size(), c.num_edges());
            for (const auto w: c.ww) {
                ASSERT_EQ(w, k);
                const auto& m = gr.cn[w];
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc, k++);
                for (EgId j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.gl[h].w, m.g[j].w);
                    ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                    ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                }
            }
            ASSERT_EQ(c.chis.cn11, undefined<ChId>);
            ASSERT_EQ(c.chis.cn22, undefined<ChId>);
            ASSERT_EQ(c.chis.cn33.size(), 1);
            ASSERT_EQ(c.chis.cn44.size(), 0);
            ASSERT_EQ(c.chis.cn13.size(), 1);
            ASSERT_EQ(c.chis.cn14.size(), 0);
            ASSERT_EQ(c.chis.cn34.size(), 0);
        }

        ASSERT_EQ(gr.cn[v].ngs[oe].num(), 2);
        ASSERT_EQ(gr.cn[v].ngs[oe][0].w, w);
        ASSERT_EQ(gr.cn[v].ngs[oe][0].e, e);
        ASSERT_EQ(gr.cn[v].ngs[oe][1].w, v);
        ASSERT_EQ(gr.cn[v].ngs[oe][1].e, e);
        ASSERT_EQ(gr.cn[v].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[v].ngs[e][0].w, w);
        ASSERT_EQ(gr.cn[v].ngs[e][0].e, e);
        ASSERT_EQ(gr.cn[v].ngs[e][1].w, v);
        ASSERT_EQ(gr.cn[v].ngs[e][1].e, oe);

        ASSERT_EQ(gr.cn[w].ngs[oe].num(), 0);
        ASSERT_EQ(gr.cn[w].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[w].ngs[e][0].w, v);
        ASSERT_EQ(gr.cn[w].ngs[e][0].e, oe);
        ASSERT_EQ(gr.cn[w].ngs[e][1].w, v);
        ASSERT_EQ(gr.cn[w].ngs[e][1].e, e);
    }
}


/// Tests vertex merger 12 case 3: a chain end to a separate linear chain.
TEST_F(VertexMergerTest, vm12_case_3_lin_lin)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 3: ",
                          "a chain end to a separate linear chain");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    constexpr auto e = Ends::B;
    constexpr auto oe = Ends::opp(e);

    constexpr auto clmass = len[w1] + len[w2];

    for (EgId a=1; a<len[w2]; ++a) {

        G gr;

        for (const auto u : len)
            gr.add_single_chain_component(u);

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w1, e}, BSlot{w2, a});

        ASSERT_EQ(gr.edgenum, clmass);
        ASSERT_EQ(gr.chain_num(), 3);
        ASSERT_EQ(gr.cmpt_num(), 1);

        ASSERT_EQ(gr.template num_vertices<0>(), 0);
        ASSERT_EQ(gr.template num_vertices<1>(), 3);
        ASSERT_EQ(gr.template num_vertices<2>(), clmass - 3);
        ASSERT_EQ(gr.template num_vertices<3>(), 1);
        ASSERT_EQ(gr.template num_vertices<4>(), 0);

        for (ChId c {}, j {}; j<gr.chain_num(); ++j) {
            const auto& m = gr.cn[j];
            ASSERT_EQ(m.c, 0);
            ASSERT_EQ(m.idw, j);
            for (EgId i {}; i<m.length(); ++i, ++c) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, c);
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c);
            }
        }

        for (szt i {}, k {}, h {}; i<gr.ct.size(); ++i) {
            const auto& c = gr.ct[i];
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 3);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.gl.size(), c.num_edges());
            for (const auto w: c.ww) {
                ASSERT_EQ(w, k);
                const auto& m = gr.cn[w];
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc, k++);
                for (EgId j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.gl[h].w, m.g[j].w);
                    ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                    ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                }
            }
            ASSERT_EQ(c.chis.cn11, undefined<ChId>);
            ASSERT_EQ(c.chis.cn22, undefined<ChId>);
            ASSERT_EQ(c.chis.cn33.size(), 0);
            ASSERT_EQ(c.chis.cn44.size(), 0);
            ASSERT_EQ(c.chis.cn13.size(), 3);
            ASSERT_EQ(c.chis.cn14.size(), 0);
            ASSERT_EQ(c.chis.cn34.size(), 0);
        }

        ASSERT_EQ(gr.cn[w1].ngs[oe].num(), 0);
        ASSERT_EQ(gr.cn[w1].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[w1].ngs[e][0].w, w2);
        ASSERT_EQ(gr.cn[w1].ngs[e][0].e, Ends::B);
        ASSERT_EQ(gr.cn[w1].ngs[e][1].w, gr.cn.size()-1);
        ASSERT_EQ(gr.cn[w1].ngs[e][1].e, Ends::A);

        ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 0);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 2);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, e);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, gr.cn.size()-1);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);

        ASSERT_EQ(gr.cn.back().ngs[Ends::A].num(), 2);
        ASSERT_EQ(gr.cn.back().ngs[Ends::A][0].w, w1);
        ASSERT_EQ(gr.cn.back().ngs[Ends::A][0].e, Ends::B);
        ASSERT_EQ(gr.cn.back().ngs[Ends::A][1].w, w2);
        ASSERT_EQ(gr.cn.back().ngs[Ends::A][1].e, e);
        ASSERT_EQ(gr.cn.back().ngs[Ends::B].num(), 0);
    }
}


/// Tests vertex merger 12 case 3 to a cycle chain.
TEST_F(VertexMergerTest, vm12_case_3_lin_cyc)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 3: ",
                          "a chain end to a cycle chain");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr auto e = Ends::B;
    constexpr auto oe = Ends::opp(e);
    constexpr auto clmass = len[w1] + len[w2];

    for (EgId a=1; a<len[w2]; ++a) {

        G gr;
        for (const auto u : len)
            gr.add_single_chain_component(u);

        VertexMergerCore fc {gr};
        fc.to_cycle(w2);

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w1, e}, BSlot{w2, a});

        ASSERT_EQ(gr.edgenum, clmass);
        ASSERT_EQ(gr.chain_num(), 2);
        ASSERT_EQ(gr.cmpt_num(), 1);

        ASSERT_EQ(gr.template num_vertices<0>(), 0);
        ASSERT_EQ(gr.template num_vertices<1>(), 1);
        ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
        ASSERT_EQ(gr.template num_vertices<3>(), 1);
        ASSERT_EQ(gr.template num_vertices<4>(), 0);

        ASSERT_EQ(gr.cn[w1].length(), len[w1]);
        ASSERT_EQ(gr.cn[w2].length(), len[w2]);

        EgIds t(gr.cn[w2].length());
        std::iota(t.begin(), t.end(), 0);
        std::rotate(t.begin(), t.begin() + a, t.end());
        for (EgId i=0; i<gr.cn[w2].length(); ++i)
            ASSERT_EQ(gr.cn[w2].g[i].ind, t[i]);
        for (EgId q {gr.cn[w2].length()};
             const auto& g : gr.cn[w1].g)
            ASSERT_EQ(g.ind, q++);

        const auto& c = gr.ct.front();
        ASSERT_EQ(c.ind, 0);
        ASSERT_EQ(c.num_chains(), len.size());
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.gl.size(), c.num_edges());
        EgId h {};
        for (ChId i {c.num_chains()}, k {}; const auto w: c.ww) {
            ASSERT_EQ(w, --i);
            const auto& m = gr.cn[w];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k++);
            for (EgId j {}; j<m.length(); ++j, ++h) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(m.g[j].indc, h);
                ASSERT_EQ(c.gl[h].w, m.g[j].w);
                ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                ASSERT_EQ(c.gl[h].i, m.g[j].ind);
            }
        }
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);

        ASSERT_EQ(gr.cn[w1].ngs[oe].num(), 0);
        ASSERT_EQ(gr.cn[w1].ngs[e].num(), 2);
        ASSERT_EQ(gr.cn[w1].ngs[e][0].w, w2);
        ASSERT_EQ(gr.cn[w1].ngs[e][0].e, Ends::B);
        ASSERT_EQ(gr.cn[w1].ngs[e][1].w, w2);
        ASSERT_EQ(gr.cn[w1].ngs[e][1].e, Ends::A);

        ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 2);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, e);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w2);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::B);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 2);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, e);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
        ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    }
}


/// Tests vertex merger 10: end A of a linear chain to boundary of a cycle chain.
TEST_F(VertexMergerTest, vm10_endA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 10: end A of a linear chain ",
                          "to boundary of a cycle chain.");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr auto e = Ends::A;
    constexpr auto oe = Ends::opp(e);

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<1, 0, G> merge10 {gr};

    fc.to_cycle(w2);
    merge10(ESlot{w1, e}, w2);

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);

   EgIds v(gr.cn[w2].length());
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin()+0, v.end());
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v[i]);
    for (EgId c {gr.cn[w2].length()};
         const auto& g : gr.cn[w1].g)
        ASSERT_EQ(g.ind, c++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());
    EgId h {};
    for (ChId i {c.num_chains()}, k {}; const auto w: c.ww) {
        ASSERT_EQ(w, --i);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 1);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 1);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[oe].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[e].num(), 2);
    ASSERT_EQ(gr.cn[w1].ngs[e][0].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[e][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[e][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[e][1].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, e);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, e);
}


/// Tests vertex merger 10: end B of a linear chain to boundary of a cycle chain.
TEST_F(VertexMergerTest, vm10_endB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 10: end B of a linear chain ",
                          "to boundary of a cycle chain.");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr auto e = Ends::B;
    constexpr auto oe = Ends::opp(e);

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<1, 0, G> merge10 {gr};

    fc.to_cycle(w2);
    merge10(ESlot{w1, e}, w2);

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);

   EgIds v(gr.cn[w2].length());
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin()+0, v.end());
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v[i]);
    for (EgId c {gr.cn[w2].length()};
         const auto& g : gr.cn[w1].g)
        ASSERT_EQ(g.ind, c++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());
    EgId h {};
    for (ChId i {c.num_chains()}, k {}; const auto w: c.ww) {
        ASSERT_EQ(w, --i);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 1);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 1);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[oe].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[e].num(), 2);
    ASSERT_EQ(gr.cn[w1].ngs[e][0].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[e][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[e][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[e][1].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, e);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, e);
}


/// Tests vertex merger 20 case 1: a disconnected cycle chain with itself.
TEST_F(VertexMergerTest, vm20_case_1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 20 case 1: ",
                          "a disconnected cycle chain with itself.");

    EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr std::array vv {w1, w2};

    constexpr EgId a {2};

    G gr;
    gr.add_single_chain_component(len);

    VertexMergerCore fc {gr};
    VertexMerger<2, 0, G> merge20 {gr};

    fc.to_cycle(w1);
    merge20(BSlot{w1, a}, w1);

    const auto clmass = len;
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a);
    ASSERT_EQ(gr.cn[w2].length(), len - a);

    for (EgId c {};
         const auto& m: gr.cn)
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, c++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 2);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 20 case 2: a linear chain at a bulk vertex to a cycle chain.
TEST_F(VertexMergerTest, vm20_case_2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 20 case 2: a linear chain",
                          " at a bulk vertex to a cycle chain");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr ChId w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w2, w3};

    constexpr EgId a {1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<2, 0, G> merge20 {gr};

    fc.to_cycle(w2);
    merge20(BSlot{w1, a}, w2);

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);
    ASSERT_EQ(gr.cn[w3].length(), len[w1] - a);

    EgIds v(gr.cn[w2].length());
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin(), v.end());
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v[i]);
    for (EgId c {gr.cn[w2].length()};
        const auto& g : gr.cn[w1].g)
        ASSERT_EQ(g.ind, c++);
    for (EgId c {gr.cn[w1].length() + gr.cn[w2].length()};
        const auto& g : gr.cn[w3].g)
        ASSERT_EQ(g.ind, c++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size()+1);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 1);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 2);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w3].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B].num(), 0);
}


/// Tests vertex merger 20 case 3: of a cycle chain at a bulk vertex to another cycle chain.
TEST_F(VertexMergerTest, vm20_case_3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 20 of a cycle chain ",
                          "at a bulk vertex to another cycle chain");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr std::array vv {w1, w2};

    constexpr EgId a {1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<2, 0, G> merge20 {gr};

    fc.to_cycle(w2);
    fc.to_cycle(w1);
    merge20(BSlot{w1, a}, w2);

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);

    for (EgId c {}; const auto& g : gr.cn[w2].g)
        ASSERT_EQ(g.ind, c++);

    EgIds v(gr.cn[w2].length());
    std::iota(v.begin(), v.end(), len[w1]);
    std::rotate(v.begin(), v.begin()+1, v.end());
    for (EgId i=0; i<gr.cn[w1].length(); ++i)
        ASSERT_EQ(gr.cn[w1].g[i].ind, v[i]);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 2);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 00: two disconnected cycle chains at their boundary vertexes.
TEST_F(VertexMergerTest, vm00)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 0000: two disconnected cycle ",
                          "chains at their boundary vertexes");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr std::array vv {w1, w2};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<0, 0, G> merge00 {gr};

    fc.to_cycle(w2);
    fc.to_cycle(w1);
    merge00(w1, w2);

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);

    for (EgId c {};
         const auto& m: gr.cn)
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, c++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 2);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 22 case 1: internal vertices of the same linear chain.
TEST_F(VertexMergerTest, vm22_case_1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 1: ",
                          "internal vertices of the same linear chain");

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w3, w2};

    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w1, a2});

    const auto clmass = len;
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a1);
    ASSERT_EQ(gr.cn[w2].length(), a2);
    ASSERT_EQ(gr.cn[w3].length(), len - a1 - a2);

    EgId d {};
    for (const auto& g : gr.cn[w1].g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.cn[w3].g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.cn[w2].g)
        ASSERT_EQ(g.ind, d++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), vv.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 1);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 2);
    ASSERT_EQ(c.chis.cn34.size(), 0);


    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.cn[w3].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].w, w3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][0].w, w3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][1].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 22 case 2: internal vertices of the same disconnected cycle chain.
TEST_F(VertexMergerTest, vm22_case_2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 2: internal vertices ",
                          "of the same disconnected cycle chain");

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};   // chain created by the vertex merger
    constexpr std::array vv {w1, w2};

    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;

    gr.add_single_chain_component(len);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w1, a2});

    const auto clmass = len;
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a1 + len - a2);
    ASSERT_EQ(gr.cn[w2].length(), a2 - a1);

    EgIds v(len);
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin() + len - a2, v.end());
    for (EgId i=0; i<gr.cn[w1].length(); ++i)
        ASSERT_EQ(gr.cn[w1].g[i].ind, v[i]);
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v[i + gr.cn[w1].length()]);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 2);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 22 case 3: internal vertices of distinct linear chains.
TEST_F(VertexMergerTest, vm22_case_3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 3: ",
                          "internal vertices of distinct linear chains");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {1};
    constexpr ChId w2 {0};
    constexpr ChId w3 {2};   // chain created by the vertex merger
    constexpr ChId w4 {3};   // chain created by the vertex merger
    constexpr std::array vv {w1, w2, w3, w4};

    constexpr EgId a1 {1};
    constexpr EgId a2 {2};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 4);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 4);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a1);
    ASSERT_EQ(gr.cn[w2].length(), a2);
    ASSERT_EQ(gr.cn[w3].length(), len[w1] - a1);
    ASSERT_EQ(gr.cn[w4].length(), len[w2] - a2);

    EgId d {};
    for (const auto& g : gr.cn[w2].g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.cn[w4].g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.cn[w1].g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.cn[w3].g)
        ASSERT_EQ(g.ind, d++);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 4);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w4);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w4);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w3].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].w, w4);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.cn[w4].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][0].w, w2);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][1].w, w3);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][2].w, w1);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w4].ngs[Ends::B].num(), 0);
}


/// Tests vertex merger 22 case 4: internal vertices of distinct cycle chains.
TEST_F(VertexMergerTest, vm22_case_4)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 4: ",
                          "internal vertices of distinct cycle chains");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr std::array vv {w1, w2};

    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);
    fc.to_cycle(w2);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);

    EgIds v(len[w1]);
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin() + 1, v.end());
    for (EgId i=0; i<gr.cn[w1].length(); ++i)
        ASSERT_EQ(gr.cn[w1].g[i].ind, v[i]);

    std::iota(v.begin(), v.end(), len[w1]);
    std::rotate(v.begin(), v.begin() + len[w2] - 1, v.end());
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v[i]);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 2);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);
}


/// Tests vertex merger 22 case 5: internal vertices of distinct
/// chains - a disconected cycle and a linear chain.
TEST_F(VertexMergerTest, vm22_case_5)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 5: internal vertices ",
                          "of distinct chains - a disconected cycle ",
                          "and a linear chain");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w2, w3};

    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), len[w1]);
    ASSERT_EQ(gr.cn[w2].length(), a2);
    ASSERT_EQ(gr.cn[w3].length(), len[w2] - a2);

    EgIds v1(len[w1]);
    std::iota(v1.begin(), v1.end(), 0);
    std::rotate(v1.begin(), v1.begin() + 1, v1.end());
    for (EgId i=0; i<gr.cn[w1].length(); ++i)
        ASSERT_EQ(gr.cn[w1].g[i].ind, v1[i]);

    EgIds v2(len[w2]);
    std::iota(v2.begin(), v2.end(), len[w1]);
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v2[i]);

    EgIds v3(len[w3]);
    std::iota(v3.begin(), v3.end(), len[w1] + a2);
    for (EgId i=0; i<gr.cn[w3].length(); ++i)
        ASSERT_EQ(gr.cn[w3].g[i].ind, v3[i]);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 1);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 2);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::A);

    ASSERT_EQ(gr.cn[w3].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B].num(), 0);
}


/// Tests vertex merger 22 case 6: internal vertices of distinct
/// chains - a linear chain and a disconected cycle.
TEST_F(VertexMergerTest, vm22_case_6)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 6: internal vertices ",
                          "of distinct chains - a linear chain ",
                          "and a disconected cycle");

    constexpr std::array<EgId, 2> len {4, 4};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w3, w2};

    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w2);

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    const auto clmass = len[w1] + len[w2];
    ASSERT_EQ(gr.edgenum, clmass);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), clmass - 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);

    ASSERT_EQ(gr.cn[w1].length(), a1);
    ASSERT_EQ(gr.cn[w2].length(), len[w2]);
    ASSERT_EQ(gr.cn[w3].length(), len[w1] - a1);

    EgIds v1(a1);
    std::iota(v1.begin(), v1.end(), 0);
    for (EgId i=0; i<gr.cn[w1].length(); ++i)
        ASSERT_EQ(gr.cn[w1].g[i].ind, v1[i]);

    EgIds v2(len[w2]);
    std::iota(v2.begin(), v2.end(), len[w1]);
    std::rotate(v2.begin(), v2.begin() + len[w2] - a1, v2.end());
    for (EgId i=0; i<gr.cn[w2].length(); ++i)
        ASSERT_EQ(gr.cn[w2].g[i].ind, v2[i]);

    EgIds v3(len[w1] - a1);
    std::iota(v3.begin(), v3.end(), a1);
    for (EgId i=0; i<gr.cn[w3].length(); ++i)
        ASSERT_EQ(gr.cn[w3].g[i].ind, v3[i]);

    const auto& c = gr.ct.front();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.gl.size(), c.num_edges());

    for (EgId h {}, k {};
         const auto w: c.ww) {
        ASSERT_EQ(w, vv[k]);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (EgId j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.gl[h].w, m.g[j].w);
            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 1);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 2);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cn[w1].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].w, w3);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].w, w2);
    ASSERT_EQ(gr.cn[w1].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w2].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][1].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].w, w3);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][0].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].w, w2);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].w, w1);
    ASSERT_EQ(gr.cn[w2].ngs[Ends::B][2].e, Ends::B);

    ASSERT_EQ(gr.cn[w3].ngs[Ends::A].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].w, w1);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][0].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][1].e, Ends::A);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].w, w2);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::A][2].e, Ends::B);
    ASSERT_EQ(gr.cn[w3].ngs[Ends::B].num(), 0);
}


}  // namespace graph_mutator::tests::vertex_merger
