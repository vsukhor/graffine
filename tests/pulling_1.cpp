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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>

#include "pulling.h"


namespace graffine::tests::pulling::d1 {

using Pull_1 = Test;
using PullForw = Pulling<1, Orientation::Forwards, G>;

// =============================================================================
// ===== Single Chain ==========================================================
// =============================================================================


/// Tests pulling degree 1 of a compartment consisting of a single linear chain.
/// The chain is driven at its end A.
/// This is a trivial case of a standalone chain where no topological change
/// is induced.
TEST_F(Pull_1, SingleLinChain_DrA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests pulling degree 1 of a compartment consisting of a single ",
            "linear chain\n",
            "The chain is driven at its end A\n",
            "This is a trivial case of a standalone chain where no topological ",
            "transformation is induced"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {0};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const Driver drv {&gr.chain(w).tail(), eA};
    constexpr Source src {w, eB};
    Path pp {&gr.compt(w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto num_edges = gr.compt(ic).num_edges();
    const auto num_chains = gr.compt(ic).num_chains();
    const auto num_vertices = gr.compt(ic).num_vertices();
    const auto cnw = gr.chain(w); // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), w);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(pp.source_chid(), w);
    ASSERT_EQ(pp.source_end(), eB);

    // Do the transformation.

    PullForw pu1f {gr};
    constexpr int n {2};
    pu1f(pp, n);

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& m = gr.chain(w);
    ASSERT_EQ(m, cnw);
    ASSERT_EQ(m.length(), len);
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

}  // SingleLinChain_DrAEgId len


/// Tests pulling degree 1 of a compartment consisting of a single linear chain.
/// The path is driven at the chain end B.
/// This is a trivial case of a standalone chain where no topological change
/// is induced.
TEST_F(Pull_1, SingleLinChain_DrB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests pulling degree 1 of a compartment consisting of a single ",
            "linear chain\n",
            "The path is driven at chain end B\n",
            "This is a trivial case of a standalone chain where no topological ",
            "transformation is induced"
        );

    // Create initial graph.

    constexpr std::size_t len {5};
    constexpr ChIdG w {0};

    G gr;
    gr.add_single_chain_component(len);

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const Driver drv {&gr.chain(w).head(), eB};
    constexpr Source src {w, eA};
    Path pp {&gr.compt(w), drv, src};

    if constexpr (profuse)
        pp.print_detailed(tagPathBefore);

    // Save initial values.

    const auto ic = pp.cmp->ind;
    const auto& c0 = gr.compt(ic);
    const auto num_edges = c0.num_edges();
    const auto num_chains = c0.num_chains();
    const auto num_vertices = c0.num_vertices();
    const auto cnw = gr.chain(w); // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 1);
    ASSERT_EQ(pp.driver_chid(), w);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(pp.source_chid(), w);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    constexpr int n {2};
    pu1f(pp, n);

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& m = gr.chain(w);
    ASSERT_EQ(m, cnw);
    ASSERT_EQ(m.ngs[eA].num(), 0);
    ASSERT_EQ(m.ngs[eB].num(), 0);

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

}  // SingleLinChain_DrB


// =============================================================================
// ===== Vertex Degree 3: Linear Source ========================================
// =============================================================================

/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path is driven at its end A.
/// The source chain is linear, its free end A survives.
TEST_F(Pull_1, J3_DrA_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at end A\n",
            "The source chain is linear, its free end A survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eB},
            BSlot{w0, 2});  // splits w0 to create w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eA;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto v1 = w2;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J3_DrA_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path is driven at its end A.
/// The source chain is linear, its free end B survives.
TEST_F(Pull_1, J3_DrA_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at end A\n",
            "The source chain is linear, its free end B survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});  // splits w0 to create w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w1;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};

    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& eg: mD.g) {
        ASSERT_EQ(eg.indw, i++);
        ASSERT_EQ(eg.w, wD);
        ASSERT_EQ(eg.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J3_DrA_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path driver is at chain end B.
/// The source chain is linear, its free end A survives.
TEST_F(Pull_1, J3_DrB_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path driver is at chain end B\n",
            "The source chain is linear, its free end A survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 3});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    const auto v1 = w2;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& eg: mD.g) {
        ASSERT_EQ(eg.indw, i++);
        ASSERT_EQ(eg.w, wD);
        ASSERT_EQ(eg.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J3_DrB_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path driver is at chain end B.
/// The source chain is linear, its free end B survives.
TEST_F(Pull_1, J3_DrB_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path driver is at chain end B\n",
            "The source chain is linear, its free end B survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};

    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[0].indc, cnwS.g[1].indc);
    ASSERT_EQ(mD.g[1].indc, cnwS.g[0].indc);
    for (EgIdA i {}; const auto& eg: mD.g) {
        ASSERT_EQ(eg.indw, i++);
        ASSERT_EQ(eg.w, wD);
        ASSERT_EQ(eg.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J3_DrB_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 3-way junction,
/// The path is driven at a chain end A.
/// The source chain is linear with free end A. It is engulfed completely.
TEST_F(Pull_1, J3_DrA_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at a chain end A\n",
            "The source chain is linear with free end A\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {3, 2};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};

    merge12(ESlot{w1, eB},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w0;
    constexpr auto eS = eA;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(CmpId{});
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 2);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m0 = gr.chain(w0);
    ASSERT_EQ(m0.length(), num_edges);
    ASSERT_EQ(m0.idw, 0);
    ASSERT_EQ(m0.idc, 0);
    ASSERT_EQ(m0.c, 0);
    constexpr std::array<size_t, 5> inds {3, 4, 1, 0, 2};
    for (std::size_t i {}; const auto& eg: m0.g) {
        ASSERT_EQ(eg.ind, inds[i]);
        ASSERT_EQ(eg.indc, i);
        ASSERT_EQ(eg.indw, i);
        ASSERT_EQ(eg.c, 0);
        ASSERT_EQ(eg.w, w0);
        ++i;
    }

}  // J3_DrA_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path is driven at a chain end A.
/// The source chain is linear with free end B. It is engulfed completely.
TEST_F(Pull_1, J3_DrA_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at a chain end A\n",
            "The source chain is linear with free end B\n",
            "The source is engulfed completely."
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w0;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {3};

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

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m0 = gr.chain(w0);
    ASSERT_EQ(m0.length(), c.num_edges());
    ASSERT_EQ(m0.idw, 0);
    ASSERT_EQ(m0.idc, 0);
    ASSERT_EQ(m0.c, 0);
    constexpr std::array<size_t, 6> inds {0, 1, 2, 3, 4, 5};
    for (std::size_t i {}; const auto& eg: m0.g) {
        ASSERT_EQ(eg.ind, inds[i]);
        ASSERT_EQ(eg.indc, i);
        ASSERT_EQ(eg.indw, i);
        ASSERT_EQ(eg.c, 0);
        ASSERT_EQ(eg.w, w0);
        ++i;
    }

}  // end J3_DrA_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 3-way junction.
/// The path is driven at a chain end B.
/// The source chain is linear with free end B. It is engulfed completely.
TEST_F(Pull_1, J3_DrB_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at a chain end B\n",
            "The source chain is linear with free end B\n",
            "The source is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {5, 1};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (5) -> (2, 3)
    merge12(ESlot{w1, eA},
            BSlot{w0, 2});     // creates w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    constexpr int n {3};

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

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 2);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m0 = gr.chain(w0);
    ASSERT_EQ(m0.length(), num_edges);
    ASSERT_EQ(m0.idw, 0);
    ASSERT_EQ(m0.idc, 0);
    ASSERT_EQ(m0.c, 0);
    constexpr std::array<size_t, 6> inds {0, 1, 4, 3, 2, 5};
    for (std::size_t i {}; const auto& eg: m0.g) {
        ASSERT_EQ(eg.ind, inds[i]);
        ASSERT_EQ(eg.indc, i);
        ASSERT_EQ(eg.indw, i);
        ASSERT_EQ(eg.c, 0);
        ASSERT_EQ(eg.w, w0);
        ++i;
    }

}  // J3_DrB_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 3-way junction,
/// The path is driven at a chain end B.
/// The source chain is linear with free end A. It is engulfed completely.
TEST_F(Pull_1, J3_DrB_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way junction\n",
            "The path is driven at a chain end B\n",
            "The source chain is linear with free end A\n",
            "The source is engulfed completely"
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

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
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

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    ASSERT_EQ(gr.num_compts(), 1);
    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 2);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& m0 = gr.chain(w0);
    ASSERT_EQ(m0.length(), num_edges);
    ASSERT_EQ(m0.idw, 0);
    ASSERT_EQ(m0.idc, 0);
    ASSERT_EQ(m0.c, 0);
    constexpr std::array<size_t, 5> inds {4, 3, 0, 1, 2};
    for (std::size_t i {}; const auto& eg: m0.g) {
        ASSERT_EQ(eg.ind, inds[i]);
        ASSERT_EQ(eg.indc, i);
        ASSERT_EQ(eg.indw, i);
        ASSERT_EQ(eg.c, 0);
        ASSERT_EQ(eg.w, w0);
        ++i;
    }

}  // J3_DrB_SrLiA_Rmv


// =============================================================================
// ===== Vertex Degree 3: Cycle Source =========================================
// =============================================================================

/// Tests degree 1 pulling through a 3-way junction joining a linear and a
/// cycle chains. Driver edge is at the free end A of the linear chain while
/// source is at the end A of the cycle chain. Preserves the component topology.
TEST_F(Pull_1, J3_DrA_SrCyA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling through a 3-way junction joining a ",
            "linear and a cycle chains\n",
            "Driver edge is at the free end A of the linear chain\n",
            "The source is at the end A of the cycle chain\n",
            "Preserves the component topology"
        );

    // Create initial graph.

    constexpr std::size_t len {6};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (6) -> (2, 4)
    // create a linear chain w1 connected at end B to a cycle chain w0
    merge12(ESlot{w0, eB},
            BSlot{w0, 2});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto eD = eA;
    const auto wD = w1;
    constexpr auto eS = eA;
    const auto wS = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
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
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_TRUE(cnwS.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n + minCycleLength);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to get min length of the source cycle chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_TRUE(mS.is_connected_cycle());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[3].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[2].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(mS.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.ngs[eA].num(), 2);
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wD, eB}));
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_EQ(mS.ngs[eB].num(), 2);
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wD, eB}));
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wS, eA}));
    ASSERT_EQ(mD.ngs[eA].num(), 0);
    ASSERT_EQ(mD.ngs[eB].num(), 2);
    ASSERT_TRUE(mD.ngs[eB].contains(ESlot{wS, eA}));
    ASSERT_TRUE(mD.ngs[eB].contains(ESlot{wS, eB}));

}  //  J3_DrA_SrCyA_Srv


