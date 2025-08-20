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
#include <filesystem>
#include <memory>
#include <string>

#include "common.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/transforms/vertex_merger/from_00.h"
#include "graph-mutator/transforms/vertex_merger/from_10.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"
#include "graph-mutator/transforms/vertex_merger/from_13.h"
#include "graph-mutator/transforms/vertex_merger/from_20.h"
#include "graph-mutator/transforms/vertex_merger/from_22.h"
#include "graph-mutator/transforms/vertex_split/to_11.h"
#include "graph-mutator/transforms/vertex_split/to_1B.h"
#include "graph-mutator/transforms/vertex_split/to_13.h"
#include "graph-mutator/transforms/vertex_split/to_BB.h"


namespace graph_mutator::tests::vertex_split {

using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using Ends = Chain::Ends;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G>
struct VertexMerger
    : public graph_mutator::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : vertex_merger::From<D1, D2, G> {graph}
    {}
};


/// Subclass to make protected members accessible for testing:
/// Use Fusability rather than Divisibility to be able to produce
/// branched vertex merger constracts needed for testing 'Divide'.
template<Degree D1,
         Degree D2,
         typename G>
struct VertexSplit
    : public graph_mutator::vertex_split::To<D1, D2, G> {
    static_assert(are_compatible_degrees<D1, D2>);

    explicit VertexSplit(G& graph)
        : graph_mutator::vertex_split::To<D1, D2, G> {graph}
    {}
};


class VertexSplitTest
    : public Test {

protected:

    static constexpr auto eA = Ends::A;
    static constexpr auto eB = Ends::B;
};

// =============================================================================


TEST_F(VertexSplitTest, Constructor)
{
    ++testCount;

    G gr;
    VertexSplit<1, 2, G> ct {gr};

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


/// Tests vertex split in a separate linear chain.
TEST_F(VertexSplitTest, Divide11_linA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex split in a separate linear chain");

    constexpr EgId len {4};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    for (EgId a {1}; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        VertexSplit<1, 1, G> divide {gr};
        divide(BSlot{w1, a});

        ASSERT_EQ(gr.edgenum, len);
        ASSERT_EQ(gr.chain_num(), 2);
        ASSERT_EQ(gr.cmpt_num(), 2);

        ASSERT_EQ(gr.template num_vertices<0>(), 0);
        ASSERT_EQ(gr.template num_vertices<1>(), 4);
        ASSERT_EQ(gr.template num_vertices<2>(), len - 2);
        ASSERT_EQ(gr.template num_vertices<3>(), 0);
        ASSERT_EQ(gr.template num_vertices<4>(), 0);

        ASSERT_EQ(gr.cn[w1].length(), a);
        ASSERT_EQ(gr.cn[w2].length(), len - a);

        for (ChId c {}, j {}; j<gr.chain_num(); ++j) {
            const auto& m = gr.cn[j];
            ASSERT_EQ(m.c, j);
            ASSERT_EQ(m.idw, j);
            for (EgId i {}; i<m.length(); ++i) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, i);
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c++);
            }
            ASSERT_EQ(m.ngs[eA].num(), 0);
            ASSERT_EQ(m.ngs[eB].num(), 0);
        }

        for (CmpId i {}; i<gr.ct.size(); ++i) {
            const auto& c = gr.ct[i];
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 1);
            ASSERT_EQ(c.num_edges(), gr.cn[i].length());
            ASSERT_EQ(c.ww[0], i);
            const auto& m = gr.cn[c.ww[0]];
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, 0);
            for (EgId j {}; j<m.length(); ++j) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(m.g[j].indc, j);
            }
            ASSERT_EQ(c.gl.size(), c.num_edges());
            for (EgId j {}; j<m.length(); ++j) {
                ASSERT_EQ(c.gl[j].w, m.g[j].w);
                ASSERT_EQ(c.gl[j].a, m.g[j].indw);
                ASSERT_EQ(c.gl[j].i, m.g[j].ind);
            }
            ASSERT_EQ(c.chis.cn11, i);
            ASSERT_EQ(c.chis.cn22, undefined<ChId>);
            ASSERT_EQ(c.chis.cn33.size(), 0);
            ASSERT_EQ(c.chis.cn44.size(), 0);
            ASSERT_EQ(c.chis.cn13.size(), 0);
            ASSERT_EQ(c.chis.cn14.size(), 0);
            ASSERT_EQ(c.chis.cn34.size(), 0);
        }
    }
}


