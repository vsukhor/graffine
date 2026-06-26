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
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_split/functor.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"

#include <array>
#include <iostream>
#include <numeric>  // accumulate
#include <string>


namespace graffine::tests::edge_creation {

using namespace graffine;
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
         typename G> requires isImplementedDegree<D>
struct EdgeCreationNewChain
    : public trs::edge_creation::InNewChain<D, G> {

    explicit EdgeCreationNewChain(G& graph)
        : trs::edge_creation::InNewChain<D, G> {graph}
    {}
};

template<typename G>
struct EdgeCreationOldChain
    : public trs::edge_creation::InExistingChain<G> {

    explicit EdgeCreationOldChain(G& graph)
        : trs::edge_creation::InExistingChain<G> {graph}
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

    constexpr std::array<size_t, 3> len {1, 1, 1};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChIdG, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;
    constexpr EgIdA a {1};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    // edge creating functors
    EdgeCreationOldChain<G> create_edge {gr};

    // create edge at the end A of the chain w0
    const auto e0 = create_edge(ESlot{w0, End::A});

    // create edge at the end B of the chain w1
    const auto e1 = create_edge(ESlot{w1, End::B});

    // create edge at chain internal position a = 1
    const auto e2 = create_edge(BSlot{w2, a});

    ASSERT_EQ(gr.num_edges(), lensum + 3);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.chain(w0).length(), len[0] + 1);
    ASSERT_EQ(gr.chain(w1).length(), len[1] + 1);
    ASSERT_EQ(gr.chain(w2).length(), len[2] + 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, lensum);
    ASSERT_EQ(gr.chain(w0).g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.chain(w2).g[a].ind, lensum + 2);
    ASSERT_TRUE(e0[0]);
    ASSERT_TRUE(e1[0]);
    ASSERT_TRUE(e2[0]);
}


/// Tests creation of an edge in an existing linear chain.
TEST_F(CreateEdgeTest, LineMultiEdgeExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in an existing linear chain"
        );

    constexpr std::array len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChIdG, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;

    constexpr EgIdA a {2};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    // edge creating functors
    EdgeCreationOldChain<G> create_edge {gr};

    // create edge at the end A of the chain w0
    const auto e0 = create_edge(ESlot{w0, End::A});

    // create edge at the end B of the chain w1
    const auto e1 = create_edge(ESlot{w1, End::B});

    // create edge at chain internal position a = 1
    const auto e2 = create_edge(BSlot{w2, a});

    ASSERT_EQ(gr.num_edges(), lensum + 3);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.chain(w0).length(), len[0] + 1);
    ASSERT_EQ(gr.chain(w1).length(), len[1] + 1);
    ASSERT_EQ(gr.chain(w2).length(), len[2] + 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, lensum);
    ASSERT_EQ(gr.chain(w0).g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.chain(w2).g[a].ind, lensum + 2);
    ASSERT_EQ(gr.chain(w2).g[len[2]].ind, lensum - 1);
    ASSERT_TRUE(e0[0]);
    ASSERT_TRUE(e1[0]);
    ASSERT_TRUE(e2[0]);
}


