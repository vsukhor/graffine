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

#include <array>
#include <iostream>
#include <string>

#include "common.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/structure/basic/vertices/degrees.h"
#include "graph-mutator/transforms/edge_creation/in_new_chain.h"
#include "graph-mutator/transforms/edge_creation/in_existing_chain.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"


namespace graph_mutator::tests::edge_creation {

using namespace graph_mutator;
using namespace graph_mutator::structure::basic;

using G = Graph<Chain<Edge<maxDegree>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using Ends = Chain::Ends;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G> requires (is_implemented_degree<D1> &&
                               is_implemented_degree<D2>)
struct VertexMerger
    : public vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : vertex_merger::From<D1, D2, G> {graph}
    {}
};

/// Subclass to make protected members accessible for testing:
template<Degree D,
         typename G> requires is_implemented_degree<D>
struct EdgeCreationNewChain
    : public graph_mutator::edge_creation::InNewChain<D, G> {

    explicit EdgeCreationNewChain(G& graph)
        : graph_mutator::edge_creation::InNewChain<D, G> {graph}
    {}
};

template<Degree D,
         typename G> requires is_implemented_degree<D>
struct EdgeCreationOldChain
    : public graph_mutator::edge_creation::InExistingChain<D, G> {

    explicit EdgeCreationOldChain(G& graph)
        : graph_mutator::edge_creation::InExistingChain<D, G> {graph}
    {}
};


class CreateEdgeTest
    : public Test
{};

// =============================================================================


/// Tests creation of an edge in an existing single-edge linear chain
TEST_F(CreateEdgeTest, LineSingleEdgeExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in an existing single-edge linear chain"
        );

    constexpr std::array<EgId, 3> len {1, 1, 1};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChId, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;
    constexpr EgId a {1};

    G gr;

    for (const auto u: len)
        gr.add_single_chain_component(u);

    // edge creating functors
    EdgeCreationOldChain<1, G> create_edge_1 {gr};
    EdgeCreationOldChain<2, G> create_edge_2 {gr};

    // create edge at the end A of the chain w0
    const auto c0 = create_edge_1(ESlot{w0, Ends::A});

    // create edge at the end B of the chain w1
    const auto c1 = create_edge_1(ESlot{w1, Ends::B});

    // create edge at the bulk position a = 1
    const auto c2 = create_edge_2(BSlot{w2, a});

    ASSERT_EQ(gr.edgenum, lensum + 3);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 3);
    ASSERT_EQ(gr.cn[w0].length(), len[0] + 1);
    ASSERT_EQ(gr.cn[w1].length(), len[1] + 1);
    ASSERT_EQ(gr.cn[w2].length(), len[2] + 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, lensum);
    ASSERT_EQ(gr.cn[w0].g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.cn[w2].g[a].ind, lensum + 2);
    ASSERT_EQ(c0[0], 0);
    ASSERT_EQ(c1[0], 1);
    ASSERT_EQ(c2[0], 2);

    ASSERT_EQ(gr.chis.cn11.size(), 3);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests creation of an edge in an existing linear chain
TEST_F(CreateEdgeTest, LineMultiEdgeExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in an existing linear chain"
        );

    constexpr std::array len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChId, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;

    constexpr EgId a {2};

    G gr;

    for (const auto u: len)
        gr.add_single_chain_component(u);

    // edge creating functors
    EdgeCreationOldChain<1, G> create_edge_1 {gr};
    EdgeCreationOldChain<2, G> create_edge_2 {gr};

    // create edge at the end A of the chain w0
    const auto c0 = create_edge_1(ESlot{w0, Ends::A});

    // create edge at the end B of the chain w1
    const auto c1 = create_edge_1(ESlot{w1, Ends::B});

    // create edge at the bulk position a = 1
    const auto c2 = create_edge_2(BSlot{w2, a});

    ASSERT_EQ(gr.edgenum, lensum + 3);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 3);
    ASSERT_EQ(gr.cn[w0].length(), len[0] + 1);
    ASSERT_EQ(gr.cn[w1].length(), len[1] + 1);
    ASSERT_EQ(gr.cn[w2].length(), len[2] + 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, lensum);
    ASSERT_EQ(gr.cn[w0].g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.cn[w2].g[a].ind, lensum + 2);
    ASSERT_EQ(gr.cn[w2].g[len[2]].ind, lensum - 1);
    ASSERT_EQ(c0[0], 0);
    ASSERT_EQ(c1[0], 1);
    ASSERT_EQ(c2[0], 2);

    ASSERT_EQ(gr.chis.cn11.size(), 3);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests creation of an edge in an existing disconnected cycle chain
