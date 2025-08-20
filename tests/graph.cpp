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
#include "graph-mutator/transforms/vertex_merger/from_12.h"


namespace graph_mutator::tests::graph {


using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
using Chain = G::Chain;
using Eg = Chain::Edge;
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

    ASSERT_EQ(gr.edgenum, 0);
    ASSERT_EQ(gr.chain_num(), 0);
    ASSERT_EQ(gr.cmpt_num(), 0);
    ASSERT_EQ(gr.cn.size(), 0);
    ASSERT_EQ(gr.glm.size(), 0);
    ASSERT_EQ(gr.gla.size(), 0);
    ASSERT_EQ(gr.ct.size(), 0);
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests add_single_chain_component(): creation of an free-standing linear chain
TEST_F(GraphTest, AddSingleChainComponent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests add_single_chain_component(): ",
                          "creation of an free-standing linear chain");

    constexpr std::array<EgId, 2> len {4, 1};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), len.size());
    ASSERT_EQ(gr.cmpt_num(), len.size());
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    for (szt i {}; i<len.size(); ++i)
        ASSERT_EQ(gr.cn[i].length(), len[i]);

    for (szt i {}; i<len.size(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len[i]);
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

    ASSERT_EQ(gr.chis.cn11.size(), gr.chain_num());
    for (ChId i {}; i<gr.chain_num(); ++i)
        ASSERT_EQ(gr.chis.cn11[i], i);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests creation of multiple single-chain components
TEST_F(GraphTest, GenerateSingleChainComponents)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests generate_single_chain_components(): ",
                          "creation of multiple single-chain components");

    constexpr ChId num {3};
    constexpr EgId len {4};

    G gr;

    gr.generate_single_chain_components(num, len);

    ASSERT_EQ(gr.edgenum, num*len);
    ASSERT_EQ(gr.chain_num(), num);
    ASSERT_EQ(gr.cmpt_num(), num);
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    ASSERT_EQ(gr.cn.size(), gr.cmpt_num());
    for (ChId i {}; i<gr.chain_num(); ++i)
        ASSERT_EQ(gr.cn[i].length(), len);

    for (CmpId i {}; i<gr.cmpt_num(); ++i) {
        const auto& c = gr.ct[i];
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len);
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

    ASSERT_EQ(gr.chis.cn11.size(), gr.chain_num());
    for (ChId i {}; i<gr.chain_num(); ++i)
        ASSERT_EQ(gr.chis.cn11[i], i);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
}


/// Tests add_component(): addition of a multi-chain component to the graph
TEST_F(GraphTest, AddComponent)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests add_component(): addition of a multi-chain ",
                          "component to the graph");

    constexpr ChId num {3};
    constexpr EgId len {4};

    G::Chains chains;

    for (ChId i {}; i<num; ++i)
        chains.emplace_back(len, i, i*len);

    G gr;

    gr.add_component(std::move(chains));

    ASSERT_EQ(gr.edgenum, num*len);
    ASSERT_EQ(gr.chain_num(), num);
    ASSERT_EQ(gr.cmpt_num(), 1);
    // update() is called by add_component()
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    for (ChId i {}; i<gr.chain_num(); ++i)
        ASSERT_EQ(gr.cn[i].length(), len);

    // IMPORTANT: correct connectivity of the chains is not ensured:
    for (const auto& m : gr.cn) {
        ASSERT_EQ(m.ngs[Ends::A].num(), 0);
        ASSERT_EQ(m.ngs[Ends::A].num(), 0);
    }

    const auto& c = gr.ct.back();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_edges(), gr.edgenum);
    ASSERT_EQ(c.num_chains(), gr.chain_num());
    ASSERT_EQ(c.gl.size(), c.num_edges());
    for (ChId i {}; const auto w: c.ww) {
        ASSERT_EQ(w, i++);
        const auto& m = gr.cn[w];
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, w);
        for (EgId j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            const auto indc = w*len + j;
            ASSERT_EQ(m.g[j].indc, indc);
            ASSERT_EQ(c.gl[indc].w, m.g[j].w);
            ASSERT_EQ(c.gl[indc].a, m.g[j].indw);
            ASSERT_EQ(c.gl[indc].i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.cn11, c.ww.back());  // was overwritten
    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
    ASSERT_EQ(c.chis.cn33.size(), 0);
    ASSERT_EQ(c.chis.cn44.size(), 0);
    ASSERT_EQ(c.chis.cn13.size(), 0);
    ASSERT_EQ(c.chis.cn14.size(), 0);
    ASSERT_EQ(c.chis.cn34.size(), 0);

    // populate_component_vectors() is called by update(), so:
    ASSERT_EQ(gr.chis.cn11.size(), gr.chain_num());
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), 0);
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), 2*gr.chain_num());
    ASSERT_EQ(gr.template num_vertices<2>(), num*(len - 1));
    ASSERT_EQ(gr.template num_vertices<3>(), 0);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
}


