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

/*
 * Degree 2 pulls driven from a vertex of an unconnected cycle, to a degree 2
 * source inside the cycle or to its boundary vertex. The split at the driver
 * opens the cycle, and the merger after the first step closes it into a loop
 * one edge shorter, the driver edge becoming a tail at the next path vertex;
 * each further step moves an edge from the loop to the tail. The last tests
 * cover paths to an end of a connected cycle, which the same checks bound.
 *
 * The ring of length 7 used below is chain w0 of edges 0...6, all pointing
 * forwards: vertex k, for 0 < k < 7, joins edges k - 1 and k, and the
 * boundary vertex joins edges 6 and 0.
 */

#include "pulling_hooks.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace graffine::tests::pulling::unconnected_cycle {

using hooks::Maker;
using hooks::Pullee;
using hooks::check_hooks;
using Vertex = G::Vertex;
using VId = decltype(std::declval<Vertex>().get_id());

template<Degree D>
using Pull = trs::pulling::On<D, G>;

constexpr ChIdG w0 {0};
constexpr std::size_t ringLength {7};

/// Unconnected cycle w0 of \p len edges.
void make_ring(G& gr, const std::size_t len)
{
    gr.add_single_chain_component(len);
    VertexMerger<1, 1, G> {gr}(ESlot {w0, eA}, ESlot {w0, eB});
}

/// Path over the ring from driver edge \p d at its end \p ed to source \p s.
auto ring_maker(const std::size_t d,
                const End::Id ed,
                const Source s) -> Maker
{
    return [=](Pullee& p) {
        make_ring(p.gr, ringLength);
        const auto& m = p.gr.chain(w0);
        p.path = Path {&p.gr.compt(CmpId {}), Driver {&m.g[d], ed}, s};
    };
}

/// As ring_maker(), over the edges \p ee given explicitly.
auto ring_maker(const std::size_t d,
                const End::Id ed,
                const Source s,
                const std::vector<std::size_t>& ee) -> Maker
{
    return [=](Pullee& p) {
        make_ring(p.gr, ringLength);
        const auto& m = p.gr.chain(w0);
        Path::PathW pw;
        for (const auto a : ee)
            pw.push_back(m.g[a].ind);
        p.path = Path {&p.gr.compt(CmpId {}), Driver {&m.g[d], ed}, s, pw};
    };
}

/// Ids of the vertices of \p path, from the driver to the source.
auto ids(const Path& path) -> std::vector<VId>
{
    std::vector<VId> ii;
    for (const auto* v : path.vertices())
        ii.push_back(v->get_id());

    return ii;
}

/// Edges off \p path at its vertex \p i, sorted.
auto branches_at(const G& gr, const Path& path, const std::size_t i)
    -> std::vector<EgIdG>
{
    auto bb = trs::plans::edges_at_other_than(gr, *path.vertices()[i], path.pthw);
    std::ranges::sort(bb);

    return bb;
}

/**
 * Property processor recording the ids of the path vertices seen by the
 * pulling hooks.
 */
struct IdRecorder
    : hooks::Recorder
{
    struct {
        std::vector<VId> start;
        std::vector<std::vector<VId>> ends;
        void on_start(Path& path) { start = ids(path); }
        void on_end(Path& path) { ends.push_back(ids(path)); }
    } pulling;

    using hooks::Recorder::Recorder;
};

static_assert(trs::pulling::Processor<IdRecorder, G>);

/**
 * Pulls the path made by \p make by \p steps, and checks the outcome against
 * the plan: the path keeps its vertex objects, also in the hooks after each
 * step, and the branches at path vertex i < n end at path vertex
 * i + min(steps, shifts). Also checks the ring turned into a loop of length
 * \a ringLength - \p steps with a tail of \p steps edges.
 * \details The path made by \p make should not need a driver correction.
 */
