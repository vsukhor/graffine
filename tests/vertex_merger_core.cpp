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

class VertexMergerCoreTest
    : public Test {

protected:

    using VMCore = trs::vertex_merger::Core<G>;

    /// Subclass to make protected members accessible for testing:
    class Core
        : public VMCore {

    public:
        using VMCore::antiparallel;
        using VMCore::parallel;
        using VMCore::to_cycle;
        using VMCore::cn;

        explicit Core(G& gr) : VMCore {gr, nullptr, "vm_core"} {}
    };
};

/// Tests constructor of vertex merger Core functor.
TEST_F(VertexMergerCoreTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests constructor of vertex merger Core functor");

    G gr;
    Core ct {gr};

    ASSERT_TRUE(gr.get_egl().empty());
    ASSERT_TRUE(gr.chains().empty());
    EXPECT_EQ(gr.num_chains(), 0);
    EXPECT_EQ(gr.num_compts(), 0);
    EXPECT_EQ(gr.num_edges(), 0);
    ASSERT_TRUE(gr.compts().empty());
}


/// Tests vertex merger antiparallel of two separate chains at ends A.
TEST_F(VertexMergerCoreTest, AntiparallelEndsA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests vertex merger antiparallel of two separate chains at ends A"
        );

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto E = End::A;

    for (ChIdG w1 {}; w1 < len.size(); ++w1)
        for (ChIdG w2 {}; w2 < len.size(); ++w2)
            if (w1 != w2) {

                if constexpr (profuse)
                    jot("==== Test case: w1 = ", w1, ",  w2 = ", w2, " ====\n");

                G gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgIdG ind {}; const auto& m: gr.chains())
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto ind1 = gr.chain(w1).end_edge(E).ind;
                const auto ind2 = gr.chain(w2).end_edge(E).ind;
                const auto g1 = gr.chain(w1).g;  // copy
                const auto g2 = gr.chain(w2).g;  // copy

                if constexpr (profuse) {

                    gr.compt(w1).print_chains("before:");
                    gr.compt(w1).print_vertices("before: w1=", w1);

                    gr.compt(w2).print_chains("before:");
                    gr.compt(w2).print_vertices("before: w2=", w2);
                    jot("");
                }

                const auto [v] = core.antiparallel(E, w1, w2);

                ASSERT_TRUE(v);

                // 'res' is the merged chain:
                const auto res = (w1 != len.size()-1) ? w1 : w2;

                ASSERT_EQ(gr.num_edges(), lensum);
                ASSERT_EQ(gr.num_chains(), len.size()-1);
                ASSERT_EQ(gr.num_compts(), len.size()-1);

                const auto& r = gr.chain(res);

                if constexpr (profuse) {

                    v->print("v");
                    gr.compt(res).print_chains("after:");
                    gr.compt(res).print_vertices("after: res=", res);
                    jot("");
                }

                ASSERT_EQ(r.c, v->get_c());
                ASSERT_EQ(v->deg(), Deg2);
                const auto indc1 = gr.edge(ind1).indc;
                const auto indc2 = gr.edge(ind2).indc;
                for (const auto& s : v->edges())
                    ASSERT_TRUE(indc1 == s.w || indc2 == s.w);

                for (ChIdG i {}; i < gr.num_chains(); ++i) {

                    const auto& m = gr.chain(i);

                    ASSERT_EQ(m.c, i());
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1()] + len[w2()]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the previously last chain.
                        ASSERT_EQ(m.length(), len[gr.num_chains()]);
                    else    // the rest is not affected
                        ASSERT_EQ(m.length(), len[i()]);

                    // check vertices

                    const auto& vertices = gr.compt(m.c).vertices();
                    for (const auto& eg : m.g) {
                        const auto& vids = eg.vids_inner();
                         for (const auto e : End::Ids) {
                            const auto vp = vertices.find(vids[e]);
                            ASSERT_TRUE(vp);
                        }
                    }
                }
                for (std::size_t c {}, i = len[w1()]-1; c < g1.size(); ++c, --i)
                    ASSERT_EQ(r.g[i].ind, g1[c].ind);

                for (std::size_t c {}, i = len[w1()]; i < r.length(); ++c, ++i)
                    ASSERT_EQ(r.g[i].ind, g2[c].ind);

                for (CmpId i {}; i<gr.num_compts(); ++i) {

                    const auto& c = gr.compt(i);
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
                    ASSERT_EQ(c.get_egl().size(), c.num_edges());
                    c.template check<profuse>();
                    EgIdC h {};
                    for (ChIdC k {}; const auto& w: c.ww) {

                        const auto& m = gr.chain(w);
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgIdA j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
                            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
                            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
                        }
                    }
                    ASSERT_EQ(c.chis.cn_11()[0], i());
                    ASSERT_EQ(c.chis.num({2, 2}), 0);
                    ASSERT_EQ(c.chis.num({3, 3}), 0);
                    ASSERT_EQ(c.chis.num({4, 4}), 0);
                    ASSERT_EQ(c.chis.num({1, 3}), 0);
                    ASSERT_EQ(c.chis.num({1, 4}), 0);
                    ASSERT_EQ(c.chis.num({3, 4}), 0);
                    ASSERT_EQ(c.num_vertices(Deg1), 2);
                    ASSERT_EQ(c.num_vertices(Deg2), c.num_edges()-1);
                    ASSERT_EQ(c.num_vertices(Deg3), 0);
                    ASSERT_EQ(c.num_vertices(Deg4), 0);
                    ASSERT_EQ(c.num_vertices(), c.num_vertices(Deg1) +
                                                c.num_vertices(Deg2));
                    for (const auto& v : c.vertices().get())
                        ASSERT_EQ(c.ind, v->get_c());
                }
            }
}