/// Tests degree 1 pulling through a 3-way junction joining a linear and a
/// cycle chains. Driver edge is at the free end A of the linear chain while
/// source is at the end B of the cycle chain. Preserves the component topology.
TEST_F(Pull_1, J3_DrA_SrCyB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling through a 3-way junction joining a ",
            "linear and a cycle chains\n",
            "Driver edge is at the free end A of the linear chain\n",
            "The source is at the end B of the cycle chain\n",
            "Preserves the component topology"
        );

    // Create initial graph.

    constexpr std::size_t len {6};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // create a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eB},
            BSlot{w0, 2});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto eD = eA;
    const auto wD = w1;
    constexpr auto eS = eB;
    const auto wS = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
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
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_TRUE(cnwS.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n + minCycleLength);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to get min length of the source cycle chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_TRUE(mS.is_connected_cycle());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mS.g[1].ind, cnwS.g[3].ind);
    ASSERT_EQ(mS.ngs[eA].num(), 2);
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wD, eB}));
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_EQ(mS.ngs[eB].num(), 2);
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wD, eB}));
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wS, eA}));
    ASSERT_EQ(mD.ngs[eA].num(), 0);
    ASSERT_EQ(mD.ngs[eB].num(), 2);
    ASSERT_TRUE(mD.ngs[eB].contains(ESlot{wS, eA}));
    ASSERT_TRUE(mD.ngs[eB].contains(ESlot{wS, eB}));

}  //  J3_DrA_SrCyB_Srv


