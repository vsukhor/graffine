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

#include "pulling.h"

namespace graffine::tests::pulling::d2 {

using Pull_2 = Test;
using PullForw = Pulling<2, Orientation::Forwards, G>;


/// Tests single-step pulling degree 2 over a single-edge path.
/// Source is chain end A.
TEST_F(Pull_2, SingleLinChain_Step1_PathLen1_SrcA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests single-step pulling a degree 2 vertex over a single-edge ",
            "path\n",
            "Source is chain end A"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto wD = w;
    constexpr auto aD = 0UL;
    constexpr auto egeD = eB;
    constexpr auto wS = w;
    constexpr auto eS = eA;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).g[aD], egeD};
    constexpr Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() > n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // Pull single step.

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m = gr.chain(w);
    ASSERT_EQ(m.length(), 5);
    ASSERT_EQ(m.g[0].ind, 0);
    ASSERT_EQ(m.g[1].ind, 1);
    ASSERT_EQ(m.g[2].ind, 2);
    ASSERT_EQ(m.g[3].ind, 3);
    ASSERT_EQ(m.g[4].ind, 4);
    ASSERT_EQ(m.g[0].indw, 0);
    ASSERT_EQ(m.g[1].indw, 1);
    ASSERT_EQ(m.g[2].indw, 2);
    ASSERT_EQ(m.g[3].indw, 3);
    ASSERT_EQ(m.g[4].indw, 4);
    ASSERT_EQ(m.g[0].w, w);
    ASSERT_EQ(m.g[1].w, w);
    ASSERT_EQ(m.g[2].w, w);
    ASSERT_EQ(m.g[3].w, w);
    ASSERT_EQ(m.g[4].w, w);
    ASSERT_EQ(m.g[0].c, ic);
    ASSERT_EQ(m.g[1].c, ic);
    ASSERT_EQ(m.g[2].c, ic);
    ASSERT_EQ(m.g[3].c, ic);
    ASSERT_EQ(m.g[4].c, ic);
    ASSERT_FALSE(m.g[0].points_forwards());
    ASSERT_TRUE(m.g[1].points_forwards());
    ASSERT_TRUE(m.g[2].points_forwards());
    ASSERT_TRUE(m.g[3].points_forwards());
    ASSERT_TRUE(m.g[4].points_forwards());
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

}  // SingleLinChain_Step1_PathLen1_SrcA


/// Tests single-step pulling degree 2 over a single-edge path.
/// Source is chain end B.
TEST_F(Pull_2, SingleLinChain_Step1_PathLen1_SrcB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests single-step pulling degree 2 vertex over a single-edge ",
            "path\n",
            "Source is chain end B"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto wD = w;
    constexpr auto aD = 4;
    constexpr auto egeD = eA;
    constexpr auto wS = w;
    constexpr auto eS = eB;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).g[aD], egeD};
    constexpr Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull one step

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m = gr.chain(w);
    ASSERT_EQ(m.length(), 5);
    ASSERT_EQ(m.g[0].ind, 0);
    ASSERT_EQ(m.g[1].ind, 1);
    ASSERT_EQ(m.g[2].ind, 2);
    ASSERT_EQ(m.g[3].ind, 3);
    ASSERT_EQ(m.g[4].ind, 4);
    ASSERT_EQ(m.g[0].indw, 0);
    ASSERT_EQ(m.g[1].indw, 1);
    ASSERT_EQ(m.g[2].indw, 2);
    ASSERT_EQ(m.g[3].indw, 3);
    ASSERT_EQ(m.g[4].indw, 4);
    ASSERT_EQ(m.g[0].w, w);
    ASSERT_EQ(m.g[1].w, w);
    ASSERT_EQ(m.g[2].w, w);
    ASSERT_EQ(m.g[3].w, w);
    ASSERT_EQ(m.g[4].w, w);
    ASSERT_EQ(m.g[0].c, ic);
    ASSERT_EQ(m.g[1].c, ic);
    ASSERT_EQ(m.g[2].c, ic);
    ASSERT_EQ(m.g[3].c, ic);
    ASSERT_EQ(m.g[4].c, ic);
    ASSERT_TRUE(m.g[0].points_forwards());
    ASSERT_TRUE(m.g[1].points_forwards());
    ASSERT_TRUE(m.g[2].points_forwards());
    ASSERT_TRUE(m.g[3].points_forwards());
    ASSERT_FALSE(m.g[4].points_forwards());
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

}  // SingleLinChain_Step1_PathLen1_SrcB


/// Tests single-step pulling degree 2 inside a compartment composed
/// of a single linear chain.
/// Path contains multiple edges, all inside the same chain.
/// Source edge is at chain end A.
TEST_F(Pull_2, SingleLinChain_Step1_PathLenMult_SrcA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests single-step pulling degree 2 inside a compartment composed ",
            "of a single linear chain\n",
            "Path contains multiple edges, all inside the same chain\n"
            "Source edge is at chain end A"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    const auto aD = 2;
    const auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = wD;
    const auto eS = eA;
    constexpr int n {1};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), End::undefined);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() > n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull one step

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w0).g[1].ind, 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, 3);
    ASSERT_EQ(gr.chain(w1).g[1].ind, 4);
    ASSERT_EQ(gr.chain(w2).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w0).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w0).g[1].indw, 1);
    ASSERT_EQ(gr.chain(w1).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w1).g[1].indw, 1);
    ASSERT_EQ(gr.chain(w2).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w0).g[0].w, w0);
    ASSERT_EQ(gr.chain(w0).g[1].w, w0);
    ASSERT_EQ(gr.chain(w1).g[0].w, w1);
    ASSERT_EQ(gr.chain(w1).g[1].w, w1);
    ASSERT_EQ(gr.chain(w2).g[0].w, w2);
    ASSERT_EQ(gr.chain(w0).g[0].c, ic);
    ASSERT_EQ(gr.chain(w0).g[1].c, ic);
    ASSERT_EQ(gr.chain(w1).g[0].c, ic);
    ASSERT_EQ(gr.chain(w1).g[1].c, ic);
    ASSERT_EQ(gr.chain(w2).g[0].c, ic);
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_TRUE(gr.chain(w0).ngs[eB].contains(ESlot{w1, eA}));
    ASSERT_TRUE(gr.chain(w0).ngs[eB].contains(ESlot{w2, eA}));
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(w1).ngs[eA].contains(ESlot{w0, eB}));
    ASSERT_TRUE(gr.chain(w1).ngs[eA].contains(ESlot{w2, eA}));
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 0);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 2);
    ASSERT_TRUE(gr.chain(w2).ngs[eA].contains(ESlot{w1, eA}));
    ASSERT_TRUE(gr.chain(w2).ngs[eA].contains(ESlot{w0, eB}));
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);

}  // SingleLinChain_Step1_PathLenMult_SrcA