/// Tests vertex split of a linear chain connected in a non-cycle component.
TEST_F(VertexSplitTest, Divide11_linB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex split of a linear chain connected ",
                          "in a non-cycle component");

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};  // chain produced by the vertex merge below
    constexpr ChId w4 {3};  // chain produced by the vertex split below

    constexpr EgId a1 {2};
    constexpr EgId a2 {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w1 and w2, joining w1 at its end B to w2 at a2:
    // produces a 3-w junction and a third chain
    merge12(ESlot{w1, eB},
            BSlot{w2, a2});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 3);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 3);
    ASSERT_EQ(gr.template num_vertices<2>(), 5);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w1].length(), len[0]);
    ASSERT_EQ(gr.cn[w2].length(), a2);
    ASSERT_EQ(gr.cn[w3].length(), len[1] - a2);

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
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w3].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w3].ngs[eB].num(), 0);

    for (szt i {}, k {}, h {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), lensum);
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

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w1, a1});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.template num_vertices<0>(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 5);
    ASSERT_EQ(gr.template num_vertices<2>(), 4);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w1].length(), a1);
    ASSERT_EQ(gr.cn[w4].length(), len[0] - a1);
    ASSERT_EQ(gr.cn[w2].length(), a2);
    ASSERT_EQ(gr.cn[w3].length(), len[1] - a2);

    for (ChId j {}; j<gr.chain_num(); ++j) {
        const auto& m = gr.cn[j];
        ASSERT_EQ(m.idw, j);
        for (EgId i {}; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].indw, i);
        }
    }
    ASSERT_EQ(gr.cn[w1].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w1].ngs[eB].num(), 0);
    ASSERT_EQ(gr.cn[w4].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w4].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w2].ngs[eA].num(), 0);
    ASSERT_EQ(gr.cn[w2].ngs[eB].num(), 2);
    ASSERT_EQ(gr.cn[w3].ngs[eA].num(), 2);
    ASSERT_EQ(gr.cn[w3].ngs[eB].num(), 0);

    ASSERT_EQ(gr.ct[0].num_chains(), 1);
    ASSERT_EQ(gr.ct[1].num_chains(), 3);
    ASSERT_EQ(gr.ct[0].num_edges(), a1);
    ASSERT_EQ(gr.ct[1].num_edges(), lensum - a1);
    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.gl.size(), c.num_edges());
        EgId h {};
        for (ChId k {}; const auto w: c.ww) {
            const auto& m = gr.cn[w];
            ASSERT_EQ(w, m.idw);
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
    }
    ASSERT_EQ(gr.ct[0].chis.cn11, 0);
    ASSERT_EQ(gr.ct[0].chis.cn22, undefined<ChId>);
    ASSERT_EQ(gr.ct[0].chis.cn33.size(), 0);
    ASSERT_EQ(gr.ct[0].chis.cn44.size(), 0);
    ASSERT_EQ(gr.ct[0].chis.cn13.size(), 0);
    ASSERT_EQ(gr.ct[0].chis.cn14.size(), 0);
    ASSERT_EQ(gr.ct[0].chis.cn34.size(), 0);
    ASSERT_EQ(gr.ct[1].chis.cn11, undefined<ChId>);
    ASSERT_EQ(gr.ct[1].chis.cn22, undefined<ChId>);
    ASSERT_EQ(gr.ct[1].chis.cn33.size(), 0);
    ASSERT_EQ(gr.ct[1].chis.cn44.size(), 0);
    ASSERT_EQ(gr.ct[1].chis.cn13.size(), 3);
    ASSERT_EQ(gr.ct[1].chis.cn14.size(), 0);
    ASSERT_EQ(gr.ct[1].chis.cn34.size(), 0);
}


/// Tests split of a boundary vertex in a separate cycle chain.
TEST_F(VertexSplitTest, Divide11_cyc)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a boundary vertex in a separate cycle chain"
        );

    constexpr EgId len {4};

    constexpr ChId w {};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};
    // connect ends to form a cycle
    merge11(ESlot{w, eA},
            ESlot{w, eB});

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w, 0});       // reverse by disconnecting at a = 0

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w].length(), len);

    const auto& m = gr.cn[w];

    ASSERT_EQ(m.c, w);
    for (EgId i=0; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].indw, i);
        ASSERT_EQ(m.g[i].indc, i);
        ASSERT_EQ(m.g[i].ind, i);
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[i].length());
        ASSERT_EQ(c.ww[0], i);
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
            ASSERT_EQ(c.gl[j].a, m.g[j].indw);
            ASSERT_EQ(c.gl[j].i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests split of an internal vertex in a separate cycle chain.
TEST_F(VertexSplitTest, Divide11_dic)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of an internal vertex in a separate cycle chain"
        );

    constexpr EgId len {4};
    constexpr EgId a {1};
    constexpr ChId w {};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};
    // connect ends to form a cycle
    merge11(ESlot{w, eA},
            ESlot{w, eB});

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w, a});       // disconnect at a

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.template num_vertices<1>(), 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[w].length(), len);

    const auto& m = gr.cn[w];

    ASSERT_EQ(m.c, w);
    for (EgId i=0; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].indw, i);
    }
    for (EgId i=0; i<m.length()-a; ++i) {
        ASSERT_EQ(m.g[i].indc, a+i);
        ASSERT_EQ(m.g[i].ind, a+i);
    }
    for (EgId i=m.length()-a; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].indc, i - (m.length()-a));
        ASSERT_EQ(m.g[i].ind, i - (m.length()-a));
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

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