TEST_F(CreateEdgeTest, CycleExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in an existing disconnected cycle chain"
            );

    constexpr std::array len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChId, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;
    constexpr EgId a {2};

    G gr;

    for (const auto u: len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // connect ends to form cycles
    merge11(ESlot{w0, eA}, ESlot{w0, eB});
    merge11(ESlot{w1, eA}, ESlot{w1, eB});
    merge11(ESlot{w2, eA}, ESlot{w2, eB});

    // edge creating functors
    EdgeCreationOldChain<1, G> create_edge_1 {gr};
    EdgeCreationOldChain<2, G> create_edge_2 {gr};

    // create edge at the end A of the chain w0
    const auto c0 = create_edge_1(ESlot{w0, Ends::A});

    // create edge at the end B of the chain w1
    const auto c1 = create_edge_1(ESlot{w1, Ends::B});

    // create edge at the bulk position a = 1
    const auto c2 = create_edge_2(BSlot{w2, a});

    ASSERT_EQ(gr.edgenum, lensum + 3);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 3);
    ASSERT_EQ(gr.cn[w0].length(), len[0] + 1);
    ASSERT_EQ(gr.cn[w1].length(), len[1] + 1);
    ASSERT_EQ(gr.cn[w2].length(), len[2] + 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, lensum);
    ASSERT_EQ(gr.cn[w0].g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.cn[w2].g[a].ind, lensum + 2);
    ASSERT_EQ(gr.cn[w2].g[len[2]].ind, lensum - 1);
    ASSERT_EQ(c0[0], 0);
    ASSERT_EQ(c1[0], 1);
    ASSERT_EQ(c2[0], 2);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[i].length());
        ASSERT_EQ(c.ww[0], i);
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j)
            ASSERT_EQ(m.g[j].c, c.ind);
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j)
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, c.ww[0]);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 3);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

}


/// Tests creation of an edge in an existing connected cycle and line chains
TEST_F(CreateEdgeTest, CycleLineExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests creation of an edge in an existing connected ",
            "cycle and line chains"
        );

    constexpr std::array<EgId, 3> len {3, 3, 3};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChId, 6> w {0, 1, 2, 3, 4, 5};
    const auto [w0, w1, w2, w3, w4, w5] = w;
    constexpr auto e = Ends::A;
    constexpr EgId a {2};
    constexpr EgId b {1};

    G gr;

    for (const auto u: len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};  // vertex merging

    // connect ends to form cycles linked to linear chains
    merge12(ESlot{w0, e}, BSlot{w0, a});    // creates w3
    merge12(ESlot{w1, e}, BSlot{w1, a});    // creates w4
    merge12(ESlot{w2, e}, BSlot{w2, a});    // creates w5

    // create edges
    EdgeCreationOldChain<1, G> create_edge_1 {gr};
    EdgeCreationOldChain<2, G> create_edge_2 {gr};

    // at the end A of the cycle chain w0
    const auto c0 = create_edge_1(ESlot{w0, Ends::A});

    // at the end A of the linear chain w3
    const auto c1 = create_edge_1(ESlot{w3, Ends::A});

    // at the end B of the cycle chain w1
    const auto c2 = create_edge_1(ESlot{w1, Ends::B});

    // at the end B of the linear chain w4
    const auto c3 = create_edge_1(ESlot{w4, Ends::B});

    // at bulk position a = 2 (which is the end B in this case)
    const auto c4 = create_edge_2(BSlot{w2, a});

    // at bulk position a = 1 (which is the end B in this case)
    const auto c5 = create_edge_2(BSlot{w5, b});

    ASSERT_EQ(gr.edgenum, lensum + 6);
    ASSERT_EQ(gr.chain_num(), 6);
    ASSERT_EQ(gr.cmpt_num(), 3);
    ASSERT_EQ(gr.cn[w0].length(), a + 1);
    ASSERT_EQ(gr.cn[w3].length(), len[0] - a + 1);
    ASSERT_EQ(gr.cn[w1].length(), a + 1);
    ASSERT_EQ(gr.cn[w4].length(), len[1] - a + 1);
    ASSERT_EQ(gr.cn[w2].length(), a + 1);
    ASSERT_EQ(gr.cn[w5].length(), len[2] - a + 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, lensum);
    ASSERT_EQ(gr.cn[w0].g[a].ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g[a].ind, lensum + 2);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.cn[w2].g[a].ind, lensum + 4);
    ASSERT_EQ(c0[0], 0);
    ASSERT_EQ(c1[0], c0[0]);
    ASSERT_EQ(c2[0], 1);
    ASSERT_EQ(c3[0], c2[0]);
    ASSERT_EQ(c4[0], 2);
    ASSERT_EQ(c5[0], c4[0]);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 3);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    // at bulk position a = 0 of the cycle chain (which is the end A)
    // assertion should fail, because EndSlot on end A should be used instead
    // create_edge(BulkSlot{w2, 0});

    // at bulk position a = 0 of the linear chain (which is the end A)
    // assertion should fail, because EndSlot on end A should be used instead
    // create_edge(BulkSlot{w5, 0});
}


