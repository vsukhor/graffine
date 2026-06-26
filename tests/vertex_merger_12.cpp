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

#include "vertex_merger_common.h"

namespace graffine::tests::vertex_merger {


/// Tests vertex merger 12 case 1: a chain end A to an inner vertex of the
/// same chain.
TEST_F(VertexMergerTest, vm12_case_1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 1: ",
                          "a chain end A to an inner vertex of the same chain");

    constexpr std::size_t len {10};

    constexpr ChIdG w {0};
    constexpr ChIdG v {1};    // chain resulting from the vertex merger.

    constexpr auto e = End::A;
    constexpr auto oe = End::opp(e);

    for (EgIdA a=1; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        if constexpr (profuse)
            gr.print_components("BEFORE");

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w, e}, BSlot{w, a});

        if constexpr (profuse)
            gr.print_components("AFTER");

        const auto clmass = len;
        ASSERT_EQ(gr.num_edges(), clmass);
        ASSERT_EQ(gr.num_chains(), 2);
        ASSERT_EQ(gr.num_compts(), 1);

        ASSERT_EQ(gr.num_vertices(Deg1), 1);
        ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
        ASSERT_EQ(gr.num_vertices(Deg3), 1);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);

        for (ChIdG c {}, j {}; j<gr.num_chains(); ++j) {
            const auto& m = gr.chain(j);
            ASSERT_EQ(m.c, 0);
            ASSERT_EQ(m.idw, j);
            for (EgIdA i {}; i<m.length(); ++i, ++c) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, c());
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c());
            }
        }

        ASSERT_EQ(gr.chain(w).length(), a);
        ASSERT_EQ(gr.chain(v).length(), len - a);

        ChIdG u {};
        ChIdC idc {};
        EgIdC h {};
        for (CmpId i {}; i<gr.num_compts(); ++i) {
            const auto& c = gr.compt(i);
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 2);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.get_egl().size(), c.num_edges());
            for (const auto& w : c.ww) {
                ASSERT_EQ(w, u++);
                const auto& m = gr.chain(w);
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc(), idc++);
                for (std::size_t j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.get_egl(h()).w, m.g[j].w);
                    ASSERT_EQ(c.get_egl(h()).a, m.g[j].indw);
                    ASSERT_EQ(c.get_egl(h()).i, m.g[j].ind);
                }
            }
            ASSERT_EQ(c.chis.num({1, 1}), 0);
            ASSERT_EQ(c.chis.num({2, 2}), 0);
            ASSERT_EQ(c.chis.num({3, 3}), 1);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 1);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
            c.template check<profuse>();
        }


        const ESlot we {w, e};
        const ESlot wo {w, oe};
        const ESlot ve {v, e};

        const auto& nwe = gr.chain(w).ngs[e];
        ASSERT_EQ(nwe.num(), 2);
        ASSERT_TRUE(nwe.contains(wo));
        ASSERT_TRUE(nwe.contains(ve));

        const auto& nwo = gr.chain(w).ngs[oe];
        ASSERT_EQ(nwo.num(), 2);
        ASSERT_TRUE(nwo.contains(we));
        ASSERT_TRUE(nwo.contains(ve));

        ASSERT_EQ(gr.chain(v).ngs[oe].num(), 0);

        const auto& nve = gr.chain(v).ngs[e];
        ASSERT_EQ(nve.num(), 2);
        ASSERT_TRUE(nve.contains(we));
        ASSERT_TRUE(nve.contains(wo));
    }
}


