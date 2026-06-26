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
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"
#include "graffine/transforms/vertex_split/to_11.h"

#include <numeric>  // accumulate

namespace graffine::tests::vertex_split {



TEST_F(VertexSplitTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests constructor of vertex split 1-2 functor");

    G gr;
    VertexSplit<1, 2, G> ct {gr};

    ASSERT_TRUE(gr.get_egl().empty());
    ASSERT_TRUE(gr.chains().empty());
    EXPECT_EQ(gr.num_chains(), 0);
    EXPECT_EQ(gr.num_compts(), 0);
    EXPECT_EQ(gr.num_edges(), 0);
    ASSERT_TRUE(gr.compts().empty());
}


/// Tests vertex split in a separate linear chain.
TEST_F(VertexSplitTest, Divide11_linA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex split in a separate linear chain");

    constexpr std::size_t len {4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    for (EgIdA a {1}; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        if constexpr (profuse)
            gr.print_components("BEFORE");

        VertexSplit<1, 1, G> divide {gr};
        divide(BSlot{w1, a});

        if constexpr (profuse)
            gr.print_components("AFTER");

        ASSERT_EQ(gr.num_edges(), len);
        ASSERT_EQ(gr.num_chains(), 2);
        ASSERT_EQ(gr.num_compts(), 2);

        ASSERT_EQ(gr.num_vertices(Deg1), 4);
        ASSERT_EQ(gr.num_vertices(Deg2), len - 2);
        ASSERT_EQ(gr.num_vertices(Deg3), 0);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);

        ASSERT_EQ(gr.chain(w1).length(), a);
        ASSERT_EQ(gr.chain(w2).length(), len - a);

        for (ChIdG c {}, j {}; j<gr.num_chains(); ++j) {
            const auto& m = gr.chain(j);
            ASSERT_EQ(m.c, j());
            ASSERT_EQ(m.idw, j);
            for (std::size_t i {}; i<m.length(); ++i) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, i);
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c());
                c++;
            }
            ASSERT_EQ(m.ngs[eA].num(), 0);
            ASSERT_EQ(m.ngs[eB].num(), 0);
        }

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
            for (EgIdA j {}; j<m.length(); ++j) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(m.g[j].indc, j());
            }
            ASSERT_EQ(c.get_egl().size(), c.num_edges());
            for (EgIdC j {}; j<m.length(); ++j) {
                ASSERT_EQ(c.get_egl(j).w, m.g[j()].w);
                ASSERT_EQ(c.get_egl(j).a, m.g[j()].indw);
                ASSERT_EQ(c.get_egl(j).i, m.g[j()].ind);
            }
            ASSERT_EQ(c.chis.cn_11()[0], i());
            ASSERT_EQ(c.chis.num({2, 2}), 0);
            ASSERT_EQ(c.chis.num({3, 3}), 0);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 0);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
        }
    }
}