/// Tests single-step pulling degree 2 over a multi-edge path.
/// Source edge is at end B.
TEST_F(Pull_2, SingleLinChain_SrcB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests single-step pulling degree 2 over a multi-edge path\n",
            "Source edge is at chain end B"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();
    const auto w {w0};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w;
    constexpr auto aD = 2;
    constexpr auto egeD = eA;
    const auto wS = wD;  // driver and source are on the same chain
    constexpr auto eS = eB;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).g[aD], egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), End::undefined);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull one step: creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 2);
    ASSERT_EQ(gr.chain(w0).g[0].ind, 0);
    ASSERT_EQ(gr.chain(w0).g[1].ind, 1);
    ASSERT_EQ(gr.chain(w1).g[0].ind, 2);
    ASSERT_EQ(gr.chain(w2).g[0].ind, 3);
    ASSERT_EQ(gr.chain(w2).g[1].ind, 4);
    ASSERT_EQ(gr.chain(w0).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w0).g[1].indw, 1);
    ASSERT_EQ(gr.chain(w1).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w2).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w2).g[1].indw, 1);
    ASSERT_EQ(gr.chain(w0).g[0].w, w0);
    ASSERT_EQ(gr.chain(w0).g[1].w, w0);
    ASSERT_EQ(gr.chain(w1).g[0].w, w1);
    ASSERT_EQ(gr.chain(w2).g[0].w, w2);
    ASSERT_EQ(gr.chain(w2).g[1].w, w2);
    ASSERT_EQ(gr.chain(w0).g[0].c, ic);
    ASSERT_EQ(gr.chain(w0).g[1].c, ic);
    ASSERT_EQ(gr.chain(w1).g[0].c, ic);
    ASSERT_EQ(gr.chain(w0).g[0].c, ic);
    ASSERT_EQ(gr.chain(w0).g[1].c, ic);
    ASSERT_TRUE(gr.chain(w0).g[0].points_forwards());
    ASSERT_TRUE(gr.chain(w0).g[1].points_forwards());
    ASSERT_TRUE(gr.chain(w1).g[0].points_forwards());
    ASSERT_TRUE(gr.chain(w2).g[0].points_forwards());
    ASSERT_TRUE(gr.chain(w2).g[1].points_forwards());
    ASSERT_EQ(gr.chain(w0).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[eA].num(), 0);
    ASSERT_EQ(gr.chain(w1).ngs[eB].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eA].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[eB].num(), 0);

}  //  SingleLinChain_SrcB_SrcSurvives


/// Tests pulling degree 2 over a multi-edge path laying entirely inside the
/// same chain.
/// Source edge is at chain end A.
/// Number of steps equals to the path length, so that chain part of the path
/// edges is reversed.
TEST_F(Pull_2, SingleLinChain_SrcA_SrcConsumed_Reversed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests pulling degree 2 over a multi-edge path laying entirely ",
            "inside the same chain\n",
            "Source edge is at chain end A\n",
            "Number of steps equals to the path length, so that chain part of ",
            "the path edges is reversed"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto wD = w;
    constexpr auto aD = 2;
    constexpr auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto wS = w;
    constexpr auto eS = eA;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    constexpr Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), End::undefined);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() == n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m = gr.chain(w);
    ASSERT_EQ(m.length(), 5);
    ASSERT_EQ(m.g[0].ind, 4);
    ASSERT_EQ(m.g[1].ind, 3);
    ASSERT_EQ(m.g[2].ind, 0);
    ASSERT_EQ(m.g[3].ind, 1);
    ASSERT_EQ(m.g[4].ind, 2);
    ASSERT_EQ(m.g[0].indw, 0);
    ASSERT_EQ(m.g[1].indw, 1);
    ASSERT_EQ(m.g[2].indw, 2);
    ASSERT_EQ(m.g[3].indw, 3);
    ASSERT_EQ(m.g[4].indw, 4);
    ASSERT_EQ(m.g[0].w, w);
    ASSERT_EQ(m.g[1].w, w);
    ASSERT_EQ(m.g[2].w, w);
    ASSERT_EQ(m.g[3].w, w);
    ASSERT_EQ(m.g[4].w, w);
    ASSERT_EQ(m.g[0].c, ic);
    ASSERT_EQ(m.g[1].c, ic);
    ASSERT_EQ(m.g[2].c, ic);
    ASSERT_EQ(m.g[3].c, ic);
    ASSERT_EQ(m.g[4].c, ic);
    ASSERT_FALSE(m.g[0].points_forwards());
    ASSERT_FALSE(m.g[1].points_forwards());
    ASSERT_TRUE(m.g[2].points_forwards());
    ASSERT_TRUE(m.g[3].points_forwards());
    ASSERT_TRUE(m.g[4].points_forwards());
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

}  //  SingleLinChain_SrcA_SrcConsumed_Reversed

/*
TEST_BEFORE 0  **  0 [0]  len 5 > > > > > ( 0 1 2 3 4 )
            [0] > id 16 ind 0 indc 0 w 0 c 0 weight 1 v [20 -> 22]
            [1] > id 17 ind 1 indc 1 w 0 c 0 weight 1 v [22 -> 23]
            [2] > id 18 ind 2 indc 2 w 0 c 0 weight 1 v [23 -> 24]
            [3] > id 19 ind 3 indc 3 w 0 c 0 weight 1 v [24 -> 25]
            [4] > id 20 ind 4 indc 4 w 0 c 0 weight 1 v [25 -> 21]

TEST_AFTER 0  **  0 [0]  len 5 < < > > > ( 4 3 0 1 2 )
           [0] < id 20 ind 4 indc 0 w 0 c 0 weight 1 v [21 <- 25]
           [1] < id 19 ind 3 indc 1 w 0 c 0 weight 1 v [25 <- 29]
           [2] > id 16 ind 0 indc 2 w 0 c 0 weight 1 v [29 -> 22]
           [3] > id 17 ind 1 indc 3 w 0 c 0 weight 1 v [22 -> 23]
           [4] > id 18 ind 2 indc 4 w 0 c 0 weight 1 v [23 -> 24]
*/