/// Tests vertex merger 12 case 2: a chain end B to an inner vertex of the same chain.
TEST_F(VertexMergerTest, vm12_case_2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 2: ",
                          "a chain end B to an inner vertex of the same chain");

    constexpr std::size_t len {10};

    constexpr ChIdG w {0};
    constexpr ChIdG v {1};    // chain resulting from the vertex merger.

    constexpr auto e = End::B;
    constexpr auto oe = End::opp(e);

    for (EgIdA a=1; a<len; ++a) {

        G gr;
        gr.add_single_chain_component(len);

        if constexpr (profuse)
            gr.print_components("BEFORE");

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w, e}, BSlot{w, a});

        if constexpr (profuse)
            gr.print_components("AFTER");

        const auto clmass = len;
        ASSERT_EQ(gr.num_edges(), clmass);
        ASSERT_EQ(gr.num_chains(), 2);
        ASSERT_EQ(gr.num_compts(), 1);

        ASSERT_EQ(gr.num_vertices(Deg1), 1);
        ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
        ASSERT_EQ(gr.num_vertices(Deg3), 1);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);

        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, 0);
        ASSERT_EQ(m.idw, 0);
        for (EgIdA i {}; i<m.length(); ++i) {
            ASSERT_EQ(m.g[i].c, m.c);
            ASSERT_EQ(m.g[i].w, m.idw);
            ASSERT_EQ(m.g[i].indc, i()+a());
            ASSERT_EQ(m.g[i].indw, i);
            ASSERT_EQ(m.g[i].ind, i()+a());
        }

        ASSERT_EQ(gr.chain(v).length(), a);
        ASSERT_EQ(gr.chain(w).length(), len - a);

        for (CmpId i {}, k {}; i<gr.num_compts(); ++i) {
            const auto& c = gr.compt(i);
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 2);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.get_egl().size(), c.num_edges());
            for (const auto& w : c.ww) {
                ASSERT_EQ(w, k());
                const auto& m = gr.chain(w);
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc(), k++);
            }
            ASSERT_EQ(c.chis.num({1, 1}), 0);
            ASSERT_EQ(c.chis.num({2, 2}), 0);
            ASSERT_EQ(c.chis.num({3, 3}), 1);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 1);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
            c.template check<profuse>();
        }

        const ESlot we {w, e};
        const ESlot wo {w, oe};
        const ESlot ve {v, e};

        const auto& nwe = gr.chain(w).ngs[e];
        ASSERT_EQ(nwe.num(), 2);
        ASSERT_TRUE(nwe.contains(wo));
        ASSERT_TRUE(nwe.contains(ve));

        const auto& nwo = gr.chain(w).ngs[oe];
        ASSERT_EQ(nwo.num(), 2);
        ASSERT_TRUE(nwo.contains(we));
        ASSERT_TRUE(nwo.contains(ve));

        ASSERT_EQ(gr.chain(v).ngs[oe].num(), 0);

        const auto& nve = gr.chain(v).ngs[e];
        ASSERT_EQ(nve.num(), 2);
        ASSERT_TRUE(nve.contains(we));
        ASSERT_TRUE(nve.contains(wo));
    }
}


/// Tests vertex merger 12 case 3: a chain end to a separate linear chain.
TEST_F(VertexMergerTest, vm12_case_3_lin_lin)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 3: ",
                          "a chain end to a separate linear chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr auto e = End::B;
    constexpr auto oe = End::opp(e);

    constexpr auto clmass = len[w1()] + len[w2()];

    for (EgIdA a=1; a<len[w2()]; ++a) {

        G gr;

        for (const auto u : len)
            gr.add_single_chain_component(u);

        if constexpr (profuse)
            gr.print_components("BEFORE");

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w1, e}, BSlot{w2, a});

        if constexpr (profuse)
            gr.print_components("AFTER");

        ASSERT_EQ(gr.num_edges(), clmass);
        ASSERT_EQ(gr.num_chains(), 3);
        ASSERT_EQ(gr.num_compts(), 1);

        ASSERT_EQ(gr.num_vertices(Deg1), 3);
        ASSERT_EQ(gr.num_vertices(Deg2), clmass - 3);
        ASSERT_EQ(gr.num_vertices(Deg3), 1);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);

        for (ChIdG c {}, j {}; j<gr.num_chains(); ++j) {
            const auto& m = gr.chain(j);
            ASSERT_EQ(m.c, 0);
            ASSERT_EQ(m.idw, j);
            for (std::size_t i {}; i<m.length(); ++i, ++c) {
                ASSERT_EQ(m.g[i].c, m.c);
                ASSERT_EQ(m.g[i].w, m.idw);
                ASSERT_EQ(m.g[i].indc, c());
                ASSERT_EQ(m.g[i].indw, i);
                ASSERT_EQ(m.g[i].ind, c());
            }
        }

        for (CmpId i {}, k {}, h {}; i<gr.num_compts(); ++i) {
            const auto& c = gr.compt(i);
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 3);
            ASSERT_EQ(c.num_edges(), clmass);
            ASSERT_EQ(c.get_egl().size(), c.num_edges());
            for (const auto& w : c.ww) {
                ASSERT_EQ(w, k());
                const auto& m = gr.chain(w);
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc(), k++);
                for (std::size_t j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h());
                    ASSERT_EQ(c.get_egl(h()).w, m.g[j].w);
                    ASSERT_EQ(c.get_egl(h()).a, m.g[j].indw);
                    ASSERT_EQ(c.get_egl(h()).i, m.g[j].ind);
                }
            }
            ASSERT_EQ(c.chis.num({1, 1}), 0);
            ASSERT_EQ(c.chis.num({2, 2}), 0);
            ASSERT_EQ(c.chis.num({3, 3}), 0);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 3);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
            c.template check<profuse>();
        }

        const ESlot w1e {w1, e};
        const ESlot w2e {w2, e};
        const ESlot w2o {w2, oe};
        const ChIdG v {gr.num_chains()-1};
        const ESlot vo {v, oe};
        const ESlot ve {v, e};

        const auto& nw1e = gr.chain(w1).ngs[e];
        ASSERT_EQ(nw1e.num(), 2);
        ASSERT_TRUE(nw1e.contains(w2e));
        ASSERT_TRUE(nw1e.contains(vo));

        const auto& nw1o = gr.chain(w1).ngs[oe];
        ASSERT_EQ(nw1o.num(), 0);

        const auto& nw2e = gr.chain(w2).ngs[e];
        ASSERT_EQ(nw2e.num(), 2);
        ASSERT_TRUE(nw2e.contains(w1e));
        ASSERT_TRUE(nw2e.contains(vo));

        const auto& nw2o = gr.chain(w2).ngs[oe];
        ASSERT_EQ(nw2o.num(), 0);

        const auto& nve = gr.chain(v).ngs[e];
        ASSERT_EQ(nve.num(), 0);

        const auto& nvo = gr.chain(v).ngs[oe];
        ASSERT_EQ(nvo.num(), 2);
        ASSERT_TRUE(nvo.contains(w1e));
        ASSERT_TRUE(nvo.contains(w2e));
    }
}


