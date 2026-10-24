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
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/structure/analyzers/integral_tests.h"


#include <array>
#include <iostream>
#include <memory>
#include <numeric>  // accumulate
#include <string>
#include <type_traits>
#include <vector>


namespace graffine::tests::graph {

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


class GraphTest
    : public Test {
};

// =============================================================================


TEST_F(GraphTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests constructor of the graph struct");

    G gr;
    const IntegrityCheck integrityCheck {gr};

    ASSERT_EQ(gr.num_edges(), 0);
    ASSERT_EQ(gr.num_chains(), 0);
    ASSERT_EQ(gr.num_compts(), 0);
    ASSERT_EQ(gr.num_chains(), 0);
    ASSERT_EQ(gr.get_egl().size(), 0);
    ASSERT_EQ(gr.num_compts(), 0);
/*
    ASSERT_EQ(gr.chis.num({1, 1}), 0);
    ASSERT_EQ(gr.chis.num({2, 2}), 0);
    ASSERT_EQ(gr.chis.num({3, 3}), 0);
    ASSERT_EQ(gr.chis.num({4, 4}), 0);
    ASSERT_EQ(gr.chis.num({1, 3}), 0);
    ASSERT_EQ(gr.chis.num({1, 4}), 0);
    ASSERT_EQ(gr.chis.num({3, 4}), 0);
*/
}


/// Tests add_single_chain_component(): creation of an free-standing linear chain
TEST_F(GraphTest, AddSingleChainComponent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests add_single_chain_component(): ",
                          "creation of an free-standing linear chain");

    constexpr std::array<size_t, 2> len {4, 1};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    G gr;
    const IntegrityCheck integrityCheck {gr};

    for (const auto u : len)
        gr.add_single_chain_component(u);

    if constexpr (profuse)
        gr.print_components();

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), len.size());
    ASSERT_EQ(gr.num_compts(), len.size());
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    for (ChIdG i {}; i<len.size(); ++i)
        ASSERT_EQ(gr.chain(i).length(), len[i()]);

    for (std::size_t i {}; i<len.size(); ++i) {
        const auto& c = gr.compt(CmpId{i});
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len[i]);
        ASSERT_EQ(c.ww[0], i);
        const auto& m = gr.chain(c.ww[0]);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.get_egl(j).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(j).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(j).i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn_11()[0], i);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        ASSERT_EQ(c.num_vertices(Deg1), 2);
        ASSERT_EQ(c.num_vertices(Deg2), len[i]>1 ? len[i]-1  : 0);
        ASSERT_EQ(c.num_vertices(Deg3), 0);
        ASSERT_EQ(c.num_vertices(Deg4), 0);
        ASSERT_EQ(c.num_vertices(), c.num_vertices(Deg1) +
                                    c.num_vertices(Deg2));
        c.template check<profuse>();
    }
}


/// Tests creation of multiple single-chain components
TEST_F(GraphTest, GenerateSingleChainComponents)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests generate_single_chain_components(): ",
                          "creation of multiple single-chain components");

    constexpr std::size_t num {3};
    constexpr std::size_t len {4};

    G gr;
    const IntegrityCheck integrityCheck {gr};

    gr.generate_single_chain_components(num, len);

    if constexpr (profuse)
        gr.print_components();

    ASSERT_EQ(gr.num_edges(), num*len);
    ASSERT_EQ(gr.num_chains(), num);
    ASSERT_EQ(gr.num_compts(), num);
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    ASSERT_EQ(gr.num_chains(), gr.num_compts());
    for (ChIdG i {}; i<gr.num_chains(); ++i)
        ASSERT_EQ(gr.chain(i).length(), len);

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len);
        c.template check<profuse>();
        ASSERT_EQ(c.ww[0](), i());
        const auto& m = gr.chain(c.ww[0]);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.get_egl(j).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(j).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(j).i, m.g[j].ind);
        }
        ASSERT_EQ(c.chis.cn_11()[0], i());
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        ASSERT_EQ(c.num_vertices(Deg1), 2);
        ASSERT_EQ(c.num_vertices(Deg2), len-1);
        ASSERT_EQ(c.num_vertices(Deg3), 0);
        ASSERT_EQ(c.num_vertices(Deg4), 0);
        ASSERT_EQ(c.num_vertices(), c.num_vertices(Deg1) +
                                    c.num_vertices(Deg2));
    }
}


