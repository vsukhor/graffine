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
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end A
TEST_F(VertexSplitTest, To13_LLLL_LA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end A"
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);;

    constexpr ChIdG w1 {};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain resulting from merge22.
    constexpr ChIdG w4 {3};   // chain resulting from merge22.

    constexpr auto e = eA;
    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // connect w1 and w2 at chain internal nodes to produce a 4-way junction
    // and w3 and w4
    merge22(BSlot{w1, a1},
            BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    // disconnect w3 at its end A
    divide(ESlot{w3, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);
    const auto& p = gr.chain(w3);
    const auto& q = gr.chain(w4);

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), a2);
    ASSERT_EQ(p.length(), len[w1()] - a1);
    ASSERT_EQ(q.length(), len[w2()] - a2);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w2, eB}));
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w4, eA}));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w1, eB}));
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w4, eA}));

    ASSERT_EQ(p.ngs[eA].num(), 0);
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(q.ngs[eA].num(), 2);
    ASSERT_TRUE(q.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_TRUE(q.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(q.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 2);

    const auto& c = gr.compt(CmpId{1});
    ASSERT_EQ(c.ind, 1);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
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
    c.template check<profuse>();

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), lensum - c.num_edges());
    const ChIdC idc[] = {0, 2, 1};
    for (ChIdG i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, idc[i()]);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.num({1, 1}), 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 3);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
    d.template check<profuse>();
}


/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end B.
TEST_F(VertexSplitTest, To13_LLLL_LB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end B"
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);;

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain resulting from merge22.
    constexpr ChIdG w4 {3};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // connect w1 and w2 at chain internal nodes to produce a 4-way junction and
    // w3 and w4
    merge22(BSlot{w1, a1},
            BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    // disconnect w2 at its end B
    divide(ESlot{w2, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);
    const auto& p = gr.chain(w3);
    const auto& q = gr.chain(w4);

    ASSERT_EQ(m.length(), a1);
    ASSERT_EQ(n.length(), a2);
    ASSERT_EQ(p.length(), len[w1()] - a1);
    ASSERT_EQ(q.length(), len[w2()] - a2);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w3, eA}));
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w4, eA}));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w4, eA}));
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(q.ngs[eA].num(), 2);
    ASSERT_TRUE(q.ngs[eA].contains(ESlot{w3, eA}));
    ASSERT_TRUE(q.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(q.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 2);

    const auto& c = gr.compt(CmpId{1});
    ASSERT_EQ(c.ind, 1);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
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
    c.template check<profuse>();

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), lensum - c.num_edges());
    const ChIdC idc[] = {0, 1, 2};
    for (ChIdG i {}; i<d.num_chains(); i++) {
        const auto& u = gr.chain(d.ww[i()]);
        ASSERT_EQ(u.c, d.ind);
        ASSERT_EQ(u.idc, idc[i()]);
        for (EgIdA j {}; j<u.length(); ++j) {
            ASSERT_EQ(u.g[j].c, d.ind);
            ASSERT_TRUE(u.g[j].indc < d.num_edges());
        }
    }
    ASSERT_EQ(d.chis.num({1, 1}), 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 3);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
    d.template check<profuse>();
}

/// Tests split of a degree 4 vertex joining four linear chains.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end B.
TEST_F(VertexSplitTest, To13b_LLLL_LB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining four linear chains\n",
            "Vertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end B"
        );

    constexpr std::array<size_t, 4> len {4, 4, 4, 4};

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

    VertexSplit<1, 3, G> divide {gr};

    // disconnect w2 at its end B
    divide(ESlot{w1, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

}


/// Tests split of a degree 4 vertex joining two linear chains and one cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle chain at end A.
TEST_F(VertexSplitTest, To13_LLCC_CA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains ",
            "and one cycle\nVertex split 4 -> 1+3 is done by disconnecting ",
            "a cycle chain at end A"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};   // chain resulting from merge22
    constexpr ChIdG w3 {2};   // chain resulting from merge22

    constexpr auto e = eA;
    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    divide(ESlot{w1, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 3);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);
    const auto& p = gr.chain(w3);

    ASSERT_EQ(m.length(), len - a2 - a1);
    ASSERT_EQ(n.length(), a1);
    ASSERT_EQ(p.length(), a2);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w2, eB}));
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w3, eA}));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w1, eB}));
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w3, eA}));

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
    ASSERT_EQ(d.num_edges(), len);
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
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 3);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
    d.template check<profuse>();
}


/// Tests split of a degree 4 vertex joining two linear chains and a cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle chain at end B.
TEST_F(VertexSplitTest, To13_LLCC_CB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two linear chains ",
            "and a cycle \nVertex split 4 -> 1+3 is done by disconnecting ",
            "a cycle chain at end B"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};   // chain resulting from merge22.
    constexpr ChIdG w3 {2};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // loop w1 onto itself at chain internal sites to produce a loop and two
    // linear chains
    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    // disconnect the cycle chain at end B to produce linear chains connected
    // by a 3-way junction
    divide(ESlot{w1, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 3);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);
    const auto& p = gr.chain(w3);

    ASSERT_EQ(m.length(), len - a2 - a1);
    ASSERT_EQ(n.length(), a1);
    ASSERT_EQ(p.length(), a2);

    ASSERT_EQ(m.ngs[eA].num(), 2);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w3, eA}));
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w1, eA}));
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w3, eA}));

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w1, eA}));
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 3);
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
    ASSERT_EQ(d.chis.num({1, 1}), 0);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 3);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining linear 2 chains and 1 cycle.
