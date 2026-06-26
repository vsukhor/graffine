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


TEST_F(VertexMergerTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests constructor of the vertex merger 1+2 functor");
    G gr;
    VertexMerger1<2, G> {gr};

    ASSERT_TRUE(gr.get_egl().empty());
    ASSERT_TRUE(gr.chains().empty());
    EXPECT_EQ(gr.num_chains(), 0);
    EXPECT_EQ(gr.num_compts(), 0);
    EXPECT_EQ(gr.num_edges(), 0);
    ASSERT_TRUE(gr.compts().empty());
}


/// Tests antiparallel vertex merger 11 at ends B of chains which belong to
/// distinct components.
TEST_F(VertexMergerTest, vm11a_eB_case_distinct_components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests antiparallel vertex merger 11 at ends B ",
                          "of chains which belong to distinct components");

    constexpr std::array<size_t, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len[i()]);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.cn_11()[0], i());
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }

    VertexMerger1<2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4

    if constexpr (profuse)
        gr.print_components("AFTER w2+w0");

    const auto& c20 = gr.compt(w4);
    ASSERT_EQ(c20.ind, gr.chain(w4).c);
    ASSERT_EQ(c20.num_edges(), len[2]+len[0]);
    ASSERT_EQ(c20.num_chains(), 3);
    ASSERT_EQ(c20.get_egl().size(), c20.num_edges());
    ASSERT_EQ(c20.chis.num({1, 1}), 0);
    ASSERT_EQ(c20.chis.num({2, 2}), 0);
    ASSERT_EQ(c20.chis.num({3, 3}), 0);
    ASSERT_EQ(c20.chis.num({4, 4}), 0);
    ASSERT_EQ(c20.chis.num({1, 3}), 3);
    ASSERT_EQ(c20.chis.num({1, 4}), 0);
    ASSERT_EQ(c20.chis.num({3, 4}), 0);
    for (CmpId i {}; i<gr.num_compts(); ++i)
        gr.compt(i).template check<profuse>();

    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    if constexpr (profuse)
        gr.print_components("AFTER w3+w1");

    const auto& c31 = gr.compt(w5);
    ASSERT_EQ(c31.ind, gr.chain(w5).c);
    ASSERT_EQ(c31.num_edges(), len[3]+len[1]);
    ASSERT_EQ(c31.num_chains(), 3);
    ASSERT_EQ(c31.get_egl().size(), c31.num_edges());
    ASSERT_EQ(c31.chis.num({1, 1}), 0);
    ASSERT_EQ(c31.chis.num({2, 2}), 0);
    ASSERT_EQ(c31.chis.num({3, 3}), 0);
    ASSERT_EQ(c31.chis.num({4, 4}), 0);
    ASSERT_EQ(c31.chis.num({1, 3}), 3);
    ASSERT_EQ(c31.chis.num({1, 4}), 0);
    ASSERT_EQ(c31.chis.num({3, 4}), 0);
    for (CmpId i {}; i<gr.num_compts(); ++i)
        gr.compt(i).template check<profuse>();

    // Original copies
    const auto g4 = gr.chain(w4).g;
    const auto g5 = gr.chain(w5).g;

    VertexMerger<1, 1, G> merge11 {gr};
    // w4(2> 3> 4>) + w5(7> 8> 9> 10>) ->  w4(2> 3> 4> 10< 9< 8< 7<)
    merge11(ESlot{w4, eB}, ESlot{w5, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w4).length(), g4.size() + g5.size());
    for (std::size_t i {}; i<g5.size(); ++i) {
        ASSERT_EQ(gr.chain(w4).g[i+g4.size()].ind, g5[g5.size()-1-i].ind);
        ASSERT_EQ(gr.chain(w4).g[i+g4.size()].orientation(), -g5[g5.size()-1-i].orientation());
    }
    for (std::size_t i {}; i<g4.size(); ++i) {
        ASSERT_EQ(gr.chain(w4).g[i].ind, g4[i].ind);
        ASSERT_EQ(gr.chain(w4).g[i].orientation(), g4[i].orientation());
    }
    constexpr auto clmass {lensum};
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 5);
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 4);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }
}