/// Tests vertex split in a linear chain linking parts of a separable component.
TEST_F(VertexSplitTest, Divide11_linB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex split in a linear chain linking parts ",
                          "of a separable component");

    constexpr std::array<size_t, 4> len {4, 6, 3, 7};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};  // chain produced by the vertex merge below
    constexpr ChIdG w5 {5};  // chain produced by the vertex merge below
    constexpr std::array<std::array<ChIdG, 3>, 2> ww {{{w0, w1, w4},
                                                      {w2, w3, w5}}};
    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<1, 2, G> merge12 {gr};

    // merge w1 and w2, joining w1 at its end B to w2 at a2:
    // produces a 3-w junction and a third chain
    merge12(ESlot{w0, eB},
            BSlot{w1, a2});
    merge12(ESlot{w2, eB},
            BSlot{w3, a2});

    if constexpr (profuse)
        gr.print_components("12 & 12");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), len.size() + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 6);
    ASSERT_EQ(gr.num_vertices(Deg2), len[w0()]-1 + len[w1()]-2 +
                                             len[w2()]-1 + len[w3()]-2);
    ASSERT_EQ(gr.num_vertices(Deg3), 2);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    for (CmpId c {}; const auto& v: ww) {
        ASSERT_EQ(gr.chain(v[0]).length(), len[v[0]()]);
        ASSERT_EQ(gr.chain(v[1]).length(), a2);
        ASSERT_EQ(gr.chain(v[2]).length(), len[v[1]()] - a2());
        for (EgIdC indc {}; auto j: v) {
            const auto& m = gr.chain(j);
            ASSERT_EQ(m.c, c);
            ASSERT_EQ(m.idw, j);
            for (EgIdA i {}; i<m.length(); ++i) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, indc++);
                ASSERT_EQ(m.g[i].indw, i);
            }
        }
        ++c;
    }

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eB].num(), 0);

    merge11(ESlot{w4, eB},
            ESlot{w3, eA});
    if constexpr (profuse)
        gr.print_components("merge11_43");

    merge11(ESlot{w4, eB},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components("merge11_42");

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), lensum);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    EgIdC h {};
    for (const auto& w : c.ww) {
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        for (EgIdA j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
        }
    }
    c.template check<profuse>();
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 2);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 2);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w3, a1});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 4);
    ASSERT_EQ(gr.num_vertices(Deg2), 15);
    ASSERT_EQ(gr.num_vertices(Deg3), 2);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 7);
    ASSERT_EQ(gr.chain(w3).length(), 2);
    ASSERT_EQ(gr.chain(w4).length(), 4);

    for (ChIdG j {}; j<gr.num_chains(); ++j) {
        const auto& m = gr.chain(j);
        ASSERT_EQ(m.idw, j);
        for (EgIdA i {}; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].indw, i);
        }
    }
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 2);

    ASSERT_EQ(gr.compt(CmpId{}).num_chains(), 3);
    ASSERT_EQ(gr.compt(CmpId{1}).num_chains(), 2);
    ASSERT_EQ(gr.compt(CmpId{}).num_edges(), 9);
    ASSERT_EQ(gr.compt(CmpId{1}).num_edges(), lensum - 9);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        c.template check<profuse>();
        for (const auto& w : c.ww) {
            const auto& m = gr.chain(w);
            ASSERT_EQ(w, m.idw);
            ASSERT_EQ(m.c, c.ind);
            for (EgIdA j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
    }
    const auto& c0 = gr.compt(CmpId{});
    const auto& c1 = gr.compt(CmpId{1});
    ASSERT_EQ(c0.chis.num({1, 1}), 0);
    ASSERT_EQ(c0.chis.num({2, 2}), 0);
    ASSERT_EQ(c0.chis.num({3, 3}), 0);
    ASSERT_EQ(c0.chis.num({4, 4}), 0);
    ASSERT_EQ(c0.chis.num({1, 3}), 3);
    ASSERT_EQ(c0.chis.num({1, 4}), 0);
    ASSERT_EQ(c0.chis.num({3, 4}), 0);
    ASSERT_EQ(c1.chis.num({1, 1}), 0);
    ASSERT_EQ(c1.chis.num({2, 2}), 0);
    ASSERT_EQ(c1.chis.num({3, 3}), 1);
    ASSERT_EQ(c1.chis.num({4, 4}), 0);
    ASSERT_EQ(c1.chis.num({1, 3}), 1);
    ASSERT_EQ(c1.chis.num({1, 4}), 0);
    ASSERT_EQ(c1.chis.num({3, 4}), 0);
}