/// Tests degree 1 pulling through a 3-way junction joining a linear and a
/// cycle chains. Driver edge is at the free end B of the linear chain while
/// source is at the end A of the cycle chain. Preserves the component topology.
TEST_F(Pull_1, J3_DrB_SrCyA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling through a 3-way junction joining a ",
            "linear and a cycle chains\n",
            "Driver edge is at the free end B of the linear chain\n",
            "The source is at the end A of the cycle chain\n",
            "Preserves the component topology"
        );

    // Create initial graph.

    constexpr std::size_t len {6};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // create a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto eD = eB;
    const auto wD = w1;
    constexpr auto eS = eA;
    const auto wS = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
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
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_TRUE(cnwS.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n + minCycleLength);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to get min length of the source cycle chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_TRUE(mS.is_connected_cycle());
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(mS.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[3].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mS.ngs[eA].num(), 2);
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{w1, eA}));
    ASSERT_EQ(mS.ngs[eB].num(), 2);
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{w0, eA}));
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{w1, eA}));
    ASSERT_EQ(mD.ngs[eA].num(), 2);
    ASSERT_TRUE(mD.ngs[eA].contains(ESlot{wS, eA}));
    ASSERT_TRUE(mD.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_EQ(mD.ngs[eB].num(), 0);

}  //  J3_DrB_SrCyA_Srv