/// Tests antiparallel vertex merger 11 at ends B of chains which belong to the
/// same component.
TEST_F(VertexMergerTest, vm11a_eB_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests antiparallel vertex merger 11 at ends B ",
                          "of chains which belong to the same component");

    constexpr std::array<size_t, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 1);
        ASSERT_EQ(c.num_edges(), len[i()]);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.cn_11()[0], i());
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 0);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 0);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }

    VertexMerger1<2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4

    if constexpr (profuse)
        gr.print_components("AFTER w2+w0");

    const auto& c20 = gr.compt(w4);
    ASSERT_EQ(c20.ind, gr.chain(w4).c);
    ASSERT_EQ(c20.num_edges(), len[2]+len[0]);
    ASSERT_EQ(c20.num_chains(), 3);
    ASSERT_EQ(c20.get_egl().size(), c20.num_edges());
    ASSERT_EQ(c20.chis.num({1, 1}), 0);
    ASSERT_EQ(c20.chis.num({2, 2}), 0);
    ASSERT_EQ(c20.chis.num({3, 3}), 0);
    ASSERT_EQ(c20.chis.num({4, 4}), 0);
    ASSERT_EQ(c20.chis.num({1, 3}), 3);
    ASSERT_EQ(c20.chis.num({1, 4}), 0);
    ASSERT_EQ(c20.chis.num({3, 4}), 0);
    for (CmpId i {}; i<gr.num_compts(); ++i)
        gr.compt(i).template check<profuse>();


    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    if constexpr (profuse)
        gr.print_components("AFTER w3+w1");

    const auto& c31 = gr.compt(w5);
    ASSERT_EQ(c31.ind, gr.chain(w5).c);
    ASSERT_EQ(c31.num_edges(), len[3]+len[1]);
    ASSERT_EQ(c31.num_chains(), 3);
    ASSERT_EQ(c31.get_egl().size(), c31.num_edges());
    ASSERT_EQ(c31.chis.num({1, 1}), 0);
    ASSERT_EQ(c31.chis.num({2, 2}), 0);
    ASSERT_EQ(c31.chis.num({3, 3}), 0);
    ASSERT_EQ(c31.chis.num({4, 4}), 0);
    ASSERT_EQ(c31.chis.num({1, 3}), 3);
    ASSERT_EQ(c31.chis.num({1, 4}), 0);
    ASSERT_EQ(c31.chis.num({3, 4}), 0);
    for (CmpId i {}; i<gr.num_compts(); ++i)
        gr.compt(i).template check<profuse>();


    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    if constexpr (profuse)
        gr.print_components("AFTER w2+w3");

    // Original copies
    const auto g3 = gr.chain(w3).g;
    const auto g4 = gr.chain(w4).g;

    ASSERT_EQ(gr.num_compts(), 1);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_edges(), lensum);
        ASSERT_EQ(c.num_chains(), 5);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 1);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 4);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }
    // w4(2> 3> 4>) + w3(7> 8> 9> 10>) ->  w3(2> 3> 4> 10< 9< 8< 7<)
    merge11(ESlot{w4, eB}, ESlot{w3, eB});

    if constexpr (profuse)
        gr.print_components("AFTER w4+w3");

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w3).length(), g3.size() + g4.size());
    for (std::size_t i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.chain(w3).g[i+g4.size()].ind, g3[g3.size()-1-i].ind);
        ASSERT_EQ(gr.chain(w3).g[i+g4.size()].orientation(),
                  -g3[g3.size()-1-i].orientation());
    }
    for (std::size_t i {}; i<g4.size(); ++i) {
        ASSERT_EQ(gr.chain(w3).g[i].ind, g4[i].ind);
        ASSERT_EQ(gr.chain(w3).g[i].orientation(), g4[i].orientation());
    }
    constexpr auto clmass {lensum};
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 4);
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 2);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 2);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }
}


/// Tests parallel vertex merger 11 at w1 end A of chains which belong to the same
/// component.
TEST_F(VertexMergerTest, vm11p_eA_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests parallel vertex merger 11 at w1 end A ",
                          "of chains which belong to the same component");

    constexpr std::array<size_t, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger1<2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    if constexpr (profuse)
        gr.print_components("AFTER w3+w1");

    const auto& c31 = gr.compt(w5);
    ASSERT_EQ(c31.ind, gr.chain(w5).c);
    ASSERT_EQ(c31.num_edges(), len[3]+len[1]);
    ASSERT_EQ(c31.num_chains(), 3);
    ASSERT_EQ(c31.get_egl().size(), c31.num_edges());
    ASSERT_EQ(c31.chis.num({1, 1}), 0);
    ASSERT_EQ(c31.chis.num({2, 2}), 0);
    ASSERT_EQ(c31.chis.num({3, 3}), 0);
    ASSERT_EQ(c31.chis.num({4, 4}), 0);
    ASSERT_EQ(c31.chis.num({1, 3}), 3);
    ASSERT_EQ(c31.chis.num({1, 4}), 0);
    ASSERT_EQ(c31.chis.num({3, 4}), 0);
    c31.template check<profuse>();

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    if constexpr (profuse)
        gr.print_components("AFTER w2+w3");

    const auto& c23 = gr.compt(CmpId{});
    ASSERT_EQ(c23.ind, gr.chain(w4).c);
    ASSERT_EQ(c23.num_edges(), lensum);
    ASSERT_EQ(c23.num_chains(), 5);
    ASSERT_EQ(c23.get_egl().size(), c23.num_edges());
    ASSERT_EQ(c23.chis.num({1, 1}), 0);
    ASSERT_EQ(c23.chis.num({2, 2}), 0);
    ASSERT_EQ(c23.chis.num({3, 3}), 1);
    ASSERT_EQ(c23.chis.num({4, 4}), 0);
    ASSERT_EQ(c23.chis.num({1, 3}), 4);
    ASSERT_EQ(c23.chis.num({1, 4}), 0);
    ASSERT_EQ(c23.chis.num({3, 4}), 0);
    c23.template check<profuse>();

    // Original copies
    const auto g0 = gr.chain(w0).g;
    const auto g3 = gr.chain(w3).g;

    // Call core.parallel(w0, w3)
    // w3(7> 8> 9> 10>) + w0(0> 1>)->  w0(7> 8> 9> 10> 0> 1>)
    merge11(ESlot{w0, eA}, ESlot{w3, eB});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w0).length(), g0.size() + g3.size());
    for (std::size_t i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.chain(w0).g[i].ind, g3[i].ind);
        ASSERT_EQ(gr.chain(w0).g[i].orientation(), g3[i].orientation());
    }
    for (std::size_t i {}; i<g0.size(); ++i) {
        ASSERT_EQ(gr.chain(w0).g[i+g3.size()].ind, g0[i].ind);
        ASSERT_EQ(gr.chain(w0).g[i+g3.size()].orientation(), g0[i].orientation());
    }
    constexpr auto clmass {lensum};
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 4);
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 2);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 2);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }
}