/// Tests vertex split in a linear chain linking parts of an unseparable
/// component.
TEST_F(VertexSplitTest, Divide11_linC)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex split in a linear chain linking parts ",
                          "of an unseparable component");

    constexpr std::array<size_t, 4> len {4, 6, 3, 7};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};
    constexpr ChIdG w3 {3};
    constexpr ChIdG w4 {4};  // chain produced by the vertex merge below
    constexpr ChIdG w5 {5};  // chain produced by the vertex merge below
    constexpr std::array<std::array<ChIdG, 3>, 2> ww {{{w0, w1, w4},
                                                      {w2, w3, w5}}};
    constexpr EgIdA a1 {2};
    constexpr EgIdA a2 {3};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 1, G> merge11 {gr};
    VertexMerger<1, 2, G> merge12 {gr};

    // merge w1 and w2, joining w1 at its end B to w2 at a2:
    // produces a 3-w junction and a third chain
    merge12(ESlot{w0, eB},
            BSlot{w1, a2});
    merge12(ESlot{w2, eB},
            BSlot{w3, a2});

    if constexpr (profuse)
        gr.print_components("12 & 12");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), len.size() + 2);
    ASSERT_EQ(gr.num_compts(), 2);

    ASSERT_EQ(gr.num_vertices(Deg1), 6);
    ASSERT_EQ(gr.num_vertices(Deg2), len[w0()]-1 + len[w1()]-2 +
                                             len[w2()]-1 + len[w3()]-2);
    ASSERT_EQ(gr.num_vertices(Deg3), 2);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    for (CmpId c {}; const auto& v: ww) {
        ASSERT_EQ(gr.chain(v[0]).length(), len[v[0]()]);
        ASSERT_EQ(gr.chain(v[1]).length(), a2());
        ASSERT_EQ(gr.chain(v[2]).length(), len[v[1]()] - a2());
        for (EgIdC indc {}; auto j: v) {
            const auto& m = gr.chain(j);
            ASSERT_EQ(m.c, c);
            ASSERT_EQ(m.idw, j);
            for (EgIdA i {}; i<m.length(); ++i) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, indc++);
                ASSERT_EQ(m.g[i].indw, i);
            }
        }
        ++c;
    }

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eB].num(), 0);

    merge11(ESlot{w4, eB},
            ESlot{w3, eA});
    if constexpr (profuse)
        gr.print_components("merge11_43");

    merge11(ESlot{w4, eB},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components("merge11_42");

    merge12(ESlot{w1, eA},
            BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("merge12_12");

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 5);
    ASSERT_EQ(c.num_edges(), lensum);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    for (const auto& w : c.ww) {
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
    }
    c.template check<profuse>();
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 4);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w3, a1});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 6);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 3);
    ASSERT_EQ(gr.num_vertices(Deg2), 14);
    ASSERT_EQ(gr.num_vertices(Deg3), 3);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w0).length(), 4);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    ASSERT_EQ(gr.chain(w3).length(), 2);
    ASSERT_EQ(gr.chain(w4).length(), 4);
    ASSERT_EQ(gr.chain(w5).length(), 4);

    for (ChIdG j {}; j<gr.num_chains(); ++j) {
        const auto& m = gr.chain(j);
        ASSERT_EQ(m.idw, j);
        for (EgIdA i {}; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].indw, i);
        }
    }
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w5).ngs[eB].num(), 2);

    const auto& c0 = gr.compt(CmpId{});
    ASSERT_EQ(c0.num_chains(), 6);
    ASSERT_EQ(c0.num_edges(), lensum);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        c.template check<profuse>();
        for (const auto& w : c.ww) {
            const auto& m = gr.chain(w);
            ASSERT_EQ(w, m.idw);
            ASSERT_EQ(m.c, c.ind);
            for (EgIdA j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
        }
    }
    ASSERT_EQ(c0.chis.num({1, 1}), 0);
    ASSERT_EQ(c0.chis.num({2, 2}), 0);
    ASSERT_EQ(c0.chis.num({3, 3}), 3);
    ASSERT_EQ(c0.chis.num({4, 4}), 0);
    ASSERT_EQ(c0.chis.num({1, 3}), 3);
    ASSERT_EQ(c0.chis.num({1, 4}), 0);
    ASSERT_EQ(c0.chis.num({3, 4}), 0);
}


/// Tests split of a boundary vertex in a separate cycle chain.
TEST_F(VertexSplitTest, Divide11_cyc)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of a boundary vertex in a separate cycle chain"
        );

    constexpr std::size_t len {4};

    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};
    // connect ends to form a cycle
    merge11(ESlot{w, eA},
            ESlot{w, eB});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w, 0});       // reverse by disconnecting at a = 0

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len - 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w).length(), len);

    const auto& m = gr.chain(w);

    ASSERT_EQ(m.c, w());
    for (std::size_t i {}; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].indw, i);
        ASSERT_EQ(m.g[i].indc, i);
        ASSERT_EQ(m.g[i].ind, i);
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

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
        for (std::size_t j {}; j<m.length(); ++j) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, j);
        }
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        for (EgIdA j {}; j<m.length(); ++j) {
            ASSERT_EQ(c.get_egl(j()).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(j()).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(j()).i, m.g[j].ind);
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


/// Tests split of an internal vertex in a separate cycle chain.
TEST_F(VertexSplitTest, Divide11_dic)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of an internal vertex in a separate cycle chain"
        );

    constexpr std::size_t len {4};
    constexpr EgIdA a {1};
    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 1, G> merge11 {gr};
    // connect ends to form a cycle
    merge11(ESlot{w, eA},
            ESlot{w, eB});

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexSplit<1, 1, G> divide {gr};
    divide(BSlot{w, a});       // disconnect at a

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), len);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len - 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w).length(), len);

    const auto& m = gr.chain(w);

    ASSERT_EQ(m.c, w());
    for (EgIdA i {}; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].c, m.c);
        ASSERT_EQ(m.g[i].w, m.idw);
        ASSERT_EQ(m.g[i].indw, i);
    }
    for (EgIdA i {}; i<m.length()-a; ++i) {
        ASSERT_EQ(m.g[i].indc(), a+i);
        ASSERT_EQ(m.g[i].ind(), a+i);
    }
    for (EgIdA i=m.length()-a; i<m.length(); ++i) {
        ASSERT_EQ(m.g[i].indc(), i - (m.length()-a));
        ASSERT_EQ(m.g[i].ind(), i - (m.length()-a));
    }

    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

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

        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        for (std::size_t j {}; j<m.length(); ++j)
            ASSERT_EQ(c.get_egl(j).w, m.g[j].w);

        ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
    }
}

}  // namespace graffine::tests::vertex_split
