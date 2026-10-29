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

#include "pulling_hooks.h"

#include <memory>

namespace graffine::tests::pulling::bulk_source {

using hooks::Maker;
using hooks::Pullee;
using hooks::Recorder;
using hooks::check_hooks;
using hooks::make_two_junctions;
using hooks::w1;
using hooks::w3;
using hooks::w4;

template<Degree D>
using Pull = trs::pulling::On<D, Orientation::Forwards, G>;

/// Expects vertex \p v to be at one of the ends of edge \p i.
void expect_at(const G& gr, const EgIdG i, const G::Vertex& v)
{
    const auto& eg = gr.edge(i);
    EXPECT_TRUE(*eg.vertex_at_inner(eA) == v ||
                *eg.vertex_at_inner(eB) == v) << "edge " << i;
}

/// Linear chain w1 (2), free at end A, joined at its end B to both ends of
/// a cycle w0 of length \p len - 2.
void make_lollipop(G& gr, const std::size_t len)
{
    gr.add_single_chain_component(len);
    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{ChIdG{0}, eB}, BSlot{ChIdG{0}, 2});
}

constexpr ChIdG w0 {0};


class PullingBulkSourceTest
    : public Test {};

// =============================================================================


/**
 * Degree 1 pull to a degree 2 source inside a linear chain between two
 * junctions, until the junction reaches the source.
 */
TEST_F(PullingBulkSourceTest, Deg1_LinearBetweenJunctions)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling to a degree 2 source inside a linear chain ",
            "between two junctions");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w1);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {BSlot {w3, 2}}};
    };

    {   // the path and the applicability
        Pullee s;
        make(s);
        ASSERT_TRUE(s.path.src().is_bulk());
        ASSERT_EQ(s.path.length(), 4);
        ASSERT_EQ(s.path.length_over_source_chain(), 2);
        ASSERT_TRUE(s.path.is_contiguous());
        EXPECT_TRUE(*s.path.vertices().back() ==
                    *s.gr.chain(w3).vertex_at_bulk(EgIdA{2}));

        Pull<Deg1> pull {s.gr};
        EXPECT_TRUE(pull.applicability(s.path, 1));
        EXPECT_TRUE(pull.applicability(s.path, 2));
        EXPECT_FALSE(pull.applicability(s.path, 3));
    }

    for (int n {1}; n <= 2; ++n)
        check_hooks<Deg1>(make, n);

    {   // one step: the junction moves one edge towards the source
        Pullee s;
        make(s);
        const auto len1 = s.gr.chain(w1).length();
        const auto iL = s.gr.chain(w3).g[1].ind;  // edges next to the source
        const auto iR = s.gr.chain(w3).g[2].ind;

        Pull<Deg1> {s.gr}(s.path, 1);

        ASSERT_TRUE(s.path.src().is_bulk());
        EXPECT_EQ(s.path.src().w, w3);
        EXPECT_EQ(s.path.src().pos, 1);
        EXPECT_EQ(s.gr.chain(w1).length(), len1 + 1);
        EXPECT_EQ(s.gr.chain(w3).length(), 2);
        EXPECT_EQ(s.path.length_over_source_chain(), 1);
        const auto& vS = *s.path.vertices().back();
        EXPECT_EQ(vS.deg(), Deg2);
        expect_at(s.gr, iL, vS);
        expect_at(s.gr, iR, vS);
    }
    {   // two steps: the junction reaches the source
        Pullee s;
        make(s);
        const auto iR = s.gr.chain(w3).g[2].ind;

        Pull<Deg1> pull {s.gr};
        pull(s.path, 2);

        // the source is the end at the junction of the chain hosting the
        // last path edge, here the driver chain
        EXPECT_FALSE(s.path.src().is_bulk());
        EXPECT_EQ(s.path.src().w, w1);
        EXPECT_EQ(s.path.src().e, eB);
        EXPECT_EQ(s.gr.chain(w3).length(), 1);
        EXPECT_EQ(s.gr.chain(w3).g[0].ind, iR);
        EXPECT_EQ(s.path.length_over_source_chain(), s.path.length());
        const auto& vS = *s.path.vertices().back();
        EXPECT_EQ(vS.deg(), Deg3);
        EXPECT_TRUE(vS == s.path.source_vertex());
        expect_at(s.gr, iR, vS);
        EXPECT_FALSE(s.gr.compt(CmpId{}).template check<false>());

        // the source is exhausted
        EXPECT_FALSE(pull.applicability(s.path, 1));
    }
}