/// Tests degree 3 vertex split by disconnecting a 13 chain at end B.
TEST_F(VertexSplitTest, Divide12a)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split ",
                          "by disconnecting a 13 chain at end B");

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId u {0};
    constexpr ChId v {1};

    constexpr EgId a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    VertexSplit<1, 2, G> divide {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(ESlot{u, eB},
            BSlot{v, a});

    // reverse: disconnect u at end B
    divide(ESlot{u, eB});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.template num_vertices<1>(), 4);
    ASSERT_EQ(gr.template num_vertices<2>(), lensum - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[u].length(), len[u]);
    ASSERT_EQ(gr.cn[v].length(), len[v]);

    for (ChId c=0, j=0; j<gr.chain_num(); ++j) {

        const auto& m = gr.cn[j];

        ASSERT_EQ(m.idw, j);
//        ASSERT_EQ(m.c, j);
        for (EgId i=0; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].indw, i);
            ASSERT_EQ(m.g[i].indc, i);
            ASSERT_EQ(m.g[i].ind, c++);
        }

        ASSERT_EQ(m.ngs[eA].num(), 0);
        ASSERT_EQ(m.ngs[eB].num(), 0);
    }

    ASSERT_EQ(gr.chis.cn11.size(), 2);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[i].length());
        ASSERT_EQ(c.ww[0], i);
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
            ASSERT_EQ(c.gl[j].a, m.g[j].indw);
            ASSERT_EQ(c.gl[j].i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn11, i);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests degree 3 vertex split by disconnecting a 13 chain at end A.