/// Tests degree 1 pulling through a 3-way junction joining a linear and a cycle
/// chains. Driver edge is at the free end B of the linear chain while source
/// is at the end B of the cycle chain. Preserves the component topology.
TEST_F(Pull_1, J3_DrB_SrCyB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling through a 3-way junction joining a linear",
            "and a cycle chains\n",
            "Driver edge is at the free end B of the linear chain\n",
            "The source is at the end B of the cycle chain\n",
            "Preserves the component topology"
        );

    // Create initial graph.

    constexpr std::size_t len {6};
    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0 : length (5) -> (2, 3)
    // create a linear chain w1 connected at end A to a cycle chain w0
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    constexpr auto eD = eB;
    const auto wD = w1;
    constexpr auto eS = eB;
    const auto wS = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
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
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_3way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_TRUE(cnwS.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n + minCycleLength);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to get min length of the source cycle chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_TRUE(mS.is_connected_cycle());
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mS.g[1].ind, cnwS.g[3].ind);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mS.ngs[eA].num(), 2);
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_TRUE(mS.ngs[eA].contains(ESlot{wD, eA}));
    ASSERT_EQ(mS.ngs[eB].num(), 2);
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wS, eA}));
    ASSERT_TRUE(mS.ngs[eB].contains(ESlot{wD, eA}));
    ASSERT_EQ(mD.ngs[eA].num(), 2);
    ASSERT_TRUE(mD.ngs[eA].contains(ESlot{wS, eA}));
    ASSERT_TRUE(mD.ngs[eA].contains(ESlot{wS, eB}));
    ASSERT_EQ(mD.ngs[eB].num(), 0);

}  // J3_DrB_SrCyB_Srv


// =============================================================================
// ===== Vertex Degree 4: Lin Lin Lin Lin ======================================
// =============================================================================

/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end A,
/// source edge is at tne end A of a linear chain.
/// The source chain survives.
TEST_F(Pull_1, J4_LinLin_DrA_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (5) -> (2, 3)
    // w1, w2 : length (4) -> (1, 3)
    merge22(BSlot{w1, 3},
            BSlot{w0, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w0;
    constexpr auto eS = eA;
    const auto v1 = w2;
    const auto v2 = w3;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnv2.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].c, ic);
        ASSERT_EQ(mD.g[i].w, wD);
    }

}  // J4_LinLin_DrA_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end A,
/// source edge is at tne end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_1, J4_LinLin_DrA_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "which connects four linear chains\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (5) -> (2, 3)
    // w1, w2 : length (4) -> (1, 3)
    merge22(BSlot{w1, 3},
            BSlot{w0, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
    const auto v2 = w3;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnv2.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].c, ic);
        ASSERT_EQ(mD.g[i].w, wD);
    }

}  // J4_LinLin_DrA_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end B,
/// source edge is at tne end B of a linear chain.
/// The source chain survives.
TEST_F(Pull_1, J4_LinLin_DrB_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w2 : length (6) -> (3, 3)
    // w1, w3 : length (6) -> (3, 3)
    merge22(BSlot{w0, 3},
            BSlot{w1, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w3;
    constexpr auto eS = eB;
    const auto v1 = w0;
    const auto v2 = w1;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

   const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnv2.length());
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[i].c, ic);
    }

}  // J4_LinLin_DrB_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end B,
/// source edge is at tne end A of a linear chain.
/// The source chain survives.
TEST_F(Pull_1, J4_LinLin_DrB_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w2 : length (6) -> (3, 3)
    // w1, w3 : length (6) -> (3, 3)
    merge22(BSlot{w0, 3},
            BSlot{w1, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    const auto v1 = w1;
    const auto v2 = w3;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy
    const auto cnv2 = gr.chain(v2);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(gr.chain(v2).length(), cnv2.length());
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[i].c, ic);
    }

}  // J4_LinLin_DrB_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end A,
/// source edge is at tne end A of a linear chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_LinLin_DrA_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (6) -> (3, 3)
    // w1, w2 : length (6) -> (3, 3)
    merge22(BSlot{w1, 3},
            BSlot{w0, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w0;
    constexpr auto eS = eA;
    const auto v1 = w2;
    const auto v2 = w3;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwS.g[0].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[1].c, ic);
    }

}  // J4_LinLin_DrA_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end A,
/// source edge is at tne end B of a linear chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_LinLin_DrA_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end B of a linear chain\n",
            "Source chain is engulfed completely."
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w3 : length (6) -> (3, 3)
    // w1, w2 : length (6) -> (3, 3)
    merge22(BSlot{w1, 3},
            BSlot{w0, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
    const auto v2 = w3;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwS.g[2].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[1].c, ic);
    }

}  // J4_LinLin_DrA_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end B,
/// source edge is at tne end B of a linear chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_LinLin_DrB_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting four linear chains\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w2 : length (6) -> (3, 3)
    // w1, w3 : length (6) -> (3, 3)
    merge22(BSlot{w0, 3},
            BSlot{w1, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w3;
    constexpr auto eS = eB;
    const auto v1 = w0;
    const auto v2 = w1;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());


    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(gr.chain(v2).length(), 3);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[i].c, ic);
    }

}  // J4_LinLin_DrB_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting four linear chains. Driving edge is at the chain end B,
/// source edge is at tne end A of a linear chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_LinLin_DrB_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "which connects four linear chains\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {6, 6};
    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w2 : length (6) -> (3, 3)
    // w1, w3 : length (6) -> (3, 3)
    merge22(BSlot{w0, 3},
            BSlot{w1, 3});     // creates w2, w3

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w0;
    constexpr auto eS = eA;
    const auto v1 = w1;
    const auto v2 = w3;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 4);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_TRUE(cnv1.has_single_leaf_vertex());
    ASSERT_TRUE(cnv2.has_single_leaf_vertex());
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(gr.chain(v1).length(), 3);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, wD);
        ASSERT_EQ(mD.g[i].c, ic);
    }

}  // J4_LinLin_DrB_SrLiA_Rmv