/// Tests creation of an edge in an existing unconnected cycle chain
TEST_F(CreateEdgeTest, CycleExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in an existing unconnected cycle chain"
            );

    constexpr std::array len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChIdG, len.size()> w {0, 1, 2};
    const auto [w0, w1, w2] = w;

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;
    constexpr EgIdA a {2};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // connect ends to form cycles
    merge11(ESlot{w0, eA}, ESlot{w0, eB});
    merge11(ESlot{w1, eA}, ESlot{w1, eB});
    merge11(ESlot{w2, eA}, ESlot{w2, eB});

    // edge creating functors
    EdgeCreationOldChain<G> create_edge {gr};

    // create edge at the end A of the chain w0
    const auto e0 = create_edge(ESlot{w0, End::A});

    // create edge at the end B of the chain w1
    const auto e1 = create_edge(ESlot{w1, End::B});

    // create edge at chain internal position a = 1
    const auto e2 = create_edge(BSlot{w2, a});

    ASSERT_EQ(gr.num_edges(), lensum + 3);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.chain(w0).length(), len[0] + 1);
    ASSERT_EQ(gr.chain(w1).length(), len[1] + 1);
    ASSERT_EQ(gr.chain(w2).length(), len[2] + 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, lensum);
    ASSERT_EQ(gr.chain(w0).g[len[0]].ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g[len[1]].ind, lensum + 1);
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.chain(w2).g[a].ind, lensum + 2);
    ASSERT_EQ(gr.chain(w2).g[len[2]].ind, lensum - 1);
    ASSERT_TRUE(e0[0]);
    ASSERT_TRUE(e1[0]);
    ASSERT_TRUE(e2[0]);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(ChIdG{i()}).length());
        c.template check<profuse>();
        ASSERT_EQ(c.ww[0], i());
        const auto& m = gr.chain(c.ww[0]);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (std::size_t j {}; j<m.length(); ++j)
            ASSERT_EQ(m.g[j].c, c.ind);
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}


/// Tests creation of an edge in an existing connected cycle and line chains
TEST_F(CreateEdgeTest, CycleLineExistingChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests creation of an edge in an existing connected ",
            "cycle and line chains"
        );

    constexpr std::array<size_t, 3> len {3, 3, 3};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr std::array<ChIdG, 6> w {0, 1, 2, 3, 4, 5};
    const auto [w0, w1, w2, w3, w4, w5] = w;
    constexpr auto e = End::A;
    constexpr EgIdA a {2};
    constexpr EgIdA b {1};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};  // vertex merging

    // connect ends to form cycles linked to linear chains
    merge12(ESlot{w0, e}, BSlot{w0, a});    // creates w3
    merge12(ESlot{w1, e}, BSlot{w1, a});    // creates w4
    merge12(ESlot{w2, e}, BSlot{w2, a});    // creates w5

    // create edges
    EdgeCreationOldChain<G> create_edge {gr};

    // at the end A of the cycle chain w0
    const auto e0 = create_edge(ESlot{w0, End::A});

    // at the end A of the linear chain w3
    const auto e1 = create_edge(ESlot{w3, End::A});

    // at the end B of the cycle chain w1
    const auto e2 = create_edge(ESlot{w1, End::B});

    // at the end B of the linear chain w4
    const auto e3 = create_edge(ESlot{w4, End::B});

    // at chain internal position a = 2 (which is the end B in this case)
    const auto e4 = create_edge(BSlot{w2, a});

    // at chain internal position a = 1 (which is the end B in this case)
    const auto e5 = create_edge(BSlot{w5, b});

    ASSERT_EQ(gr.num_edges(), lensum + 6);
    ASSERT_EQ(gr.num_chains(), 6);
    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.chain(w0).length(), a + 1);
    ASSERT_EQ(gr.chain(w3).length(), len[0] - a + 1);
    ASSERT_EQ(gr.chain(w1).length(), a + 1);
    ASSERT_EQ(gr.chain(w4).length(), len[1] - a + 1);
    ASSERT_EQ(gr.chain(w2).length(), a + 1);
    ASSERT_EQ(gr.chain(w5).length(), len[2] - a + 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, lensum);
    ASSERT_EQ(gr.chain(w0).g[a].ind, a() - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g[a].ind, lensum + 2);
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.chain(w2).g[a].ind, lensum + 4);
    ASSERT_TRUE(e0[0]);
    ASSERT_TRUE(e1[0]);
    ASSERT_EQ(e1[0]->c, e0[0]->c);
    ASSERT_TRUE(e2[0]);
    ASSERT_TRUE(e3[0]);
    ASSERT_EQ(e3[0]->c, e2[0]->c);
    ASSERT_TRUE(e4[0]);
    ASSERT_TRUE(e5[0]);
    ASSERT_EQ(e5[0]->c, e4[0]->c);

    // at position a = 0 of the cycle chain (which is the end A)
    // assertion should fail, because EndSlot on end A should be used instead
    // create_edge(BulkSlot{w2, 0});

    // at position a = 0 of the linear chain (which is the end A)
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

    constexpr std::size_t len {3};

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {1};

    G gr;

    gr.add_single_chain_component(len);

    EdgeCreationNewChain<2, G> create_edge {gr};  // edge creating functor

    // create edge at chain internal position a1
    const auto e0 = create_edge(BSlot{w0, a1});

    ASSERT_EQ(gr.num_edges(), len + 1);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a1);
    ASSERT_EQ(gr.chain(w1).length(), 1);       // new chain
    ASSERT_EQ(gr.chain(w2).length(), len - a1);
    ASSERT_TRUE(e0[0]);

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 3);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    // create another edge at chain internal position a2
    const auto e1 = create_edge(BSlot{w0, a2});

    ASSERT_EQ(gr.num_edges(), len + 2);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_TRUE(e1[0]);

    ASSERT_EQ(c.num_chains(), 5);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 1);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 4);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
}