void check_pull(const Maker& make, const int steps)
{
    SCOPED_TRACE("steps " + std::to_string(steps));

    Pullee s;
    make(s);
    ASSERT_TRUE(s.path.is_contiguous());
    const auto n = s.path.length();
    const auto plan = Pull<Deg2> {s.gr}.plan(s.path, steps);
    const auto before = ids(s.path);

    ASSERT_EQ(plan.vertices.size(), n + 1);
    for (std::size_t i {}; i <= n; ++i)
        EXPECT_EQ(plan.vertices[i]->get_id(), before[i]);
    EXPECT_EQ(plan.shifts, std::min(steps, static_cast<int>(n)));
    // the only branches: the cycle edges next to the driver and to the source
    for (std::size_t i {1}; i < n; ++i)
        EXPECT_TRUE(plan.branches[i].empty()) << "path vertex " << i;
    ASSERT_EQ(plan.branches[0].size(), 1);
    ASSERT_EQ(plan.branches[n].size(), 1);

    const auto rec = std::make_shared<IdRecorder>(s.gr);
    trs::pulling::On<Deg2, G, IdRecorder> pull {s.gr, rec};
    ASSERT_TRUE(pull.applicability(s.path, steps));
    pull(s.path, steps);

    // the path vertices, in the hooks and after the pull
    EXPECT_EQ(rec->pulling.start, before);
    ASSERT_EQ(rec->pulling.ends.size(), static_cast<std::size_t>(steps));
    for (std::size_t k {}; k < rec->pulling.ends.size(); ++k)
        EXPECT_EQ(rec->pulling.ends[k], before) << "after step " << k + 1;
    EXPECT_EQ(ids(s.path), before);
    EXPECT_EQ(s.path.length(), n);
    EXPECT_TRUE(s.path.is_contiguous());

    // the branches, moved as planned
    std::vector<std::vector<EgIdG>> expected(n + 1);
    for (std::size_t i {}; i <= n; ++i) {
        if (plan.branches[i].empty())
            continue;
        const auto t = i < n ? i + static_cast<std::size_t>(
                                   std::min(steps, plan.shifts))
                             : n;
        expected[t].insert(expected[t].end(), plan.branches[i].begin(),
                           plan.branches[i].end());
    }
    for (std::size_t i {}; i <= n; ++i) {
        std::ranges::sort(expected[i]);
        EXPECT_EQ(branches_at(s.gr, s.path, i), expected[i]) << "path vertex " << i;
    }

    // a loop of ringLength - steps edges, and a tail of steps edges
    const auto& c = s.gr.compt(CmpId {});
    ASSERT_EQ(c.num_chains(), 2);
    std::vector<std::size_t> lens;
    for (const auto w : c.chids())
        lens.push_back(s.gr.chain(w).length());
    std::ranges::sort(lens);
    const auto k = static_cast<std::size_t>(steps);
    EXPECT_EQ(lens, (std::vector {std::min(k, ringLength - k),
                                  std::max(k, ringLength - k)}));
    const auto& mS = s.gr.chain(s.path.src().w);
    const auto& mD = s.gr.chain(s.path.drv().w);
    EXPECT_EQ(mD.length(), k);
    EXPECT_EQ(s.path.driver_vertex().deg(), Deg1);
    EXPECT_FALSE(c.template check<false>());

    // the source: still a degree 2 vertex inside the loop, or the junction
    // once reached
    if (k < n) {
        EXPECT_TRUE(s.path.src().is_bulk());
        EXPECT_TRUE(mS.is_connected_cycle());
        EXPECT_EQ(s.path.source_vertex().deg(), Deg2);
    }
    else {
        EXPECT_FALSE(s.path.src().is_bulk());
        EXPECT_EQ(s.path.source_vertex().deg(), Deg3);
    }
    EXPECT_EQ(s.path.source_vertex().get_id(), before.back());
}


class PullingUnconnectedCycleTest
    : public Test {};

// =============================================================================


/// Degree 2 source inside the cycle, by every number of steps allowed.
TEST_F(PullingUnconnectedCycleTest, BulkSource)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling to a degree 2 source inside an unconnected ",
            "cycle");

    // driver at vertex 2, source at vertex 5: path 2, 3, 4
    const auto make = ring_maker(2, eA, Source {BSlot {w0, 5}});

    Pullee s;
    make(s);
    ASSERT_EQ(s.path.length(), 3);
    Pull<Deg2> pull {s.gr};
    for (int k {1}; k <= 3; ++k)
        EXPECT_TRUE(pull.applicability(s.path, k)) << k;
    EXPECT_FALSE(pull.applicability(s.path, 4));  // beyond the source
    EXPECT_TRUE(trs::pulling::Functor<G> {s.gr}.applicability(s.path, 3));

    for (int k {1}; k <= 3; ++k) {
        check_pull(make, k);
        check_hooks<Deg2>(make, k);
    }
}


