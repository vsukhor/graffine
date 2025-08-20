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
#include "graph-mutator/definitions.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/transforms/edge_deletion/deleting_host_chain.h"
#include "graph-mutator/transforms/edge_deletion/preserving_host_chain.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"
#include "graph-mutator/transforms/vertex_merger/from_22.h"

namespace graph_mutator::tests::edge_deletion {

using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
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
template<Degree D1,
         Degree D2,
         typename G> requires (is_implemented_degree<D1> &&
                               is_implemented_degree<D2>)
struct EdgeDeletion
    : public std::conditional_t<D1 == 1,
                graph_mutator::edge_deletion::DeletingHostChain<D2, G>,
                graph_mutator::edge_deletion::PreservingHostChain<D2, G>> {

    explicit EdgeDeletion(G& graph)
        : std::conditional_t<D1 == 1,
            graph_mutator::edge_deletion::DeletingHostChain<D2, G>,
            graph_mutator::edge_deletion::PreservingHostChain<D2, G>> {graph}
    {}
};


using DeleteEdgeTest = Test;

// =============================================================================


/// Tests deletion of an edge in a linear chain with both ends free.
TEST_F(DeleteEdgeTest, LineFree)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a linear chain with both ends free"
        );

    constexpr std::array<EgId, 3> len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};

    constexpr EgId a {2};

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    EdgeDeletion<2, 1, G> delete_edge_21 {gr};  // edge deleting functor
    EdgeDeletion<2, 2, G> delete_edge_22 {gr};  // edge deleting functor

    // delete edge at the end A of the chain w0
    const auto c0 = delete_edge_21(BSlot{w0, 0});

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
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at the end B of the chain w1
    const auto c1 = delete_edge_21(BSlot{w1, len[1] - 1});

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
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge inside w2 at position a = 2
    const auto c2 = delete_edge_22(BSlot{w2, a});

    ASSERT_EQ(gr.edgenum, lensum - 3);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 3);
    ASSERT_EQ(gr.cn[w0].length(), len[0] - 1);
    ASSERT_EQ(gr.cn[w1].length(), len[1] - 1);
    ASSERT_EQ(gr.cn[w2].length(), len[2] - 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g.back().ind, len[0] - 1 + len[1] - 1);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.cn[w2].g.back().ind, 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 6);
    ASSERT_EQ(gr.template num_vertices<2>(), lensum - gr.template num_vertices<1>());
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 3);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
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
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests deletion of an edge in a linear chain connected to two linear chains.
TEST_F(DeleteEdgeTest, Line3wayLines)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a linear chain connected ",
            "to two linear chains");

    constexpr std::array len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a {1};

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    VertexMerger<1, 2, G> merge12 {gr};  // merging free chain end to a bulk vertex

    // produce three connected linear chains
    merge12(ESlot{w0, eB}, BSlot{w1, a});

    EdgeDeletion<2, 1, G> delete_edge_21 {gr};

    // delete edge at the end A of the chain w0
    const auto c0 = delete_edge_21(BSlot{w0, 0});

    ASSERT_EQ(gr.edgenum, lensum - 1);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len[0] - 1);
    ASSERT_EQ(gr.cn[w1].length(), a);
    ASSERT_EQ(gr.cn[w2].length(), len[1] - a);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g.back().ind, len[0] - 1 + a);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + a);
    ASSERT_EQ(gr.cn[w2].g.back().ind, 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 3);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge inside w0
    EdgeDeletion<2, 2, G> delete_edge_22 {gr};
    const auto c1 = delete_edge_22(BSlot{w0, 1});

    ASSERT_EQ(gr.edgenum, lensum - 2);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len[0] - 2);
    ASSERT_EQ(gr.cn[w1].length(), a);
    ASSERT_EQ(gr.cn[w2].length(), len[1] - a);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g.back().ind, len[0] - 1 + a);
    ASSERT_EQ(gr.cn[w2].g[0].ind, len[0] + a);
    ASSERT_EQ(gr.cn[w2].g[len[1]-1 - a - 1].ind, 2);
    ASSERT_EQ(gr.cn[w2].g.back().ind, 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 3);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at the end of w2 connected to the junction
    EdgeDeletion<2, 3, G> delete_edge_23 {gr};
    const auto c2 = delete_edge_23(BSlot{w2, 0});

    ASSERT_EQ(gr.edgenum, lensum - 3);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len[0] - 2);
    ASSERT_EQ(gr.cn[w1].length(), a);
    ASSERT_EQ(gr.cn[w2].length(), len[1] - a - 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, len[0]);
    ASSERT_EQ(gr.cn[w1].g.back().ind, len[0] - 1 + a);
    ASSERT_EQ(gr.cn[w2].g[0].ind, 2);
    ASSERT_EQ(gr.cn[w2].g.back().ind, 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 3);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 3);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete the only edge of w1
    EdgeDeletion<1, 3, G> delete_edge_13 {gr};
    const auto c3 = delete_edge_13(ESlot{w1, Ends::B});

    ASSERT_EQ(gr.edgenum, lensum - 4);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len[0] + len[1] - 4);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, 0);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c3[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, 0);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests deletion of an edge in a linear chain connected to cycle by a degree