TEST_F(VertexSplitTest, Divide12b)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split ",
                          "by disconnecting a 13 chain at end A");

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId u {0};
    constexpr ChId v {1};
    constexpr ChId w {2};

    constexpr EgId a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge {gr};
    VertexSplit<1, 2, G> divide {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain w
    merge(ESlot{u, eB},
          BSlot{v, a});

    // disconnect w at end A
    divide(ESlot{w, eA});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.template num_vertices<1>(), 4);
    ASSERT_EQ(gr.template num_vertices<2>(), lensum - 2);
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    ASSERT_EQ(gr.cn[u].length(), len[u] + a);
    ASSERT_EQ(gr.cn[v].length(), len[v] - a);

    ASSERT_EQ(gr.cn[u].c, 1);
    ASSERT_EQ(gr.cn[v].c, 0);

    for (ChId c {}, j {gr.ind_last_chain()}; j<=0; --j) {
        const auto& m = gr.cn[j];
        for (EgId i {}; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].c, m.c);
            ASSERT_EQ(m.g[i].indw, i);
            ASSERT_EQ(m.g[i].indc, i);
            ASSERT_EQ(m.g[i].ind, ++c);
        }
        ASSERT_EQ(m.ngs[eA].num(), 0);
        ASSERT_EQ(m.ngs[eB].num(), 0);
    }

    ASSERT_EQ(gr.chis.cn11.size(), 2);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
            ASSERT_EQ(c.gl[j].a, m.g[j].indw);
            ASSERT_EQ(c.gl[j].i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests degree 3 vertex split in a circular chain at end A.
TEST_F(VertexSplitTest, Divide12c)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 vertex split in a circular chain at end A"
        );

    constexpr EgId len {10};

    constexpr ChId w {};
    constexpr auto e = eA;
    constexpr EgId a {6};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge {gr};
    VertexSplit<1, 2, G> divide {gr};

    // create a cycle by looping the chain w onto itself at edge a with end e
    merge(ESlot{w, e},
          BSlot{w, a});
    // disconnect w from the loop end A
    divide(ESlot{w, eA});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);

    const auto& m = gr.cn[w];

    ASSERT_EQ(m.idw, w);
    ASSERT_EQ(m.c, w);
    ASSERT_EQ(m.length(), len);
    for (EgId i=0; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].indw, i);
        ASSERT_EQ(m.g[i].indc, i);
        ASSERT_EQ(m.g[i].ind, i);
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);
    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
            ASSERT_EQ(c.gl[j].a, m.g[j].indw);
            ASSERT_EQ(c.gl[j].i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests degree 3 vertex split in a circular chain at end B.
TEST_F(VertexSplitTest, Divide12d)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 vertex split in a circular chain at end B"
        );

    constexpr EgId len {10};

    constexpr ChId w {};
    constexpr auto e = eA;
    constexpr EgId a {6};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge {gr};
    VertexSplit<1, 2, G> divide {gr};

    // create a cycle by looping the chain w onto itself at edge a with end e
    merge(ESlot{w, e},
          BSlot{w, a});

    // disconnect w from at the loop end B
    divide(ESlot{w, eB});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);

    const auto& m = gr.cn[w];

    ASSERT_EQ(m.c, w);
    ASSERT_EQ(m.length(), len);
    for (EgId i=0; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].indw, i);
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].indc, i);
        if (i < a)
            ASSERT_EQ(m.g[i].ind, a - 1 - i);
        else
            ASSERT_EQ(m.g[i].ind, i);
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);
    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[j].w);
            ASSERT_EQ(c.gl[j].a, m.g[j].indw);
            ASSERT_EQ(c.gl[j].i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests degree 3 vertex split by disconnecting a linear chain at end A
/// from a disconnected cycle one.
TEST_F(VertexSplitTest, Divide10)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split by disconnecting",
                          " a linear chain at end A from a disconnected cycle");

    constexpr EgId len {10};

    constexpr ChId w {};
    constexpr ChId v {1};    // chain resulting from the vertex merger.
    constexpr auto e = eA;
    constexpr EgId a {6};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge {gr};

    merge(ESlot{w, e}, BSlot{w, a});

    VertexSplit<1, 0, G> divide {gr};

    divide(ESlot{v, eA});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w];
    const auto& n = gr.cn[v];

    ASSERT_EQ(m.length(), a);
    ASSERT_EQ(n.length(), len - a);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_EQ(m.ngs[eA][0].w, w);
    ASSERT_EQ(m.ngs[eA][0].e, eB);
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_EQ(m.ngs[eB][0].w, w);
    ASSERT_EQ(m.ngs[eB][0].e, eA);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    EgId c {};
    for (ChId j {}; j<gr.chain_num(); ++j) {
        const auto& x = gr.cn[j];
        ASSERT_EQ(x.idw, j);
        for (EgId i {}; i<x.length(); ++i) {
            ASSERT_EQ(x.g[i].w, x.idw);
            ASSERT_EQ(x.g[i].indw, i);
            ASSERT_EQ(x.g[i].ind, c++);
        }
    }

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);
    for (CmpId i {}; i<gr.ct.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& m = gr.cn[c.ww[0]];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_TRUE(m.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.gl[j].w, m.g[c.gl[j].a].w);
            ASSERT_EQ(c.gl[j].a, m.g[c.gl[j].a].indw);
            ASSERT_EQ(c.gl[j].i, m.g[c.gl[j].a].ind);
        }
        if (m.is_disconnected_cycle()) {
            ASSERT_EQ(c.chis.cn11, undefined<ChId>);
            ASSERT_EQ(c.chis.cn22, c.ww[0]);
        }
        else {
            ASSERT_EQ(c.chis.cn11, c.ww[0]);
            ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        }
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end A
TEST_F(VertexSplitTest, Divide13a)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end A"
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);;

    constexpr ChId w1 {};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};   // chain resulting from merge22.
    constexpr ChId w4 {3};   // chain resulting from merge22.

    constexpr auto e = eA;
    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // connect w1 and w2 at bulk nodes to produce a 4-way junction and w3 and w4
    merge22(BSlot{w1, a1},
            BSlot{w2, a2});

    // disconnect w3 at its end A
    divide(ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];
    const auto& p = gr.cn[w3];
    const auto& q = gr.cn[w4];

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), a2);
    ASSERT_EQ(p.length(), len[w1] - a1);
    ASSERT_EQ(q.length(), len[w2] - a2);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_EQ(m.ngs[eB][0].w, w2);
    ASSERT_EQ(m.ngs[eB][0].e, eB);
    ASSERT_EQ(m.ngs[eB][1].w, w4);
    ASSERT_EQ(m.ngs[eB][1].e, eA);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_EQ(n.ngs[eB][0].w, w1);
    ASSERT_EQ(n.ngs[eB][0].e, eB);
    ASSERT_EQ(n.ngs[eB][1].w, w4);
    ASSERT_EQ(n.ngs[eB][1].e, eA);

    ASSERT_EQ(p.ngs[eA].num(), 0);
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(q.ngs[eA].num(), 2);
    ASSERT_EQ(q.ngs[eA][0].w, w2);
    ASSERT_EQ(q.ngs[eA][0].e, eB);
    ASSERT_EQ(q.ngs[eA][1].w, w1);
    ASSERT_EQ(q.ngs[eA][1].e, eB);
    ASSERT_EQ(q.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
    const auto& u = gr.cn[c.ww[0]];
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgId j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (EgId j {}; j<c.num_edges(); ++j) {
        ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
        ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
        ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
    }
    ASSERT_EQ(c.chis.cn11, c.ww[0]);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    const auto& d = gr.ct[1];
    ASSERT_EQ(d.ind, 1);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), lensum - c.num_edges());
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 3);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end B.
TEST_F(VertexSplitTest, Divide13b)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end B"
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);;

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};   // chain resulting from merge22.
    constexpr ChId w4 {3};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // connect w1 and w2 at bulk nodes to produce a 4-way junction and w3 and w4
    merge22(BSlot{w1, a1},
            BSlot{w2, a2});

    // disconnect w2 at its end B
    divide(ESlot{w2, e});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];
    const auto& p = gr.cn[w3];
    const auto& q = gr.cn[w4];

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), a2);
    ASSERT_EQ(p.length(), len[w1] - a1);
    ASSERT_EQ(q.length(), len[w2] - a2);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_EQ(m.ngs[eB][0].w, w3);
    ASSERT_EQ(m.ngs[eB][0].e, eA);
    ASSERT_EQ(m.ngs[eB][1].w, w4);
    ASSERT_EQ(m.ngs[eB][1].e, eA);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_EQ(p.ngs[eA][0].w, w1);
    ASSERT_EQ(p.ngs[eA][0].e, eB);
    ASSERT_EQ(p.ngs[eA][1].w, w4);
    ASSERT_EQ(p.ngs[eA][1].e, eA);
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(q.ngs[eA].num(), 2);
    ASSERT_EQ(q.ngs[eA][0].w, w3);
    ASSERT_EQ(q.ngs[eA][0].e, eA);
    ASSERT_EQ(q.ngs[eA][1].w, w1);
    ASSERT_EQ(q.ngs[eA][1].e, eB);
    ASSERT_EQ(q.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
    const auto& u = gr.cn[c.ww[0]];
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgId j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (EgId j {}; j<c.num_edges(); ++j) {
        ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
        ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
        ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
    }
    ASSERT_EQ(c.chis.cn11, c.ww[0]);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    const auto& d = gr.ct[1];
    ASSERT_EQ(d.ind, 1);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), lensum - c.num_edges());
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 3);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and one cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle chain at end A.
TEST_F(VertexSplitTest, Divide13c)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains ",
            "and one cycle\nVertex split 4 -> 1+3 is done by disconnecting ",
            "a cycle chain at end A"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};   // chain resulting from merge22
    constexpr ChId w3 {2};   // chain resulting from merge22

    constexpr auto e = eA;
    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    divide(ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 3);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];
    const auto& p = gr.cn[w3];

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), len - a2);
    ASSERT_EQ(p.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_EQ(m.ngs[eB][0].w, w3);
    ASSERT_EQ(m.ngs[eB][0].e, eB);
    ASSERT_EQ(m.ngs[eB][1].w, w2);
    ASSERT_EQ(m.ngs[eB][1].e, eA);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_EQ(n.ngs[eA][0].w, w3);
    ASSERT_EQ(n.ngs[eA][0].e, eB);
    ASSERT_EQ(n.ngs[eA][1].w, w1);
    ASSERT_EQ(n.ngs[eA][1].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(p.ngs[eA].num(), 0);
    ASSERT_EQ(p.ngs[eB].num(), 2);
    ASSERT_EQ(p.ngs[eB][0].w, w1);
    ASSERT_EQ(p.ngs[eB][0].e, eB);
    ASSERT_EQ(p.ngs[eB][1].w, w2);
    ASSERT_EQ(p.ngs[eB][1].e, eA);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), len);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 3);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle chain at end B.
