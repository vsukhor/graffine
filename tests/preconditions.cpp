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
#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/edge_split/functor.h"
#include "graffine/transforms/pulling/functor.h"
#include "graffine/transforms/vertex_split/functor.h"

#include <algorithm>
#include <string>
#include <vector>

namespace graffine::tests::preconditions {

namespace trs = graffine::transforms;

using Compt = G::Compt;
using Path = trs::pulling::Path<Compt>;
using Driver = Path::Driver;
using Source = Path::Source;
using EgDescr = G::Vertex::EgDescr;

class PreconditionsTest
    : public Test {

protected:

    static constexpr ChIdG w0 {0};
    static constexpr ChIdG w1 {1};
    static constexpr ChIdG w2 {2};
    static constexpr auto eA = End::A;
    static constexpr auto eB = End::B;

    /**
     * Expects transformation \p f to reject \p args for \p what: through
     * is_applicable, try_apply and operator(), leaving the graph unchanged.
     */
    template<typename F,
             typename... A>
    static void expect_rejected(
        const G& gr,
        F& f,
        const std::string& what,
        A&... args
    )
    {
        const Snapshot before {gr};

        EXPECT_FALSE(f.is_applicable(args...));

        const auto res = f.try_apply(args...);
        ASSERT_FALSE(res.has_value());
        EXPECT_NE(std::string {res.error().reason}.find(what), std::string::npos)
            << "reason: " << res.error().reason;

        EXPECT_THROW(f(args...), graffine::precondition_error);

        EXPECT_TRUE(Snapshot {gr} == before) << "graph changed by a rejection";
    }

    /// Vertex at end \p e of chain \p w.
    static auto end_vertex(G& gr, const ChIdG w, const End::Id e) -> G::Vertex&
    {
        return *gr.chain(w).vertex_at_end(e);
    }

    /// Vertex at bulk position \p a of chain \p w.
    static auto bulk_vertex(G& gr, const ChIdG w, const std::size_t a)
        -> G::Vertex&
    {
        return *gr.chain(w).vertex_at_bulk(EgIdA {a});
    }
};

// =============================================================================


/// Merger of two vertices of degree 1.
TEST_F(PreconditionsTest, VertexMerger11)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the preconditions of vertex merger 1+1");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(1);
    gr.add_single_chain_component(3);
    trs::vertex_merger::From<Deg1, Deg1, G> merge {gr};

    ESlot invalid {ChIdG {7}, eA};
    ESlot s0A {w0, eA};
    ESlot s0B {w0, eB};
    ESlot s1A {w1, eA};
    expect_rejected(gr, merge, "invalid end slot", invalid, s1A);
    expect_rejected(gr, merge, "with itself", s1A, s1A);
    expect_rejected(gr, merge, "minimal cycle length", s0A, s0B);

    // A vertex of another graph.
    G other;
    other.add_single_chain_component(2);
    expect_rejected(gr, merge, "not in the graph",
                    end_vertex(other, w0, eA), end_vertex(gr, w1, eA));

    EXPECT_TRUE(merge.is_applicable(s0B, s1A));
    ASSERT_TRUE(merge.try_apply(s0B, s1A).has_value());
    EXPECT_EQ(gr.num_chains(), 1);

    // The ends are now connected.
    ESlot s1B {w0, eB};
    EXPECT_TRUE(merge.is_applicable(s0A, s1B));
}


/// Merger of vertices of degree 1 and 2.
TEST_F(PreconditionsTest, VertexMerger12)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the preconditions of vertex merger 1+2");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);
    trs::vertex_merger::From<Deg1, Deg2, G> merge {gr};

    ESlot s0A {w0, eA};
    BSlot b1 {w0, 1};
    BSlot b2 {w0, 2};
    BSlot b4 {w0, 4};
    ChIdG w = w0;
    expect_rejected(gr, merge, "minimal cycle length", s0A, b1);
    expect_rejected(gr, merge, "invalid bulk slot", s0A, b4);
    expect_rejected(gr, merge, "not an unconnected cycle", s0A, w);
    expect_rejected(gr, merge, "degrees are not 1 and 2",
                    end_vertex(gr, w0, eA), end_vertex(gr, w0, eB));

    // A loop of two edges is long enough.
    ASSERT_TRUE(merge.try_apply(s0A, b2).has_value());
    EXPECT_EQ(gr.num_vertices(Deg3), 1);
}


