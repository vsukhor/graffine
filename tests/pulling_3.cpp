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


namespace graffine::tests::pulling::d3 {

using Pull3 = Test;
using PullForw = Pulling<3, Orientation::Forwards, G>;

// =============================================================================
// ===== Linear Linear Linear ==================================================
// =============================================================================

/// Tests degree 3 pulling inside a linear chain connected to two other linear
/// chains.
/// Driver is at the chain end A, source edge is at the end B of the same chain.
/// Applies single step, to ensure that the source chain survives.
TEST_F(Pull3, DrLiA_SrLiB_SameCnDS_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling inside a linear chain connected to two ",
            "linear chains\n",
            "The path driver is at chain end A\n",
            "Source edge is at end B of the same chain\n",
            "Applies single step, to ensure that the source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

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
    constexpr auto eD = eA;
    const auto wS = wD;  // the source belongs to the same chain as the driver
    constexpr auto eS = End::opp(eD);
    const auto v1 = w0;
    const auto v2 = w2;
    const auto v3 = w3;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v3).length(), cnv2.length());
    ASSERT_EQ(gr.chain(wD).length(), n);
    ASSERT_EQ(gr.chain(v2).length(), cnwD.length() - n);
    ASSERT_EQ(gr.chain(wD).g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(gr.chain(v2).g[0].ind, cnwD.g[1].ind);
    ASSERT_EQ(gr.chain(v1).g[0].ind, cnv1.g[0].ind);
    ASSERT_EQ(gr.chain(v1).g[1].ind, cnv1.g[1].ind);
    ASSERT_EQ(gr.chain(v3).g[0].ind, cnv2.g[0].ind);
    ASSERT_EQ(gr.chain(v3).g[1].ind, cnv2.g[1].ind);
    ASSERT_EQ(gr.chain(v3).g[2].ind, cnv2.g[2].ind);

}  // DrLiA_SrLiB_SameCnDS_Survive