/// Tests add_component(): addition of a component to the graph.
TEST_F(GraphTest, AddComponent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests add_component(): addition of a component to ",
                          "the graph");

    constexpr EgIdG egidIni0 {};
    EgIdG egidIni {egidIni0};
    constexpr std::size_t num {3};
    constexpr std::size_t len {4};

    G gr;
    const IntegrityCheck integrityCheck {gr};
    for (CmpId i {}; i<num; ++i) {
        G::Compt cmp{i, gr.chains()};
        cmp.create_chain(len, egidIni);
        gr.add_component(std::move(cmp));
    }

    if constexpr (profuse)
        gr.print_components();

    ASSERT_EQ(gr.num_edges(), num*len);
    ASSERT_EQ(gr.num_chains(), num);
    ASSERT_EQ(gr.num_compts(), num);
    // update() is called by add_component()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    for (ChIdG i {}; i<gr.num_chains(); ++i)
        ASSERT_EQ(gr.chain(i).length(), len);

    // IMPORTANT: correct connectivity of the chains is not ensured:
    for (const auto& m : gr.chains()) {
        ASSERT_EQ(m.ngs[End::A].num(), 0);
        ASSERT_EQ(m.ngs[End::A].num(), 0);
    }

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, num-1);
    ASSERT_EQ(c.num_edges(), len);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    for (const auto& w : c.ww) {
        ASSERT_EQ(w, num-1);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            const auto indc = j;
            ASSERT_EQ(m.g[j].indc, indc);
            ASSERT_EQ(c.get_egl(indc).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(indc).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(indc).i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn_11()[0], c.ww.back());  // was overwritten
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    ASSERT_EQ(c.num_vertices(Deg1), 2);
    ASSERT_EQ(c.num_vertices(Deg2), (len-1));
    ASSERT_EQ(c.num_vertices(Deg3), 0);
    ASSERT_EQ(c.num_vertices(Deg4), 0);
    ASSERT_EQ(c.num_vertices(), c.num_vertices(Deg1) +
                                c.num_vertices(Deg2));
    c.template check<profuse>();
    ASSERT_EQ(gr.num_vertices(Deg1), 2*gr.num_chains());
    ASSERT_EQ(gr.num_vertices(Deg2), num*(len - 1));
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Tests merging two connected components.
TEST_F(GraphTest, MergeComponents)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests merging two connected components");

    constexpr EgIdG egidIni0 {};
    EgIdG egidIni {egidIni0};
    constexpr std::size_t num {4};
    constexpr std::array<size_t, num> len {4, 3, 5, 1};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    G gr;
    const IntegrityCheck integrityCheck {gr};
    G::Chains& cn = gr.chains();
    for (CmpId i {}; i<num; ++i) {
        G::Compt cmp{i, cn};
        cmp.create_chain(len[i()], egidIni);
        gr.add_component(std::move(cmp));
    }

    constexpr CmpId cAcc1 {2};  // acceptor component ind
    constexpr CmpId cDon1 {1};  // donor component ind

    const auto nvAcc1 = gr.compt(cAcc1).num_vertices(Deg1);
    const auto nvDon1 = gr.compt(cDon1).num_vertices(Deg1);
    const auto nvAcc2 = gr.compt(cAcc1).num_vertices(Deg2);
    const auto nvDon2 = gr.compt(cDon1).num_vertices(Deg2);

    if constexpr (profuse) {
        gr.compt(cAcc1). print("Acc before");
        gr.compt(cDon1). print("Don before");
    }

    gr.merge_components(cAcc1, cDon1);

    if constexpr (profuse)
        gr.print_components("After merge");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), len.size());
    ASSERT_EQ(gr.num_compts(), num - 1);
    // update() is called by add_component()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    for (ChIdG i {}; i<gr.num_chains(); ++i)
        ASSERT_EQ(gr.chain(i).length(), len[i()]);

    // IMPORTANT: correct connectivity of the chains is not ensured:
    for (const auto& m : gr.chains()) {
        ASSERT_EQ(m.ngs[End::A].num(), 0);
        ASSERT_EQ(m.ngs[End::A].num(), 0);
    }

    const auto& cAcc = gr.compt(cAcc1);
    ASSERT_EQ(cAcc.ind, cAcc1);
    ASSERT_EQ(cAcc.num_edges(), len[cAcc1()] + len[cDon1()]);
    ASSERT_EQ(cAcc.num_chains(), 2);
    ASSERT_EQ(cAcc.get_egl().size(), cAcc.num_edges());
    for (EgIdC indc {}; const auto& w: cAcc.ww) {
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, cAcc.ind);
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, cAcc.ind);
            ASSERT_EQ(m.g[j].indc, indc++);
        }
    }
    ASSERT_EQ(cAcc.chis.num({2, 2}), 0);
    ASSERT_EQ(cAcc.chis.num({3, 3}), 0);
    ASSERT_EQ(cAcc.chis.num({4, 4}), 0);
    ASSERT_EQ(cAcc.chis.num({1, 3}), 0);
    ASSERT_EQ(cAcc.chis.num({1, 4}), 0);
    ASSERT_EQ(cAcc.chis.num({3, 4}), 0);
    ASSERT_EQ(cAcc.num_vertices(Deg1), nvAcc1 + nvDon1);
    ASSERT_EQ(cAcc.num_vertices(Deg2), nvAcc2 + nvDon2);
    ASSERT_EQ(cAcc.num_vertices(Deg3), 0);
    ASSERT_EQ(cAcc.num_vertices(Deg4), 0);
    ASSERT_EQ(cAcc.num_vertices(), nvAcc1 + nvDon1 + nvAcc2 + nvDon2);
    for (const auto& c : gr.compts()) {
        c.template check_chain_idc<profuse>();
        c.template check_edge_indc<profuse>();
        c.template check_egl<profuse>();
        c.template check_vertex_data<profuse>();
    }

    ASSERT_EQ(gr.num_vertices(Deg1), 2*gr.num_chains());
    ASSERT_EQ(gr.num_vertices(Deg2), lensum - num);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Tests rename_chain(from, to): chain indexes are updated so that the chain
