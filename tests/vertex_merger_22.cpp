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


/// Tests vertex merger 22 case 1: internal vertices of the same linear chain.
TEST_F(VertexMergerTest, vm22_case_1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 1: ",
                          "internal vertices of the same linear chain");

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w3, w2};

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;

    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w1, a2});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len;
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len - a1 - a2);
    ASSERT_EQ(gr.chain(w2).length(), a1);
    ASSERT_EQ(gr.chain(w3).length(), a2);

    EgIdG d {};
    for (const auto& g : gr.chain(w2).g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.chain(w1).g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.chain(w3).g)
        ASSERT_EQ(g.ind, d++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), vv.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 1);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 2);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};
    const ESlot w3A {w3, End::A};
    const ESlot w3B {w3, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2B));
    ASSERT_TRUE(n1A.contains(w3A));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2B));
    ASSERT_TRUE(n1B.contains(w3A));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 0);

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w3A));

    const auto& n3A = gr.ngs_at(w3A);
    ASSERT_EQ(n3A.num(), 3);
    ASSERT_TRUE(n3A.contains(w1A));
    ASSERT_TRUE(n3A.contains(w1B));
    ASSERT_TRUE(n3A.contains(w2B));

    const auto& n3B = gr.ngs_at(w3B);
    ASSERT_EQ(n3B.num(), 0);

}


/// Tests vertex merger 22 case 2: internal vertices of the same unconnected
/// cycle chain.
TEST_F(VertexMergerTest, vm22_case_2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 2: internal vertices ",
                          "of the same unconnected cycle chain");

    constexpr std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};   // chain created by the vertex merger

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;

    gr.add_single_chain_component(len);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w1, a2});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len;
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), a1 + len - a2);
    ASSERT_EQ(gr.chain(w2).length(), a2 - a1);

    EgIdG ind {};
    for (EgIdA i=0; i<a1; ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, ind++);
    for (EgIdA i=0; i<gr.chain(w2).length(); ++i)
        ASSERT_EQ(gr.chain(w2).g[i].ind, ind++);
    for (EgIdA i=a1; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, ind++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 2);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2A));
    ASSERT_TRUE(n1A.contains(w2B));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2A));
    ASSERT_TRUE(n1B.contains(w2B));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 3);
    ASSERT_TRUE(n2A.contains(w2B));
    ASSERT_TRUE(n2A.contains(w1A));
    ASSERT_TRUE(n2A.contains(w1B));

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w2A));
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
}


/// Tests vertex merger 22 case 3: internal vertices of distinct linear chains.
TEST_F(VertexMergerTest, vm22_case_3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 3: ",
                          "internal vertices of distinct linear chains");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {0};
    constexpr ChIdG w3 {2};   // chain created by the vertex merger
    constexpr ChIdG w4 {3};   // chain created by the vertex merger

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {2};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 4);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 4);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 4);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), a1);
    ASSERT_EQ(gr.chain(w2).length(), a2);
    ASSERT_EQ(gr.chain(w3).length(), len[w1()] - a1);
    ASSERT_EQ(gr.chain(w4).length(), len[w2()] - a2);

    EgIdG d {};
    for (const auto& g : gr.chain(w2).g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.chain(w4).g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.chain(w1).g)
        ASSERT_EQ(g.ind, d++);
    for (const auto& g : gr.chain(w3).g)
        ASSERT_EQ(g.ind, d++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 4);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 0);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 4);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};
    const ESlot w3A {w3, End::A};
    const ESlot w3B {w3, End::B};
    const ESlot w4A {w4, End::A};
    const ESlot w4B {w4, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 0);

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w2B));
    ASSERT_TRUE(n1B.contains(w3A));
    ASSERT_TRUE(n1B.contains(w4A));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 0);

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w3A));
    ASSERT_TRUE(n2B.contains(w4A));

    const auto& n3A = gr.ngs_at(w3A);
    ASSERT_EQ(n3A.num(), 3);
    ASSERT_TRUE(n3A.contains(w2B));
    ASSERT_TRUE(n3A.contains(w1B));
    ASSERT_TRUE(n3A.contains(w4A));

    const auto& n3B = gr.ngs_at(w3B);
    ASSERT_EQ(n3B.num(), 0);

    const auto& n4A = gr.ngs_at(w4A);
    ASSERT_EQ(n4A.num(), 3);
    ASSERT_TRUE(n4A.contains(w2B));
    ASSERT_TRUE(n4A.contains(w3A));
    ASSERT_TRUE(n4A.contains(w1B));

    const auto& n4B = gr.ngs_at(w4B);
    ASSERT_EQ(n4B.num(), 0);
}