/// Tests pulling degree 2 over a multi-edge path.
/// The source edge is at end B.
/// Number of steps equals to the path length so that the path is consumed.
TEST_F(Pull_2, SingleLinChain_SrcB_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests pulling degree 2 over a multi-edge path driven at chain ",
            "intermediate edge\n",
            "Source edge is at end B\n",
            "Number of steps equals to the path length"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto wD = w;
    constexpr auto aD = 2;
    constexpr auto egeD = eA;
    constexpr auto wS = w;
    constexpr auto eS = eB;
    constexpr int n {3};

    const Driver drv {&gr.chain(wD).g[aD], egeD};
    constexpr Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), End::undefined);
    ASSERT_FALSE(cnwD.is_connected());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_FALSE(cnwS.is_connected());
    ASSERT_TRUE(cnwS.length() == n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(ic, 0);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m = gr.chain(w);
    ASSERT_EQ(m.length(), 5);
    ASSERT_EQ(m.g[0].ind, 0);
    ASSERT_EQ(m.g[1].ind, 1);
    ASSERT_EQ(m.g[2].ind, 4);
    ASSERT_EQ(m.g[3].ind, 3);
    ASSERT_EQ(m.g[4].ind, 2);
    ASSERT_EQ(m.g[0].indw, 0);
    ASSERT_EQ(m.g[1].indw, 1);
    ASSERT_EQ(m.g[2].indw, 2);
    ASSERT_EQ(m.g[3].indw, 3);
    ASSERT_EQ(m.g[4].indw, 4);
    ASSERT_EQ(m.g[0].w, w);
    ASSERT_EQ(m.g[1].w, w);
    ASSERT_EQ(m.g[2].w, w);
    ASSERT_EQ(m.g[3].w, w);
    ASSERT_EQ(m.g[4].w, w);
    ASSERT_EQ(m.g[0].c, ic);
    ASSERT_EQ(m.g[1].c, ic);
    ASSERT_EQ(m.g[2].c, ic);
    ASSERT_EQ(m.g[3].c, ic);
    ASSERT_EQ(m.g[4].c, ic);
    ASSERT_TRUE(m.g[0].points_forwards());
    ASSERT_TRUE(m.g[1].points_forwards());
    ASSERT_FALSE(m.g[2].points_forwards());
    ASSERT_FALSE(m.g[3].points_forwards());
    ASSERT_FALSE(m.g[4].points_forwards());
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

}  //  SingleLinChain_SrcB_SrcConsumed
/*
TEST_BEFORE 0  **  0 [0]  len 5 > > > > > ( 0 1 2 3 4 )
            [0] > id 27 ind 0 indc 0 w 0 c 0 weight 1 v [37 -> 39]
            [1] > id 28 ind 1 indc 1 w 0 c 0 weight 1 v [39 -> 40]
            [2] > id 29 ind 2 indc 2 w 0 c 0 weight 1 v [40 -> 41]
            [3] > id 30 ind 3 indc 3 w 0 c 0 weight 1 v [41 -> 42]
            [4] > id 31 ind 4 indc 4 w 0 c 0 weight 1 v [42 -> 38]
TEST_AFTER 0  **  0 [0]  len 5 > > < < < ( 0 1 4 3 2 )
           [0] > id 27 ind 0 indc 0 w 0 c 0 weight 1 v [37 -> 39]
           [1] > id 28 ind 1 indc 1 w 0 c 0 weight 1 v [39 -> 46]
           [2] < id 31 ind 4 indc 2 w 0 c 0 weight 1 v [46 <- 42]
           [3] < id 30 ind 3 indc 3 w 0 c 0 weight 1 v [42 <- 41]
           [4] < id 29 ind 2 indc 4 w 0 c 0 weight 1 v [41 <- 43]
*/

/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver is inside a linear chain connected to the junction with its end B.
/// Source is at the free end A of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J3_DrvLinB_SrcLinA_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver is inside a linear chain connected to the junction with ",
            "its end B\n",
            "Source is at the free end A of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eB},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w0;
    constexpr auto eS = eA;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.is_connected_at(eB));
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eB));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 2);
    ASSERT_EQ(gr.chain(w3).length(), 2);
    ASSERT_EQ(gr.chain(w4).length(), 1);

}  // J3_DrvLinB_SrcLinA_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver is inside a linear chain connected to the junction with its end A.
/// Source is at the free end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J3_DrvLinA_SrcLinB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver is inside a linear chain connected to the junction with ",
            "its end A\n",
            "Source is at the free end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 0;
    constexpr auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {w2, eB};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_EQ(gr.chain(w4).length(), 2);

}  // J3_DrvLinA_SrcLinB_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver is inside a linear chain connected to the junction with its end A.
/// Source is at the free end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J3_DrvLinB_SrcLinB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver is inside a linear chain connected to the junction with ",
            "its end A\n",
            "Source is at the free end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), 2);
    ASSERT_EQ(gr.chain(w4).length(), 1);

}  // J3_DrvLinB_SrcLinB_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver is inside a linear chain connected to the junction with its end A.
/// Source is at the free end A of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J3_DrvLinA_SrcLinA_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver is inside a linear chain connected to the junction with ",
            "its end A\n",
            "Source is at the free end A of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 0;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto egeD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};

    // Pull two steps.
    pu2f(pp, n);

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 2);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_EQ(gr.chain(w4).length(), 2);

}  // J3_DrvLinA_SrcLinA_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver edge end A of a linear chain pulls end A of a linear source chain.
/// The source chain is fully consumed.
TEST_F(Pull_2, J3_LinDrvA_LinSrcA_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver: edge end A of a linear chain\n",
            "Source: end A of a linear chain\n",
            "The source chain is fully consumed"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eB},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w0;
    constexpr auto eS = eA;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 3);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);

}  // J3_LinDrvA_LinSrcA_SrcConsumed


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver edge end B of a linear chain pulls end B of a linear source chain.
/// The source chain is fully consumed.
TEST_F(Pull_2, J3_LinDrvB_LinSrcB_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver: edge end B of a linear chain\n",
            "Source: end B of a linear chain\n",
            "The source chain is fully consumed"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 0;
    constexpr auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 3);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 1);

}  // J3_LinDrvB_LinSrcB_SrcConsumed


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver edge end A of a linear chain pulls end B of a linear source chain.
/// The source chain is fully consumed.
TEST_F(Pull_2, J3_LinDrvA_LinSrcB_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver: edge end A of a linear chain\n",
            "Source: end B of a linear chain\n",
            "The source chain is fully consumed"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 3);

}  // J3_LinDrvA_LinSrcB_SrcConsumed


/// Tests degree 2 pulling over a path traversing a 3-way junction.
/// Driver edge end A of a linear chain pulls end B of a linear source chain.
/// The source chain is consumed.
TEST_F(Pull_2, J3_LinDrvB_LinSrcA_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction\n",
            "Driver: edge end B of a linear chain\n",
            "Source: end A of a linear chain\n",
            "The source chain is fully consumed"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3, w4] = id_sequence<ChIdG, 5>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 0;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto egeD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 3);
    ASSERT_EQ(gr.chain(w2).length(), 3);

}  // J3_LinDrvB_LinSrcA_SrcConsumed