/// indexed as source will acquire the identity of the target.
TEST_F(GraphTest, RenameChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rename_chain(from, to): chain indexes are updated so that ",
            "the chain indexed as source will acquire the identity of the target"
        );

    constexpr std::array len {4UL, 4UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChIdG u {0};
    constexpr ChIdG v {1};
    constexpr ChIdG w {2};
    constexpr ChIdG x {3};

    constexpr EgIdA a {2};

    constexpr ESlot uB {u, End::B};
    constexpr ESlot vB {v, End::B};
    constexpr ESlot wA {w, End::A};
    constexpr ESlot xB {x, End::B};

    G gr;
    IntegrityCheck integrityCheck {gr};
    // Low-level neighbour manipulation leaves the graph inconsistent on purpose.
    integrityCheck.skip();
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    constexpr std::size_t lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);
    ASSERT_EQ(gr.chain(x).length(), lenx);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(vB));
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(vB));
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 0);

    ASSERT_EQ(gr.num_vertices(Deg1), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    gr.chain(x).ngs[End::A].insert(ESlot{42, End::B});

    gr.rename_chain(v, x);    // (from, to)

    // graph structure is not affected, only the ngs of the chains invilved are:
    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), 0);              // <-----
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);
    ASSERT_EQ(gr.chain(x).length(), a);              // <-----

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(xB));   // <-----
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);    // <----- | ngs of 'from' |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(xB));   // <-----
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 2);    // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::B].contains(uB));   // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::B].contains(wA));   // <-----

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    ASSERT_EQ(gr.num_vertices(Deg1), 4);
    ASSERT_EQ(gr.num_vertices(Deg2), 4);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Tests copy_neigs(from, to): copy assigns connected slots of 'from' to 'to'