/// Tests rename_chain(from, to): chain indexes are updated so that the chain
/// indexed as source will acquire the identity of the target
TEST_F(GraphTest, RenameChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rename_chain(from, to): chain indexes are updated so that ",
            "the chain indexed as source will acquire the identity of the target."
        );

    constexpr std::array len {4UL, 4UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChId u {0};
    constexpr ChId v {1};
    constexpr ChId w {2};
    constexpr ChId x {3};

    constexpr EgId a {2};

    constexpr ESlot uB {u, Ends::B};
    constexpr ESlot vB {v, Ends::B};
    constexpr ESlot wA {w, Ends::A};
    constexpr ESlot xB {x, Ends::B};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    constexpr EgId lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);
    ASSERT_EQ(gr.cn[x].length(), lenx);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(vB));
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(vB));
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    gr.cn[x].ngs[Ends::A].insert(ESlot{42, Ends::B});

    gr.rename_chain(v, x);    // (from, to)

    // graph structure is not affected, only the ngs of the chains invilved are:
    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), 0);              // <-----
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);
    ASSERT_EQ(gr.cn[x].length(), a);              // <-----

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(xB));   // <-----
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);    // <----- | ngs of 'from' |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(xB));   // <-----
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 2);    // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::B].has(uB));   // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::B].has(wA));   // <-----

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    // populate_component_vectors() is not called by replace_slot_in_neigs():
    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
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

    constexpr ChId u {0};
    constexpr ChId v {1};
    constexpr ChId w {2};
    constexpr ChId x {3};

    constexpr EgId a {2};

    constexpr ESlot uB {u, Ends::B};
    constexpr ESlot vB {v, Ends::B};
    constexpr ESlot wA {w, Ends::A};
    constexpr ESlot xA {x, Ends::A};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    constexpr EgId lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);
    ASSERT_EQ(gr.cn[x].length(), lenx);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(vB));
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(vB));
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    gr.cn[x].ngs[Ends::A].insert(ESlot{42, Ends::B});

    gr.copy_neigs(vB, xA);    // (from, to)

    // graph structure is not affected, only the ngs of the chains invilved are:
    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(xA));   // <-----
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);    // <----- | ngs of 'from' |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(xA));   // <-----
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 2);    // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::A].has(uB));   // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::A].has(wA));   // <-----
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 0);

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    // populate_component_vectors() is not called by replace_slot_in_neigs():
    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
}