/// Merger of vertices of degree 1 and 3, and degree 2 and 2.
TEST_F(PreconditionsTest, VertexMerger13And22)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the preconditions of vertex mergers 1+3 and 2+2");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);
    gr.add_single_chain_component(5);

    trs::vertex_merger::From<Deg1, Deg3, G> merge13 {gr};
    ESlot s0A {w0, eA};
    ESlot s1A {w1, eA};
    expect_rejected(gr, merge13, "not at a 3-way junction", s0A, s1A);

    trs::vertex_merger::From<Deg2, Deg2, G> merge22 {gr};
    BSlot b11 {w1, 1};
    BSlot b12 {w1, 2};
    BSlot b13 {w1, 3};
    expect_rejected(gr, merge22, "with itself", b11, b11);
    expect_rejected(gr, merge22, "minimal cycle length", b11, b12);
    expect_rejected(gr, merge22, "with itself",
                    bulk_vertex(gr, w1, 2), bulk_vertex(gr, w1, 2));

    ASSERT_TRUE(merge22.try_apply(b11, b13).has_value());
    EXPECT_EQ(gr.num_vertices(Deg4), 1);
}


/// Degree-independent vertex merger, and the low-level merger core.
TEST_F(PreconditionsTest, VertexMergerFunctorAndCore)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the preconditions of the vertex merger functor and core");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);
    gr.add_single_chain_component(3);
    trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot{w1, eA}, BSlot{w0, 2});

    trs::vertex_merger::Functor<G> merge {gr};
    auto& junction = end_vertex(gr, w1, eA);
    ASSERT_EQ(junction.deg(), Deg3);
    expect_rejected(gr, merge, "not implemented",
                    junction, bulk_vertex(gr, w1, 1));

    trs::vertex_merger::Core<G> core {gr, nullptr, "core"};
    EXPECT_THROW(core.to_cycle(w1), graffine::precondition_error);
    EXPECT_THROW(core.to_cycle(ChIdG {9}), graffine::precondition_error);
    EXPECT_THROW(core.antiparallel(eA, w1, w1), graffine::precondition_error);
}


/// Vertex splits.
TEST_F(PreconditionsTest, VertexSplit)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the preconditions of vertex splits");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);
    gr.add_single_chain_component(3);
    trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot{w1, eA}, BSlot{w0, 2});

    trs::vertex_split::Functor<G> split {gr};
    auto& junction = end_vertex(gr, w1, eA);
    std::vector<EgDescr> none;
    expect_rejected(gr, split, "number of edges", junction, none);

    trs::vertex_split::To<Deg2, Deg2, G> split22 {gr};
    ESlot s1A {w1, eA};
    ESlot s1B {w1, eB};
    expect_rejected(gr, split22, "not at a vertex of degree 4", s1A, s1B);

    trs::vertex_split::To<Deg1, Deg1, G> split11 {gr};
    BSlot b {w1, 0};
    expect_rejected(gr, split11, "vertex degree is not 2", b);

    std::vector<EgDescr> first {*junction.edges().begin()};
    ASSERT_TRUE(split.try_apply(junction, first).has_value());
    EXPECT_EQ(gr.num_vertices(Deg3), 0);
}


/// Edge creation.
TEST_F(PreconditionsTest, EdgeCreation)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the preconditions of edge creation");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);

    trs::edge_creation::InNewChain<Deg3, G> branch3 {gr};
    ESlot s0A {w0, eA};
    expect_rejected(gr, branch3, "not at a vertex of degree 3", s0A);

    trs::edge_creation::InNewChain<Deg2, G> branch2 {gr};
    ChIdG w = w0;
    expect_rejected(gr, branch2, "not an unconnected cycle", w);

    trs::edge_creation::InExistingChain<G> extend {gr};
    BSlot b0 {w0, 0};
    expect_rejected(gr, extend, "invalid bulk slot", b0);

    BSlot b1 {w0, 1};
    ASSERT_TRUE(branch2.try_apply(b1).has_value());
    ASSERT_TRUE(extend.try_apply(s0A).has_value());
    EXPECT_EQ(gr.num_edges(), 5);
}