/// Tests vertex merger 12 case 3 to a cycle chain.
TEST_F(VertexMergerTest, vm12_case_3_lin_cyc)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12 case 3: ",
                          "a chain end to a cycle chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr auto e = End::B;
    constexpr auto oe = End::opp(e);
    constexpr auto clmass = len[w1()] + len[w0()];

    for (EgIdA a=1; a<len[w0()]; ++a) {

        G gr;
        for (const auto u : len)
            gr.add_single_chain_component(u);

        VertexMergerCore fc {gr};
        fc.to_cycle(w0);

        if constexpr (profuse)
            gr.print_components("BEFORE");

        VertexMerger<1, 2, G> merge12 {gr};
        merge12(ESlot{w1, e}, BSlot{w0, a});

        if constexpr (profuse)
            gr.print_components("AFTER");

        ASSERT_EQ(gr.num_edges(), clmass);
        ASSERT_EQ(gr.num_chains(), 2);
        ASSERT_EQ(gr.num_compts(), 1);

        ASSERT_EQ(gr.num_vertices(Deg1), 1);
        ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
        ASSERT_EQ(gr.num_vertices(Deg3), 1);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);

        ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
        ASSERT_EQ(gr.chain(w0).length(), len[w0()]);

        for (EgIdG q {gr.chain(w0).length()};
             const auto& g : gr.chain(w1).g)
            ASSERT_EQ(g.ind, q++);

        const auto& c = gr.compt_last();
        ASSERT_EQ(c.ind, 0);
        ASSERT_EQ(c.num_chains(), len.size());
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        EgIdC h {};
        ChIdC k {};
        for (ChIdG i {c.num_chains()}; const auto& w: c.ww) {
            ASSERT_EQ(w, --i);
            const auto& m = gr.chain(w);
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, k++);
            for (std::size_t j {}; j<m.length(); ++j, ++h) {
                ASSERT_EQ(m.g[j].c, c.ind);
                ASSERT_EQ(m.g[j].indc, h);
                ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
                ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
                ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
            }
        }
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 1);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();

        const ESlot w0e {w0, e};
        const ESlot w0o {w0, oe};
        const ESlot w1e {w1, e};
        const ESlot w1o {w1, oe};

        ASSERT_EQ(gr.chain(w1).ngs[oe].num(), 0);

        const auto& nw1e = gr.chain(w1).ngs[e];
        ASSERT_EQ(nw1e.num(), 2);
        ASSERT_TRUE(nw1e.contains(w0e));
        ASSERT_TRUE(nw1e.contains(w0o));

        const auto& nw0e = gr.chain(w0).ngs[e];
        ASSERT_EQ(nw0e.num(), 2);
        ASSERT_TRUE(nw0e.contains(w1e));
        ASSERT_TRUE(nw0e.contains(w0o));

        const auto& nw0o = gr.chain(w0).ngs[oe];
        ASSERT_EQ(nw0o.num(), 2);
        ASSERT_TRUE(nw0o.contains(w1e));
        ASSERT_TRUE(nw0o.contains(w0e));
    }
}


