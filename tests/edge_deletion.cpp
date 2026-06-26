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
#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/edge_deletion/deleting_host_chain.h"
#include "graffine/transforms/edge_deletion/preserving_host_chain.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_22.h"

#include <array>
#include <iostream>
#include <numeric>  // accumulate
#include <string>

namespace graffine::tests::edge_deletion {

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
template<Degree D1,
         Degree D2,
         typename G> requires (isImplementedDegree<D1> &&
                               isImplementedDegree<D2>)
struct EdgeDeletion
    : public std::conditional_t<D1 == Deg1,
                trs::edge_deletion::DeletingHostChain<D2, G>,
                trs::edge_deletion::PreservingHostChain<D2, G>> {

    explicit EdgeDeletion(G& graph)
        : std::conditional_t<D1 == Deg1,
            trs::edge_deletion::DeletingHostChain<D2, G>,
            trs::edge_deletion::PreservingHostChain<D2, G>> {graph}
    {}
};


using DeleteEdgeTest = Test;

// =============================================================================


/// Tests deletion of an edge in a linear chain with both ends free.
TEST_F(DeleteEdgeTest, LineFree)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a linear chain with both ends free"
        );

    constexpr std::array<size_t, 3> len {4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    ChIdG w {ChIdG::undefined};  // index of the affected chain

    constexpr EgIdA a {2};

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    EdgeDeletion<2, 1, G> delete_edge_21 {gr};  // edge deleting functor
    EdgeDeletion<2, 2, G> delete_edge_22 {gr};  // edge deleting functor


    // Subtest: delete edge at the end A of the chain w0.
    ++subtestCount;
    w = w0;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
                                  "Tests deleting an edge at the end A");

    const auto c0 = delete_edge_21(BSlot{w, 0});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

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
        for (EgIdA j {}; j<m.length(); ++j)
            ASSERT_EQ(m.g[j].c, c.ind);
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge at the end B of the chain w1.
    ++subtestCount;
    w = w1;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
                                  "Tests deleting an edge at the end B");

    const auto c1 = delete_edge_21(BSlot{w, len[1] - 1});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

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
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge inside w2 at position a.
    ++subtestCount;
    w = w2;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
                                  "Tests deleting an internal edge");

    const auto c2 = delete_edge_22(BSlot{w, a});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), lensum - 3);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.chain(w0).length(), len[0] - 1);
    ASSERT_EQ(gr.chain(w1).length(), len[1] - 1);
    ASSERT_EQ(gr.chain(w2).length(), len[2] - 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g.back().ind, len[0] - 1 + len[1] - 1);
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + len[1]);
    ASSERT_EQ(gr.chain(w2).g.back().ind, 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 6);
    ASSERT_EQ(gr.num_vertices(Deg2), lensum - gr.num_vertices(Deg1));
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);
    ASSERT_EQ(c1[0], 1);
    ASSERT_EQ(c2[0], 2);

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
        for (EgIdA j {}; j<m.length(); ++j)
            ASSERT_EQ(m.g[j].c, c.ind);
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // LineFree