/// Tests creation of an edge in a separate chain branched from a cycle
TEST_F(CreateEdgeTest, DiscCycleTrue)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in a separate chain branched from a cycle"
        );

    constexpr std::size_t len {3};

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a {1};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // connect ends to form a cycle
    merge11(ESlot{w0, eA}, ESlot{w0, eB});

    // create edge at the boundary vertex of the cycle chain

    EdgeCreationNewChain<2, G> create_edge_2c {gr};  // edge creating functor

    const auto e0 = create_edge_2c(w0);

    ASSERT_EQ(gr.num_edges(), len + 1);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len);
    ASSERT_EQ(gr.chain(w1).length(), 1);       // new chain
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_TRUE(e0[0]);

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 1);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    // create edge at branching from an internal vertex

    EdgeCreationNewChain<2, G> create_edge_2 {gr};

    const auto e1 = create_edge_2(BSlot{w0, a});

    ASSERT_EQ(gr.num_edges(), len + 2);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), len - a);
    ASSERT_TRUE(e1[0]);

    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 2);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 2);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    // create edge at branching from a junction vertex

    EdgeCreationNewChain<3, G> create_edge_3 {gr};

    const auto e2 = create_edge_3(ESlot{w0, eA});

    ASSERT_EQ(gr.num_edges(), len + 3);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), len - a);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 3);
    ASSERT_TRUE(e2[0]);

    ASSERT_EQ(c.num_chains(), 5);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 2);
    ASSERT_EQ(c.chis.num({3, 4}), 2);
}


/// Tests creation of an edge in a separate chain branched from a junction
TEST_F(CreateEdgeTest, LineJunctionTrue)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests creation of an edge in a separate chain branched from a junction"
        );

    constexpr std::size_t len {4};

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a {1};

    G gr;

    gr.add_single_chain_component(len);

    // create a new edge and a chain branching from an internal vertex inside w0

    EdgeCreationNewChain<2, G> create_edge_2 {gr};  // edge creating functor

    const auto e0 = create_edge_2(BSlot{w0, a});

    ASSERT_EQ(gr.num_edges(), len + 1);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a);
    ASSERT_EQ(gr.chain(w1).length(), 1);           // new chain
    ASSERT_EQ(gr.chain(w2).length(), len - a);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len);
    ASSERT_EQ(gr.chain(w2).g[0].ind, a());
    ASSERT_EQ(gr.chain(w2).g[len-a-1].ind, len - 1);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_TRUE(e0[0]);

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);

    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 3);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    // create a new edge and a chain branching from a junction vertex of w0

    EdgeCreationNewChain<3, G> create_edge_3 {gr};  // edge creating functor

    const auto e1 = create_edge_3(ESlot{w0, eB});

    ASSERT_EQ(gr.num_edges(), len + 2);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), a);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), len - a);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len);
    ASSERT_EQ(gr.chain(w2).g[0].ind, a());
    ASSERT_EQ(gr.chain(w2).g[len-a-1].ind, len - 1);
    ASSERT_EQ(gr.chain(w3).g[0].ind, len + 1);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 0);
    ASSERT_TRUE(e1[0]);

    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), gr.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 4);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
}


}  // namespace graffine::tests::edge_creation