/// Tests vertex merger 22 case 4: internal vertices of distinct cycle chains.
TEST_F(VertexMergerTest, vm22_case_4)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 4: ",
                          "internal vertices of distinct cycle chains");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);
    fc.to_cycle(w2);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w2).length(), len[w2()]);

    EgIdGs v(len[w1()]);
    std::iota(v.begin(), v.end(), 0);
    std::rotate(v.begin(), v.begin() + 1, v.end());
    for (std::size_t i {}; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, v[i]);

    std::iota(v.begin(), v.end(), len[w1()]);
    std::rotate(v.begin(), v.begin() + len[w2()] - 1, v.end());
    for (std::size_t i {}; i<gr.chain(w2).length(); ++i)
        ASSERT_EQ(gr.chain(w2).g[i].ind, v[i]);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 2);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2A));
    ASSERT_TRUE(n1A.contains(w2B));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2A));
    ASSERT_TRUE(n1B.contains(w2B));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 3);
    ASSERT_TRUE(n2A.contains(w1A));
    ASSERT_TRUE(n2A.contains(w1B));
    ASSERT_TRUE(n2A.contains(w2B));

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w2A));
}


/// Tests vertex merger 22 case 5: internal vertices of distinct
/// chains - a disconected cycle and a linear chain.
TEST_F(VertexMergerTest, vm22_case_5)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 5: internal vertices ",
                          "of distinct chains - a disconected cycle ",
                          "and a linear chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain created by the vertex merger

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, BSlot{w2, a2});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w2).length(), a2);
    ASSERT_EQ(gr.chain(w3).length(), len[w2()] - a2);

    EgIdGs v1(len[w1()]);
    std::iota(v1.begin(), v1.end(), 0);
    std::rotate(v1.begin(), v1.begin() + 1, v1.end());
    for (std::size_t i {}; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, v1[i]);

    EgIdGs v2(len[w2()]);
    std::iota(v2.begin(), v2.end(), len[w1()]);
    for (std::size_t i {}; i<gr.chain(w2).length(); ++i)
        ASSERT_EQ(gr.chain(w2).g[i].ind, v2[i]);

    ASSERT_EQ(gr.chain(w3).g[0].ind, 7);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 1);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 2);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};
    const ESlot w3A {w3, End::A};
    const ESlot w3B {w3, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2B));
    ASSERT_TRUE(n1A.contains(w3A));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2B));
    ASSERT_TRUE(n1B.contains(w3A));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 0);

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w3A));

    const auto& n3A = gr.ngs_at(w3A);
    ASSERT_EQ(n3A.num(), 3);
    ASSERT_TRUE(n3A.contains(w1B));
    ASSERT_TRUE(n3A.contains(w2B));
    ASSERT_TRUE(n3A.contains(w1A));

    const auto& n3B = gr.ngs_at(w3B);
    ASSERT_EQ(n3B.num(), 0);
}

/// Tests vertex merger 22 case 6: internal vertices of distinct
/// chains - a linear chain and an unconected cycle.
TEST_F(VertexMergerTest, vm22_case_6)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 6: internal vertices ",
                          "of distinct chains - a linear chain and ",
                          "an unconected cycle");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain created by the vertex merger

    constexpr EgIdA a1 {1};
    constexpr EgIdA a2 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w2, a2}, BSlot{w1, a1});

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w2).length(), a2);
    ASSERT_EQ(gr.chain(w3).length(), len[w2()] - a2);

    EgIdGs v1(len[w1()]);
    std::iota(v1.begin(), v1.end(), 0);
    std::rotate(v1.begin(), v1.begin() + 1, v1.end());
    for (std::size_t i {}; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, v1[i]);

    EgIdGs v2(len[w2()]);
    std::iota(v2.begin(), v2.end(), len[w1()]);
    for (std::size_t i {}; i<gr.chain(w2).length(); ++i)
        ASSERT_EQ(gr.chain(w2).g[i].ind, v2[i]);

    ASSERT_EQ(gr.chain(w3).g[0].ind, 7);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 1);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 2);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};
    const ESlot w3A {w3, End::A};
    const ESlot w3B {w3, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2B));
    ASSERT_TRUE(n1A.contains(w3A));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2B));
    ASSERT_TRUE(n1B.contains(w3A));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 0);

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w3A));

    const auto& n3A = gr.ngs_at(w3A);
    ASSERT_EQ(n3A.num(), 3);
    ASSERT_TRUE(n3A.contains(w1B));
    ASSERT_TRUE(n3A.contains(w2B));
    ASSERT_TRUE(n3A.contains(w1A));

    const auto& n3B = gr.ngs_at(w3B);
    ASSERT_EQ(n3B.num(), 0);
}