/// Tests deletion of an edge in a linear chain connected to two linear chains.
TEST_F(DeleteEdgeTest, Line3wayLines)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge from a linear chain connected ",
            "to two linear chains");

    constexpr std::array len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    ChIdG w {ChIdG::undefined};  // index of the affected chain

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a {1};

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // merging free chain end to a chain internal vertex
    VertexMerger<1, 2, G> merge12 {gr};

    // produce three connected linear chains
    merge12(ESlot{w0, eB}, BSlot{w1, a});

    EdgeDeletion<2, 1, G> delete_edge_21 {gr};

    // Subtest: delete edge at the free end A of the chain w0.
    ++subtestCount;
    w = w0;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at the free end of the linear chain"
        );

    const auto c0 = delete_edge_21(BSlot{w, 0});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), lensum - 1);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len[0] - 1);
    ASSERT_EQ(gr.chain(w1).length(), a);
    ASSERT_EQ(gr.chain(w2).length(), len[1] - a);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g.back().ind(), len[0] - 1 + a);
    ASSERT_EQ(gr.chain(w2).g[0].ind(), len[0] + a);
    ASSERT_EQ(gr.chain(w2).g.back().ind, 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge inside w0.
    ++subtestCount;
    w = w0;

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an internal edge of the linear chain"
        );

    const auto c1 = delete_edge_22(BSlot{w, 1});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), lensum - 2);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len[0] - 2);
    ASSERT_EQ(gr.chain(w1).length(), a);
    ASSERT_EQ(gr.chain(w2).length(), len[1] - a);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g.back().ind, len[0] - 1 + a());
    ASSERT_EQ(gr.chain(w2).g[0].ind, len[0] + a());
    ASSERT_EQ(gr.chain(w2).g[len[1]-1 - a - 1].ind, 2);
    ASSERT_EQ(gr.chain(w2).g.back().ind, 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge at the end of w2 connected to the junction.
    ++subtestCount;
    w = w2;

    EdgeDeletion<2, 3, G> delete_edge_23 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at the end of the linear ",
            "chain connected to the junction"
        );

    const auto c2 = delete_edge_23(BSlot{w, 0});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), lensum - 3);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len[0] - 2);
    ASSERT_EQ(gr.chain(w1).length(), a);
    ASSERT_EQ(gr.chain(w2).length(), len[1] - a - 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, len[0] - 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, len[0]);
    ASSERT_EQ(gr.chain(w1).g.back().ind, len[0] - 1 + a());
    ASSERT_EQ(gr.chain(w2).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w2).g.back().ind, 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete the only edge of w1.
    ++subtestCount;
    w = w1;

    EdgeDeletion<1, 3, G> delete_edge_13 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting the only edge of a linear chain ",
            "connected to a junction at one end"
        );

    const auto c3 = delete_edge_13(ESlot{w, End::B});
    if constexpr (profuse)
        gr.compt(c3[0]).print("AFTER ", subtestCount);
    ASSERT_EQ(gr.num_edges(), lensum - 4);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len[0] + len[1] - 4);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, 0);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c3[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.cn_11()[0], 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // Line3wayLines


/// Tests deletion of an edge in a linear chain connected to cycle by a degree
/// 3 vertex.
TEST_F(DeleteEdgeTest, LineConnectedToCycle3)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge from a linear chain connected to ",
            "a cycle by a degree 3 vertex"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    ChIdG w {ChIdG::undefined};  // index of the affected chain
    ChIdG v {ChIdG::undefined};  // index of the unaffected chain

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a {3};

    G gr;

    gr.add_single_chain_component(len);

    // merging free chain end to a chain internal vertex
    VertexMerger<1, 2, G> merge12 {gr};

    // connect end to a chain internal vertex on the same chain to form a cycle
    merge12(ESlot{w0, eB}, BSlot{w0, a});

    // Subtest: delete an internal edge of the linear chain.
    ++subtestCount;
    w = w1;
    v = w0;

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an internal edge of the linear chain ",
            "connected to a cycle at one end"
        );

    const auto c0 = delete_edge_22(BSlot{w, 1});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 1);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), a - 1);
    ASSERT_EQ(gr.chain(v).length(), len - a);
    ASSERT_EQ(gr.chain(w).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w).g.back().ind, a() - 1);
    ASSERT_EQ(gr.chain(v).g[0].ind, a());
    ASSERT_EQ(gr.chain(v).g.back().ind, 1);
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge the free-end edge of the linear chain.
    ++subtestCount;
    w = w1;
    v = w0;

    EdgeDeletion<2, 1, G> delete_edge_21 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at the free end of the linear chain ",
            "connected to a cycle at one end"
        );

    const auto c1 = delete_edge_21(BSlot{w, 0});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 2);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), a - 2);
    ASSERT_EQ(gr.chain(v).length(), len - a);
    ASSERT_EQ(gr.chain(w).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w).g.back().ind, a() - 1);
    ASSERT_EQ(gr.chain(v).g[0].ind, a());
    ASSERT_EQ(gr.chain(v).g.back().ind, 1);
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        c.template check<profuse>();
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete the last edge remaining in the linear chain.
    ++subtestCount;
    w = w1;
    v = w0;

    EdgeDeletion<1, 3, G> delete_edge_13 {gr};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting the last remaining edge of the linear chain ",
            "connected to a cycle at one end"
        );

    const auto c2 = delete_edge_13(ESlot{w, End::B});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    // now w0 is an unconnected cycle
    ASSERT_EQ(gr.num_edges(), len - 3);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w0).length(), len - 3);
    ASSERT_EQ(gr.chain(w0).g[0].ind, a() - 1);
    ASSERT_EQ(gr.chain(w0).g.back().ind, 1);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 1);
    ASSERT_TRUE(gr.chain(w0).ngs[eA].contains(ESlot{w0, eB}));
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 1);
    ASSERT_TRUE(gr.chain(w0).ngs[eB].contains(ESlot{w0, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // LineConnectedToCycle3


/// Tests deleting an edge from a cycle chain connected to a linear chain by a
/// degree 3 vertex.
TEST_F(DeleteEdgeTest, CycleConnectedToLine3)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deleting an edge from a cycle chain connected to ",
            "a linear chain by a degree 3 vertex"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    auto w {ChIdG::undefined};  // index of the affected chain
    auto v {ChIdG::undefined};  // index of the unaffected chain

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a {1};

    G gr;

    gr.add_single_chain_component(len);

    // merging free chain end to a chain internal vertex
    VertexMerger<1, 2, G> merge12 {gr};

    // connect end to a chain internal vertex on the same chain to form a cycle
    merge12(ESlot{w0, eB}, BSlot{w0, a});

    EdgeDeletion<2, 2, G> delete_edge_22 {gr};

    // Subtest: delete an internal edge of the cycle chain.
    ++subtestCount;
    w = w0;
    v = w1;
    constexpr EgIdA b0 {1};

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an internal edge of the cycle chain ",
            "connected to a linear chain"
        );

    const auto c0 = delete_edge_22(BSlot{w, b0});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 1);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(v).length(), a );
    ASSERT_EQ(gr.chain(w).length(), len - a - 1);
    ASSERT_EQ(gr.chain(v).g[0].ind, 0);
    ASSERT_EQ(gr.chain(v).g.back().ind, a() - 1);
    ASSERT_EQ(gr.chain(w).g[0].ind, a());
    ASSERT_EQ(gr.chain(w).g.back().ind, a() + b0());
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (EgIdA j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge at the end A of the cycle chain.
    ++subtestCount;
    w = w0;
    v = w1;
    constexpr EgIdA b1 {};

    EdgeDeletion<2, 3, G> delete_edge_23 {gr};

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at end A of the cycle chain ",
            "connected to a linear chain"
        );

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    const auto c1 = delete_edge_23(BSlot{w, b1});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 2);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len - a - 2);
    ASSERT_EQ(gr.chain(v).g[0].ind, 0);
    ASSERT_EQ(gr.chain(v).g.back().ind, a() - 1);
    ASSERT_EQ(gr.chain(w).g[0].ind, a() + 2);   // a+1 was deleted in previous step
    ASSERT_EQ(gr.chain(w).g.back().ind, a() + b0());
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete edge at the end B of the cycle chain.
    ++subtestCount;
    w = w0;
    v = w1;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at end B of the cycle chain ",
            "connected to a linear chain"
        );

    const auto c2 = delete_edge_23(BSlot{w, gr.chain(w).length() - 1});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 3);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len - a - 3);
    ASSERT_EQ(gr.chain(v).g[0].ind, 0);
    ASSERT_EQ(gr.chain(v).g.back().ind, a() - 1);
    ASSERT_EQ(gr.chain(w).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w).g.back().ind, a());
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (EgIdA j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // CycleConnectedToLine3


/// Tests deletion of an edge in a component composed of two linear chains and
/// a cycle chain connected by a degree 4 vertex.
TEST_F(DeleteEdgeTest, LineConnectedToCycle4)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deletion of an edge in a component composed of two linear ",
            "chains and a cycle chain connected by a degree 4 vertex"
        );

    constexpr std::size_t len {7};

    constexpr ChIdG w0 {};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    ChIdG w {ChIdG::undefined};  // index of the affected chain
    ChIdG u {ChIdG::undefined};  // index of the first unaffected chain
    ChIdG v {ChIdG::undefined};  // index of the second unaffected chain

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    constexpr EgIdA a1 {3};
    constexpr EgIdA a2 {6};

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

    // Subtest: delete the last edge of a linear chain w1.
    ++subtestCount;
    w = w1;
    u = w2;
    v = w0;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting the last edge of a linear chain"
        );

    const auto c0 = delete_edge_24(BSlot{w, gr.chain(w).length() - 1});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 1);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), a1 - 1);
    ASSERT_EQ(gr.chain(v).length(), a2 - a1);
    ASSERT_EQ(gr.chain(u).length(), len - a2);
    ASSERT_EQ(gr.chain(w).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w).g.back().ind, a1() - 2);
    ASSERT_EQ(gr.chain(v).g[0].ind, a1());
    ASSERT_EQ(gr.chain(v).g.back().ind, a2() - 1);
    ASSERT_EQ(gr.chain(u).g.back().ind, a1() - 1);    // index of the deleted edge
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 3);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 3);
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eA].contains(ESlot{u, eA}));
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 3);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{v, eA}));
    ASSERT_EQ(gr.chain(u).ngs[eA].num(), 3);
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{v, eA}));
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_EQ(gr.chain(u).ngs[eB].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (EgIdA j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 1);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 2);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete the last edge of the cycle chain w0
    ++subtestCount;
    w = w0;
    u = w2;
    v = w1;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting the last edge of the cycle chain"
        );

    const auto c1 = delete_edge_24(BSlot{w, gr.chain(w).length() - 1});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 2);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(v).length(), a1 - 1);
    ASSERT_EQ(gr.chain(w).length(), a2 - a1 - 1);
    ASSERT_EQ(gr.chain(u).length(), len - a2);
    ASSERT_EQ(gr.chain(v).g[0].ind, 0);
    ASSERT_EQ(gr.chain(v).g.back().ind, a1() - 2);
    ASSERT_EQ(gr.chain(w).g[0].ind, a1());
    ASSERT_EQ(gr.chain(w).g.back().ind, a2() - 2);
    ASSERT_EQ(gr.chain(u).g.back().ind, a1() - 1);      // index of the deleted edge
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 3);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 3);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{u, eA}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 3);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.chain(u).ngs[eA].num(), 3);
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{w, eA}));
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(u).ngs[eB].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 3);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 1);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 2);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    // Subtest: delete the only edge of the linear chain w2: w1 is to become w2.
    ++subtestCount;
    w = w2;
    u = w0;
    v = w1;

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting the only edge of a linear chain"
        );

    const auto c2 = delete_edge_14(ESlot{w, End::A});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 3);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(v).length(), a1 - 1);
    ASSERT_EQ(gr.chain(u).length(), a2 - a1 - 1);
    ASSERT_EQ(gr.chain(v).g[0].ind, 0);
    ASSERT_EQ(gr.chain(v).g.back().ind, 1);
    ASSERT_EQ(gr.chain(u).g[0].ind, a1());
    ASSERT_EQ(gr.chain(v).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_TRUE(gr.chain(v).ngs[eB].contains(ESlot{u, eB}));
    ASSERT_EQ(gr.chain(u).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(u).ngs[eA].contains(ESlot{u, eB}));
    ASSERT_EQ(gr.chain(u).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[eB].contains(ESlot{v, eB}));
    ASSERT_TRUE(gr.chain(u).ngs[eB].contains(ESlot{u, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 2);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // LineConnectedToCycle4


/// Tests deleting an edge from an unconnected cycle chain.
TEST_F(DeleteEdgeTest, DisconnectedCycle)
{
    ++testCount;
    auto subtestCount {subtestCountIni};

    if constexpr (verboseT)
        print_description(
            "Tests deleting an edge from an unconnected cycle chain"
        );

    constexpr std::size_t len {5};

    constexpr ChIdG w {};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};  // functor merging free chain ends

    // join internal chain ends to form an unconnected cycle
    merge11(ESlot{w, eA}, ESlot{w, eB});

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    // Subtest: delete an internal edge
    ++subtestCount;

    EdgeDeletion<2, 2, G> delete_edge {gr};

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an internal edge of the cycle chain"
        );

    const auto c0 = delete_edge(BSlot{w, 1});

    if constexpr (profuse)
        gr.compt(c0[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 1);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), len - 1);
    ASSERT_EQ(gr.chain(w).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w).g.back().ind, 1);  // index of the deleted edge
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c0[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    if constexpr (profuse)
        gr.compt(w).print("BEFORE C1");

    // Subtest: delete edge at end A.
    ++subtestCount;

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at end A of the cycle chain"
        );

    const auto c1 = delete_edge(BSlot{w, 0});

    if constexpr (profuse)
        gr.compt(c1[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 2);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), len - 2);
    ASSERT_EQ(gr.chain(w).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w).g.back().ind, 1);
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c1[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }

    if constexpr (profuse)
        gr.compt(w).print("BEFORE ", subtestCount);

    // Subtest: delete edge at end B.
    ++subtestCount;

    if constexpr (verboseT)
        print_description_subtest(subtestCount,
            "Tests deleting an edge at end B of the cycle chain"
        );

    const auto c2 = delete_edge(BSlot{w, gr.chain(w).length() - 1});

    if constexpr (profuse)
        gr.compt(c2[0]).print("AFTER ", subtestCount);

    ASSERT_EQ(gr.num_edges(), len - 3);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.chain(w).length(), len - 3);
    ASSERT_EQ(gr.chain(w).g[0].ind, 1);
    ASSERT_EQ(gr.chain(w).ngs[eA].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(gr.chain(w).ngs[eB].num(), 1);
    ASSERT_TRUE(gr.chain(w).ngs[eB].contains(ESlot{w, eA}));
    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
    ASSERT_EQ(c2[0], 0);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.num_edges());
        c.template check<profuse>();
        for (std::size_t k {}; k<c.num_chains(); ++k) {
            ASSERT_EQ(c.ww[k], k);
            const auto& m = gr.chain(c.ww[k]);
            ASSERT_EQ(m.c, c.ind);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}  // DisconnectedCycle


}  // namespace graffine::tests::edge_deletion