/// Degree 2 pull to a degree 2 source inside a linear chain having a free end.
TEST_F(PullingBulkSourceTest, Deg2_LinearAfterJunction)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling to a degree 2 source past a junction");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.g[1], eA},
                       Source {BSlot {w4, 2}}};
    };

    Pullee s;
    make(s);
    ASSERT_EQ(s.path.length_over_source_chain(), 2);
    Pull<Deg2> pull {s.gr};
    EXPECT_TRUE(pull.applicability(s.path, 2));
    EXPECT_FALSE(pull.applicability(s.path, 3));

    for (int n {1}; n <= 2; ++n)
        check_hooks<Deg2>(make, n);
}


/// Degree 2 pull to a degree 2 source in the driver chain.
TEST_F(PullingBulkSourceTest, Deg2_SameChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling to a degree 2 source in the driver chain");

    // w0 (1) and w1 (2) joined at the end A of w2 (5)
    const Maker make = [](Pullee& s) {
        s.gr.add_single_chain_component(6);
        s.gr.add_single_chain_component(2);
        VertexMerger<1, 2, G> merge12 {s.gr};
        merge12(ESlot{ChIdG{1}, eB}, BSlot{ChIdG{0}, 1});
        const auto& m = s.gr.chain(ChIdG{2});
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.g[1], eA},
                       Source {BSlot {ChIdG{2}, 4}}};
    };

    Pullee s;
    make(s);
    ASSERT_EQ(s.path.length(), 3);
    ASSERT_EQ(s.path.length_over_source_chain(), 3);
    Pull<Deg2> pull {s.gr};
    EXPECT_TRUE(pull.applicability(s.path, 3));
    EXPECT_FALSE(pull.applicability(s.path, 4));

    for (int n {1}; n <= 3; ++n)
        check_hooks<Deg2>(make, n);

    // a single-edge path from the driver to the source is not pulled
    Pullee t;
    t.gr.add_single_chain_component(4);
    t.path = Path {&t.gr.compt(CmpId{}), Driver {&t.gr.chain(w0).g[2], eA},
                   Source {BSlot {w0, 3}}};
    ASSERT_EQ(t.path.length(), 1);
    EXPECT_FALSE(Pull<Deg2> {t.gr}.applicability(t.path, 1));
}


/// Degree 3 pull to a degree 2 source, in the driver chain and past a junction.
TEST_F(PullingBulkSourceTest, Deg3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 3 pulling to a degree 2 source");

    const Maker same = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {BSlot {w3, 2}}};
    };
    const Maker past = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {BSlot {w4, 2}}};
    };

    for (const auto& make : {same, past}) {
        Pullee s;
        make(s);
        ASSERT_EQ(s.path.length_over_source_chain(), 2);
        Pull<Deg3> pull {s.gr};
        EXPECT_TRUE(pull.applicability(s.path, 2));
        EXPECT_FALSE(pull.applicability(s.path, 3));

        for (int n {1}; n <= 2; ++n)
            check_hooks<Deg3>(make, n);
    }
}


/// Degree 1 pull to a degree 2 source inside a connected cycle.
TEST_F(PullingBulkSourceTest, ConnectedCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 1 pulling to a degree 2 source inside a connected cycle");

    // cycle w0 (6): both routes to position 3 are 3 edges long
    const Maker make = [](Pullee& s) {
        make_lollipop(s.gr, 8);
        const auto& m = s.gr.chain(w1);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {BSlot {w0, 3}}};
    };

    {
        Pullee s;
        make(s);
        ASSERT_TRUE(s.gr.chain(w0).is_connected_cycle());
        ASSERT_EQ(s.gr.chain(w0).length(), 6);
        ASSERT_EQ(s.path.length_over_source_chain(), 3);
        Pull<Deg1> pull {s.gr};
        EXPECT_TRUE(pull.applicability(s.path, 3));
        EXPECT_FALSE(pull.applicability(s.path, 4));

        pull(s.path, 3);

        EXPECT_TRUE(s.gr.chain(w0).is_connected_cycle());
        EXPECT_EQ(s.gr.chain(w0).length(), 3);
        EXPECT_EQ(s.path.vertices().back()->deg(), Deg3);
        EXPECT_FALSE(pull.applicability(s.path, 1));
    }
    for (int n {1}; n <= 3; ++n)
        check_hooks<Deg1>(make, n);
}