/// Degree 2 source, over a path passing the boundary vertex of the cycle.
TEST_F(PullingUnconnectedCycleTest, PathOverBoundary)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling in an unconnected cycle over its boundary ",
            "vertex");

    // driver at vertex 5, source at vertex 2: path 5, 6, 0, 1
    const auto make = ring_maker(5, eA, Source {BSlot {w0, 2}}, {5, 6, 0, 1});

    Pullee s;
    make(s);
    ASSERT_TRUE(s.path.is_contiguous());
    Pull<Deg2> pull {s.gr};
    for (int k {1}; k <= 4; ++k)
        EXPECT_TRUE(pull.applicability(s.path, k)) << k;
    EXPECT_FALSE(pull.applicability(s.path, 5));  // beyond the source

    for (int k {1}; k <= 4; ++k) {
        check_pull(make, k);
        check_hooks<Deg2>(make, k);
    }

    // the driver next to the boundary vertex, the path passing it first
    const auto next = ring_maker(6, eA, Source {BSlot {w0, 3}}, {6, 0, 1, 2});
    for (int k {1}; k <= 4; ++k) {
        check_pull(next, k);
        check_hooks<Deg2>(next, k);
    }
}


/// The boundary vertex of the cycle as the source, reached from either side.
TEST_F(PullingUnconnectedCycleTest, BoundarySource)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling to the boundary vertex of an unconnected ",
            "cycle");

    // driver at vertex 3; to end B: path 3, 4, 5, 6; to end A: path 2, 1, 0
    const auto toB = ring_maker(3, eA, Source {ESlot {w0, eB}});
    const auto toA = ring_maker(2, eB, Source {ESlot {w0, eA}});

    {
        Pullee s;
        toB(s);
        ASSERT_EQ(s.path.pthw, (Path::PathW {EgIdG {3}, EgIdG {4}, EgIdG {5},
                                             EgIdG {6}}));
        Pull<Deg2> pull {s.gr};
        for (int k {1}; k <= 4; ++k)
            EXPECT_TRUE(pull.applicability(s.path, k)) << k;
        EXPECT_FALSE(pull.applicability(s.path, 5));
    }
    {
        Pullee s;
        toA(s);
        ASSERT_EQ(s.path.pthw, (Path::PathW {EgIdG {2}, EgIdG {1}, EgIdG {0}}));
        Pull<Deg2> pull {s.gr};
        for (int k {1}; k <= 3; ++k)
            EXPECT_TRUE(pull.applicability(s.path, k)) << k;
        EXPECT_FALSE(pull.applicability(s.path, 4));
    }

    for (int k {1}; k <= 4; ++k) {
        check_pull(toB, k);
        check_hooks<Deg2>(toB, k);
    }
    for (int k {1}; k <= 3; ++k) {
        check_pull(toA, k);
        check_hooks<Deg2>(toA, k);
    }

    // the boundary vertex reached over the edge at the other end
    Pullee s;
    ring_maker(3, eA, Source {ESlot {w0, eA}}, {3, 4, 5, 6})(s);
    ASSERT_TRUE(s.path.is_contiguous());
    EXPECT_FALSE(Pull<Deg2> {s.gr}.applicability(s.path, 1));
}


/// The boundary vertex of the cycle as the driver.
TEST_F(PullingUnconnectedCycleTest, BoundaryDriver)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling from the boundary vertex of an unconnected ",
            "cycle");

    const auto fromA = ring_maker(0, eA, Source {BSlot {w0, 3}});  // 0, 1, 2
    const auto fromB = ring_maker(6, eB, Source {BSlot {w0, 4}});  // 6, 5, 4

    for (const auto& make : {fromA, fromB}) {
        Pullee s;
        make(s);
        ASSERT_EQ(s.path.length(), 3);
        ASSERT_TRUE(s.path.driver_vertex() ==
                    *s.gr.chain(w0).vertex_at_end(eA));
        Pull<Deg2> pull {s.gr};
        EXPECT_TRUE(pull.applicability(s.path, 3));
        EXPECT_FALSE(pull.applicability(s.path, 4));
        EXPECT_TRUE(trs::pulling::Functor<G> {s.gr}.applicability(s.path, 3));

        for (int k {1}; k <= 3; ++k) {
            check_pull(make, k);
            check_hooks<Deg2>(make, k);
        }
    }
}