/// 3 vertex.
TEST_F(DeleteEdgeTest, LineConnectedToCycle3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a linear chain connected to ",
            "a cycle by a degree 3 vertex"
        );

    constexpr EgId len {6};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a {3};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};  // merging free chain end to a bulk vertex

    // connect end to bulk vertex on the same chain to form a cycle
    merge12(ESlot{w0, eB}, BSlot{w0, a});

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    // delete an internal edge of the linear chain
    const auto c0 = delete_edge_22(BSlot{w0, 1});

    ASSERT_EQ(gr.edgenum, len - 1);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a - 1);
    ASSERT_EQ(gr.cn[w1].length(), len - a);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, a);
    ASSERT_EQ(gr.cn[w1].g.back().ind, 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge the free-end edge of the linear chain
    EdgeDeletion<2, 1, G> delete_edge_21 {gr};
    const auto c1 = delete_edge_21(BSlot{w0, 0});

    ASSERT_EQ(gr.edgenum, len - 2);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a - 2);
    ASSERT_EQ(gr.cn[w1].length(), len - a);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 2);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, a);
    ASSERT_EQ(gr.cn[w1].g.back().ind, 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete the last edge remaining in the linear chain
    EdgeDeletion<1, 3, G> delete_edge_13 {gr};
    const auto c2 = delete_edge_13(ESlot{w0, Ends::B});

    // now w0 is a disconnected cycle
    ASSERT_EQ(gr.edgenum, len - 3);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len - 3);
    ASSERT_EQ(gr.cn[w0].g[0].ind, a - 1);
    ASSERT_EQ(gr.cn[w0].g.back().ind, 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w0, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, 0);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

}


/// Tests deletion of an edge in a cycle chain connected to a linear one by a
/// degree 3 vertex.
TEST_F(DeleteEdgeTest, CycleConnectedToLine3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a cycle chain connected to ",
            "a linear one by a degree 3 vertex"
        );

    constexpr EgId len {6};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a {1};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};  // merging free chain end to a bulk vertex

    // connect end to bulk vertex on the same chain to form a cycle
    merge12(ESlot{w0, eB}, BSlot{w0, a});

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    // delete an internal edge of the cycle chain
    constexpr EgId b0 {1};
    const auto c0 = delete_edge_22(BSlot{w1, b0});

    ASSERT_EQ(gr.edgenum, len - 1);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a );
    ASSERT_EQ(gr.cn[w1].length(), len - a - 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, a);
    ASSERT_EQ(gr.cn[w1].g.back().ind, a + b0);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at the end A of the cycle chain
    constexpr EgId b1 {};
    EdgeDeletion<2, 3, G> delete_edge_23 {gr};
    const auto c1 = delete_edge_23(BSlot{w1, b1});

    ASSERT_EQ(gr.edgenum, len - 2);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), len - a - 2);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, a + 2);   // a+1 was deleted in previous step
    ASSERT_EQ(gr.cn[w1].g.back().ind, a + b0);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at the end A of the cycle chain
    const auto c2 = delete_edge_23(BSlot{w1, gr.cn[w1].length() - 1});

    ASSERT_EQ(gr.edgenum, len - 3);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a);
    ASSERT_EQ(gr.cn[w1].length(), len - a - 3);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a - 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, 2);
    ASSERT_EQ(gr.cn[w1].g.back().ind, a);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests deletion of an edge in two linear and a cycle chains connected by