TEST_F(VertexSplitTest, Divide13d)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains ",
            "and a cycle \nVertex split 4 -> 1+3 is done by disconnecting ",
            "a cycle chain at end B"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};   // chain resulting from merge22.
    constexpr ChId w3 {2};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // loop w1 onto itself at bulk sites to produce a loop and two linear chains
    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // disconnect the cycle chain at end B to produce linear chains connected
    // by a 3-way junction
    divide(ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 3);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];
    const auto& p = gr.cn[w3];

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), len - a2);
    ASSERT_EQ(p.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_EQ(m.ngs[eB][0].w, w3);
    ASSERT_EQ(m.ngs[eB][0].e, eA);
    ASSERT_EQ(m.ngs[eB][1].w, w2);
    ASSERT_EQ(m.ngs[eB][1].e, eA);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_EQ(n.ngs[eA][0].w, w3);
    ASSERT_EQ(n.ngs[eA][0].e, eA);
    ASSERT_EQ(n.ngs[eA][1].w, w1);
    ASSERT_EQ(n.ngs[eA][1].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_EQ(p.ngs[eA][0].w, w1);
    ASSERT_EQ(p.ngs[eA][0].e, eB);
    ASSERT_EQ(p.ngs[eA][1].w, w2);
    ASSERT_EQ(p.ngs[eA][1].e, eA);
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 3);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);
    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), len);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 3);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining linear 2 chains and 1 cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end B.