/// Tests vertex merger 12C: end A of a linear chain to boundary of a cycle chain.
TEST_F(VertexMergerTest, vm12C_endA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12: end A of a linear chain ",
                          "to boundary of a cycle chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr auto e = End::A;
    constexpr auto oe = End::opp(e);

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};

    fc.to_cycle(w0);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{w1, e}, w0);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w0()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w0).length(), len[w0()]);

   EgIdGs v(gr.chain(w0).length());
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin()+0, v.end());
    for (std::size_t i=0; i<gr.chain(w0).length(); ++i)
        ASSERT_EQ(gr.chain(w0).g[i].ind, v[i]);
    for (EgIdG c {gr.chain(w0).length()};
         const auto& g : gr.chain(w1).g)
        ASSERT_EQ(g.ind, c++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    EgIdC h {};
    ChIdC k {};
    for (ChIdG i {c.num_chains()}; const auto& w: c.ww) {
        ASSERT_EQ(w, --i);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (std::size_t j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 1);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w0e {w0, e};
    const ESlot w0o {w0, oe};
    const ESlot w1e {w1, e};
    const ESlot w1o {w1, oe};

    const auto& nw1o = gr.chain(w1).ngs[oe];
    ASSERT_EQ(nw1o.num(), 0);

    const auto& nw1e = gr.chain(w1).ngs[e];
    ASSERT_EQ(nw1e.num(), 2);
    ASSERT_TRUE(nw1e.contains(w0e));
    ASSERT_TRUE(nw1e.contains(w0o));

    const auto& nw0e = gr.chain(w0).ngs[e];
    ASSERT_EQ(nw0e.num(), 2);
    ASSERT_TRUE(nw0e.contains(w1e));
    ASSERT_TRUE(nw0e.contains(w0o));

    const auto& nw0o = gr.chain(w0).ngs[oe];
    ASSERT_EQ(nw0o.num(), 2);
    ASSERT_TRUE(nw0o.contains(w1e));
    ASSERT_TRUE(nw0o.contains(w0e));
}


/// Tests vertex merger 12: end B of a linear chain to boundary of a cycle chain.
TEST_F(VertexMergerTest, vm12C_endB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 12: end B of a linear chain ",
                          "to boundary of a cycle chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {1};
    constexpr ChIdG w0 {0};
    constexpr auto e = End::B;
    constexpr auto oe = End::opp(e);

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};

    fc.to_cycle(w0);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{w1, e}, w0);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w0()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 1);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(gr.num_vertices(Deg4), 0);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w0).length(), len[w0()]);

   EgIdGs v(gr.chain(w0).length());
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin()+0, v.end());
    for (std::size_t i {}; i<gr.chain(w0).length(); ++i)
        ASSERT_EQ(gr.chain(w0).g[i].ind, v[i]);
    for (EgIdG c {gr.chain(w0).length()};
         const auto& g : gr.chain(w1).g)
        ASSERT_EQ(g.ind, c++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    EgIdC h {};
    ChIdC k {};
    for (ChIdG i {c.num_chains()};
         const auto& w: c.ww) {
        ASSERT_EQ(w, --i);
        const auto& m = gr.chain(w);
        ASSERT_EQ(m.c, c.ind);
        ASSERT_EQ(m.idc, k++);
        for (std::size_t j {}; j<m.length(); ++j, ++h) {
            ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(m.g[j].indc, h);
            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
        }
    }
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 1);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 1);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w0e {w0, e};
    const ESlot w0o {w0, oe};
    const ESlot w1e {w1, e};

    const auto& nw0e = gr.chain(w0).ngs[e];
    ASSERT_EQ(nw0e.num(), 2);
    ASSERT_TRUE(nw0e.contains(w0o));
    ASSERT_TRUE(nw0e.contains(w1e));

    const auto& nw0o = gr.chain(w0).ngs[oe];
    ASSERT_EQ(nw0o.num(), 2);
    ASSERT_TRUE(nw0o.contains(w0e));
    ASSERT_TRUE(nw0o.contains(w1e));

    const auto& nw1e = gr.chain(w1).ngs[e];
    ASSERT_EQ(nw1e.num(), 2);
    ASSERT_TRUE(nw1e.contains(w0e));
    ASSERT_TRUE(nw1e.contains(w0o));

    const auto& nw1o = gr.chain(w1).ngs[oe];
    ASSERT_EQ(nw1o.num(), 0);
}


}  // namespace graffine::tests::vertex_merger