/// Tests parallel vertex merger 11 at w1 end B of chains which belong to the
/// same component.
TEST_F(VertexMergerTest, vm11p_eB_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests parallel vertex merger 11 at w1 end B ",
                          "of chains which belong to the same component");

    constexpr std::array<size_t, 4> len {5, 6, 2, 2};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger1<2, G> merge12 {gr};
    merge12(ESlot{w2, eA}, BSlot{w0, 2});  // produces w4
    merge12(ESlot{w3, eA}, BSlot{w1, 2});  // produces w5

    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w2, eB}, ESlot{w3, eB});

    if constexpr (profuse)
        gr.print_components("AFTER w2+w3");

    const auto& c23 = gr.compt(CmpId{});
    ASSERT_EQ(c23.ind, gr.chain(w4).c);
    ASSERT_EQ(c23.num_edges(), lensum);
    ASSERT_EQ(c23.num_chains(), 5);
    ASSERT_EQ(c23.get_egl().size(), c23.num_edges());
    ASSERT_EQ(c23.chis.num({1, 1}), 0);
    ASSERT_EQ(c23.chis.num({2, 2}), 0);
    ASSERT_EQ(c23.chis.num({3, 3}), 1);
    ASSERT_EQ(c23.chis.num({4, 4}), 0);
    ASSERT_EQ(c23.chis.num({1, 3}), 4);
    ASSERT_EQ(c23.chis.num({1, 4}), 0);
    ASSERT_EQ(c23.chis.num({3, 4}), 0);
    c23.template check<profuse>();

    // Original copies
    const auto g0 = gr.chain(w0).g;
    const auto g3 = gr.chain(w3).g;

    // Call core.parallel(w0, w3)
    // w3(7> 8> 9> 10>) + w0(0> 1>)->  w0(7> 8> 9> 10> 0> 1>)
    merge11(ESlot{w3, eB}, ESlot{w0, eA});

    if constexpr (profuse)
        gr.print_components("AFTER");

    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w0).length(), g0.size() + g3.size());
    for (std::size_t i {}; i<g3.size(); ++i) {
        ASSERT_EQ(gr.chain(w0).g[i].ind, g3[i].ind);
        ASSERT_EQ(gr.chain(w0).g[i].orientation(), g3[i].orientation());
    }
    for (std::size_t i {}; i<g0.size(); ++i) {
        ASSERT_EQ(gr.chain(w0).g[i+g3.size()].ind, g0[i].ind);
        ASSERT_EQ(gr.chain(w0).g[i+g3.size()].orientation(), g0[i].orientation());
    }
    constexpr auto clmass {lensum};
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        ASSERT_EQ(c.ind, i);
        ASSERT_EQ(c.num_chains(), 4);
        ASSERT_EQ(c.num_edges(), clmass);
        ASSERT_EQ(c.get_egl().size(), c.num_edges());
        ASSERT_EQ(c.chis.num({1, 1}), 0);
        ASSERT_EQ(c.chis.num({2, 2}), 0);
        ASSERT_EQ(c.chis.num({3, 3}), 2);
        ASSERT_EQ(c.chis.num({4, 4}), 0);
        ASSERT_EQ(c.chis.num({1, 3}), 2);
        ASSERT_EQ(c.chis.num({1, 4}), 0);
        ASSERT_EQ(c.chis.num({3, 4}), 0);
        c.template check<profuse>();
    }
}

}  // namespace graffine::tests::vertex_merger