/// Paths given over the edge on the other side of the driver vertex.
TEST_F(PullingUnconnectedCycleTest, DriverCorrection)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling in an unconnected cycle from a path ",
            "starting on the other side of the driver vertex");

    // driver at vertex 2, given by edge 1: path 1, 2, 3, 4 pulled as 2, 3, 4
    const auto bulk = ring_maker(1, eB, Source {BSlot {w0, 5}});
    // driver at the boundary vertex, given by edge 0: path 0, 6, 5 pulled as
    // 6, 5
    const auto boundary = ring_maker(0, eA, Source {BSlot {w0, 5}}, {0, 6, 5});

    for (const auto& [make, n] : {std::pair {bulk, 3}, std::pair {boundary, 2}}) {
        Pullee s;
        make(s);
        ASSERT_FALSE(s.path.is_contiguous());
        Pull<Deg2> pull {s.gr};
        EXPECT_TRUE(pull.applicability(s.path, n));
        EXPECT_FALSE(pull.applicability(s.path, n + 1));

        pull(s.path, n);

        // the junction reached the source
        EXPECT_EQ(s.path.length(), static_cast<std::size_t>(n));
        EXPECT_TRUE(s.path.is_contiguous());
        EXPECT_EQ(s.path.driver_vertex().deg(), Deg1);
        EXPECT_EQ(s.path.source_vertex().deg(), Deg3);
        EXPECT_EQ(s.gr.chain(s.path.drv().w).length(), static_cast<std::size_t>(n));
        EXPECT_FALSE(s.gr.compt(CmpId {}).template check<false>());
    }
}


/// Pulls rejected by the applicability.
TEST_F(PullingUnconnectedCycleTest, Rejected)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rejection of degree 2 pulling in an unconnected cycle");

    const auto rejects = [](const Maker& make, const int steps) {
        Pullee s;
        make(s);
        EXPECT_FALSE(Pull<Deg2> {s.gr}.applicability(s.path, steps));
        EXPECT_FALSE(trs::pulling::Functor<G> {s.gr}.applicability(s.path, steps));
    };

    // a single-edge path: to a degree 2 source, and to the boundary vertex
    rejects(ring_maker(2, eA, Source {BSlot {w0, 3}}), 1);
    rejects(ring_maker(6, eA, Source {ESlot {w0, eB}}), 1);
    rejects(ring_maker(0, eB, Source {ESlot {w0, eA}}), 1);

    // a single-edge path after the driver correction
    rejects(ring_maker(1, eB, Source {BSlot {w0, 3}}), 1);

    // a path around the whole cycle, back to the driver
    rejects(ring_maker(2, eA, Source {BSlot {w0, 2}},
                       {2, 3, 4, 5, 6, 0, 1}), 1);

    // a path turning back
    rejects(ring_maker(2, eA, Source {BSlot {w0, 2}}, {2, 2}), 1);

    // a loop shorter than the minimal cycle length
    for (const auto len : {std::size_t {2}, std::size_t {3}}) {
        Pullee s;
        make_ring(s.gr, len);
        const auto& m = s.gr.chain(w0);
        s.path = Path {&s.gr.compt(CmpId {}), Driver {&m.g[0], eA},
                       Source {BSlot {w0, len - 1}}};
        Pull<Deg2> pull {s.gr};
        EXPECT_EQ(static_cast<bool>(pull.applicability(s.path, 1)),
                  len - 1 >= minCycleLength) << len;
        EXPECT_FALSE(pull.applicability(s.path, 2)) << len;
    }
}