/// Tests vertex merger antiparallel of two separate chains at ends B.
TEST_F(VertexMergerCoreTest, AntiparallelEndsB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests vertex merger antiparallel of two separate chains at ends B"
        );

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);
    constexpr auto E = End::B;

    for (ChIdG w1=0; w1<len.size(); ++w1)
        for (ChIdG w2=0; w2<len.size(); ++w2)
            if (w1 != w2) {

                G gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgIdG ind {}; const auto& m: gr.chains())
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto ind1 = gr.chain(w1).end_edge(E).ind;
                const auto ind2 = gr.chain(w2).end_edge(E).ind;

                const auto g1 = gr.chain(w1).g;  // copy
                const auto g2 = gr.chain(w2).g;  // copy

                if constexpr (profuse) {

                    gr.compt(w1).print_chains("before:");
                    gr.compt(w1).print_vertices("before: w1=", w1);

                    gr.compt(w2).print_chains("before:");
                    gr.compt(w2).print_vertices("before: w2=", w2);
                }

                const auto [v] = core.antiparallel(E, w1, w2);

                ASSERT_TRUE(v);

                ASSERT_EQ(gr.num_edges(), lensum);
                ASSERT_EQ(gr.num_chains(), len.size()-1);
                ASSERT_EQ(gr.num_compts(), len.size()-1);

                // 'res' is the merged chain:
                const auto res = (w1 != len.size()-1) ? w1: w2;
                const auto& r = gr.chain(res);

                if constexpr (profuse) {

                    v->print("v");
                    gr.compt(res).print_chains("after:");
                    gr.compt(res).print_vertices("after: res=", res);
                }

                ASSERT_EQ(r.c, v->get_c());
                ASSERT_EQ(v->deg(), Deg2);
                const auto indc1 = gr.edge(ind1).indc;
                const auto indc2 = gr.edge(ind2).indc;
                for (const auto& s : v->edges())
                    ASSERT_TRUE(indc1 == s.w || indc2 == s.w);

                for (ChIdG i=0; i<gr.num_chains(); ++i) {

                    const auto& m = gr.chain(i);

                    ASSERT_EQ(m.c, i());
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1()]+len[w2()]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the previously last chain
                        ASSERT_EQ(m.length(), len[gr.num_chains()]);
                    else    // not affected
                        ASSERT_EQ(m.length(), len[i()]);

                    for (const auto& gg : m.g)
                        ASSERT_EQ(gg.c, m.c);
                }

                int c {};
                for (std::size_t i {}; i < len[w1()]; ++c, ++i)
                    ASSERT_EQ(gr.chain(res).g[i].ind, g1[c].ind);
                c = len[w2()] - 1;
                for (std::size_t i=len[w1()]; c >= 0; --c, ++i)
                    ASSERT_EQ(gr.chain(res).g[i].ind, g2[c].ind);

                for (const auto e : End::Ids)
                    ASSERT_EQ(gr.chain(res).ngs[e].num(), 0);

                for (CmpId i {}; i<gr.num_compts(); ++i) {

                    const auto& c = gr.compt(i);
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
                    ASSERT_EQ(c.get_egl().size(), c.num_edges());
                    c.template check<profuse>();
                    EgIdC h {};
                    for (ChIdC k {}; const auto& w: c.ww) {
                        const auto& m = gr.chain(w);
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgIdA j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
                            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
                            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
                        }
                    }
                    ASSERT_EQ(c.chis.cn_11()[0], i());
                    ASSERT_EQ(c.chis.num({2, 2}), 0);
                    ASSERT_EQ(c.chis.num({3, 3}), 0);
                    ASSERT_EQ(c.chis.num({4, 4}), 0);
                    ASSERT_EQ(c.chis.num({1, 3}), 0);
                    ASSERT_EQ(c.chis.num({1, 4}), 0);
                    ASSERT_EQ(c.chis.num({3, 4}), 0);
                    ASSERT_EQ(c.num_vertices(Deg1), 2);
                    ASSERT_EQ(c.num_vertices(Deg2), c.num_edges()-1);
                    ASSERT_EQ(c.num_vertices(Deg3), 0);
                    ASSERT_EQ(c.num_vertices(Deg4), 0);
                    ASSERT_EQ(c.num_vertices(), c.num_vertices(Deg1) +
                                                c.num_vertices(Deg2));
                    for (const auto& v : c.vertices().get())
                        ASSERT_EQ(c.ind, v->get_c());
                }
            }
}