/// Tests degree 2 pulling over a path traversing a 4-way junction.
/// Driver: edge end A of a linear chain.
/// Source: end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J4_LinDrvA_LinSrcB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 4-way junction\n",
            "Driver: edge end A of a linear chain\n",
            "Source: end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 4};
    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (5) -> (2, 3)
    // w1, w2 : length (4) -> (1, 3)
    merge22(BSlot{w1, 1},
            BSlot{w0, 2});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w3;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 1);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_EQ(gr.chain(w4).length(), 2);
    ASSERT_EQ(gr.chain(w5).length(), 2);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
}  // J4_LinDrvA_LinSrcB_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 4-way junction.
/// Driver: edge end B of a linear chain.
/// Source: end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J4_LinDrvB_LinSrcB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 4-way junction\n",
            "Driver: edge end B of a linear chain\n",
            "Source: end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 4};
    const auto [w0, w1, w2, w3, w4, w5] = id_sequence<ChIdG, 6>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (5) -> (2, 3)
    // w1, w2 : length (4) -> (1, 3)
    merge22(BSlot{w1, 1},
            BSlot{w0, 2});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto aD = 0;
    constexpr auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w3;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    ASSERT_EQ(gr.chain(w3).length(), 1);
    ASSERT_EQ(gr.chain(w4).length(), 2);
    ASSERT_EQ(gr.chain(w5).length(), 1);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
}  // J4_LinDrvB_LinSrcB_SrcSurvives


/// Tests degree 2 pulling over a path traversing a 3-way junction along with
/// two 4-way junctions.
/// Driver: linear chain connected with end A to the rest of the path and
/// another end free.
/// Source: end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J344_LinDrvA_LinSrcB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction ",
            "along with two 4-way junctions\n",
            "Driver: linear chain connected with end A to the rest of the ",
            "path and another end free\n",
            "Source: end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 4> len {5, 7, 3, 3};
    const auto [w0, w1, w2, w3, w4, w5, w6, w7, w8, w9]
        = id_sequence<ChIdG, 10>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};

    // w0, w5 : length (5) -> (2, 3)
    // w1, w4 : length (7) -> (1, 6)
    merge22(BSlot{w1, 1},
            BSlot{w0, 2});     // creates w4, w5

    // w4, w6 : length (6) -> (1, 5)
    // w2, w7 : length (3) -> (2, 1)
    merge22(BSlot{w4, 1},
            BSlot{w2, 2});     // creates w6, w7

    VertexMerger<1, 1, G> merge11 {gr};

    // make w2 cycle removing w7
    merge11(ESlot{w2, eA},
            ESlot{w7, eB});

    VertexMerger<1, 2, G> merge12 {gr};

    // w6, w7 : length (5) -> (2, 3)
    merge12(ESlot{w3, eA},
            BSlot{w6, 2});     // creates w7

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w5;
    constexpr auto aD = 1;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w7;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 8);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    ASSERT_EQ(gr.chain(w3).length(), 3);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(w5).length(), 1);
    ASSERT_EQ(gr.chain(w6).length(), 2);
    ASSERT_EQ(gr.chain(w7).length(), 1);
    ASSERT_EQ(gr.chain(w8).length(), 2);
    ASSERT_EQ(gr.chain(w9).length(), 2);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
}  // J344_LinDrvA_LinSrcB_SrcSurvives
/*
Component 0: 18 edges in 8 chains
TEST_BEFORE 3 {6 B} {7 A}  **  0 [0]  len 3 > > > ( 15 16 17 )
            [0] > id 126 ind 15 indc 0 w 3 c 0 weight 1 v [174 -> 183]
            [1] > id 127 ind 16 indc 1 w 3 c 0 weight 1 v [183 -> 184]
            [2] > id 128 ind 17 indc 2 w 3 c 0 weight 1 v [184 -> 182]
TEST_BEFORE 1  ** {0 B} {4 A} {5 A}  0 [1]  len 1 > ( 5 )
            [0] > id 116 ind 5 indc 3 w 1 c 0 weight 1 v [169 -> 166]
TEST_BEFORE 4 {0 B} {1 B} {5 A}  ** {2 B} {6 A} {2 A}  0 [2]  len 1 > ( 6 )
            [0] > id 117 ind 6 indc 4 w 4 c 0 weight 1 v [166 -> 180]
TEST_BEFORE 0  ** {1 B} {4 A} {5 A}  0 [3]  len 2 > > ( 0 1 )
            [0] > id 111 ind 0 indc 5 w 0 c 0 weight 1 v [163 -> 165]
            [1] > id 112 ind 1 indc 6 w 0 c 0 weight 1 v [165 -> 166]
TEST_BEFORE 5 {0 B} {4 A} {1 B}  **  0 [4]  len 3 > > > ( 2 3 4 )
            [0] > id 113 ind 2 indc 7 w 5 c 0 weight 1 v [166 -> 167]
            [1] > id 114 ind 3 indc 8 w 5 c 0 weight 1 v [167 -> 168]
            [2] > id 115 ind 4 indc 9 w 5 c 0 weight 1 v [168 -> 164]
TEST_BEFORE 6 {2 B} {4 B} {2 A}  ** {3 A} {7 A}  0 [5]  len 2 > > ( 7 8 )
            [0] > id 118 ind 7 indc 10 w 6 c 0 weight 1 v [180 -> 173]
            [1] > id 119 ind 8 indc 11 w 6 c 0 weight 1 v [173 -> 174]
TEST_BEFORE 2 {2 B} {6 A} {4 B}  ** {4 B} {6 A} {2 A}  0 [6]  len 3 > > > ( 14 12 13 )
            [0] > id 125 ind 14 indc 12 w 2 c 0 weight 1 v [180 -> 178]
            [1] > id 123 ind 12 indc 13 w 2 c 0 weight 1 v [178 -> 179]
            [2] > id 124 ind 13 indc 14 w 2 c 0 weight 1 v [179 -> 180]
TEST_BEFORE 7 {3 A} {6 B}  **  0 [7]  len 3 > > > ( 9 10 11 )
            [0] > id 120 ind 9 indc 15 w 7 c 0 weight 1 v [174 -> 175]
            [1] > id 121 ind 10 indc 16 w 7 c 0 weight 1 v [175 -> 176]
            [2] > id 122 ind 11 indc 17 w 7 c 0 weight 1 v [176 -> 170]
TEST_BEFORE: PATH
driver (egEnd A) 0:   [1] > id 114 ind 3 indc 8 w 5 c 0 weight 1 v [167 -> 168]
                 1:   [0] > id 113 ind 2 indc 7 w 5 c 0 weight 1 v [166 -> 167]
                 2:   [0] > id 117 ind 6 indc 4 w 4 c 0 weight 1 v [166 -> 180]
                 3:   [0] > id 118 ind 7 indc 10 w 6 c 0 weight 1 v [180 -> 173]
                 4:   [1] > id 119 ind 8 indc 11 w 6 c 0 weight 1 v [173 -> 174]
                 5:   [0] > id 120 ind 9 indc 15 w 7 c 0 weight 1 v [174 -> 175]
                 6:   [1] > id 121 ind 10 indc 16 w 7 c 0 weight 1 v [175 -> 176]
source (end B)   7:   [2] > id 122 ind 11 indc 17 w 7 c 0 weight 1 v [176 -> 170]
Correcting driver. After correction:
driver (egEnd B) 0:   [0] > id 113 ind 2 indc 7 w 5 c 0 weight 1 v [166 -> 167]
                 1:   [0] > id 117 ind 6 indc 4 w 4 c 0 weight 1 v [166 -> 180]
                 2:   [0] > id 118 ind 7 indc 10 w 6 c 0 weight 1 v [180 -> 173]
                 3:   [1] > id 119 ind 8 indc 11 w 6 c 0 weight 1 v [173 -> 174]
                 4:   [0] > id 120 ind 9 indc 15 w 7 c 0 weight 1 v [174 -> 175]
                 5:   [1] > id 121 ind 10 indc 16 w 7 c 0 weight 1 v [175 -> 176]
source (end B)   6:   [2] > id 122 ind 11 indc 17 w 7 c 0 weight 1 v [176 -> 170]

Component 0: 18 edges in 10 chains
TEST_AFTER 8 {5 B} {9 A}  **  0 [0]  len 2 > > ( 3 4 )
           [0] > id 114 ind 3 indc 0 w 8 c 0 weight 1 v [180 -> 168]
           [1] > id 115 ind 4 indc 1 w 8 c 0 weight 1 v [168 -> 164]
TEST_AFTER 3 {6 B} {7 A}  **  0 [1]  len 3 > > > ( 15 16 17 )
           [0] > id 126 ind 15 indc 2 w 3 c 0 weight 1 v [176 -> 183]
           [1] > id 127 ind 16 indc 3 w 3 c 0 weight 1 v [183 -> 184]
           [2] > id 128 ind 17 indc 4 w 3 c 0 weight 1 v [184 -> 182]
TEST_AFTER 1  ** {0 B} {4 A} {5 A}  0 [2]  len 1 > ( 5 )
           [0] > id 116 ind 5 indc 5 w 1 c 0 weight 1 v [169 -> 173]
TEST_AFTER 4 {0 B} {1 B} {5 A}  ** {2 B} {6 A} {2 A}  0 [3]  len 1 > ( 8 )
           [0] > id 119 ind 8 indc 10 w 4 c 0 weight 1 v [173 -> 174]
TEST_AFTER 0  ** {1 B} {4 A} {5 A}  0 [4]  len 2 > > ( 0 1 )
           [0] > id 111 ind 0 indc 7 w 0 c 0 weight 1 v [163 -> 165]
           [1] > id 112 ind 1 indc 8 w 0 c 0 weight 1 v [165 -> 173]
TEST_AFTER 5 {0 B} {4 A} {1 B}  ** {8 A} {9 A}  0 [5]  len 1 < ( 7 )
           [0] < id 118 ind 7 indc 6 w 5 c 0 weight 1 v [173 <- 180]
TEST_AFTER 6 {2 B} {4 B} {2 A}  ** {3 A} {7 A}  0 [6]  len 2 > > ( 9 10 )
           [0] > id 120 ind 9 indc 11 w 6 c 0 weight 1 v [174 -> 175]
           [1] > id 121 ind 10 indc 15 w 6 c 0 weight 1 v [175 -> 176]
TEST_AFTER 2 {2 B} {6 A} {4 B}  ** {4 B} {6 A} {2 A}  0 [7]  len 3 > > > ( 14 12 13 )
           [0] > id 125 ind 14 indc 12 w 2 c 0 weight 1 v [174 -> 178]
           [1] > id 123 ind 12 indc 13 w 2 c 0 weight 1 v [178 -> 179]
           [2] > id 124 ind 13 indc 14 w 2 c 0 weight 1 v [179 -> 174]
TEST_AFTER 7 {3 A} {6 B}  **  0 [8]  len 1 > ( 11 )
           [0] > id 122 ind 11 indc 16 w 7 c 0 weight 1 v [176 -> 170]
TEST_AFTER 9 {8 A} {5 B}  **  0 [9]  len 2 < > ( 6 2 )
           [0] < id 117 ind 6 indc 9 w 9 c 0 weight 1 v [180 <- 166]
           [1] > id 113 ind 2 indc 17 w 9 c 0 weight 1 v [166 -> 167]

*/