/// A ring of the smallest length pullable by one step.
TEST_F(PullingUnconnectedCycleTest, ShortestRing)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling in an unconnected cycle of 3 edges");

    for (const auto& s0 : {Source {BSlot {w0, 2}}, Source {ESlot {w0, eB}}}) {
        Pullee s;
        make_ring(s.gr, 3);
        const auto& m = s.gr.chain(w0);
        // driver at vertex 1 or 2, path of 2 edges
        s.path = Path {&s.gr.compt(CmpId {}),
                       Driver {&m.g[s0.is_bulk() ? 0 : 1], eA}, s0};
        ASSERT_EQ(s.path.length(), 2);
        Pull<Deg2> pull {s.gr};
        ASSERT_TRUE(pull.applicability(s.path, 1));
        ASSERT_FALSE(pull.applicability(s.path, 2));
        pull(s.path, 1);

        EXPECT_FALSE(s.gr.compt(CmpId {}).template check<false>());
        EXPECT_TRUE(s.path.src().is_bulk());
        EXPECT_TRUE(s.gr.chain(s.path.src().w).is_connected_cycle());
        EXPECT_EQ(s.gr.chain(s.path.src().w).length(), minCycleLength);
        // the driver is now a leaf, and the loop cannot shrink
        EXPECT_FALSE(trs::pulling::Functor<G> {s.gr}.applicability(s.path, 1));
    }
}

/**
 * Degree 2 driver inside a connected cycle, pulling towards the end of the
 * cycle at its junction.
 * \details The split at the driver opens the cycle, so that the source chain
 * then runs from the next path vertex to the junction; it cannot be dissolved
 * there, the source end not being free, which bounds the steps by the path
 * length less one. A shortest path would reach the end edge from the
 * junction instead, and so end next to it: the path runs along the cycle.
 */
TEST_F(PullingUnconnectedCycleTest, ConnectedCycleEndSource)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests degree 2 pulling from inside a connected cycle to its end");

    // cycle w0 of edges 2...7; driver at its vertex 2: path 4, 5, 6, 7
    const Maker make = [](Pullee& s) {
        hooks::make_lollipop(s.gr, 8);
        const auto& m = s.gr.chain(w0);
        s.path = Path {&s.gr.compt(CmpId {}), Driver {&m.g[2], eA},
                       Source {ESlot {w0, eB}}};
    };

    Pullee s;
    make(s);
    ASSERT_TRUE(s.gr.chain(w0).is_connected_cycle());
    ASSERT_EQ(s.path.length(), 4);
    ASSERT_TRUE(*s.path.vertices().back() == *s.gr.chain(w0).vertex_at_end(eB));
    Pull<Deg2> pull {s.gr};
    for (int k {1}; k <= 3; ++k)
        EXPECT_TRUE(pull.applicability(s.path, k)) << k;
    EXPECT_FALSE(pull.applicability(s.path, 4));

    for (int k {1}; k <= 3; ++k)
        check_hooks<Deg2>(make, k);

    // the shortest path, over the junction, ending next to it
    Pullee t;
    make(t);
    const auto& m = t.gr.chain(w0);
    t.path = Path {&t.gr.compt(CmpId {}), Driver {&m.g[2], eB},
                   Source {ESlot {w0, eB}},
                   Path::PathW {m.g[2].ind, m.g[1].ind, m.g[0].ind, m.g[5].ind}};
    EXPECT_FALSE(pull.applicability(t.path, 1));
}


/// Rejection of a path around a connected cycle back to the driver vertex.
TEST_F(PullingUnconnectedCycleTest, SourceAtDriver)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests rejection of pulling from the junction of a cycle around it");

    // driver at the junction, source at the other end of the cycle w0 (6)
    Pullee s;
    hooks::make_lollipop(s.gr, 8);
    const auto& m = s.gr.chain(w0);
    s.path = Path {&s.gr.compt(CmpId {}), Driver {&m.end_edge(eA), eA},
                   Source {ESlot {w0, eB}}};
    ASSERT_EQ(s.path.length(), m.length());
    ASSERT_TRUE(s.path.driver_vertex() == s.path.source_vertex());
    ASSERT_EQ(s.path.driver_vertex().deg(), Deg3);

    EXPECT_FALSE(Pull<Deg3> {s.gr}.applicability(s.path, 1));
    EXPECT_FALSE(trs::pulling::Functor<G> {s.gr}.applicability(s.path, 1));
}

}  // namespace graffine::tests::pulling::unconnected_cycle