/// Tests degree 3 pulling a single linear chain connected to two linear chains.
/// Driver is at end A, source edge is at end B of the same chain.
/// Applies enough steps to engulf the whole source chain.
TEST_F(Pull3, DrLiA_SrLiB_SameCnDS_Consume)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a single linear chain connected to two ",
            "linear chains\n",
            "Driver is at end A, source edge is at end B of the same chain\n",
            "Applies enough steps to engulf the whole source chain"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});  // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = wD;  // the source belongs to the same chain as the driver
    constexpr auto eS = End::opp(eD);
    const auto v1 = w0;
    const auto v2 = w2;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull two steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), cnwD.length());
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnv2.length());
    ASSERT_EQ(gr.chain(wD).g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(gr.chain(wD).g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(gr.chain(v1).g[0].ind, cnv1.g[0].ind);
    ASSERT_EQ(gr.chain(v1).g[1].ind, cnv1.g[1].ind);
    ASSERT_EQ(gr.chain(v2).g[0].ind, cnv2.g[0].ind);
    ASSERT_EQ(gr.chain(v2).g[1].ind, cnv2.g[1].ind);
    ASSERT_EQ(gr.chain(v2).g[2].ind, cnv2.g[2].ind);
    ASSERT_EQ(gr.chain(wD).ngs[eD].num(), 0);
    ASSERT_EQ(gr.chain(wD).ngs[eS].num(), 2);
    ASSERT_EQ(gr.chain(wD).ngs[eS], cnwD.ngs[eD]);
    ASSERT_EQ(gr.chain(wD).ngs[eS], cnwD.ngs[eD]);

}  // DrLiA_SrLiB_SameCnDS_Consume


/// Tests degree 3 pulling a single linear chain connected to two linear chains.
/// Driver is at end B, source edge is at end A of the same chain.
/// Applies single step to ensure that the source chain survives.
TEST_F(Pull3, DrLiB_SrLiA_SameCnDS_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a single linear chain connected to two ",
            "linear chains\n",
            "Driver is at end B, source edge is at end A of the same chain\n",
            "Applies single step to ensure that the source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 2};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

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
    constexpr auto eD = eB;
    const auto wS = wD;  // the source belongs to the same chain as the driver
    constexpr auto eS = End::opp(eD);
    const auto v1 = w1;
    const auto v2 = w2;
    const auto v3 = w3;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnwD.length() - n);
    ASSERT_EQ(gr.chain(v3).length(), cnv2.length());
    ASSERT_EQ(gr.chain(wD).g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(gr.chain(v1).g[0].ind, cnv1.g[1].ind);
    ASSERT_EQ(gr.chain(v1).g[1].ind, cnv1.g[0].ind);
    ASSERT_EQ(gr.chain(v2).g[0].ind, cnwD.g[1].ind);
    ASSERT_EQ(gr.chain(v3).g[0].ind, cnv2.g[0].ind);
    ASSERT_EQ(gr.chain(v3).g[1].ind, cnv2.g[1].ind);
    ASSERT_EQ(gr.chain(v3).g[2].ind, cnv2.g[2].ind);

}  // DrLiB_SrLiA_SameCnDS_Survive


/// Tests degree 3 pulling a single linear chain connected to two linear chains.
/// Driver is at end B, source edge is at end A of the same chain.
/// Applies enough steps to engulf the whole source chain.
TEST_F(Pull3, DrLiB_SrLiA_SameCnDS_Consume)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a single linear chain connected to two ",
            "linear chains\n",
            "Driver is at end B, source edge is at end A of the same chain\n",
            "Applies enough steps to engulf the whole source chain"
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
    constexpr auto eD = eB;
    const auto wS = wD;  // the source belongs to the same chain as the driver
    constexpr auto eS = End::opp(eD);
    const auto v1 = w1;
    const auto v2 = w2;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull two steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mwD = gr.chain(wD);
    const auto& mv1 = gr.chain(v1);
    const auto& mv2 = gr.chain(v2);

    ASSERT_EQ(mwD.length(), cnv2.length());
    ASSERT_EQ(mv1.length(), cnv1.length());
    ASSERT_EQ(mv2.length(), cnwD.length());
    ASSERT_EQ(mwD.g[0].ind, cnv2.g[0].ind);
    ASSERT_EQ(mwD.g[1].ind, cnv2.g[1].ind);
    ASSERT_EQ(mwD.g[2].ind, cnv2.g[2].ind);
    ASSERT_EQ(mv1.g[0].ind, cnv1.g[1].ind);
    ASSERT_EQ(mv1.g[1].ind, cnv1.g[0].ind);
    ASSERT_EQ(mv2.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mv2.g[1].ind, cnwD.g[1].ind);
    const ChIdG v {c.num_chains() - 1};
    ASSERT_EQ(gr.chain(v).ngs[eD].num(), 0);
    ASSERT_EQ(gr.chain(v).ngs[eS].num(), 2);
    ASSERT_TRUE(gr.chain(v).ngs[eS].contains((*cnwD.ngs[eD].begin()).opp()));
//    ASSERT_EQ(gr.chain(v).ngs[eS][1].w, wD);
//    ASSERT_EQ(gr.chain(v).ngs[eS][1].e, cnwD.ngs[eD][1].e);

}  // DrLiB_SrLiA_SameCnDS_Consume


// =============================================================================
// ===== Linear Cycle Out ======================================================
// =============================================================================

/// Tests degree 3 pulling a linear chain connected to a cycle chain.
/// Driver is at end A of the linear chain, source is at its end B.
/// Path does not include the cycle chain.
/// Applies single step, so that the source chain survives.
TEST_F(Pull3, DrLiA_SrLiB_OutCyc_SameCnDS_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a linear chain connected to a cycle chain\n",
            "Driver is at end A of the linear chain, source is at its end B\n",
            "Path does not include the cycle chain\n",
            "Applies single step, so that the source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 3});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = wD;  // the source belongs to the same chain as the driver
    constexpr auto eS = End::opp(eD);
    const auto v1 = w0;
    const auto v2 = w2;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnwD.length() - n);
    ASSERT_EQ(gr.chain(wD).g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(gr.chain(v1).g[0].ind, cnv1.g[0].ind);
    ASSERT_EQ(gr.chain(v1).g[1].ind, cnv1.g[1].ind);
    ASSERT_EQ(gr.chain(v1).g[2].ind, cnv1.g[2].ind);
    ASSERT_EQ(gr.chain(v2).g[0].ind, cnwD.g[1].ind);

}  // DrLiA_SrLiB_OutCyc_SameCnDS_Survive