/// Tests remove_slot_from_neigs(s): removal of Slot s from chains connected to s
TEST_F(GraphTest, RemSlotFromNeigs)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests remove_slot_from_neigs(s): ",
            "removal of Slot s from neibs of chains connected to s "
        );

    constexpr std::array<EgId, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto num = len.size();

    constexpr ChId u {0};
    constexpr ChId v {1};
    constexpr ChId w {2};

    constexpr EgId a {2};

    constexpr ESlot uB {u, Ends::B};
    constexpr ESlot vB {v, Ends::B};
    constexpr ESlot wA {w, Ends::A};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), num + 1);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(vB));
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(vB));
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.chain_num());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.chain_num());
    ASSERT_EQ(gr.template num_vertices<2>(), 5);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    gr.remove_slot_from_neigs(uB);

    // graph structure is not affected, only the ngs are:
    ASSERT_EQ(gr.edgenum, lensum);
    ASSERT_EQ(gr.chain_num(), num + 1);
    ASSERT_EQ(gr.cmpt_num(), 1);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 1);    // <-----
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));   // <-----
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 1);    // <-----
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(vB));   // <-----
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);

    // update() is not called by remove_slot_from_neigs()
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    // populate_component_vectors() is not called by remove_slot_from_neigs():
    ASSERT_EQ(gr.chis.cn11.size(), 0);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.chain_num());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.chain_num());
    ASSERT_EQ(gr.template num_vertices<2>(), 5);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
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

    constexpr ChId u {0};
    constexpr ChId v {1};
    constexpr ChId w {2};
    constexpr ChId x {3};

    constexpr EgId a {2};

    constexpr ESlot uB {u, Ends::B};
    constexpr ESlot vB {v, Ends::B};
    constexpr ESlot wA {w, Ends::A};
    constexpr ESlot xA {x, Ends::A};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge u and v, joining u at its end B to v at a: produces a 3-w junction
    // and a third chain
    merge12(uB, BSlot{v, a});

    constexpr EgId lenx {3};

    gr.add_single_chain_component(lenx);       // add 'x'

    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);
    ASSERT_EQ(gr.cn[x].length(), lenx);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(vB));
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(vB));
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 0);

    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);

    // ngs of 'nov' = xA will be checked and if there is 'old', it will be replaced by 'nov'
    gr.cn[x].ngs[Ends::A].insert(uB);
    gr.cn[x].ngs[Ends::A].insert(wA);

    gr.replace_slot_in_neigs(vB, xA);    // (old, nov)

    // graph structure is not affected, only the ngs are:
    ASSERT_EQ(gr.edgenum, lensum + lenx);
    ASSERT_EQ(gr.chain_num(), num + 2);
    ASSERT_EQ(gr.cmpt_num(), 2);

    ASSERT_EQ(gr.cn[u].length(), len[0]);
    ASSERT_EQ(gr.cn[v].length(), a);
    ASSERT_EQ(gr.cn[w].length(), len[1] - a);

    ASSERT_EQ(gr.cn[u].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[u].ngs[Ends::B].num(), 2);
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(xA));   // <-----
    ASSERT_TRUE(gr.cn[u].ngs[Ends::B].has(wA));
    ASSERT_EQ(gr.cn[v].ngs[Ends::A].num(), 0);
    ASSERT_EQ(gr.cn[v].ngs[Ends::B].num(), 2);    // <----- | ngs of 'old' |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(uB));   // <----- | are not      |
    ASSERT_TRUE(gr.cn[v].ngs[Ends::B].has(wA));   // <----- | removed !!!! |
    ASSERT_EQ(gr.cn[w].ngs[Ends::A].num(), 2);
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(uB));
    ASSERT_TRUE(gr.cn[w].ngs[Ends::A].has(xA));   // <-----
    ASSERT_EQ(gr.cn[w].ngs[Ends::B].num(), 0);
    ASSERT_EQ(gr.cn[x].ngs[Ends::A].num(), 2);    // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::A].has(uB));   // <-----
    ASSERT_TRUE(gr.cn[x].ngs[Ends::A].has(wA));   // <-----
    ASSERT_EQ(gr.cn[x].ngs[Ends::B].num(), 0);

    // update() is not called by replace_slot_in_neigs()
    ASSERT_EQ(gr.glm.size(), gr.edgenum);
    ASSERT_EQ(gr.gla.size(), gr.edgenum);

    // populate_component_vectors() is not called by replace_slot_in_neigs():
    ASSERT_EQ(gr.chis.cn11.size(), 1);
    ASSERT_EQ(gr.chis.cn22.size(), 0);
    ASSERT_EQ(gr.chis.cn33.size(), 0);
    ASSERT_EQ(gr.chis.cn44.size(), 0);
    ASSERT_EQ(gr.chis.cn13.size(), gr.ind_last_chain());
    ASSERT_EQ(gr.chis.cn14.size(), 0);
    ASSERT_EQ(gr.chis.cn34.size(), 0);
    ASSERT_EQ(gr.template num_vertices<1>(), gr.ind_last_chain() + 2);
    ASSERT_EQ(gr.template num_vertices<2>(), len[0] - 1 + len[1] - 2 + lenx - 1);
    ASSERT_EQ(gr.template num_vertices<3>(), 1);
    ASSERT_EQ(gr.template num_vertices<4>(), 0);
}


}  // namespace graph_mutator::tests::graph