/// Tests vertex merger 22 case 7: an unconnected cycle chain with itself:
/// boundary vertex with an internal one.
TEST_F(VertexMergerTest, vm22_case_7)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 7: ",
                          "an unconnected cycle chain with itself: ",
                          "boundary vertex with an internal one");

    std::size_t len {6};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr std::array vv {w1, w2};

    constexpr EgIdA a {2};

    G gr;
    gr.add_single_chain_component(len);

    VertexMergerCore fc {gr};
    VertexMerger<2, 2, G> merge_22 {gr};

    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    merge_22(BSlot{w1, a}, w1);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len;
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), a);
    ASSERT_EQ(gr.chain(w2).length(), len - a);

    for (EgIdG c {};
         const auto& m: gr.chains())
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, c++);

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());
    std::size_t k {};
    for (EgIdC h {};
         const auto& w: c.ww) {
        ASSERT_EQ(w, vv[k]);
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
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 2);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2A));
    ASSERT_TRUE(n1A.contains(w2B));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2A));
    ASSERT_TRUE(n1B.contains(w2B));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 3);
    ASSERT_TRUE(n2A.contains(w1A));
    ASSERT_TRUE(n2A.contains(w1B));
    ASSERT_TRUE(n2A.contains(w2B));

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w2A));
}


/// Tests vertex merger 22 case 8: internal vertex of a linear chain to a
/// boundary vertex of an unsconected cycle.
TEST_F(VertexMergerTest, vm22_case_8)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger 22 case 8: internal vertex of ",
                          "a linear chain to a boundary vertex of ",
                          "an unconected cycle");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {0};
    constexpr ChIdG w2 {1};
    constexpr ChIdG w3 {2};   // chain created by the vertex merger
    constexpr std::array vv {w1, w3, w2};

    constexpr EgIdA a1 {3};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    fc.to_cycle(w2);   // make w2 an unconnected cycle chain

    if constexpr (profuse)
        gr.print_components("BEFORE");

    VertexMerger<2, 2, G> merge22 {gr};
    merge22(BSlot{w1, a1}, w2);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 3);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), a1);
    ASSERT_EQ(gr.chain(w2).length(), len[w2()]);
    ASSERT_EQ(gr.chain(w3).length(), len[w1()] - a1);

    EgIdGs v1(a1());
    std::iota(v1.begin(), v1.end(), 0);
    for (std::size_t i {}; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, v1[i]);

    EgIdGs v2(len[w2()]);
    std::iota(v2.begin(), v2.end(), len[w1()]);
    for (std::size_t i {}; i<gr.chain(w2).length(); ++i)
        ASSERT_EQ(gr.chain(w2).g[i].ind, v2[i]);

    EgIdGs v3(len[w1()] - a1());
    std::iota(v3.begin(), v3.end(), a1());
    for (std::size_t i {}; i<gr.chain(w3).length(); ++i)
        ASSERT_EQ(gr.chain(w3).g[i].ind, v3[i]);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 3);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    std::size_t k {};
    for (EgIdC h {};
         const auto& w: c.ww) {
        ASSERT_EQ(w, vv[k]);
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
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 1);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 2);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};
    const ESlot w3A {w3, End::A};
    const ESlot w3B {w3, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 0);

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w3A));
    ASSERT_TRUE(n1B.contains(w2A));
    ASSERT_TRUE(n1B.contains(w2B));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 3);
    ASSERT_TRUE(n2A.contains(w3A));
    ASSERT_TRUE(n2A.contains(w1B));
    ASSERT_TRUE(n2A.contains(w2B));

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w3A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w2A));

    const auto& n3A = gr.ngs_at(w3A);
    ASSERT_EQ(n3A.num(), 3);
    ASSERT_TRUE(n3A.contains(w1B));
    ASSERT_TRUE(n3A.contains(w2A));
    ASSERT_TRUE(n3A.contains(w2B));

    const auto& n3B = gr.ngs_at(w3B);
    ASSERT_EQ(n3B.num(), 0);
}


/// Tests vm22 case 9: vertex merger of a cycle chain at an internal vertex to
/// the boundary vertex of another cycle chain.
TEST_F(VertexMergerTest, vm22_case_9)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vm22 case 9: vertex merger of a cycle ",
                          "chain at an internal vertex to the boundary vertex ",
                          "of another cycle chain");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {0};

    constexpr EgIdA a {1};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<2, 2, G> merge_22 {gr};

    fc.to_cycle(w2);
    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    merge_22(BSlot{w1, a}, w2);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w2()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w2).length(), len[w2()]);

    for (EgIdG c {}; const auto& g : gr.chain(w2).g)
        ASSERT_EQ(g.ind, c++);

    EgIdGs v(gr.chain(w2).length());
    std::iota(v.begin(), v.end(), len[w1()]);
    std::rotate(v.begin(), v.begin()+1, v.end());
    for (std::size_t i=0; i<gr.chain(w1).length(); ++i)
        ASSERT_EQ(gr.chain(w1).g[i].ind, v[i]);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), len.size());
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    ASSERT_EQ(c.chis.num({1, 1}), 0);
    ASSERT_EQ(c.chis.num({2, 2}), 0);
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 2);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};
    const ESlot w2A {w2, End::A};
    const ESlot w2B {w2, End::B};

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w1B));
    ASSERT_TRUE(n1A.contains(w2A));
    ASSERT_TRUE(n1A.contains(w2B));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w1A));
    ASSERT_TRUE(n1B.contains(w2A));
    ASSERT_TRUE(n1B.contains(w2B));

    const auto& n2A = gr.ngs_at(w2A);
    ASSERT_EQ(n2A.num(), 3);
    ASSERT_TRUE(n2A.contains(w1A));
    ASSERT_TRUE(n2A.contains(w1B));
    ASSERT_TRUE(n2A.contains(w2B));

    const auto& n2B = gr.ngs_at(w2B);
    ASSERT_EQ(n2B.num(), 3);
    ASSERT_TRUE(n2B.contains(w1A));
    ASSERT_TRUE(n2B.contains(w1B));
    ASSERT_TRUE(n2B.contains(w2A));
}