/// Tests degree 3 pulling a linear chain connected to a cycle chain.
/// Driver is at end A of the linear chain, source is at its end B.
/// Path does not include the cycle chain.
/// Applies enough steps to engulf the whole source chain.
TEST_F(Pull3, DrLiA_SrLiB_OutCyc_SameCnDS_Consume)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a linear chain connected to a cycle chain\n",
            "Driver is at end A of the linear chain, source is at its end B\n",
            "Path does not include the cycle chain\n",
            "Applies enough steps to engulf the whole source chain"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 3});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = wD;  // the same chain for the driver and the source
    constexpr auto eS = End::opp(eD);
    const auto v = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv = gr.chain(v);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnv.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull two steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mwD = gr.chain(wD);
    const auto& mwS = gr.chain(wS);
    const auto& mv = gr.chain(v);

    ASSERT_EQ(mwD.length(), cnwD.length());
    ASSERT_EQ(gr.chain(v).length(), cnv.length());
    ASSERT_EQ(mwD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mwD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mv.g[0].ind, cnv.g[0].ind);
    ASSERT_EQ(mv.g[1].ind, cnv.g[1].ind);
    ASSERT_EQ(mv.g[2].ind, cnv.g[2].ind);

    ASSERT_EQ(mwD.ngs[eD].num(), 0);

    ASSERT_EQ(mwS.ngs[eS].num(), 2);
    const auto aux = cnwD.ngs[eD].as_vector();
    ASSERT_TRUE(mwS.ngs[eS].contains(aux[0]));
    ASSERT_TRUE(mwS.ngs[eS].contains(aux[1]));

    ASSERT_EQ(mv.ngs[eA].num(), 2);
    ASSERT_TRUE(mv.ngs[eA].contains(ESlot{v, eB}));
    ASSERT_TRUE(mv.ngs[eA].contains(ESlot{wS, eS}));

    ASSERT_EQ(mv.ngs[eB].num(), 2);
    ASSERT_TRUE(mv.ngs[eB].contains(ESlot{wS, eS}));
    ASSERT_TRUE(mv.ngs[eB].contains(ESlot{v, eA}));

}  // DrLiA_SrLiB_OutCyc_SameCnDS_Consume


/// Tests degree 3 pulling a linear chain connected to a cycle chain.
/// Driver is at end B of the linear chain, source is at its end A.
/// Path does not include the cycle chain.
/// Applies single step, so that the source chain survives.
TEST_F(Pull3, DrLiB_SrLiA_OutCyc_SameCnDS_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a linear chain connected to a cycle chain\n",
            "Driver is at end B of the linear chain, source is at its end A\n",
            "Path does not include the cycle chain\n",
            "Applies single step, so that the source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    // a linear chain w0 connected at end B to a cycle chain w1
    merge12(ESlot{w0, eB},
            BSlot{w0, 2});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = wD;  // the same chain for the driver and the source
    constexpr auto eS = End::opp(eD);
    const auto v1 = w0;
    const auto v2 = w2;
    constexpr int n {1};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
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
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnwD.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), cnwD.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), n);

}  // DrLiB_SrLiA_OutCyc_SameCnDS_Survive