// =============================================================================
// ===== Vertex Degree 4: Lin OutCyc Lin =======================================
// =============================================================================

/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A;
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_OutCyc_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eA;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto v1 = w0;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_OutCyc_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_OutCyc_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_OutCyc_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_OutCyc_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_OutCyc_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_OutCyc_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto v1 = w0;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnv1 = gr.chain(v1);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(gr.chain(v1).length(), cnv1.length());
    ASSERT_EQ(mD.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& eg: mD.g) {
        ASSERT_EQ(eg.indw, i++);
        ASSERT_EQ(eg.w, wD);
        ASSERT_EQ(eg.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_OutCyc_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_OutCyc_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eA;
    const auto iD = gr.chain(wD).end_edge(eD).ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto v1 = w0;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());
    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, vD);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_OutCyc_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at the end B of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_OutCyc_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at the end B of a linear chain\n",
            "Pulled edge sequence omits the cycle chain\n",
            "Source chain is engulfed completely."
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_OutCyc_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_OutCyc_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled sequence omits the cycle chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto v1 = w0;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrB_OutCyc_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at the end A of a linear chain.
/// Pulled edge sequence omits the cycle chain.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_OutCyc_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at the end A of a linear chain\n",
            "Pulled edge sequence omits the cycle chain\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto iD = gr.chain(wD).end_edge(eD).ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto v1 = w0;
    constexpr int n {3};

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

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnv1.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain completely

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mD.g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, vD);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrB_OutCyc_SrLiA_Rmv


// =============================================================================
// ===== Vertex Degree 4: Lin InCyc Lin ========================================
// =============================================================================