/// Tests degree 2 pulling over a path traversing a 3-way junction along with
/// two 4-way junctions.
/// Driver: linear chain connected with end B to the rest of the path and
/// another end free.
/// Source: end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_2, J344_LinDrvB_LinSrcB_SrcSurvives)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction ",
            "along with two 4-way junctions\n",
            "Driver: linear chain connected with end B to the rest of the ",
            "path and another end free\n",
            "Source: end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 4> len {5, 7, 3, 3};
    const auto [w0, w1, w2, w3, w4, w5, w6, w7, w8, w9]
        = id_sequence<ChIdG, 10>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};

    // w0, w5 : length (5) -> (2, 3)
    // w1, w4 : length (7) -> (1, 6)
    merge22(BSlot{w1, 1},
            BSlot{w0, 2});     // creates w4, w5

    // w4, w6 : length (6) -> (1, 5)
    // w2, w7 : length (3) -> (2, 1)
    merge22(BSlot{w4, 1},
            BSlot{w2, 2});     // creates w6, w7

    VertexMerger<1, 1, G> merge11 {gr};

    // make w2 cycle removing w7
    merge11(ESlot{w2, eA},
            ESlot{w7, eB});

    VertexMerger<1, 2, G> merge12 {gr};

    // w6, w7 : length (5) -> (2, 3)
    merge12(ESlot{w3, eA},
            BSlot{w6, 2});     // creates w7

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto aD = 0;
    constexpr auto egeD = eB;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w7;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 8);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, 2);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    ASSERT_EQ(gr.chain(w3).length(), 3);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(w5).length(), 3);
    ASSERT_EQ(gr.chain(w6).length(), 2);
    ASSERT_EQ(gr.chain(w7).length(), 1);
    ASSERT_EQ(gr.chain(w8).length(), 2);
    ASSERT_EQ(gr.chain(w9).length(), 1);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
}  // J344_LinDrvB_LinSrcB_SrcSurvives