/// Tests degree 3 pulling a linear chain connected a cycle chain.\n
/// Source and driver edges belong to the same chain;
/// Applies enough steps to engulf the whole source chain.
TEST_F(Pull3, DrLiB_SrLiA_OutCyc_SameCnDS_Consume)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a linear chain connected a cycle chain\n",
            "Source and driver edges belong to the same chain\n",
            "Applies enough steps to engulf the whole source chain"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w1, w0 : length (5) -> (2, 3)
    // a linear chain w0 connected at end B to a cycle chain w1
    merge12(ESlot{w0, eB},
            BSlot{w0, 2});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto eD = eB;
    constexpr auto eS = End::opp(eD);
    const auto w = w1;  // the same chain for the driver and the source
    const auto v = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(w).end_edge(eD), eD};
    const Source src {w, End::opp(eD)};
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
    const auto cnw = gr.chain(w);  // copy
    const auto cnv = gr.chain(v);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), w);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnw.get_single_leaf_end(), End::opp(eD));
    ASSERT_EQ(cnw.get_single_3way_end(), eD);
    ASSERT_EQ(pp.source_chid(), w);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_TRUE(cnv.is_connected_cycle());
    ASSERT_TRUE(cnw.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull two steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m = gr.chain(w);  // pulled
    const auto& p = gr.chain(v);
    ASSERT_EQ(m.length(), 2);
    ASSERT_EQ(p.length(), 3);
    ASSERT_TRUE(p.is_connected_cycle());
    ASSERT_EQ(m.ngs[eD].num(), 0);
    ASSERT_EQ(m.ngs[eS].num(), 2);
    ASSERT_TRUE(m.ngs[eS].contains(ESlot{v, eB}));
    ASSERT_TRUE(m.ngs[eS].contains(ESlot{v, eA}));

}  // DrLiB_SrLiA_OutCyc_SameCnDS_Consume


// =============================================================================
// ===== Cycle Linear In =======================================================
// =============================================================================

/// Tests degree 3 pulling a cycle chain connected a linear chain.
/// Driver is at end A of the cycle, source is at end B of the linear chain.
/// Applies single step, so that the source chain survives.
TEST_F(Pull3, DrCyA_SrLiB_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a cycle chain connected a linear chain\n",
            "Driver is at end A of the cycle\n",
            "Source is at end B of the linear chain\n",
            "Applies single step, so that the source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 3});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eA;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eB;
    const auto iS = gr.chain(wS).end_edge(eS).ind;
    const auto v = w0;
    constexpr int n {1};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    std::vector<Driver> internals {
        Driver {&gr.chain(wD).end_edge(End::opp(eD)), End::opp(eD)},
        Driver {&gr.chain(wS).end_edge(End::opp(eS)), End::opp(eS)}
    };
    Path pp {&gr.compt(wS),
             drv,
             src,
             internals};

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

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_TRUE(cnwD.is_connected_cycle());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto d = gr.get_egl(iD).w;
    const auto s = gr.get_egl(iS).w;
    const auto& mD = gr.chain(d);
    const auto& mS = gr.chain(s);
    const auto& mV = gr.chain(v);
    ASSERT_EQ(mD.length(), n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mV.length(), cnwD.length());
    ASSERT_TRUE(mV.is_connected_cycle());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mV.g[0].ind, cnwD.g[1].ind);
    ASSERT_EQ(mV.g[1].ind, cnwD.g[2].ind);
    ASSERT_EQ(mV.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.ngs[eD].num(), 0);
    ASSERT_EQ(mD.ngs[End::opp(eD)].num(), 3);
    ASSERT_TRUE(mD.ngs[End::opp(eD)].contains(ESlot{v, eA}));
    ASSERT_TRUE(mD.ngs[End::opp(eD)].contains(ESlot{v, eB}));
    ASSERT_TRUE(mD.ngs[End::opp(eD)].contains(ESlot{s, End::opp(eS)}));
    ASSERT_EQ(mS.ngs[eS].num(), 0);
    ASSERT_EQ(mS.ngs[End::opp(eS)].num(), 3);
    ASSERT_TRUE(mS.ngs[End::opp(eS)].contains(ESlot{v, eA}));
    ASSERT_TRUE(mS.ngs[End::opp(eS)].contains(ESlot{v, eB}));
    ASSERT_TRUE(mS.ngs[End::opp(eS)].contains(ESlot{d, End::opp(eD)}));

}  // DrCyA_SrLiB_Survive


