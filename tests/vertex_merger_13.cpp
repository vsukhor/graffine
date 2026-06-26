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


/// Tests vertex merger 13 at free end A of chains which belong to the
/// same component.
TEST_F(VertexMergerTest, vm13_eA_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 13 at free end A of chains ",
                          "which belong to the same component");

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

    VertexMerger<1, 2, G> merge12 {gr};
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
    c20.template check<profuse>();

    VertexMerger<1, 3, G> merge13 {gr};
    merge13(ESlot{w0, eA}, ESlot{w4, eA});

    if constexpr (profuse)
        gr.print_components("AFTER w0+w4");

    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 3);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).length(), 3);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        c.template check<profuse>();
    }
}


/// Tests vertex merger 13 at free end B of chains which belong to the
/// same component.
TEST_F(VertexMergerTest, vm13_eB_case_same_component)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 13 at free end B of chains ",
                          "which belong to the same component");

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

    VertexMerger<1, 2, G> merge12 {gr};
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
    c20.template check<profuse>();

    VertexMerger<1, 3, G> merge13 {gr};
    merge13(ESlot{w4, eB}, ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components("AFTER w4+w0");

    ASSERT_EQ(gr.num_compts(), 3);
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 5);
    ASSERT_EQ(gr.num_compts(), 3);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w4).length(), 3);
    for (CmpId i {}; i<gr.num_compts(); ++i) {
        const auto& c = gr.compt(i);
        c.template check<profuse>();
    }
}

/// Tests vertex merger 13 at free end A of chains which belong to distinct
/// components.
TEST_F(VertexMergerTest, vm13_eA_case_distinct_components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 13 at free end A of chains ",
                          "which belong to distinct components");

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

    VertexMerger<1, 2, G> merge12 {gr};
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
    c20.template check<profuse>();

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

    VertexMerger<1, 3, G> merge13 {gr};
    merge13(ESlot{w1, eA}, ESlot{w4, eA});

    if constexpr (profuse)
        gr.print_components("AFTER w1+w4");

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 6);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w5).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).length(), 3);
    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, gr.chain(w5).c);
    ASSERT_EQ(c.num_edges(), lensum);
    ASSERT_EQ(c.num_chains(), 6);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 2);
    ASSERT_EQ(c.chis.num({1, 4}), 3);
    ASSERT_EQ(c.chis.num({3, 4}), 1);
    c.template check<profuse>();
}


/// Tests vertex merger 13 at free end B of chains which belong to distinct
/// components.
TEST_F(VertexMergerTest, vm13_eB_case_distinct_components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 13 at free end B of chains ",
                          "which belong to distinct components");

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

    VertexMerger<1, 2, G> merge12 {gr};
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
    c20.template check<profuse>();

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

    VertexMerger<1, 3, G> merge13 {gr};
    merge13(ESlot{w5, eB}, ESlot{w4, eA});

    if constexpr (profuse)
        gr.print_components("AFTER w5+w4");

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(gr.num_edges(), lensum);
    ASSERT_EQ(gr.num_chains(), 6);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w3).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w3).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w4).ngs[eA].num(), 3);
    ASSERT_EQ(gr.chain(w4).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w5).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w5).ngs[eB].num(), 3);
    ASSERT_EQ(gr.chain(w4).length(), 3);
    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, gr.chain(w5).c);
    ASSERT_EQ(c.num_edges(), lensum);
    ASSERT_EQ(c.num_chains(), 6);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 2);
    ASSERT_EQ(c.chis.num({1, 4}), 3);
    ASSERT_EQ(c.chis.num({3, 4}), 1);
    c.template check<profuse>();
}


}  // namespace graffine::tests::vertex_merger