/// Rejection of a pull which would shrink a cycle below its minimal length.
TEST_F(PullingBulkSourceTest, CycleNotShrinkable)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rejection of pulling which would make a cycle too short");

    // cycle w0 (4), reached over its end A at position 3: 3 edges of the
    // path are over the cycle, but it can only lose 2 of them
    const Maker make = [](Pullee& s) {
        make_lollipop(s.gr, 6);
        const auto& l = s.gr.chain(w1);
        const auto& m = s.gr.chain(w0);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&l.end_edge(eA), eA},
                       Source {BSlot {w0, 3}},
                       Path::PathW {l.g[0].ind, l.g[1].ind,
                                    m.g[0].ind, m.g[1].ind, m.g[2].ind}};
    };

    Pullee s;
    make(s);
    ASSERT_TRUE(s.path.is_contiguous());
    ASSERT_EQ(s.gr.chain(w0).length(), 4);
    ASSERT_EQ(s.path.length_over_source_chain(), 3);
    ASSERT_TRUE(s.gr.chain(w0).is_shrinkable_by(2));
    ASSERT_FALSE(s.gr.chain(w0).is_shrinkable_by(3));

    Pull<Deg1> pull {s.gr};
    EXPECT_TRUE(pull.applicability(s.path, 2));
    const auto r = pull.applicability(s.path, 3);
    EXPECT_FALSE(r);

    pull(s.path, 2);

    EXPECT_TRUE(s.gr.chain(w0).is_connected_cycle());
    EXPECT_EQ(s.gr.chain(w0).length(), minCycleLength);
    EXPECT_TRUE(s.path.src().is_bulk());
    EXPECT_FALSE(pull.applicability(s.path, 1));
    EXPECT_FALSE(s.gr.compt(CmpId{}).template check<false>());

    for (int n {1}; n <= 2; ++n)
        check_hooks<Deg1>(make, n);
}


/// Rejection of a degree 2 source inside an unconnected cycle.
TEST_F(PullingBulkSourceTest, UnconnectedCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rejection of pulling inside an unconnected cycle");

    // Only a degree 2 driver can share the component of a degree 2 source
    // inside an unconnected cycle; degree 2 pulls exclude unconnected cycles.
    G gr;
    gr.add_single_chain_component(5);
    VertexMerger<1, 1, G> merge11 {gr};
    merge11(ESlot{w0, eA}, ESlot{w0, eB});
    ASSERT_TRUE(gr.chain(w0).is_unconnected_cycle());

    Path path {&gr.compt(CmpId{}), Driver {&gr.chain(w0).g[1], eA},
               Source {BSlot {w0, 4}}};

    EXPECT_FALSE(Pull<Deg2> {gr}.applicability(path, 1));
    EXPECT_FALSE(trs::pulling::Functor<G> {gr}.applicability(path, 1));
}


/// Rejection of degree 2 sources not at the path end, or not inside a chain.
TEST_F(PullingBulkSourceTest, InvalidSource)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests rejection of invalid degree 2 sources");

    Pullee s;
    make_two_junctions(s.gr);
    const auto& m = s.gr.chain(w1);
    s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                   Source {BSlot {w3, 2}}};
    Pull<Deg1> pull {s.gr};
    ASSERT_TRUE(pull.applicability(s.path, 1));

    s.path.set_src(Source {BSlot {w3, 1}});  // not at the path end
    EXPECT_FALSE(pull.applicability(s.path, 1));

    s.path.set_src(Source {BSlot {w3, 0}});  // at a chain end
    EXPECT_FALSE(pull.applicability(s.path, 1));

    s.path.set_src(Source {BSlot {w3, 3}});  // at a chain end
    EXPECT_FALSE(pull.applicability(s.path, 1));
}

}  // namespace graffine::tests::pulling::bulk_source