/*
Component 0: 18 edges in 8 chains
TEST_BEFORE 3 {6 B} {7 A}  **  0 [0]  len 3 > > > ( 15 16 17 )
            [0] > id 126 ind 15 indc 0 w 3 c 0 weight 1 v [174 -> 183]
            [1] > id 127 ind 16 indc 1 w 3 c 0 weight 1 v [183 -> 184]
            [2] > id 128 ind 17 indc 2 w 3 c 0 weight 1 v [184 -> 182]
TEST_BEFORE 1  ** {0 B} {4 A} {5 A}  0 [1]  len 1 > ( 5 )
            [0] > id 116 ind 5 indc 3 w 1 c 0 weight 1 v [169 -> 166]
TEST_BEFORE 4 {0 B} {1 B} {5 A}  ** {2 B} {6 A} {2 A}  0 [2]  len 1 > ( 6 )
            [0] > id 117 ind 6 indc 4 w 4 c 0 weight 1 v [166 -> 180]
TEST_BEFORE 0  ** {1 B} {4 A} {5 A}  0 [3]  len 2 > > ( 0 1 )
            [0] > id 111 ind 0 indc 5 w 0 c 0 weight 1 v [163 -> 165]
            [1] > id 112 ind 1 indc 6 w 0 c 0 weight 1 v [165 -> 166]
TEST_BEFORE 5 {0 B} {4 A} {1 B}  **  0 [4]  len 3 > > > ( 2 3 4 )
            [0] > id 113 ind 2 indc 7 w 5 c 0 weight 1 v [166 -> 167]
            [1] > id 114 ind 3 indc 8 w 5 c 0 weight 1 v [167 -> 168]
            [2] > id 115 ind 4 indc 9 w 5 c 0 weight 1 v [168 -> 164]
TEST_BEFORE 6 {2 B} {4 B} {2 A}  ** {3 A} {7 A}  0 [5]  len 2 > > ( 7 8 )
            [0] > id 118 ind 7 indc 10 w 6 c 0 weight 1 v [180 -> 173]
            [1] > id 119 ind 8 indc 11 w 6 c 0 weight 1 v [173 -> 174]
TEST_BEFORE 2 {2 B} {6 A} {4 B}  ** {4 B} {6 A} {2 A}  0 [6]  len 3 > > > ( 14 12 13 )
            [0] > id 125 ind 14 indc 12 w 2 c 0 weight 1 v [180 -> 178]
            [1] > id 123 ind 12 indc 13 w 2 c 0 weight 1 v [178 -> 179]
            [2] > id 124 ind 13 indc 14 w 2 c 0 weight 1 v [179 -> 180]
TEST_BEFORE 7 {3 A} {6 B}  **  0 [7]  len 3 > > > ( 9 10 11 )
            [0] > id 120 ind 9 indc 15 w 7 c 0 weight 1 v [174 -> 175]
            [1] > id 121 ind 10 indc 16 w 7 c 0 weight 1 v [175 -> 176]
            [2] > id 122 ind 11 indc 17 w 7 c 0 weight 1 v [176 -> 170]
TEST_BEFORE: PATH
driver (egEnd B) 0:   [0] > id 129 ind 0 indc 5 w 0 c 0 weight 1 v [186 -> 188]
                 1:   [1] > id 130 ind 1 indc 6 w 0 c 0 weight 1 v [188 -> 189]
                 2:   [0] > id 135 ind 6 indc 4 w 4 c 0 weight 1 v [189 -> 203]
                 3:   [0] > id 136 ind 7 indc 10 w 6 c 0 weight 1 v [203 -> 196]
                 4:   [1] > id 137 ind 8 indc 11 w 6 c 0 weight 1 v [196 -> 197]
                 5:   [0] > id 138 ind 9 indc 15 w 7 c 0 weight 1 v [197 -> 198]
                 6:   [1] > id 139 ind 10 indc 16 w 7 c 0 weight 1 v [198 -> 199]
source (end B)   7:   [2] > id 140 ind 11 indc 17 w 7 c 0 weight 1 v [199 -> 193]
Correcting driver. After correction:
driver (egEnd A) 0:   [1] > id 130 ind 1 indc 6 w 0 c 0 weight 1 v [188 -> 189]
                 1:   [0] > id 135 ind 6 indc 4 w 4 c 0 weight 1 v [189 -> 203]
                 2:   [0] > id 136 ind 7 indc 10 w 6 c 0 weight 1 v [203 -> 196]
                 3:   [1] > id 137 ind 8 indc 11 w 6 c 0 weight 1 v [196 -> 197]
                 4:   [0] > id 138 ind 9 indc 15 w 7 c 0 weight 1 v [197 -> 198]
                 5:   [1] > id 139 ind 10 indc 16 w 7 c 0 weight 1 v [198 -> 199]
source (end B)   6:   [2] > id 140 ind 11 indc 17 w 7 c 0 weight 1 v [199 -> 193]
Component 0: 18 edges in 10 chains
TEST_AFTER 0  ** {8 B} {9 A}  0 [0]  len 1 > ( 0 )
           [0] > id 129 ind 0 indc 0 w 0 c 0 weight 1 v [186 -> 203]
TEST_AFTER 8  ** {0 B} {9 A}  0 [1]  len 2 > > ( 1 6 )
           [0] > id 130 ind 1 indc 1 w 8 c 0 weight 1 v [208 -> 189]
           [1] > id 135 ind 6 indc 17 w 8 c 0 weight 1 v [189 -> 203]
TEST_AFTER 7 {3 A} {6 B}  **  0 [2]  len 1 > ( 11 )
           [0] > id 140 ind 11 indc 3 w 7 c 0 weight 1 v [199 -> 193]
TEST_AFTER 6 {2 B} {4 B} {2 A}  ** {3 A} {7 A}  0 [3]  len 2 > > ( 9 10 )
           [0] > id 138 ind 9 indc 5 w 6 c 0 weight 1 v [197 -> 198]
           [1] > id 139 ind 10 indc 2 w 6 c 0 weight 1 v [198 -> 199]
TEST_AFTER 5 {9 B} {4 A} {1 B}  **  0 [4]  len 3 > > > ( 2 3 4 )
           [0] > id 131 ind 2 indc 6 w 5 c 0 weight 1 v [196 -> 190]
           [1] > id 132 ind 3 indc 7 w 5 c 0 weight 1 v [190 -> 191]
           [2] > id 133 ind 4 indc 8 w 5 c 0 weight 1 v [191 -> 187]
TEST_AFTER 4 {9 B} {1 B} {5 A}  ** {2 B} {6 A} {2 A}  0 [5]  len 1 > ( 8 )
           [0] > id 137 ind 8 indc 4 w 4 c 0 weight 1 v [196 -> 197]
TEST_AFTER 3 {6 B} {7 A}  **  0 [6]  len 3 > > > ( 15 16 17 )
           [0] > id 144 ind 15 indc 10 w 3 c 0 weight 1 v [199 -> 206]
           [1] > id 145 ind 16 indc 11 w 3 c 0 weight 1 v [206 -> 207]
           [2] > id 146 ind 17 indc 12 w 3 c 0 weight 1 v [207 -> 205]
TEST_AFTER 2 {2 B} {6 A} {4 B}  ** {4 B} {6 A} {2 A}  0 [7]  len 3 > > > ( 14 12 13 )
           [0] > id 143 ind 14 indc 13 w 2 c 0 weight 1 v [197 -> 201]
           [1] > id 141 ind 12 indc 14 w 2 c 0 weight 1 v [201 -> 202]
           [2] > id 142 ind 13 indc 15 w 2 c 0 weight 1 v [202 -> 197]
TEST_AFTER 1  ** {9 B} {4 A} {5 A}  0 [8]  len 1 > ( 5 )
           [0] > id 134 ind 5 indc 16 w 1 c 0 weight 1 v [192 -> 196]
TEST_AFTER 9 {0 B} {8 B}  ** {1 B} {4 A} {5 A}  0 [9]  len 1 > ( 7 )
           [0] > id 136 ind 7 indc 9 w 9 c 0 weight 1 v [203 -> 196]

*/
/// Tests degree 2 pulling a chain connected to a 3-way junction.
/// Driver: linear chain attached with end A and having the other end free.
/// Source: end B of a of the driver chain.
/// source chain is engulfed completely.
TEST_F(Pull_2, J3_SameChainAB_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling a chain connected to a 3-way junction\n",
            "Driver: linear chain attached with end A and having the other ",
            "end free\n",
            "Source: end B of a of the driver chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (6) -> (3, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto aD = 1;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto egeD = eA;
    const auto wS = wD;
    constexpr auto eS = eB;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};


    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 3);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
    ASSERT_EQ(gr.chain(wS).ngs[eA].num(), cnwS.ngs[eA].num());
    ASSERT_EQ(gr.chain(wS).ngs[eB].num(), cnwS.ngs[eB].num());
    ASSERT_EQ(gr.chain(wS).g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(gr.chain(wS).g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(gr.chain(wS).g[2].ind, cnwS.g[1].ind);
    ASSERT_TRUE(gr.chain(wS).g[0].points_forwards());
    ASSERT_FALSE(gr.chain(wS).g[1].points_forwards());
    ASSERT_FALSE(gr.chain(wS).g[2].points_forwards());
}  // J3_SameChainAB_SrcConsumed
/*
Component 0: 6 edges in 3 chains
TEST_BEFORE 1 {0 B} {2 A}  **  0 [0]  len 1 > ( 6 )
            [0] > id 153 ind 6 indc 0 w 1 c 0 weight 1 v [213 -> 217]
TEST_BEFORE 0  ** {1 A} {2 A}  0 [1]  len 3 > > > ( 0 1 2 )
            [0] > id 147 ind 0 indc 1 w 0 c 0 weight 1 v [209 -> 211]
            [1] > id 148 ind 1 indc 2 w 0 c 0 weight 1 v [211 -> 212]
            [2] > id 149 ind 2 indc 3 w 0 c 0 weight 1 v [212 -> 213]
TEST_BEFORE 2 {1 A} {0 B}  **  0 [2]  len 3 > > > ( 3 4 5 )
            [0] > id 150 ind 3 indc 4 w 2 c 0 weight 1 v [213 -> 214]
            [1] > id 151 ind 4 indc 5 w 2 c 0 weight 1 v [214 -> 215]
            [2] > id 152 ind 5 indc 6 w 2 c 0 weight 1 v [215 -> 210]
Component 0: 6 edges in 3 chains
TEST_AFTER 1 {0 B} {2 A}  **  0 [0]  len 1 > ( 6 )
           [0] > id 153 ind 6 indc 0 w 1 c 0 weight 1 v [213 -> 217]
TEST_AFTER 0  ** {1 A} {2 A}  0 [1]  len 3 > > > ( 0 1 2 )
           [0] > id 147 ind 0 indc 1 w 0 c 0 weight 1 v [209 -> 211]
           [1] > id 148 ind 1 indc 2 w 0 c 0 weight 1 v [211 -> 212]
           [2] > id 149 ind 2 indc 3 w 0 c 0 weight 1 v [212 -> 213]
TEST_AFTER 2 {1 A} {0 B}  **  0 [2]  len 3 > < < ( 3 5 4 )
           [0] > id 150 ind 3 indc 4 w 2 c 0 weight 1 v [213 -> 221]
           [1] < id 152 ind 5 indc 5 w 2 c 0 weight 1 v [221 <- 215]
           [2] < id 151 ind 4 indc 6 w 2 c 0 weight 1 v [215 <- 218]

*/

/// Tests degree 2 pulling a chain connected to a 3-way junction.
/// Driver: linear chain attached with end B and having the other end free.
/// Source: end A of a of the driver chain.
/// source chain is engulfed completely.
TEST_F(Pull_2, J3_SameChainBA_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling a chain connected to a 3-way junction\n",
            "Driver: linear chain attached with end B and having the other ",
            "end free\n",
            "Source: end A of a of the driver chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (6) -> (3, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto aD = 1;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto egeD = eB;
    const auto wS = wD;
    constexpr auto eS = eA;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n + aD);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 3);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), 3);
    for(EgIdC i {}; const auto& h : c.get_egl()) {
        const auto& eg = gr.chain(h.w).g[h.a];
        ASSERT_EQ(eg.w, h.w);
        ASSERT_EQ(eg.indw, h.a);
        ASSERT_EQ(eg.ind, h.i);
        ASSERT_EQ(eg.c, ic);
        ASSERT_EQ(eg.indc, i++);
    }
    ASSERT_EQ(gr.chain(wS).ngs[eA].num(), cnwS.ngs[eB].num());
    ASSERT_EQ(gr.chain(wS).ngs[eB].num(), cnwS.ngs[eA].num());
    ASSERT_EQ(gr.chain(wS).g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(gr.chain(wS).g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(gr.chain(wS).g[2].ind, cnwS.g[1].ind);
    ASSERT_FALSE(gr.chain(wS).g[0].points_forwards());
    ASSERT_TRUE(gr.chain(wS).g[1].points_forwards());
    ASSERT_TRUE(gr.chain(wS).g[2].points_forwards());
}  // J3_SameChainBA_SrcConsumed
/*
Component 0: 7 edges in 3 chains
TEST_BEFORE 1 {0 B} {2 A}  **  0 [0]  len 1 > ( 6 )
            [0] > id 161 ind 6 indc 0 w 1 c 0 weight 1 v [226 -> 230]
TEST_BEFORE 0  ** {1 A} {2 A}  0 [1]  len 3 > > > ( 0 1 2 )
            [0] > id 155 ind 0 indc 1 w 0 c 0 weight 1 v [222 -> 224]
            [1] > id 156 ind 1 indc 2 w 0 c 0 weight 1 v [224 -> 225]
            [2] > id 157 ind 2 indc 3 w 0 c 0 weight 1 v [225 -> 226]
TEST_BEFORE 2 {1 A} {0 B}  **  0 [2]  len 3 > > > ( 3 4 5 )
            [0] > id 158 ind 3 indc 4 w 2 c 0 weight 1 v [226 -> 227]
            [1] > id 159 ind 4 indc 5 w 2 c 0 weight 1 v [227 -> 228]
            [2] > id 160 ind 5 indc 6 w 2 c 0 weight 1 v [228 -> 223]
Component 0: 7 edges in 3 chains
TEST_AFTER 0 {1 A} {2 A}  **  0 [0]  len 3 < > > ( 2 0 1 )
           [0] < id 157 ind 2 indc 0 w 0 c 0 weight 1 v [226 <- 234]
           [1] > id 155 ind 0 indc 1 w 0 c 0 weight 1 v [234 -> 224]
           [2] > id 156 ind 1 indc 2 w 0 c 0 weight 1 v [224 -> 225]
TEST_AFTER 2 {1 A} {0 A}  **  0 [1]  len 3 > > > ( 3 4 5 )
           [0] > id 158 ind 3 indc 3 w 2 c 0 weight 1 v [226 -> 227]
           [1] > id 159 ind 4 indc 4 w 2 c 0 weight 1 v [227 -> 228]
           [2] > id 160 ind 5 indc 5 w 2 c 0 weight 1 v [228 -> 223]
TEST_AFTER 1 {0 A} {2 A}  **  0 [2]  len 1 > ( 6 )
           [0] > id 161 ind 6 indc 6 w 1 c 0 weight 1 v [226 -> 230]
*/


/// Tests degree 2 pulling over a path traversing a 3-way junction,
/// source chain is engulfed completely.
TEST_F(Pull_2, Deg3junctionSrcRem_X)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way junction, ",
            "source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {3, 2};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto aD = 0;
    const auto& egD = gr.chain(wD).g[aD];
    constexpr auto egeD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    constexpr int n {2};

    const Driver drv {&egD, egeD};
    const Source src {wS, eS};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eB);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, 2);  // pull two steps

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.chain(w1).length(), 2);
    ASSERT_EQ(gr.chain(w2).length(), 2);
    ChIdC k {};
    for (ChIdG i {}; const auto& cn: gr.chains()) {
        ASSERT_EQ(cn.idw, i);
        ASSERT_EQ(cn.idc, k++);
        ASSERT_EQ(cn.c, 0);
        ++i;
    }
    constexpr std::array<size_t, 5> inds {4, 3, 0, 1, 2};
    for (std::size_t i {}; const auto& eg: gr.chain(w0).g) {
        ASSERT_EQ(eg.ind, inds[i]);
        ASSERT_EQ(eg.indc, i);
        ASSERT_EQ(eg.indw, i);
        ASSERT_EQ(eg.c, 0);
        ASSERT_EQ(eg.w, w0);
        ++i;
    }
}
/*
Component 0: 5 edges in 3 chains
TEST_BEFORE 1 {0 B} {2 A}  **  0 [0]  len 2 > > ( 3 4 )
            [0] > id 166 ind 3 indc 0 w 1 c 0 weight 1 v [238 -> 241]
            [1] > id 167 ind 4 indc 1 w 1 c 0 weight 1 v [241 -> 240]
TEST_BEFORE 0  ** {1 A} {2 A}  0 [1]  len 2 > > ( 0 1 )
            [0] > id 163 ind 0 indc 2 w 0 c 0 weight 1 v [235 -> 237]
            [1] > id 164 ind 1 indc 3 w 0 c 0 weight 1 v [237 -> 238]
TEST_BEFORE 2 {1 A} {0 B}  **  0 [2]  len 1 > ( 2 )
            [0] > id 165 ind 2 indc 4 w 2 c 0 weight 1 v [238 -> 236]
TEST_BEFORE: PATH
driver (egEnd B) 0:   [0] > id 166 ind 3 indc 0 w 1 c 0 weight 1 v [238 -> 241]
                 1:   [1] > id 164 ind 1 indc 3 w 0 c 0 weight 1 v [237 -> 238]
source (end A)   2:   [0] > id 163 ind 0 indc 2 w 0 c 0 weight 1 v [235 -> 237]
Component 0: 5 edges in 3 chains
TEST_AFTER 0 {1 A} {2 A}  **  0 [0]  len 1 > ( 4 )
           [0] > id 167 ind 4 indc 0 w 0 c 0 weight 1 v [237 -> 240]
TEST_AFTER 1 {0 A} {2 A}  **  0 [1]  len 2 < > ( 0 2 )
           [0] < id 163 ind 0 indc 2 w 1 c 0 weight 1 v [237 <- 245]
           [1] > id 165 ind 2 indc 3 w 1 c 0 weight 1 v [245 -> 236]
TEST_AFTER 2 {0 A} {1 A}  **  0 [2]  len 2 > > ( 1 3 )
           [0] > id 164 ind 1 indc 1 w 2 c 0 weight 1 v [237 -> 238]
           [1] > id 166 ind 3 indc 4 w 2 c 0 weight 1 v [238 -> 241]
*/