/// Tests creation of an edge in a separate chain branched from linear chain
TEST_F(CreateEdgeTest, LineNewChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests creation of an edge in a separate chain ",
                          "branched from linear chain");

    constexpr EgId len {3};

    constexpr ChId w0 {0};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};

    constexpr EgId a1 {2};
    constexpr EgId a2 {1};

    G gr;

    gr.add_single_chain_component(len);

    EdgeCreationNewChain<2, G> create_edge {gr};  // edge creating functor

    // create edge at the bulk position a1
    const auto c0 = create_edge(BSlot{w0, a1});

    ASSERT_EQ(gr.edgenum, len + 1);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a1);
    ASSERT_EQ(gr.cn[w1].length(), 1);       // new chain
    ASSERT_EQ(gr.cn[w2].length(), len - a1);
    ASSERT_EQ(c0[0], 0);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 3);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    // create another edge at the bulk position a2
    const auto c1 = create_edge(BSlot{w0, a2});

    ASSERT_EQ(gr.edgenum, len + 2);
    ASSERT_EQ(gr.chain_num(), 5);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a2);
    ASSERT_EQ(gr.cn[w1].length(), 1);
    ASSERT_EQ(gr.cn[w2].length(), 1);
    ASSERT_EQ(gr.cn[w3].length(), 1);
    ASSERT_EQ(c1[0], 0);

    ASSERT_EQ(c.num_chains(), 5);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 1);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 4);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 4);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests creation of an edge in a separate chain branched from a cycle
TEST_F(CreateEdgeTest, DiscCycleTrue)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in a separate chain branched from a cycle"
        );

    constexpr EgId len {3};

    constexpr ChId w0 {0};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};
    constexpr ChId w4 {4};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a {1};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // connect ends to form a cycle
    merge11(ESlot{w0, eA}, ESlot{w0, eB});

    // create edge at the boundary vertex

    EdgeCreationNewChain<0, G> create_edge_0 {gr};  // edge creating functor

    const auto c0 = create_edge_0(w0);

    ASSERT_EQ(gr.edgenum, len + 1);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len);
    ASSERT_EQ(gr.cn[w1].length(), 1);       // new chain
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_EQ(c0[0], 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 1);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 1);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    // create edge at branching from an internal vertex

    EdgeCreationNewChain<2, G> create_edge_2 {gr};

    const auto c1 = create_edge_2(BSlot{w0, a});

    ASSERT_EQ(gr.edgenum, len + 2);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), 1);
    ASSERT_EQ(gr.cn[w2].length(), 1);
    ASSERT_EQ(gr.cn[w3].length(), len - a);
    ASSERT_EQ(c1[0], 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 2);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 2);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 2);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 2);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    // create edge at branching from a junction vertex

    EdgeCreationNewChain<3, G> create_edge_3 {gr};

    const auto c2 = create_edge_3(ESlot{w0, eA});

    ASSERT_EQ(gr.edgenum, len + 3);
    ASSERT_EQ(gr.chain_num(), 5);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), 1);
    ASSERT_EQ(gr.cn[w2].length(), 1);
    ASSERT_EQ(gr.cn[w3].length(), len - a);
    ASSERT_EQ(gr.cn[w4].length(), 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[eB].num(), 3);
    ASSERT_EQ(gr.cn[w4].ngs[eA].num(), 3);
    ASSERT_EQ(c2[0], 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 2);
    ASSERT_EQ(gr.chis.cn34.size(), 2);

    ASSERT_EQ(c.num_chains(), 5);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 1);
    ASSERT_EQ(c.chis.cn14.size(), 2);
    ASSERT_EQ(c.chis.cn34.size(), 2);
}


/// Tests creation of an edge in a separate chain branched from a junction
TEST_F(CreateEdgeTest, LineJunctionTrue)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in a separate chain branched from a junction"
        );

    constexpr EgId len {4};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a {1};

    G gr;

    gr.add_single_chain_component(len);

    // create a new edge and a chain branching from a bulk vertex inside w0

    EdgeCreationNewChain<2, G> create_edge_2 {gr};  // edge creating functor

    const auto c0 = create_edge_2(BSlot{w0, a});

    ASSERT_EQ(gr.edgenum, len + 1);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), 1);           // new chain
    ASSERT_EQ(gr.cn[w2].length(), len - a);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len);
    ASSERT_EQ(gr.cn[w2].g[0].ind, a);
    ASSERT_EQ(gr.cn[w2].g[len-a-1].ind, len - 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 0);
    ASSERT_EQ(c0[0], 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 3);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    // create a new edge and a chain branching from a junction vertex of w0

    EdgeCreationNewChain<3, G> create_edge_3 {gr};  // edge creating functor

    const auto c1 = create_edge_3(ESlot{w0, eB});

    ASSERT_EQ(gr.edgenum, len + 2);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), 1);
    ASSERT_EQ(gr.cn[w2].length(), len - a);
    ASSERT_EQ(gr.cn[w3].length(), 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len);
    ASSERT_EQ(gr.cn[w2].g[0].ind, a);
    ASSERT_EQ(gr.cn[w2].g[len-a-1].ind, len - 1);
    ASSERT_EQ(gr.cn[w3].g[0].ind, len + 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 3);
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 3);
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 0);
    ASSERT_EQ(gr.cn[w3].ngs[eA].num(), 3);
    ASSERT_EQ(gr.cn[w3].ngs[eB].num(), 0);
    ASSERT_EQ(c1[0], 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 4);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 4);
    ASSERT_EQ(c.chis.cn34.size(), 0);
}


}  // namespace graph_mutator::tests::edge_creation