TEST_F(VertexSplitTest, Divide13e)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining 2 linear chains ",
            "and 1 cycle \nVertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end B"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};   // chain resulting from merge22.
    constexpr ChId w3 {2};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgId a1 {1};
    constexpr EgId a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // loop w1 onto itself at bulk sites to produce a loop and two linear chains
    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // disconnect the linear chain w1 at end B
    divide(ESlot{w1, e});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 3);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];
    const auto& p = gr.cn[w3];

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), len - a2);
    ASSERT_EQ(p.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_EQ(n.ngs[eA][0].w, w3);
    ASSERT_EQ(n.ngs[eA][0].e, eA);
    ASSERT_EQ(n.ngs[eA][1].w, w3);
    ASSERT_EQ(n.ngs[eA][1].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_EQ(p.ngs[eA][0].w, w3);
    ASSERT_EQ(p.ngs[eA][0].e, eB);
    ASSERT_EQ(p.ngs[eA][1].w, w2);
    ASSERT_EQ(p.ngs[eA][1].e, eA);
    ASSERT_EQ(p.ngs[eB].num(), 2);
    ASSERT_EQ(p.ngs[eB][0].w, w3);
    ASSERT_EQ(p.ngs[eB][0].e, eA);
    ASSERT_EQ(p.ngs[eB][1].w, w2);
    ASSERT_EQ(p.ngs[eB][1].e, eA);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
    const auto& u = gr.cn[c.ww[0]];
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgId j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (EgId j {}; j<c.num_edges(); ++j) {
        ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
        ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
        ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
    }
    ASSERT_EQ(c.chis.cn11, c.ww[0]);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    const auto& d = gr.ct[1];
    ASSERT_EQ(d.ind, 1);
    ASSERT_EQ(d.num_chains(), 2);
    ASSERT_EQ(d.num_edges(), len - c.num_edges());
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 1);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 1);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two cycles.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle at end A.
TEST_F(VertexSplitTest, Divide13f)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two cycles\n",
            "Vertex split 4 -> 1+3 is done by disconnecting a cycle at end A"
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    constexpr auto e = eA;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<0, 0, G> merge00 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge11(ESlot{w1, eA}, ESlot{w1, eB});  // make a cycle out of w1
    merge11(ESlot{w2, eA}, ESlot{w2, eB});  // make a cycle out of w2
    merge00(w1, w2);        // join the two cycles at their end vertices

    divide(ESlot{w1, e});    // disconnect end A of w1

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];

    ASSERT_EQ(m.length(), len[w1]);
    ASSERT_EQ(n.length(), len[w2]);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_EQ(m.ngs[eB][0].w, w2);
    ASSERT_EQ(m.ngs[eB][0].e, eA);
    ASSERT_EQ(m.ngs[eB][1].w, w2);
    ASSERT_EQ(m.ngs[eB][1].e, eB);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_EQ(n.ngs[eA][0].w, w1);
    ASSERT_EQ(n.ngs[eA][0].e, eB);
    ASSERT_EQ(n.ngs[eA][1].w, w2);
    ASSERT_EQ(n.ngs[eA][1].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_EQ(n.ngs[eB][0].w, w1);
    ASSERT_EQ(n.ngs[eB][0].e, eB);
    ASSERT_EQ(n.ngs[eB][1].w, w2);
    ASSERT_EQ(n.ngs[eB][1].e, eA);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 2);
    ASSERT_EQ(d.num_edges(), lensum);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 1);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 1);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two cycles.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle at end B.
TEST_F(VertexSplitTest, Divide13g)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two cycles\n",
            "Vertex split 4 -> 1+3 is done by disconnecting a cycle at end B"
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    constexpr auto e = eB;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<0, 0, G> merge00 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge11(ESlot{w1, eA},
            ESlot{w1, eB});  // make a cycle out of w1
    merge11(ESlot{w2, eA},
            ESlot{w2, eB});  // make a cycle out of w2
    merge00(w1, w2);        // join the two cycles at their internal ends

    divide(ESlot{w1, e});    // disconnect end B of w1

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w2];

    ASSERT_EQ(m.length(), len[w1]);
    ASSERT_EQ(n.length(), len[w2]);

    ASSERT_EQ(m.ngs[eA].num(), 2);
    ASSERT_EQ(m.ngs[eA][0].w, w2);
    ASSERT_EQ(m.ngs[eA][0].e, eA);
    ASSERT_EQ(m.ngs[eA][1].w, w2);
    ASSERT_EQ(m.ngs[eA][1].e, eB);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_EQ(n.ngs[eA][0].w, w1);
    ASSERT_EQ(n.ngs[eA][0].e, eA);
    ASSERT_EQ(n.ngs[eA][1].w, w2);
    ASSERT_EQ(n.ngs[eA][1].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_EQ(n.ngs[eB][0].w, w2);
    ASSERT_EQ(n.ngs[eB][0].e, eA);
    ASSERT_EQ(n.ngs[eB][1].w, w1);
    ASSERT_EQ(n.ngs[eB][1].e, eA);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 1);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 1);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 2);
    ASSERT_EQ(d.num_edges(), lensum);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 1);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 1);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting the two chains at ends A.
