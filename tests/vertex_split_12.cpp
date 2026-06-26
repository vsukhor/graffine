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

#include <numeric>  // accumulate

namespace graffine::tests::vertex_split {

/// Tests degree 3 vertex split by disconnecting a linear chain at end A
/// from a connected cycle.
TEST_F(VertexSplitTest, Divide_A_CC)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split by disconnecting",
                          " a linear chain at end A from a connected cycle");

    constexpr std::size_t len {10};

    constexpr ChIdG w {};
    constexpr ChIdG v {1};    // chain resulting from the vertex merger.
    constexpr auto e = eA;
    constexpr EgIdA a {6};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<Deg1, Deg2, G> merge {gr};

    merge(ESlot{w, e}, BSlot{w, a});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<Deg1, Deg2, G> divide {gr};

    divide(ESlot{v, eA});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 2);

    const auto& m = gr.chain(w);
    const auto& n = gr.chain(v);

    ASSERT_EQ(m.length(), a);
    ASSERT_EQ(n.length(), len - a);

    ASSERT_EQ(m.ngs[eA].num(), 1);
    ASSERT_TRUE(m.ngs[eA].contains(ESlot{w, eB}));
    ASSERT_EQ(m.ngs[eB].num(), 1);
    ASSERT_TRUE(m.ngs[eB].contains(ESlot{w, eA}));

    ASSERT_EQ(n.ngs[eA].num(), 0);
    ASSERT_EQ(n.ngs[eB].num(), 0);

    EgIdG c {};
    for (ChIdG j {}; j<gr.num_chains(); ++j) {
        const auto& x = gr.chain(j);
        ASSERT_EQ(x.idw, j);
        for (EgIdA i {}; i<x.length(); ++i) {
            ASSERT_EQ(x.g[i].w, x.idw);
            ASSERT_EQ(x.g[i].indw, i);
            ASSERT_EQ(x.g[i].ind, c++);
        }
    }

    ASSERT_EQ(gr.num_compts(), 2);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
        c.template check<profuse>();
        const auto& m = gr.chain(c.ww[0]);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, 0);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_TRUE(m.g[j].indc < c.num_edges());
        }
        if (m.is_unconnected_cycle()) {
            ASSERT_EQ(c.chis.num({1, 1}), 0);
            ASSERT_EQ(c.chis.cn_22()[0], c.ww[0]);
        }
        else {
            ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
            ASSERT_EQ(c.chis.num({2, 2}), 0);
        }
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}


/// Tests degree 3 vertex split by disconnecting a 13 chain at end B.
TEST_F(VertexSplitTest, Divide12_unconnected13B)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split ",
                          "by disconnecting a 13 chain at end B");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w2 at a:
    // produces another 3-w junction and a chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w1, eB});  // reverse: disconnect w1 at end B

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 5);
    ASSERT_EQ(gr.num_vertices(Deg2), 11);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 3);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 4);
    ASSERT_EQ(gr.chain(w3).length(), 6);

    const std::array ww0 {2, 3, 0};
    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 13);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, i);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic);
            ic++;
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
}

/// Tests degree 3 vertex split by disconnecting a 13 chain at end A.
TEST_F(VertexSplitTest, Divide12_unconnected13A)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split ",
                          "by disconnecting a 13 chain at end A");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces another 3-w junction and a chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w2, eA});  // reverse: disconnect w2 at end A

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 5);
    ASSERT_EQ(gr.num_vertices(Deg2), 11);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 4);
    ASSERT_EQ(gr.chain(w3).length(), 5);

    const std::array ww0 {0, 1, 3};
    const std::array idc0 {1, 2, 0};
    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 11);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
}

/// Tests degree 3 vertex split in a circular chain at end A.
TEST_F(VertexSplitTest, Divide12_circularA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split in a circular chain ",
                          "at end A");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces a nother 3-w junction and a linear chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    VertexMerger<1, 1, G> merge11 {gr};

    // to make a circular chain connected a both ends
    merge11(ESlot{w2, eB},
            ESlot{w4, eB});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w2, eA});  // disconnect w2 at end A

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg2), 12);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 9);

    const std::array ww0 {2, 0, 1};
    const std::array idc0 {0, 1, 2};
    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 15);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
}

/// Tests degree 3 vertex split in a circular chain at end B.
TEST_F(VertexSplitTest, Divide12_circularB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split in a circular chain ",
                          "at end B");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces a nother 3-w junction and a linear chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    VertexMerger<1, 1, G> merge11 {gr};

    // to make a circular chain connected a both ends
    merge11(ESlot{w2, eB},
            ESlot{w4, eB});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w2, eB});  // disconnect w2 at end B

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg2), 12);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 9);

    const std::array ww0 {2, 0, 1};
    const std::array idc0 {0, 1, 2};
    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 15);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
}