TEST_F(GraphTest, CopyNeigs)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests copy_neigs(from, to): copies assigning the connected slots of ",
            "'from' to 'to' and updates neigs of 'from' to become neigs of 'to"
        );

    constexpr std::array len {4UL, 4UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChIdG u {0};
    constexpr ChIdG v {1};
    constexpr ChIdG w {2};
    constexpr ChIdG x {3};

    constexpr EgIdA a {2};

    constexpr ESlot uB {u, End::B};
    constexpr ESlot vB {v, End::B};
    constexpr ESlot wA {w, End::A};
    constexpr ESlot xA {x, End::A};

    G gr;
    IntegrityCheck integrityCheck {gr};
    // Low-level neighbour manipulation leaves the graph inconsistent on purpose.
    integrityCheck.skip();
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    if constexpr (profuse)
        gr.print_chains("on merge");

    constexpr std::size_t lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    if constexpr (profuse)
        gr.print_chains("add new cmpt");

    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);
    ASSERT_EQ(gr.chain(x).length(), lenx);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(vB));
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(vB));
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 0);

    ASSERT_EQ(gr.num_vertices(Deg1), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    gr.chain(x).ngs[End::A].insert(ESlot{42, End::B});

    gr.copy_neigs(vB, xA);    // (from, to)

    // graph structure is not affected, only the ngs of the chains invilved are:
    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(xA));   // <-----
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);    // <----- | ngs of 'from' |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(xA));   // <-----
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 2);    // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::A].contains(uB));   // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::A].contains(wA));   // <-----
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 0);

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    ASSERT_EQ(gr.num_vertices(Deg1), gr.ind_last_chain() + 1);
    ASSERT_EQ(gr.num_vertices(Deg2), 7);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Tests remove_slot_from_neigs(s): removal of Slot s from chains connected to s
TEST_F(GraphTest, RemSlotFromNeigs)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests remove_slot_from_neigs(s): ",
            "removal of Slot s from neibs of chains connected to s "
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChIdG u {0};
    constexpr ChIdG v {1};
    constexpr ChIdG w {2};

    constexpr EgIdA a {2};

    constexpr ESlot uB {u, End::B};
    constexpr ESlot vB {v, End::B};
    constexpr ESlot wA {w, End::A};

    G gr;
    IntegrityCheck integrityCheck {gr};
    // Low-level neighbour manipulation leaves the graph inconsistent on purpose.
    integrityCheck.skip();
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), num + 1);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(vB));
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(vB));
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), gr.num_chains());
    ASSERT_EQ(gr.num_vertices(Deg2), 5);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    gr.remove_slot_from_neigs(uB);

    // graph structure is not affected, only the ngs are:
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), num + 1);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 1);    // <-----
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));   // <-----
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 1);    // <-----
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(vB));   // <-----
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);

    // update() is not called by remove_slot_from_neigs()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

   ASSERT_EQ(gr.num_vertices(Deg1), 4);
    ASSERT_EQ(gr.num_vertices(Deg2), 5);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Tests replace_slot_in_neigs(s): slots connected to 'nov' will be checked,