TEST_F(VertexSplitTest, Divide22_4Lines_AA_BB)
{
    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "the two chains at ends A"
        );

    constexpr std::array<EgId, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w3 {2};
    constexpr ChId w5 {4};  // produced by merge22
    constexpr ChId w6 {5};  // produced by merge22

    constexpr EgId a1 {2};
    constexpr EgId a2 {2};
    constexpr auto e = eA;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at bulk vertices

    VertexMerger<Deg2, Deg2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains at ends A of w5  and w6

    VertexSplit<Deg2, Deg2, G> divide {gr};

    divide(ESlot{w5, e},
           ESlot{w6, e});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w3];

    ASSERT_EQ(m.length(), a1 + len[w3] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 4);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 4);

    for (CmpId ic {}; ic<gr.ct.size(); ++ic) {
        const auto& c = gr.ct[ic];
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& u = gr.cn[c.ww[0]];
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<c.num_edges(); ++j) {
            ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
            ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
            ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting one chain at end A
/// and the other one at end B.
TEST_F(VertexSplitTest, Divide22_4Lines_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "one chain at end A and the other one at end B");

    constexpr std::array<EgId, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w3 {2};
    constexpr ChId w6 {5};

    constexpr EgId a1 {2};
    constexpr EgId a2 {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at bulk vertices

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains at end B of w1 and end A of w6

    VertexSplit<Deg2, Deg2, G> divide {gr};

    divide(ESlot{w6, eA},
           ESlot{w1, eB});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w3];

    ASSERT_EQ(m.length(), a1 + len[w3] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 4);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 4);

    for (CmpId ic {}; ic<gr.ct.size(); ++ic) {
        const auto& c = gr.ct[ic];
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& u = gr.cn[c.ww[0]];
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<c.num_edges(); ++j) {
            ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
            ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
            ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting the two chains at ends B.
TEST_F(VertexSplitTest, Divide22_4Lines_BB_AA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains.\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "the two chains at ends B"
        );

    constexpr std::array<EgId, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w3 {2};

    constexpr EgId a1 {2};
    constexpr EgId a2 {2};
    constexpr auto e = eB;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at bulk vertices

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains w1 and w2 at end B

    VertexSplit<2, 2, G> divide {gr};

    divide(ESlot{w1, e},
           ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 4);

    const auto& m = gr.cn[w1];
    const auto& n = gr.cn[w3];

    ASSERT_EQ(m.length(), a1 + len[w3] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 4);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 4);

    for (CmpId ic {}; ic<gr.ct.size(); ++ic) {
        const auto& c = gr.ct[ic];
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& u = gr.cn[c.ww[0]];
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<c.num_edges(); ++j) {
            ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
            ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
            ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
        }
        ASSERT_EQ(c.chis.cn11, c.ww[0]);
        ASSERT_EQ(c.chis.cn22, undefined<ChId>);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has ends A on both chains, the other pair has ends B.
TEST_F(VertexSplitTest, Divide22_2Lines_Cycle_AA_BB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "One pair has ends A on both chains, the other pair has ends B"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};

    constexpr EgId a1 {2};
    constexpr EgId a2 {4};
    constexpr auto e = eA;

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at bulk vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    VertexSplit<2, 2, G> divide {gr};

    divide(ESlot{w2, e},
           ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, 0);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 0);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has ends B on both chains, the other pair has ends A.
TEST_F(VertexSplitTest, Divide22_2Lines_Cycle_BB_AA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "One pair has ends B on both chains, the other pair has ends A"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w3 {2};

    constexpr EgId a1 {2};
    constexpr EgId a2 {4};
    constexpr auto e = eB;

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at bulk vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    VertexSplit<2, 2, G> divide {gr};

    divide(ESlot{w1, e},
           ESlot{w3, e});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, 0);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 0);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has ends A on both chains, the other pair has ends B.
TEST_F(VertexSplitTest, Divide22_2Lines_Cycle_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "The first pair has end A on lunear chane and end B on cycle"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};

    constexpr EgId a1 {2};
    constexpr EgId a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at bulk vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    VertexSplit<2, 2, G> divide {gr};

    divide(ESlot{w2, eA},
           ESlot{w3, eB});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 1);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 1);

    const auto& d = gr.ct[0];
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, 0);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 0);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+0 is done using linear chain slots.
TEST_F(VertexSplitTest, Divide20_2Lines_Cycle1_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+0 is done using linear chain slots"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    constexpr EgId a1 {2};
    constexpr EgId a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at bulk vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    VertexSplit<2, 0, G> divide {gr};

    divide(ESlot{w2, eA},
           ESlot{w1, eB});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_EQ(m.ngs[eA][0].w, w1);
    ASSERT_EQ(m.ngs[eA][0].e, eB);
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_EQ(m.ngs[eB][0].w, w1);
    ASSERT_EQ(m.ngs[eB][0].e, eA);

    const auto& n = gr.cn[w2];
    ASSERT_EQ(n.length(), len - (a2 - a1));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 1);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
    const auto& u = gr.cn[c.ww[0]];
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgId j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (EgId j {}; j<c.num_edges(); ++j) {
        ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
        ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
        ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
    }
    ASSERT_EQ(c.chis.cn11, c.ww[0]);
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    const auto& d = gr.ct[1];
    ASSERT_EQ(d.ind, 1);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len - c.num_edges());
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, undefined<ChId>);
    ASSERT_EQ(d.chis.cn22, d.ww[0]);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 0);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+0 is done using the cycle chain slots.