/// Tests degree 2 pulling over a path traversing a 3-way and
/// two 4-way junctions.
/// Source chain is engulfed completely.
TEST_F(Pull_2, J344_DrvLinA_SrcLinB_SrcConsumed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling over a path traversing a 3-way and ",
            "two 4-way junctions\n",
            "Driver: edge end A of a linear chain\n",
            "Source: end B of a linear chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 4> len {5, 7, 3, 3};
    const auto [w0, w1, w2, w3, w4, w5, w6, w7, w8] =
        id_sequence<ChIdG, 9>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};

    // w0, w5 : length (5) -> (2, 3)
    // w1, w4 : length (7) -> (1, 6)
    merge22(BSlot{w1, 1},
            BSlot{w0, 2});     // creates w4, w5

    // w4, w6 : length (6) -> (1, 5)
    // w2, w7 : length (3) -> (2, 1)
    merge22(BSlot{w4, 1},
            BSlot{w2, 2});     // creates w6, w7

    VertexMerger<1, 1, G> merge11 {gr};

    // make w2 cycle removing w7
    merge11(ESlot{w2, eA},
            ESlot{w7, eB});

    VertexMerger<1, 2, G> merge12 {gr};

    // w6, w7 : length (5) -> (2, 3)
    merge12(ESlot{w3, eA},
            BSlot{w6, 2});     // creates w7

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w5;
    constexpr auto aD = 2;
    constexpr auto egeD = eA;
    const auto& egD = gr.chain(wD).g[aD];
    const auto wS = w7;
    constexpr auto eS = eB;
    constexpr int n {3};

    const Driver drv {&egD, egeD};
    const Source src {w7, eB};
    Path pp {&gr.compt(src.w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnw3 = gr.chain(w3);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 8);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(egD.outer_endId(pp.drv().egEnd), eA);
    ASSERT_TRUE(cnwD.has_single_leaf_vertex());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu2f {gr};
    pu2f(pp, n);  // pull three steps

    if constexpr (profuse) {
        gr.print_components(tagAfter);
        pp.print_detailed(tagAfter);
    }

    // Compare the result to the expectation.

    ASSERT_EQ(pp.length(), 8);

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(w3).length(), 5);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(w5).length(), 2);
    ASSERT_EQ(gr.chain(w6).length(), 3);

}  // J344_DrvLinA_SrcLinB_SrcConsumed


}  // namespace graffine::tests::pulling::d3