/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_InCyAB_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_InCyAB_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_InCyBA_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_InCyBA_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_InCyAB_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_InCyAB_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// The source chain survives.
TEST_F(Pull_1, J4_DrA_InCyBA_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrA_InCyBA_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_InCyAB_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});  // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_InCyAB_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_InCyBA_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});  // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[1].ind);
    ASSERT_EQ(mS.g[0].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_InCyBA_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_InCyAB_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[0].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_InCyAB_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// The source chain survives.
TEST_F(Pull_1, J4_DrB_InCyBA_SrLiA_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "The source chain survives"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {2};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() >= n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mS = gr.chain(wS);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mS.length(), cnwS.length() - n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[4].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[1].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }
    ASSERT_EQ(mS.g[0].indw, 0);
    ASSERT_EQ(mS.g[0].w, wS);
    ASSERT_EQ(mS.g[0].c, ic);

}  // J4_DrB_InCyBA_SrLiA_Srv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_InCyAB_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eA;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, vD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: mI.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_InCyAB_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_InCyBA_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end A\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eB},
            BSlot{w0, 3});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eB},
            ESlot{w0, eB});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eA;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[2].ind);
    ASSERT_EQ(mD.g[5].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, vD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: mI.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_InCyBA_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_InCyAB_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: mI.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_InCyAB_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end A.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrA_InCyBA_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[3].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[4].ind, cnwI.g[2].ind);
    ASSERT_EQ(mD.g[5].ind, cnwI.g[1].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: mI.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrA_InCyBA_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_InCyAB_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[3].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[2].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrB_InCyAB_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end B of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_InCyBA_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end B of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::array<size_t, 2> len {7, 3};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    VertexMerger<1, 2, G> merge12 {gr};
    // w0, w2 : length (7) -> (3, 4)
    merge12(ESlot{w0, eA},
            BSlot{w0, 4});     // creates w2

    VertexMerger<1, 3, G> merge13 {gr};

    merge13(ESlot{w1, eA},
            ESlot{w2, eA});

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w1;
    constexpr auto eD = eB;
    const auto wS = w2;
    constexpr auto eS = eB;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&gr.chain(wD).end_edge(eD), eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto& mD = gr.chain(wD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.length(), cnwD.length() + n);
    ASSERT_EQ(mI.length(), cnwI.length());
    ASSERT_EQ(mD.g[0].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[2].ind);
    ASSERT_EQ(mD.g[2].ind, cnwI.g[3].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[3].ind, cnwI.g[0].ind);
    for (EgIdA i {}; const auto& g: mD.g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wD);
        ASSERT_EQ(g.c, ic);
    }
    for (EgIdA i {}; const auto& g: gr.chain(wI).g) {
        ASSERT_EQ(g.indw, i++);
        ASSERT_EQ(g.w, wI);
        ASSERT_EQ(g.c, ic);
    }

}  // J4_DrB_InCyBA_SrLiB_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction A -> B.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_InCyAB_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction A->B\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eA), eA},
        Driver {&gr.chain(wI).end_edge(eB), eB}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.g[2].c, ic);
    ASSERT_EQ(mI.length(), 4);
    ASSERT_EQ(mD.length(), 6);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwI.g[0].ind);
    ASSERT_EQ(mD.g[1].ind, cnwI.g[1].ind);
    ASSERT_EQ(mD.g[0].ind, cnwI.g[2].ind);
    ASSERT_EQ(mI.g[3].ind, cnwS.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[0].ind, cnwI.g[3].ind);
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, vD);
        ASSERT_EQ(mD.g[i].c, ic);
    }
    for (std::size_t i {}; i<mI.length(); ++i) {
        ASSERT_EQ(mI.g[i].indw, i);
        ASSERT_EQ(mI.g[i].w, wI);
        ASSERT_EQ(mI.g[i].c, ic);
    }

}  // J4_DrB_InCyAB_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 4-way junction
/// connecting two linear chains and a cycle.
/// Driving edge is at the chain end B.
/// Source edge is at tne end A of a linear chain.
/// Pulled edge sequence includes the cycle chain in direction B -> A.
/// Source chain is engulfed completely.
TEST_F(Pull_1, J4_DrB_InCyBA_SrLiA_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 4-way junction ",
            "connecting two linear chains and a cycle\n",
            "Driving edge is at the chain end B\n",
            "Source edge is at tne end A of a linear chain\n",
            "Pulled edge sequence includes the cycle chain in direction B->A\n",
            "Source chain is engulfed completely"
        );

    // Create initial graph.

    constexpr std::size_t len {10};
    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    gr.add_single_chain_component(len);

    VertexMerger<2, 2, G> merge22 {gr};
    // w0, w1, w2 : length (10) -> (3, 3, 4)
    merge22(BSlot{w0, 3},
            BSlot{w0, 7});     // creates w1, w2

    if constexpr (profuse)
        gr.print_components(tagBefore);

    // Define a sequence of edges to pull.

    const auto wD = w2;
    constexpr auto eD = eB;
    const auto& egD = gr.chain(wD).end_edge(eD);
    const auto iD = egD.ind;
    const auto wS = w1;
    constexpr auto eS = eA;
    const auto wI = w0;
    constexpr int n {3};

    const Driver drv {&egD, eD};
    const Source src {wS, eS};
    const std::vector<Driver> internals {
        Driver {&gr.chain(wI).end_edge(eB), eB},
        Driver {&gr.chain(wI).end_edge(eA), eA}
    };
    Path pp {&gr.compt(src.w), drv, src, internals};

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
    const auto cnwI = gr.chain(wI);  // copy
    const auto cnwQ = gr.chain(w0);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 3);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eB);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eA);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_4way_end(), End::opp(eS));
    ASSERT_TRUE(cnwI.is_connected_cycle());
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 1);
    ASSERT_FALSE(c.template check<profuse>());

    const auto vD = gr.get_egl(iD).w;
    const auto& mD = gr.chain(vD);
    const auto& mI = gr.chain(wI);
    ASSERT_EQ(mD.g[1].c, ic);
    ASSERT_EQ(mD.g[2].c, ic);
    ASSERT_EQ(mI.length(), 4);
    ASSERT_EQ(mD.length(), 6);
    ASSERT_EQ(mD.g[5].ind, cnwD.g[2].ind);
    ASSERT_EQ(mD.g[4].ind, cnwD.g[1].ind);
    ASSERT_EQ(mD.g[3].ind, cnwD.g[0].ind);
    ASSERT_EQ(mD.g[2].ind, cnwQ.g[3].ind);
    ASSERT_EQ(mD.g[1].ind, cnwQ.g[2].ind);
    ASSERT_EQ(mD.g[0].ind, cnwQ.g[1].ind);
    ASSERT_EQ(mI.g[3].ind, cnwQ.g[0].ind);
    ASSERT_EQ(mI.g[2].ind, cnwS.g[2].ind);
    ASSERT_EQ(mI.g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(mI.g[0].ind, cnwS.g[0].ind);
    for (std::size_t i {}; i<mI.length(); ++i) {
        ASSERT_EQ(mI.g[i].indw, i);
        ASSERT_EQ(mI.g[i].w, wI);
    }
    for (std::size_t i {}; i<mD.length(); ++i) {
        ASSERT_EQ(mD.g[i].indw, i);
        ASSERT_EQ(mD.g[i].w, vD);
    }
}  // J4_DrB_InCyBA_SrLiA_Rmv