/// Tests vm22 case 10: vertex merger of two unconnected cycle chains at their
/// respective boundary vertices.
TEST_F(VertexMergerTest, vm22_case10)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vm22 case 10: vertex merger of two unconnected ",
                          "cycle chains at their respective boundary vertices");

    constexpr std::array<size_t, 2> len {4, 4};

    constexpr ChIdG w1 {1};
    constexpr ChIdG w0 {0};
    constexpr std::array vv {w1, w0};

    G gr;

    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMergerCore fc {gr};
    VertexMerger<2, 2, G> merge_2c_2c {gr};

    fc.to_cycle(w0);
    fc.to_cycle(w1);

    if constexpr (profuse)
        gr.print_components("BEFORE");

    merge_2c_2c(w1, w0);

    if constexpr (profuse)
        gr.print_components("AFTER");

    const auto clmass = len[w1()] + len[w0()];
    ASSERT_EQ(gr.num_edges(), clmass);
    ASSERT_EQ(gr.num_chains(), 2);
    ASSERT_EQ(gr.num_compts(), 1);

    ASSERT_EQ(gr.num_vertices(Deg1), 0);
    ASSERT_EQ(gr.num_vertices(Deg2), clmass - 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 0);
    ASSERT_EQ(gr.num_vertices(Deg4), 1);

    ASSERT_EQ(gr.chain(w1).length(), len[w1()]);
    ASSERT_EQ(gr.chain(w0).length(), len[w0()]);

    for (EgIdG c {};
         const auto& m: gr.chains())
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, c++);

    const auto& c = gr.compt_last();
    ASSERT_EQ(c.ind, 0);
    ASSERT_EQ(c.num_chains(), 2);
    ASSERT_EQ(c.num_edges(), clmass);
    ASSERT_EQ(c.get_egl().size(), c.num_edges());

    std::size_t k {};
    for (EgIdC h {};
         const auto& w: c.ww) {
        ASSERT_EQ(w, vv[k]);
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
    ASSERT_EQ(c.chis.num({3, 3}), 0);
    ASSERT_EQ(c.chis.num({4, 4}), 2);
    ASSERT_EQ(c.chis.num({1, 3}), 0);
    ASSERT_EQ(c.chis.num({1, 4}), 0);
    ASSERT_EQ(c.chis.num({3, 4}), 0);
    c.template check<profuse>();

    const ESlot w0A {w0, End::A};
    const ESlot w0B {w0, End::B};
    const ESlot w1A {w1, End::A};
    const ESlot w1B {w1, End::B};

    const auto& n0A = gr.ngs_at(w0A);
    ASSERT_EQ(n0A.num(), 3);
    ASSERT_TRUE(n0A.contains(w1A));
    ASSERT_TRUE(n0A.contains(w1B));
    ASSERT_TRUE(n0A.contains(w0B));

    const auto& n0B = gr.ngs_at(w0B);
    ASSERT_EQ(n0B.num(), 3);
    ASSERT_TRUE(n0B.contains(w1A));
    ASSERT_TRUE(n0B.contains(w1B));
    ASSERT_TRUE(n0B.contains(w0A));

    const auto& n1A = gr.ngs_at(w1A);
    ASSERT_EQ(n1A.num(), 3);
    ASSERT_TRUE(n1A.contains(w0A));
    ASSERT_TRUE(n1A.contains(w0B));
    ASSERT_TRUE(n1A.contains(w1B));

    const auto& n1B = gr.ngs_at(w1B);
    ASSERT_EQ(n1B.num(), 3);
    ASSERT_TRUE(n1B.contains(w0A));
    ASSERT_TRUE(n1B.contains(w0B));
    ASSERT_TRUE(n1B.contains(w1A));
}

}  // namespace graffine::tests::vertex_merger
