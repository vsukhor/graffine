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

#include "vertex_split_common.h"
#include "graffine/transforms/vertex_split/to_11.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"
#include "graffine/transforms/vertex_split/to_22.h"

namespace graffine::tests::vertex_split {



/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting the two chains at ends A.
TEST_F(VertexSplitTest, To22_4Lines_AA_BB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "the two chains at ends A"
        );

    constexpr std::array<size_t, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w3 {2};
    constexpr ChIdG w5 {4};  // produced by merge22
    constexpr ChIdG w6 {5};  // produced by merge22

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {2};
    constexpr auto e = eA;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at internal vertices

    VertexMerger<Deg2, Deg2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains at ends A of w5  and w6

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<Deg2, Deg2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w5, e},
                                                   ESlot{w6, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w3);

    ASSERT_EQ(m.length(), a1 + len[w3()] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1()] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 4);

    for (CmpId ic {}; ic<gr.num_compts(); ++ic) {
        const auto& c = gr.compt(ic);
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
        c.template check<profuse>();
        const auto& u = gr.chain(c.ww[0]);
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting one chain at end A
/// and the other one at end B.
TEST_F(VertexSplitTest, To22_4Lines_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "one chain at end A and the other one at end B");

    constexpr std::array<size_t, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w3 {2};
    constexpr ChIdG w6 {5};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at internal vertices

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains at end B of w1 and end A of w6

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<Deg2, Deg2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w6, eA},
                                                   ESlot{w1, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w3);

    ASSERT_EQ(m.length(), a1 + len[w3()] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1()] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 4);

    for (CmpId ic {}; ic<gr.num_compts(); ++ic) {
        const auto& c = gr.compt(ic);
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
        c.template check<profuse>();
        const auto& u = gr.chain(c.ww[0]);
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 2+2 is done by disconnecting the two chains at ends B.
TEST_F(VertexSplitTest, To22_4Lines_BB_AA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains.\n",
            "Vertex split 4 -> 2+2 is done by disconnecting ",
            "the two chains at ends B"
        );

    constexpr std::array<size_t, 4> len {4, 4, 4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w3 {2};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {2};
    constexpr auto e = eB;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // connect two linear chains at internal vertices

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w3, a2});

    // disconnect two of the 4 resulting chains w1 and w2 at end B

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w1, e},
                                                   ESlot{w3, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w3);

    ASSERT_EQ(m.length(), a1 + len[w3()] - a2);
    ASSERT_EQ(n.length(), a2 + len[w1()] - a1);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 4);

    for (CmpId ic {}; ic<gr.num_compts(); ++ic) {
        const auto& c = gr.compt(ic);
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
        c.template check<profuse>();
        const auto& u = gr.chain(c.ww[0]);
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has ends A on both chains, the other pair has ends B.
TEST_F(VertexSplitTest, To22_2Lines_Cycle_AA_BB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "One pair has ends A on both chains, the other pair has ends B"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w3 {2};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {4};
    constexpr auto e = eA;

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at internal vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w1, e},
                                                   ESlot{w3, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 1);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    d.template check<profuse>();
    for (ChIdC i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.cn_11()[0], 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has ends B on both chains, the other pair has ends A.
TEST_F(VertexSplitTest, To22_2Lines_Cycle_BB_AA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "One pair has ends B on both chains, the other pair has ends A"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {4};
    constexpr auto e = eB;

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at internal vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w2, e},
                                                   ESlot{w1, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 1);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    d.template check<profuse>();
    for (ChIdC i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.cn_11()[0], 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end].
/// The first pair has end A on linear chane and end B on cycle.
TEST_F(VertexSplitTest, To22_2Lines_Cycle_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done between pairs [linear chain end, cycle end]\n",
            "The first pair has end A on linear chane and end B on cycle"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w3 {2};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at internal vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, false>(ESlot{w3, eA},
                                                   ESlot{w1, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 1);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), len);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len);
    d.template check<profuse>();
    for (ChIdC i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.cn_11()[0], 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done using linear chain slots.
TEST_F(VertexSplitTest, To22_2Lines_Cycle1_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a cycle\n",
            "Vertex split 4 -> 2+2 is done using linear chain slots"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at internal vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, true>(ESlot{w3, eA},
                                                  ESlot{w2, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w1, eA}));

    const auto& n = gr.chain(w2);
    ASSERT_EQ(n.length(), len - (a2 - a1));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 2);

    const auto& c = gr.compt(CmpId{1});
    ASSERT_EQ(c.ind, 1);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
    c.template check<profuse>();

    const auto& u = gr.chain(c.ww[0]);
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgIdA j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len - c.num_edges());
    d.template check<profuse>();
    for (ChIdC i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.num({1, 1}), 0);
    ASSERT_EQ(d.chis.cn_22()[0], d.ww[0]);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 2+2 is done using the cycle chain slots.
TEST_F(VertexSplitTest, To22_2Lines_Cycle2_AB_AB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains and a ",
            "cycle\nVertex split 4 -> 2+2 is done using the cycle chain slots"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {4};

    G gr;
    gr.add_single_chain_component(len);

    // connect two linear chains at internal vertices to make a cycle and
    // two linear chains

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};

    divide.template operator()<true, false, true>(ESlot{w1, eA},
                                                  ESlot{w1, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), a2 - a1);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w1, eA}));

    const auto& n = gr.chain(w2);
    ASSERT_EQ(n.length(), len - (a2 - a1));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 2);

    const auto& c = gr.compt(CmpId{1});
    ASSERT_EQ(c.ind, 1);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
    c.template check<profuse>();

    const auto& u = gr.chain(c.ww[0]);
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgIdA j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.cn_22()[0], c.ww[0]);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), len - c.num_edges());
    d.template check<profuse>();
    for (ChIdC i {}; i<d.num_chains(); i++) {
        const auto& u = d.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, i);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.cn_11()[0], d.ww[0]);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split 4 -> 2+2 of a vertex joining two cycle chains at boundaries.
TEST_F(VertexSplitTest, To22_2Cycles)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split 4 -> 2+2 of a vertex joining two cycle chains at ",
            "boundaries"
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w1, eA},
            ESlot{w1, eB});  // make cycle of the 1st chain

    merge11(ESlot{w2, eA},
            ESlot{w2, eB});  // make cycle of the 2nd chain

    VertexMerger<2, 2, G> merge22 {gr};

    merge22(w1, w2);  // join the two cycles on boundary vertices

    // split the component symmetrically

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<2, 2, G> divide {gr};  // disconnect the cycles

    divide.template operator()<true, true, true>(ESlot{w1, eA},
                                                 ESlot{w1, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w1);

    ASSERT_EQ(m.length(), len[w1()]);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w1, eA}));

    const auto& n = gr.chain(w2);
    ASSERT_EQ(n.length(), len[w2()]);

    ASSERT_EQ(n.ngs[eA].num(), 1);
    ASSERT_TRUE(n.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_EQ(n.ngs[eB].num(), 1);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w2, eA}));

    ASSERT_EQ(gr.num_compts(), 2);

    for (CmpId ic {}; ic<gr.num_compts(); ++ic) {
        const auto& c = gr.compt(ic);
        ASSERT_EQ(c.ind, ic);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
        c.template check<profuse>();
        const auto& u = gr.chain(c.ww[0]);
        ASSERT_EQ(u.c, c.ind);
        ASSERT_EQ(u.idc, 0);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, c.ind);
            ASSERT_TRUE(u.g[j].indc < c.num_edges());
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.cn_22()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}
#if 0
#endif
}  // namespace graffine::tests::vertex_split