/// Tests degree 1 pulling over a path traversing a 3-way and two 4-way
/// junctions. Pulling is driven at chain end A to source end B.
/// The source chain survives.
TEST_F(Pull_1, J344_DrA_SrLiB_Srv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way and ",
            "two 4-way junctions\n",
            "Pulling is driven at chain end A to source end B\n",
            "The source chain survives"
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

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w7;
    constexpr auto eS = eB;
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
    const auto num_vertices = c0.num_vertices();
    const auto ww = c0.ww;  // copy
    const auto chis = c0.chis;  // copy
    const auto cnwD = gr.chain(wD);  // copy
    const auto cnwS = gr.chain(wS);  // copy
    const auto cnw0 = gr.chain(w0);  // copy
    const auto cnw4 = gr.chain(w4);  // copy
    const auto cnw6 = gr.chain(w6);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 8);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() > n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull two steps to leave source edge in the original chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains);
    ASSERT_EQ(c.num_vertices(), num_vertices);
    ASSERT_EQ(c.ww, ww);
    ASSERT_EQ(c.chis, chis);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), 3);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(w6).length(), 2);
    ASSERT_EQ(gr.chain(wS).length(), 1);
    ASSERT_EQ(gr.chain(wD).g[1].ind, cnw4.g[0].ind);
    ASSERT_EQ(gr.chain(wD).g[2].ind, cnw6.g[0].ind);
    ASSERT_EQ(gr.chain(w4).g[0].ind, cnw6.g[1].ind);
    ASSERT_EQ(gr.chain(w6).g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(gr.chain(w6).g[1].ind, cnwS.g[1].ind);
    ASSERT_EQ(gr.chain(wD).g[1].indc, cnw4.g[0].indc);
    ASSERT_EQ(gr.chain(wD).g[2].indc, cnw6.g[0].indc);
    ASSERT_EQ(gr.chain(w4).g[0].indc, cnw6.g[1].indc);
    ASSERT_EQ(gr.chain(w6).g[0].indc, cnwS.g[0].indc);
    ASSERT_EQ(gr.chain(w6).g[1].indc, cnwS.g[1].indc);
    ASSERT_EQ(gr.chain(wD).g[1].indw, 1);
    ASSERT_EQ(gr.chain(wD).g[2].indw, 2);
    ASSERT_EQ(gr.chain(w4).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w6).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w6).g[1].indw, 1);
    ASSERT_EQ(gr.chain(wD).g[1].w, wD);
    ASSERT_EQ(gr.chain(wD).g[2].w, wD);
    ASSERT_EQ(gr.chain(w4).g[0].w, w4);
    ASSERT_EQ(gr.chain(w6).g[0].w, w6);
    ASSERT_EQ(gr.chain(w6).g[1].w, w6);
    ASSERT_EQ(gr.chain(wD).g[1].c, ic);
    ASSERT_EQ(gr.chain(wD).g[2].c, ic);
    ASSERT_EQ(gr.chain(w4).g[0].c, ic);
    ASSERT_EQ(gr.chain(w6).g[0].c, ic);
    ASSERT_EQ(gr.chain(w6).g[1].c, ic);

}  // J344_DrA_SrLiB_Srv