/// Edge deletion, split and merger.
TEST_F(PreconditionsTest, EdgeDeletionSplitMerger)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the preconditions of edge deletion, split and merger");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(1);
    gr.add_single_chain_component(3);

    trs::edge_deletion::Functor<G> del {gr};
    EgIdG invalid {99};
    EgIdG single {gr.chain(w0).g[0].ind};
    expect_rejected(gr, del, "invalid edge index", invalid);
    expect_rejected(gr, del, "not shrinkable", single);

    trs::edge_split::Functor<G> split {gr};
    expect_rejected(gr, split, "invalid edge index", invalid);

    trs::edge_merger::Functor<G> merge {gr};
    EgIdG first {gr.chain(w1).g[0].ind};
    EgIdG last {gr.chain(w1).g[2].ind};
    expect_rejected(gr, merge, "not adjacent", first, last);
    expect_rejected(gr, merge, "with itself", first, first);

    EgIdG second {gr.chain(w1).g[1].ind};
    ASSERT_TRUE(merge.try_apply(first, second).has_value());
    ASSERT_TRUE(split.try_apply(single).has_value());
    EXPECT_EQ(gr.num_edges(), 4);
}


/// Component creation and deletion.
TEST_F(PreconditionsTest, Components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the preconditions of component creation and deletion");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(2);

    trs::component_creation::Functor<G> create {gr};
    std::size_t zero {};
    expect_rejected(gr, create, "no edges", zero);

    trs::component_deletion::Functor<G> remove {gr};
    CmpId invalid {5};
    expect_rejected(gr, remove, "invalid component index", invalid);

    ASSERT_TRUE(create.try_apply().has_value());
    EXPECT_EQ(gr.num_compts(), 2);
    ASSERT_TRUE(remove.try_apply(CmpId {0}).has_value());
    EXPECT_EQ(gr.num_compts(), 1);
}


/// Pulling.
TEST_F(PreconditionsTest, Pulling)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the preconditions of pulling");

    constexpr std::size_t len {5};

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(len);
    Path path {&gr.compt(w0), Driver {&gr.chain(w0).tail(), eA}, Source {w0, eB}};

    trs::pulling::On<Deg1, Orientation::Forwards, G> pull1 {gr};
    int zero {};
    int tooMany {static_cast<int>(len) + 1};
    expect_rejected(gr, pull1, "steps is not positive", path, zero);
    expect_rejected(gr, pull1, "too short", path, tooMany);

    // A path over another graph.
    G other;
    other.add_single_chain_component(len);
    Path foreign {&other.compt(w0),
                  Driver {&other.chain(w0).tail(), eA}, Source {w0, eB}};
    int two {2};
    expect_rejected(gr, pull1, "component of the graph", foreign, two);

    trs::pulling::Functor<G> pull {gr};
    expect_rejected(gr, pull, "component of the graph", foreign, two);
    EXPECT_TRUE(pull.is_applicable(path, two));
    ASSERT_TRUE(pull.try_apply(path, two).has_value());
}

/// Vertex-based overloads act on the given vertex in chains of edges having
/// opposite orientations.
TEST_F(PreconditionsTest, VertexOverloadsInMixedChains)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that vertex-based overloads act on the given vertex in ",
            "chains of edges having opposite orientations");

    // A chain of 7 edges: < < < < > > >
    auto make = [](G& gr) {
        gr.add_single_chain_component(4);
        gr.add_single_chain_component(3);
        trs::vertex_merger::Core<G> {gr, nullptr, "c"}.antiparallel(eA, w0, w1);
    };
    constexpr std::size_t len {7};

    auto sorted_lengths = [](const G& gr) {
        std::vector<std::size_t> l;
        for (ChIdG w {}; w < gr.num_chains(); ++w)
            l.push_back(gr.chain(w).length());
        std::ranges::sort(l);
        return l;
    };
    auto expected = [](std::vector<std::size_t> l) {
        std::ranges::sort(l);
        return l;
    };

    for (std::size_t a {1}; a < len; ++a) {
        SCOPED_TRACE("bulk position " + std::to_string(a));
        {
            G gr;
            const IntegrityCheck integrityCheck {gr};
            make(gr);
            trs::vertex_split::To<Deg1, Deg1, G> split {gr};
            ASSERT_TRUE(split.try_apply(bulk_vertex(gr, w0, a)).has_value());
            EXPECT_EQ(sorted_lengths(gr), expected({a, len - a}));
        }
        {
            G gr;
            const IntegrityCheck integrityCheck {gr};
            make(gr);
            gr.add_single_chain_component(2);
            const auto wL = gr.ind_last_chain();
            trs::vertex_merger::From<Deg1, Deg2, G> merge {gr};
            ASSERT_TRUE(merge.try_apply(end_vertex(gr, wL, eA),
                                        bulk_vertex(gr, w0, a)).has_value());
            EXPECT_EQ(sorted_lengths(gr), expected({a, len - a, 2}));
        }
    }
}

}  // namespace graffine::tests::preconditions