/// and if there is 'old', it will be replaced by 'nov'.
TEST_F(GraphTest, ReplaceSlotInNeigs)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests replace_slot_in_neigs(old, nov): slots connected to 'nov' ",
            "will be checked, and if there is 'old', it will be replaced by 'nov'"
        );

    constexpr std::array len {4UL, 4UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChIdG u {0};
    constexpr ChIdG v {1};
    constexpr ChIdG w {2};
    constexpr ChIdG x {3};

    constexpr EgIdA a {2};

    constexpr ESlot uB {u, End::B};
    constexpr ESlot vB {v, End::B};
    constexpr ESlot wA {w, End::A};
    constexpr ESlot xA {x, End::A};

    G gr;
    IntegrityCheck integrityCheck {gr};
    // Low-level neighbour manipulation leaves the graph inconsistent on purpose.
    integrityCheck.skip();
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    constexpr std::size_t lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);
    ASSERT_EQ(gr.chain(x).length(), lenx);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(vB));
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(vB));
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    // ngs of 'nov' = xA will be checked and if there is 'old', it will be replaced by 'nov'
    gr.chain(x).ngs[End::A].insert(uB);
    gr.chain(x).ngs[End::A].insert(wA);

    gr.replace_slot_in_neigs(vB, xA);    // (old, nov)

    // graph structure is not affected, only the ngs are:
    ASSERT_EQ(gr.num_edges(), lensum + lenx);
    ASSERT_EQ(gr.num_chains(), num + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.chain(u).length(), len[0]);
    ASSERT_EQ(gr.chain(v).length(), a);
    ASSERT_EQ(gr.chain(w).length(), len[1] - a);

    ASSERT_EQ(gr.chain(u).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(u).ngs[End::B].num(), 2);
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(xA));   // <-----
    ASSERT_TRUE(gr.chain(u).ngs[End::B].contains(wA));
    ASSERT_EQ(gr.chain(v).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[End::B].num(), 2);    // <----- | ngs of 'old' |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.chain(v).ngs[End::B].contains(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.chain(w).ngs[End::A].num(), 2);
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(uB));
    ASSERT_TRUE(gr.chain(w).ngs[End::A].contains(xA));   // <-----
    ASSERT_EQ(gr.chain(w).ngs[End::B].num(), 0);
    ASSERT_EQ(gr.chain(x).ngs[End::A].num(), 2);    // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::A].contains(uB));   // <-----
    ASSERT_TRUE(gr.chain(x).ngs[End::A].contains(wA));   // <-----
    ASSERT_EQ(gr.chain(x).ngs[End::B].num(), 0);

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.get_egl().size(), gr.num_edges());

    ASSERT_EQ(gr.num_vertices(Deg1), 4);
    ASSERT_EQ(gr.num_vertices(Deg2), 7);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);
}


/// Move construction of a graph.
TEST_F(GraphTest, MoveConstruction)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that a moved graph stays consistent and usable after ",
            "the moved-from graph is destroyed");

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};

    // The source lives on the heap: once it is destroyed, any reference
    // still pointing into it is a use after free.
    auto src = std::make_unique<G>();
    src->add_single_chain_component(4);
    src->add_single_chain_component(3);
    src->add_single_chain_component(2);
    VertexMerger<Deg1, Deg2, G> {*src}(ESlot{w1, End::A}, BSlot{w0, 2});

    const Snapshot before {*src};
    const auto adjacent = src->adj.are_adjacent(ESlot{w1, End::A}, w0, 1);

    G gr {std::move(*src)};
    src.reset();
    const IntegrityCheck integrityCheck {gr};

    EXPECT_TRUE(Snapshot {gr} == before);
    EXPECT_EQ(gr.adj.are_adjacent(ESlot{w1, End::A}, w0, 1), adjacent);

    // Transformations of the moved graph reach its components and chains:
    // close the unconnected 2-edge chain into a cycle.
    const auto numChains = gr.num_chains();
    VertexMerger<Deg1, Deg1, G> {gr}(ESlot{w2, End::A}, ESlot{w2, End::B});
    EXPECT_EQ(gr.num_chains(), numChains);
    EXPECT_TRUE(gr.chain(w2).is_unconnected_cycle());
}


/// Graphs held in a growing std::vector.
TEST_F(GraphTest, MoveOnReallocation)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that graphs stay consistent when a std::vector holding ",
            "them reallocates");

    static_assert(std::is_nothrow_move_constructible_v<G>);

    std::vector<G> grs;
    for (std::size_t k {}; k < 9; ++k) {  // several reallocations
        auto& gr = grs.emplace_back();
        gr.add_single_chain_component(2 + k);
        gr.add_single_chain_component(3);
    }

    for (std::size_t k {}; k < grs.size(); ++k) {
        auto& gr = grs[k];
        VertexMerger<Deg1, Deg1, G> {gr}(ESlot{ChIdG{0}, End::B},
                                         ESlot{ChIdG{1}, End::A});
        EXPECT_EQ(gr.num_compts(), 1);
        EXPECT_EQ(gr.num_edges(), 2 + k + 3);

        const auto res = structure::IntegralTests<G, profuse> {gr}.validate();
        EXPECT_TRUE(res.has_value()) << k << ": " << res.error().message;
    }
}


}  // namespace graffine::tests::graph