/// a degree 4 vertex.
TEST_F(DeleteEdgeTest, LineConnectedToCycle4)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in two linear and a cycle chains ",
            "connected by a degree 4 vertex"
        );

    constexpr EgId len {7};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    constexpr EgId a1 {3};
    constexpr EgId a2 {6};

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};  // functor merging internal chain edges

    // join internal vertices to form a cycle connected to two linear chains:
    // w2 becomes a cycle and w1 is a new single-edge linear chain linked with eA
    merge22(BSlot{w0, a1}, BSlot{w0, a2});

    EdgeDeletion<1, 4, G> delete_edge_14 {gr};
    EdgeDeletion<2, 1, G> delete_edge_21 {gr};
    EdgeDeletion<2, 2, G> delete_edge_22 {gr};
    EdgeDeletion<2, 3, G> delete_edge_23 {gr};
    EdgeDeletion<2, 4, G> delete_edge_24 {gr};

    // delete the last edge of the linear chain w0
    const auto c0 = delete_edge_24(BSlot{w0, gr.cn[w0].length() - 1});

    ASSERT_EQ(gr.edgenum, len - 1);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a1 - 1);
    ASSERT_EQ(gr.cn[w2].length(), a2 - a1);
    ASSERT_EQ(gr.cn[w1].length(), len - a2);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a1 - 2);
    ASSERT_EQ(gr.cn[w2].g[0].ind, a1);
    ASSERT_EQ(gr.cn[w2].g.back().ind, a2 - 1);
    ASSERT_EQ(gr.cn[w1].g.back().ind, a1 - 1);      // index of the deleted edge
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 3);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w2, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w2, eB}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 3);
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w2, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 3);
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w2, eA}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 3);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w2, eA}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w2, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 2);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 1);
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 1);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 2);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete the last edge of the cycle chain w2
    const auto c1 = delete_edge_24(BSlot{w2, gr.cn[w2].length() - 1});

    ASSERT_EQ(gr.edgenum, len - 2);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a1 - 1);
    ASSERT_EQ(gr.cn[w2].length(), a2 - a1 - 1);
    ASSERT_EQ(gr.cn[w1].length(), len - a2);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, a1 - 2);
    ASSERT_EQ(gr.cn[w2].g[0].ind, a1);
    ASSERT_EQ(gr.cn[w2].g.back().ind, a2 - 2);
    ASSERT_EQ(gr.cn[w1].g.back().ind, a1 - 1);      // index of the deleted edge
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 3);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w2, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w2, eB}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 3);
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w2, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eA].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 3);
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w2].ngs[eB].has(ESlot{w2, eA}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 3);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w2, eA}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w2, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 1);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 2);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 1);
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 1);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 2);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete the only edge of the linear chain w1: w2 is to become w1
    const auto c2 = delete_edge_14(ESlot{w1, Ends::A});

    ASSERT_EQ(gr.edgenum, len - 3);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), a1 - 1);
    ASSERT_EQ(gr.cn[w1].length(), a2 - a1 - 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, 1);
    ASSERT_EQ(gr.cn[w1].g[0].ind, a1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eA].has(ESlot{w1, eB}));
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w0, eB}));
    ASSERT_TRUE(gr.cn[w1].ngs[eB].has(ESlot{w1, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 1);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 1);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests deletion of an edge in a disconnected cycle chain.
TEST_F(DeleteEdgeTest, DisconnectedCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a disconnected cycle chain"
        );

    constexpr EgId len {5};

    constexpr ChId w0 {};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // join internal chain ends to form a disconnected cycle
    merge11(ESlot{w0, eA}, ESlot{w0, eB});

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    // delete an internal edge
    const auto c0 = delete_edge_22(BSlot{w0, 1});

    ASSERT_EQ(gr.edgenum, len - 1);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len - 1);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 0);
    ASSERT_EQ(gr.cn[w0].g.back().ind, 1);  // index of the deleted edge
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w0, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(c.gl[j].w, m.g[j].w);
            }
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, 0);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at end A
    EdgeDeletion<2, 0, G> delete_edge_20 {gr};
    const auto c1 = delete_edge_20(BSlot{w0, 0});

    ASSERT_EQ(gr.edgenum, len - 2);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len - 2);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 2);
    ASSERT_EQ(gr.cn[w0].g.back().ind, 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w0, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(c.gl[j].w, m.g[j].w);
            }
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, 0);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }

    // delete edge at end B
    const auto c2 = delete_edge_20(BSlot{w0, gr.cn[w0].length() - 1});

    ASSERT_EQ(gr.edgenum, len - 3);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);
    ASSERT_EQ(gr.cn[w0].length(), len - 3);
    ASSERT_EQ(gr.cn[w0].g[0].ind, 1);
    ASSERT_EQ(gr.cn[w0].ngs[eA].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eA].has(ESlot{w0, eB}));
    ASSERT_EQ(gr.cn[w0].ngs[eB].num(), 1);
    ASSERT_TRUE(gr.cn[w0].ngs[eB].has(ESlot{w0, eA}));
    ASSERT_EQ(gr.template num_vertices<1>(), 0);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.edgenum);
        for (szt k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.cn[c.ww[k]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k);
            for (EgId j {}; j<m.length(); ++j) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(c.gl[j].w, m.g[j].w);
            }
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, 0);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


}  // namespace graph_mutator::tests::edge_deletion