/// Vertex split 4 -> 1+3 is done by disconnecting a linear chain at end B.
TEST_F(VertexSplitTest, To13_LLCC_LB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining 2 linear chains ",
            "and 1 cycle \nVertex split 4 -> 1+3 is done by disconnecting ",
            "a linear chain at end B"
        );

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};   // chain resulting from merge22.
    constexpr ChIdG w3 {2};   // chain resulting from merge22.

    constexpr auto e = eB;
    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    // loop w1 onto itself at chain internal sites to produce a loop and two
    // linear chains
    merge22(BSlot{w1, a1},
            BSlot{w1, a2});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    // disconnect the linear chain w1 at end B
    divide(ESlot{w2, e});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 3);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);
    const auto& p = gr.chain(w3);

    ASSERT_EQ(m.length(), len - a2 - a1);
    ASSERT_EQ(n.length(), a1);
    ASSERT_EQ(p.length(), a2);

    ASSERT_EQ(m.ngs[eA].num(), 2);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w3, eA}));
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w1, eA}));
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w3, eA}));

    ASSERT_EQ(p.ngs[eA].num(), 2);
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w1, eA}));
    ASSERT_TRUE(p.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_EQ(p.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    ASSERT_EQ(gr.num_compts(), 2);

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), 5);
    c.template check<profuse>();
    const auto& u = gr.chain(c.ww[0]);
    ASSERT_EQ(u.c, c.ind);
    ASSERT_EQ(u.idc, 0);
    for (EgIdA j {}; j<u.length(); ++j) {
        ASSERT_EQ(u.g[j].c, c.ind);
        ASSERT_TRUE(u.g[j].indc < c.num_edges());
    }
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 1);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    const auto& d = gr.compt(CmpId{1});
    ASSERT_EQ(d.ind, 1);
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
    ASSERT_EQ(d.chis.cn_11()[0], 1);
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 0);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 0);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two cycles.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle at end A.
TEST_F(VertexSplitTest, To13_CCCC_CA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two cycles\n",
            "Vertex split 4 -> 1+3 is done by disconnecting a cycle at end A"
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr auto e = eA;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge11(ESlot{w1, eA}, ESlot{w1, eB});  // make a cycle out of w1
    merge11(ESlot{w2, eA}, ESlot{w2, eB});  // make a cycle out of w2
    merge22(w1, w2);        // join the two cycles at their end vertices

    if constexpr (profuse)
        gr.print_components("BEFORE");

    divide(ESlot{w1, e});    // disconnect end A of w1

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);

    ASSERT_EQ(m.length(), len[w1()]);
    ASSERT_EQ(n.length(), len[w2()]);

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 2);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w2, eA}));
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w2, eB}));

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_TRUE(n.ngs[eA].contains(ESlot{w1, eB}));
    ASSERT_TRUE(n.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w1, eB}));
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w2, eA}));

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 2);
    ASSERT_EQ(d.num_edges(), lensum);
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
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 1);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 1);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


/// Tests split of a degree 4 vertex joining two cycles.
/// Vertex split 4 -> 1+3 is done by disconnecting a cycle at end B.
TEST_F(VertexSplitTest, To13_CCCC_CB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a degree 4 vertex joining two cycles\n",
            "Vertex split 4 -> 1+3 is done by disconnecting a cycle at end B"
        );

    constexpr std::array<size_t, 2> len {4, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr auto e = eB;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<2, 2, G> merge22 {gr};
    VertexSplit<1, 3, G> divide {gr};

    merge11(ESlot{w1, eA},
            ESlot{w1, eB});  // make a cycle out of w1
    merge11(ESlot{w2, eA},
            ESlot{w2, eB});  // make a cycle out of w2
    merge22(w1, w2);        // join the two cycles at their internal ends

    if constexpr (profuse)
        gr.print_components("BEFORE");

    divide(ESlot{w1, e});    // disconnect end B of w1

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w1);
    const auto& n = gr.chain(w2);

    ASSERT_EQ(m.length(), len[w1()]);
    ASSERT_EQ(n.length(), len[w2()]);

    ASSERT_EQ(m.ngs[eA].num(), 2);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w2, eA}));
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w2, eB}));
    ASSERT_EQ(m.ngs[eB].num(), 0);

    ASSERT_EQ(n.ngs[eA].num(), 2);
    ASSERT_TRUE(n.ngs[eA].contains(ESlot{w1, eA}));
    ASSERT_TRUE(n.ngs[eA].contains(ESlot{w2, eB}));

    ASSERT_EQ(n.ngs[eB].num(), 2);
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w2, eA}));
    ASSERT_TRUE(n.ngs[eB].contains(ESlot{w1, eA}));

    ASSERT_EQ(gr.num_compts(), 1);

    const auto& d = gr.compt(CmpId{});
    ASSERT_EQ(d.ind, 0);
    ASSERT_EQ(d.num_chains(), 2);
    ASSERT_EQ(d.num_edges(), lensum);
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
    ASSERT_EQ(d.chis.num({2, 2}), 0);
    ASSERT_EQ(d.chis.num({3, 3}), 1);
    ASSERT_EQ(d.chis.num({4, 4}), 0);
    ASSERT_EQ(d.chis.num({1, 3}), 1);
    ASSERT_EQ(d.chis.num({1, 4}), 0);
    ASSERT_EQ(d.chis.num({3, 4}), 0);
}


}  // namespace graffine::tests::vertex_split