/// Tests degree 1 pulling over a path traversing a 3-way and
/// two 4-way junctions. Source chain is engulfed completely.
TEST_F(Pull_1, J344_DrA_SrLiB_Rmv)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling over a path traversing a 3-way and ",
            "two 4-way junctions\n",
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

    const auto wD = w1;
    constexpr auto eD = eA;
    const auto wS = w7;
    constexpr auto eS = eB;
    constexpr int n {3};

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
    const auto cnw0 = gr.chain(w0);  // copy
    const auto cnw4 = gr.chain(w4);  // copy
    const auto cnw6 = gr.chain(w6);  // copy

    // Check initial state.

    ASSERT_EQ(num_chains, 8);
    ASSERT_EQ(pp.driver_chid(), wD);
    ASSERT_EQ(pp.driver_chain_end(), eA);
    ASSERT_EQ(cnwD.get_single_leaf_end(), eD);
    ASSERT_EQ(cnwD.get_single_4way_end(), End::opp(eD));
    ASSERT_EQ(pp.source_chid(), wS);
    ASSERT_EQ(pp.source_end(), eB);
    ASSERT_EQ(cnwS.get_single_leaf_end(), eS);
    ASSERT_EQ(cnwS.get_single_3way_end(), End::opp(eS));
    ASSERT_TRUE(cnwS.length() == n);
    ASSERT_FALSE(c0.template check<profuse>());

    // Do the transformation.

    PullForw pu1f {gr};
    pu1f(pp, n);  // pull three steps to consume the source chain

    if constexpr (profuse)
        gr.print_components(tagAfter);

    // Compare the result to the expectation.

    const auto& c = gr.compt(ic);
    ASSERT_EQ(c.num_edges(), num_edges);
    ASSERT_EQ(c.num_chains(), num_chains - 2);
    ASSERT_FALSE(c.template check<profuse>());

    ASSERT_EQ(gr.chain(wD).length(), 4);
    ASSERT_EQ(gr.chain(w3).length(), 5);
    ASSERT_EQ(gr.chain(w4).length(), 1);
    ASSERT_EQ(gr.chain(wD).g[0].ind, cnwD.g[0].ind);
    ASSERT_EQ(gr.chain(wD).g[1].ind, cnw4.g[0].ind);
    ASSERT_EQ(gr.chain(wD).g[2].ind, cnw6.g[0].ind);
    ASSERT_EQ(gr.chain(wD).g[3].ind, cnw6.g[1].ind);
    ASSERT_EQ(gr.chain(w4).g[0].ind, cnwS.g[0].ind);
    ASSERT_EQ(gr.chain(w3).g[0].ind, cnwS.g[1].ind);
    ASSERT_EQ(gr.chain(w3).g[1].ind, cnwS.g[2].ind);
    ASSERT_EQ(gr.chain(wD).g[0].indw, 0);
    ASSERT_EQ(gr.chain(wD).g[1].indw, 1);
    ASSERT_EQ(gr.chain(wD).g[2].indw, 2);
    ASSERT_EQ(gr.chain(wD).g[3].indw, 3);
    ASSERT_EQ(gr.chain(w4).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w3).g[0].indw, 0);
    ASSERT_EQ(gr.chain(w3).g[1].indw, 1);
    ASSERT_EQ(gr.chain(wD).g[0].w, wD);
    ASSERT_EQ(gr.chain(wD).g[1].w, wD);
    ASSERT_EQ(gr.chain(wD).g[2].w, wD);
    ASSERT_EQ(gr.chain(wD).g[3].w, wD);
    ASSERT_EQ(gr.chain(w4).g[0].w, w4);
    ASSERT_EQ(gr.chain(w3).g[0].w, w3);
    ASSERT_EQ(gr.chain(w3).g[1].w, w3);
    ASSERT_EQ(gr.chain(wD).g[0].c, ic);
    ASSERT_EQ(gr.chain(wD).g[1].c, ic);
    ASSERT_EQ(gr.chain(wD).g[2].c, ic);
    ASSERT_EQ(gr.chain(wD).g[3].c, ic);
    ASSERT_EQ(gr.chain(w4).g[0].c, ic);
    ASSERT_EQ(gr.chain(w3).g[0].c, ic);
    ASSERT_EQ(gr.chain(w3).g[1].c, ic);

}  // J344_DrA_SrLiB_Rmv


}  // namespace graffine::tests::pulling::d1