/// Tests vertex merger parallel of two separate chains.
TEST_F(VertexMergerCoreTest, Parallel)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests vertex merger parallel of two separate chains"
        );

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    for (ChIdG w1 {}; w1 < len.size(); ++w1)
        for (ChIdG w2 {}; w2 < len.size(); ++w2)
            if (w1 != w2) {

                G gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgIdG ind {}; const auto& m: gr.chains())
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto g1 = gr.chain(w1).g;  // copy
                const auto g2 = gr.chain(w2).g;  // copy

                if constexpr (profuse)
                    gr.print_components("before");

                core.parallel(w1, w2);

                if constexpr (profuse)
                    gr.print_components("after");

                ASSERT_EQ(gr.num_edges(), lensum);
                ASSERT_EQ(gr.num_chains(), len.size()-1);
                ASSERT_EQ(gr.num_compts(), len.size()-1);

                // 'res' is the merged chain:
                const auto res = (w1 != len.size()-1) ? w1 : w2;

                for (ChIdG i {}; i<gr.num_chains(); ++i) {

                    const auto& m = gr.chain(i);

                    ASSERT_EQ(m.c, i());
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1()] + len[w2()]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the preSSERT_EQ(m.c, i);viously last chain
                        ASSERT_EQ(m.length(), len[gr.num_chains()]);
                    else    // not affected
                        ASSERT_EQ(m.length(), len[i()]);

                    for (const auto& gg : m.g)
                        ASSERT_EQ(gg.c, m.c);
                }

                for (std::size_t c {}, i {}; i < len[w2()]; ++c, ++i)
                    ASSERT_EQ(gr.chain(res).g[i].ind, g2[c].ind);

                for (std::size_t c {}, i = len[w2()]; i < gr.chain(res).length(); ++c, ++i)
                    ASSERT_EQ(gr.chain(res).g[i].ind, g1[c].ind);

                for (const auto e : End::Ids)
                    ASSERT_EQ(gr.chain(res).ngs[e].num(), 0);

                for (CmpId i {}; i<gr.num_compts(); ++i) {

                    const auto& c = gr.compt(i);
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
                    ASSERT_EQ(c.get_egl().size(), c.num_edges());
                    c.template check<profuse>();
                    EgIdC h {};
                    for (ChIdC k {}; const auto& w: c.ww) {
                        const auto& m = gr.chain(w);
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgIdA j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
                            ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
                            ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
                        }
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


/// Tests vertex merger to cycle of separate chain.
TEST_F(VertexMergerCoreTest, MergeToLoop)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger to cycle of separate chain");

    constexpr std::array<size_t, 6> len {4, 3, 6, 5, 2, 7};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    G gr;
    Core core {gr};

    const auto& mt = gr.chains();

    for (std::size_t w=0; w<len.size(); ++w)
        gr.add_single_chain_component(len[w]);

    for (EgIdG ind {}; const auto& m: mt)
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, ind++);

    for (std::size_t i {}; i<gr.num_chains(); ++i) {

        ASSERT_EQ(mt[i].idw, i);
        ASSERT_EQ(mt[i].c, i);
    }

    for (ChIdG w {}; w<len.size(); ++w) {

        const auto g = mt[w].g;  // copy

        core.to_cycle(w);

        if constexpr (profuse)
            gr.print_components("");

        ASSERT_EQ(gr.num_edges(), lensum);
        ASSERT_EQ(gr.num_chains(), len.size());
        ASSERT_EQ(gr.num_compts(), len.size());

        for (std::size_t i {}; i < gr.num_chains(); ++i) {
            ASSERT_EQ(mt[i].length(), len[i]);
            ASSERT_EQ(mt[i].idw, i);
            ASSERT_EQ(mt[i].c, i);
        }
        for (std::size_t i {}; i < len[w()]; ++i) {
            ASSERT_EQ(mt[w].g[i].ind, g[i].ind);
            ASSERT_EQ(mt[w].g[i].c, mt[w].c);
        }

        for (CmpId i {}; i<gr.num_compts(); ++i) {

            const auto& c = gr.compt(i);
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 1);
            ASSERT_EQ(c.num_edges(), len[i()]);
            ASSERT_EQ(c.num_edges(), gr.chain(c.ww[0]).length());
            ASSERT_EQ(c.get_egl().size(), c.num_edges());
            c.template check<profuse>();
            EgIdC h {};
            for (std::size_t k {}; const auto& w: c.ww) {
                const auto& m = gr.chain(w);
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc, k++);
                for (EgIdA j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.get_egl(h).w, m.g[j].w);
                    ASSERT_EQ(c.get_egl(h).a, m.g[j].indw);
                    ASSERT_EQ(c.get_egl(h).i, m.g[j].ind);
                }
            }
            if (w >= i()) {
                ASSERT_EQ(c.chis.num({1, 1}), 0);
                ASSERT_EQ(c.chis.cn_22()[0], i());
            }
            else {
                ASSERT_EQ(c.chis.cn_11()[0], i());
                ASSERT_EQ(c.chis.num({2, 2}), 0);
            }
            ASSERT_EQ(c.chis.num({3, 3}), 0);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 0);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
        }
        ASSERT_EQ(mt[w].ngs[End::A].num(), 1);
        ASSERT_EQ(mt[w].ngs[End::A].begin()->w, w);
        ASSERT_EQ(mt[w].ngs[End::A].begin()->e, End::B);

        ASSERT_EQ(mt[w].ngs[End::B].num(), 1);
        ASSERT_EQ(mt[w].ngs[End::B].begin()->w, w);
        ASSERT_EQ(mt[w].ngs[End::B].begin()->e, End::A);
    }
}


}  // namespace graffine::tests::vertex_merger