/// Tests degree 3 vertex split in a linear chain at end A, in a
/// component separable at this vertex.
TEST_F(VertexSplitTest, Divide12_linearA_separable)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split in a linear chain at ",
                          "end A, in a component separable at this vertex");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces a nother 3-w junction and a linear chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w3, eA});  // disconnect w2 at end A

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 5);
    ASSERT_EQ(gr.num_vertices(Deg2), 11);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 6);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 4);
    ASSERT_EQ(gr.chain(w3).length(), 2);

    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    const std::array ww0 {2, 3, 1};
    const std::array idc0 {0, 1, 2};
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 9);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
    constexpr CmpId c1 {1};
    const auto& d = gr.compt(c1);
    const std::array<size_t, 1> ww1 {0};
    const std::array<size_t, 1> idc1 {0};
    ASSERT_EQ(d.ind, c1);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), 6);
    d.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: d.ww) {
        ASSERT_EQ(w, ww1[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, d.ind);
        ASSERT_EQ(m.idc, idc1[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, d.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(d.chis.cn_11()[0], ww1[i]);
        ASSERT_EQ(d.chis.num({2, 2}), 0);
        ASSERT_EQ(d.chis.num({3, 3}), 0);
        ASSERT_EQ(d.chis.num({4, 4}), 0);
        ASSERT_EQ(d.chis.num({1, 3}), 0);
        ASSERT_EQ(d.chis.num({1, 4}), 0);
        ASSERT_EQ(d.chis.num({3, 4}), 0);
        i++;
    }
}


/// Tests degree 3 vertex split in a linear chain at end B, in a
/// component separable at this vertex.
TEST_F(VertexSplitTest, Divide12_linearB_separable)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split in a linear chain at ",
                          "end B, in a component separable at this vertex");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces a nother 3-w junction and a linear chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w3, eB});  // disconnect w2 at end B

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 5);
    ASSERT_EQ(gr.num_vertices(Deg2), 11);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 7);
    ASSERT_EQ(gr.chain(w3).length(), 2);

    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    const std::array ww0 {0, 1, 3};
    const std::array idc0 {1, 2, 0};
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 8);
    c.template check<profuse>();
    for (std::size_t i {}, ic {};
         const auto& w: c.ww) {

        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
    constexpr CmpId c1 {1};
    const auto& d = gr.compt(c1);
    const std::array ww1 {2};
    const std::array idc1 {0};
    ASSERT_EQ(d.ind, c1);
    ASSERT_EQ(d.num_chains(), 1);
    ASSERT_EQ(d.num_edges(), 7);
    d.template check<profuse>();
    for (std::size_t i {}, ic {};
         const auto& w: d.ww) {

        ASSERT_EQ(w, ww1[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, d.ind);
        ASSERT_EQ(m.idc, idc1[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, d.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(d.chis.cn_11()[0], ww1[i]);
        ASSERT_EQ(d.chis.num({2, 2}), 0);
        ASSERT_EQ(d.chis.num({3, 3}), 0);
        ASSERT_EQ(d.chis.num({4, 4}), 0);
        ASSERT_EQ(d.chis.num({1, 3}), 0);
        ASSERT_EQ(d.chis.num({1, 4}), 0);
        ASSERT_EQ(d.chis.num({3, 4}), 0);
        i++;
    }
}

/// Tests degree 3 vertex split in a linear chain at end A, in a
/// component separable at this vertex.
TEST_F(VertexSplitTest, Divide12_linearA_unseparable)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex split in a linear chain at ",
                          "end A, in a component unseparable at this vertex");

    constexpr std::array<size_t, 3> len {4, 7, 4};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};

    constexpr EgIdA a {2};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    // merge w0 and w1, joining w0 at its end B to w1 at a:
    // produces a 3-w junction and w3
    merge12(ESlot{w0, eB},
            BSlot{w1, a});
    // merge w2 and w3, joining w2 at its end A to w3 at a:
    // produces a nother 3-w junction and a linear chain
    merge12(ESlot{w2, eA},
            BSlot{w3, a});

    VertexMerger<1, 1, G> merge11 {gr};

    // to make a circular chain connected a both ends
    merge11(ESlot{w1, eA},
            ESlot{w4, eB});

    if constexpr (profuse)
        gr.print_components("MERGED");

    VertexSplit<1, 2, G> divide {gr};

    divide(ESlot{w3, eA});  // disconnect w2 at end A

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg2), 12);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 9);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 4);

    constexpr CmpId c0 {};
    const auto& c = gr.compt(c0);
    const std::array ww0 {2, 0, 1};
    const std::array idc0 {0, 1, 2};
    ASSERT_EQ(c.ind, c0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), 15);
    c.template check<profuse>();
    for (std::size_t i {}, ic {}; const auto& w: c.ww) {
        ASSERT_EQ(w, ww0[i]);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, idc0[i]);
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, ic++);
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 3);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        i++;
    }
}

}  // namespace graffine::tests::vertex_split