/// Tests degree 3 pulling a cycle chain connected a linear chain.
/// Driver is at end A of the cycle, source is at end B of the linear chain.
/// Applies enough steps to engulf the whole source chain.
TEST_F(Pull3, DrCyA_SrLiB_Consume)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a cycle chain connected a linear chain\n",
            "Driver is at end A of the cycle\n",
            "Source is at end B of the linear chain\n",
            "Applies enough steps to engulf the whole source chain"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 3});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eA;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eB;
    const auto iS = gr.chain(wS).end_edge(eS).ind;
    constexpr int n {2};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    std::vector<Driver> internals {
        Driver {&gr.chain(wD).end_edge(End::opp(eD)), End::opp(eD)},
        Driver {&gr.chain(wS).end_edge(End::opp(eS)), End::opp(eS)}
    };
    Path pp {&gr.compt(wS),
             drv,
             src,
             internals};

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

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_TRUE(cnwD.is_connected_cycle());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull two steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto d = gr.get_egl(iD).w;
    const auto s = gr.get_egl(iS).w;
    const auto& mD = gr.chain(d);
    const auto& mS = gr.chain(s);
    ASSERT_EQ(mD.length(), cnwS.length());
    ASSERT_EQ(mS.length(), cnwD.length());
    ASSERT_TRUE(mS.is_connected_cycle());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwD.g[2].ind);
    ASSERT_EQ(mS.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mS.g[2].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.ngs[eD].num(), 0);
    ASSERT_EQ(mD.ngs[End::opp(eD)].num(), 2);
    ASSERT_TRUE(mD.ngs[End::opp(eD)].contains(ESlot{s, eA}));
    ASSERT_TRUE(mD.ngs[End::opp(eD)].contains(ESlot{s, eB}));
    ASSERT_EQ(mS.ngs[End::opp(eS)].num(), 2);
    ASSERT_TRUE(mS.ngs[End::opp(eS)].contains(ESlot{d, eB}));
    ASSERT_TRUE(mS.ngs[End::opp(eS)].contains(ESlot{s, eS}));
    ASSERT_EQ(mS.ngs[eS].num(), 2);
    ASSERT_TRUE(mS.ngs[eS].contains(ESlot{s, End::opp(eS)}));
    ASSERT_TRUE(mS.ngs[eS].contains(ESlot{d, End::opp(eD)}));

}  // DrCyA_SrLiB_Consume


/// Tests degree 3 pulling a cycle chain connected a linear chain.
/// Driver is at end B of the cycle, source is at end B of the linear chain.
/// Applies single step, so that the source chain survives.
TEST_F(Pull3, DrCyB_SrLiB_Survive)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling a cycle chain connected a linear chain\n",
            "Driver is at end B of the cycle\n",
            "Source is at end B of the linear chain\n",
            "Applies single step, so that the source chain survives"
    );

    // Create initial graph.

    constexpr std::size_t len {5};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 3});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eB;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eB;
    const auto iS = gr.chain(wS).end_edge(eS).ind;
    const auto v = w0;
    constexpr int n {1};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    std::vector<Driver> internals {
        Driver {&gr.chain(wD).end_edge(End::opp(eD)), End::opp(eD)},
        Driver {&gr.chain(wS).end_edge(End::opp(eS)), End::opp(eS)}
    };
    Path pp {&gr.compt(wS),
             drv,
             src,
             internals};

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

    ASSERT_EQ(num_chains, 2);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_TRUE(cnwD.is_connected_cycle());
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu3f {gr};
    pu3f(pp, n);  // pull one step to preserve source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains + 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(iD);
    const auto& mS = gr.chain(iS);
    const auto& mV = gr.chain(v);
    ASSERT_EQ(mD.length(), n);
    ASSERT_EQ(mS.length(), 1);
    ASSERT_EQ(mV.length(), 3);
    ASSERT_TRUE(mV.is_connected_cycle());
    ASSERT_EQ(mD.g[0].ind, cnwD.g.back().ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[1].ind);

}  // DrCyB_SrLiB_Survive


}  // namespace graffine::tests::pulling::d3