TEST_F(VertexSplitTest, Divide20_2Lines_Cycle2_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+0 is done using the cycle chain slots"
        );

    constexpr EgId len {6};

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};
    constexpr ChId w3 {2};

    constexpr EgId a1 {2};
    constexpr EgId a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at bulk vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    VertexSplit<2, 0, G> divide {gr};

    divide(ESlot{w3, eA},
           ESlot{w3, eB});

    ASSERT_EQ(gr.edgenum, len);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_EQ(m.ngs[eA][0].w, w1);
    ASSERT_EQ(m.ngs[eA][0].e, eB);
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_EQ(m.ngs[eB][0].w, w1);
    ASSERT_EQ(m.ngs[eB][0].e, eA);

    const auto& n = gr.cn[w2];
    ASSERT_EQ(n.length(), len - (a2 - a1));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    const auto& c = gr.ct[0];
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
    const auto& u = gr.cn[c.ww[0]];
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgId j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (EgId j {}; j<c.num_edges(); ++j) {
        ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
        ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
        ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
    }
    ASSERT_EQ(c.chis.cn11, undefined<ChId>);
    ASSERT_EQ(c.chis.cn22, c.ww[0]);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    const auto& d = gr.ct[1];
    ASSERT_EQ(d.ind, 1);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len - c.num_edges());
    for (ChId i {}; i<d.num_chains(); i++) {
        const auto& u = gr.cn[d.ww[i]];
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
        ASSERT_EQ(d.gl.size(), d.num_edges());
    }
    ASSERT_EQ(d.chis.cn11, d.ww[0]);
    ASSERT_EQ(d.chis.cn22, undefined<ChId>);
    ASSERT_EQ(d.chis.cn33.size(), 0);
    ASSERT_EQ(d.chis.cn44.size(), 0);
    ASSERT_EQ(d.chis.cn13.size(), 0);
    ASSERT_EQ(d.chis.cn14.size(), 0);
    ASSERT_EQ(d.chis.cn34.size(), 0);
}


/// Tests split 4 -> 0+0 of a vertex joining two cycle chains.
TEST_F(VertexSplitTest, Divide00_2Cycles)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split 4 -> 0+0 of a vertex joining two cycle chains"
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChId w1 {0};
    constexpr ChId w2 {1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w1, eA},
            ESlot{w1, eB});  // make cycle of the 1st chain

    merge11(ESlot{w2, eA},
            ESlot{w2, eB});  // make cycle of the 2nd chain

    VertexMerger<0, 0, G> merge00 {gr};

    merge00(w1, w2);              // join the two cycles on boundary vertices

    // split the component symmetrically

    VertexSplit<0, 0, G> divide {gr};  // disconnect the cycles

    divide(ESlot{w1, eA},
           ESlot{w1, eB});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), 2);

    const auto& m = gr.cn[w1];

    ASSERT_EQ(m.length(), len[w1]);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_EQ(m.ngs[eA][0].w, w1);
    ASSERT_EQ(m.ngs[eA][0].e, eB);
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_EQ(m.ngs[eB][0].w, w1);
    ASSERT_EQ(m.ngs[eB][0].e, eA);

    const auto& n = gr.cn[w2];
    ASSERT_EQ(n.length(), len[w2]);

    ASSERT_EQ(n.ngs[eA].num(), 1);
    ASSERT_EQ(n.ngs[eA][0].w, w2);
    ASSERT_EQ(n.ngs[eA][0].e, eB);
    ASSERT_EQ(n.ngs[eB].num(), 1);
    ASSERT_EQ(n.ngs[eB][0].w, w2);
    ASSERT_EQ(n.ngs[eB][0].e, eA);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 2);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);

    ASSERT_EQ(gr.cmpt_num(), 2);

    for (CmpId ic {}; ic<gr.ct.size(); ++ic) {
        const auto& c = gr.ct[ic];
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
        const auto& u = gr.cn[c.ww[0]];
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgId j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.gl.size(), c.num_edges());
        for (EgId j {}; j<c.num_edges(); ++j) {
            ASSERT_EQ(c.gl[j].w, u.g[c.gl[j].a].w);
            ASSERT_EQ(c.gl[j].a, u.g[c.gl[j].a].indw);
            ASSERT_EQ(c.gl[j].i, u.g[c.gl[j].a].ind);
        }
        ASSERT_EQ(c.chis.cn11, undefined<ChId>);
        ASSERT_EQ(c.chis.cn22, c.ww[0]);
        ASSERT_EQ(c.chis.cn33.size(), 0);
        ASSERT_EQ(c.chis.cn44.size(), 0);
        ASSERT_EQ(c.chis.cn13.size(), 0);
        ASSERT_EQ(c.chis.cn14.size(), 0);
        ASSERT_EQ(c.chis.cn34.size(), 0);
    }
}


}  // namespace graph_mutator::tests::vertex_split
