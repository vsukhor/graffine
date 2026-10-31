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

#include "common.h"
#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/edge_split/functor.h"
#include "graffine/transforms/plan.h"
#include "graffine/transforms/property_processor.h"
#include "graffine/transforms/pulling/functor.h"
#include "graffine/transforms/vertex_merger/functor.h"
#include "graffine/transforms/vertex_split/functor.h"

#include <array>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace graffine::tests::plan {

namespace trs = graffine::transforms;

using Vertex = G::Vertex;
using VId = decltype(std::declval<Vertex>().get_id());
using Path = trs::pulling::Path<G::Compt>;
using Edges = std::set<EgIdG>;

inline auto edges_at(const G& gr, const Vertex& v) -> Edges
{
    const auto ii = trs::plans::edges_at(gr, v);
    return {ii.begin(), ii.end()};
}

/// Vertices of graph \p gr by id.
inline auto vertices(const G& gr) -> std::map<VId, const Vertex*>
{
    std::map<VId, const Vertex*> vv;
    for (CmpId c {}; c < gr.num_compts(); ++c)
        for (const auto& v : gr.compt(c).vertices().get())
            vv[v->get_id()] = v.get();

    return vv;
}

/// Topology of graph \p gr: the edges at each vertex, by vertex id.
inline auto dump(const G& gr) -> std::map<VId, Edges>
{
    std::map<VId, Edges> d;
    for (const auto& [id, v] : vertices(gr))
        d[id] = edges_at(gr, *v);

    return d;
}

// Plans, as recorded by the hooks: by vertex ids, valid after the change.

struct SplitPlan {
    VId vertex {};
    std::array<Edges, 2> edges;
};

struct MergerPlan {
    VId preserved {};
    VId removed {};
    Edges edges;  ///< Edges of both vertices.
};

struct CreationPlan {
    VId vertex {};
    EgIdG moved {EgIdG::undefined};
    std::set<VId> ids;  ///< Vertices of the graph.
};

struct DeletionPlan {
    EgIdG edge {EgIdG::undefined};
    bool isChainDeleted {};
    VId preserved {};
    VId removed {};
    Edges edges;  ///< Edges of the remaining vertex.
    std::size_t numEdges {};  ///< Edges of the graph.
};

struct EdgeMergerPlan {
    EgIdG kept {EgIdG::undefined};
    DeletionPlan deletion;
};

struct PullingPlan {
    std::vector<std::size_t> branches;  ///< Number per path vertex.
    int steps {};
    int shifts {};
};

/// What the hooks saw.
struct Log {
    std::optional<SplitPlan> split;
    std::optional<MergerPlan> merger;
    std::optional<CreationPlan> creation;
    std::optional<DeletionPlan> deletion;
    std::optional<EdgeMergerPlan> edgeMerger;
    std::optional<PullingPlan> pulling;
    int plans {};   ///< Calls of the planning hooks.
    int starts {};  ///< Calls of on_start hooks.
    /// Number of edges off the path at each path vertex, after each step.
    std::vector<std::vector<std::size_t>> pulled;
};

inline auto to_record(const G& gr,
                      const trs::edge_deletion::Plan<G>& p) -> DeletionPlan
{
    auto ee = edges_at(gr, *p.preserved);
    if (!p.isChainDeleted)
        ee.merge(edges_at(gr, *p.removed));
    ee.erase(p.edge);

    return {p.edge, p.isChainDeleted, p.preserved->get_id(),
            p.removed->get_id(), ee, gr.num_edges()};
}

/// Edges off \p path at each of its vertices.
inline auto branches(const G& gr, const Path& path) -> std::vector<std::size_t>
{
    std::vector<std::size_t> bb;
    for (const auto* v : path.vertices())
        bb.push_back(trs::plans::edges_at_other_than(gr, *v, path.pthw).size());

    return bb;
}

/**
 * Property processor recording the plans, and vetoing them unless \a allow.
 * \details Its other hooks count the calls of on_start, and record the
 * branches of the path after each step of a pulling.
 */
struct Planner {

    struct Hooks {
        const G* gr {};
        Log* log {};
        const bool* allow {};

        auto planned() -> bool { ++log->plans; return *allow; }
        void start() { ++log->starts; }
    };

    struct : Hooks {
        void on_start(const ESlot&, const ESlot&) { start(); }
        void on_start(const BSlot&, const ESlot&) { start(); }
        void on_start(const BSlot&, const BSlot&) { start(); }
        void on_end(Vertex&) {}
        bool plan(const trs::vertex_merger::Plan<G>& p)
        {
            auto ee = edges_at(*gr, *p.preserved);
            ee.merge(edges_at(*gr, *p.removed));
            log->merger = {p.preserved->get_id(), p.removed->get_id(), ee};
            return planned();
        }
    } vertexMerger;

    struct : Hooks {
        void on_start(ChIdG) { start(); }
        void on_start(const ESlot&) { start(); }
        void on_start(const BSlot&) { start(); }
        void on_start(const ESlot&, const ESlot&) { start(); }
        void on_end(const std::array<Vertex*, 2>&) {}
        bool plan(const trs::vertex_split::Plan<G>& p)
        {
            log->split = {p.vertex->get_id(),
                          {Edges {p.edges[0].begin(), p.edges[0].end()},
                           Edges {p.edges[1].begin(), p.edges[1].end()}}};
            return planned();
        }
    } vertexSplit;

    struct : Hooks {
        void on_start(ChIdG) { start(); }
        template<typename S> void on_start(const S&) { start(); }
        void on_start(ChIdG, EgIdA, bool) { start(); }
        template<bool isNewChain> void on_end(Edge&) {}
        bool plan(const trs::edge_creation::Plan<G>& p)
        {
            std::set<VId> ids;
            for (const auto& [id, v] : vertices(*gr))
                ids.insert(id);
            log->creation = {p.vertex->get_id(), p.moved, ids};
            return planned();
        }
    } edgeCreation;

    struct : Hooks {
        template<bool isChainDeleted> void on_start(Edge&) { start(); }
        template<bool isChainDeleted> void on_end(Vertex&) {}
        bool plan(const trs::edge_deletion::Plan<G>& p)
        {
            log->deletion = to_record(*gr, p);
            return planned();
        }
    } edgeDeletion;

    struct : Hooks {
        void on_start(Edge&, Edge&) { start(); }
        void on_end(Edge&) {}
        bool plan(const trs::edge_merger::Plan<G>& p)
        {
            log->edgeMerger = {p.kept, to_record(*gr, p.deletion)};
            return planned();
        }
    } edgeMerger;

    struct : Hooks {
        void on_start() { start(); }
        void on_end() {}
        bool plan(const trs::component_creation::Plan<G>&) { return planned(); }
    } componentCreation;

    struct : Hooks {
        void on_start() { start(); }
        void on_end() {}
        bool plan(const trs::component_deletion::Plan<G>&) { return planned(); }
    } componentDeletion;

    struct : Hooks {
        void on_start(Path&) { start(); }
        void on_end(Path& path) { log->pulled.push_back(branches(*gr, path)); }
        bool plan(const trs::pulling::Plan<G>& p)
        {
            PullingPlan r {{}, p.steps, p.shifts};
            for (const auto& b : p.branches)
                r.branches.push_back(b.size());
            log->pulling = r;
            return planned();
        }
    } pulling;

    Log log;
    bool allow {true};

    explicit Planner(const G& gr)
    {
        for (Hooks* h : std::initializer_list<Hooks*> {
                 &vertexMerger, &vertexSplit, &edgeCreation, &edgeDeletion,
                 &edgeMerger, &componentCreation, &componentDeletion, &pulling})
            *h = {&gr, &log, &allow};
    }
};

static_assert(trs::PropertyProcessor<Planner, G>);
static_assert(trs::plans::Hooked<Planner, trs::vertex_split::Plan<G>>);


// Checks of the outcome against the plan. ====================================

inline void check(const G& gr, const SplitPlan& p,
                  const std::array<Vertex*, 2>& res)
{
    for (std::size_t i {}; i < 2; ++i)
        EXPECT_EQ(edges_at(gr, *res[i]), p.edges[i]) << "resulting vertex " << i;
    EXPECT_NE(res[0], res[1]);
}

inline void check(const G& gr, const MergerPlan& p, const Vertex& res)
{
    EXPECT_EQ(res.get_id(), p.preserved);
    EXPECT_EQ(edges_at(gr, res), p.edges);
    EXPECT_FALSE(vertices(gr).contains(p.removed));
}

inline void check(const G& gr, const CreationPlan& p, const Edge& eg)
{
    const auto* vA = eg.vertex_at_outer(End::A);
    const auto* vB = eg.vertex_at_outer(End::B);
    const auto atA = vA->get_id() == p.vertex;
    ASSERT_TRUE(atA || vB->get_id() == p.vertex) << "new edge not at the vertex";
    const auto* vNew = atA ? vB : vA;
    EXPECT_FALSE(p.ids.contains(vNew->get_id())) << "other vertex is not new";

    if (p.moved.is_defined()) {
        const auto& m = gr.edge(p.moved);
        const auto* mA = m.vertex_at_outer(End::A);
        const auto* mB = m.vertex_at_outer(End::B);
        EXPECT_TRUE(mA == vNew || mB == vNew) << "moved edge not at the new vertex";
        EXPECT_TRUE(mA->get_id() != p.vertex && mB->get_id() != p.vertex)
            << "moved edge still at the vertex";
        EXPECT_EQ(vNew->deg(), Deg2);
    }
    else
        EXPECT_EQ(vNew->deg(), Deg1);
}

/// Index after the deletion of an edge with index \p i before it.
inline auto after_deletion(const DeletionPlan& p, const EgIdG i) -> EgIdG
{
    return i == EgIdG {p.numEdges - 1} ? p.edge : i;
}

inline void check(const G& gr, const DeletionPlan& p)
{
    SCOPED_TRACE(p.isChainDeleted ? "deleting the chain" : "contracting");
    EXPECT_EQ(gr.num_edges(), p.numEdges - 1);
    const auto vv = vertices(gr);
    EXPECT_FALSE(vv.contains(p.removed)) << "removed vertex remains";
    Edges ee;
    for (const auto i : p.edges)
        ee.insert(after_deletion(p, i));

    if (!p.isChainDeleted) {
        ASSERT_TRUE(vv.contains(p.preserved)) << "preserved vertex is removed";
        EXPECT_EQ(edges_at(gr, *vv.at(p.preserved)), ee);
    }
    else {  // the junction keeps its other edges, possibly as a new vertex
        ASSERT_FALSE(ee.empty());
        const auto& eg = gr.edge(*ee.begin());
        const auto* vA = eg.vertex_at_outer(End::A);
        const auto* vB = eg.vertex_at_outer(End::B);
        EXPECT_TRUE(edges_at(gr, *vA) == ee || edges_at(gr, *vB) == ee);
    }
}

inline void check(const G& gr, const EdgeMergerPlan& p, const Edge& eg)
{
    check(gr, p.deletion);
    EXPECT_EQ(eg.ind, after_deletion(p.deletion, p.kept));
}

/// Branches after each step, from the plan.
inline auto expected_pulled(const PullingPlan& p)
    -> std::vector<std::vector<std::size_t>>
{
    const auto n = p.branches.size() - 1;
    std::vector<std::vector<std::size_t>> after;
    for (int k {1}; k <= p.steps; ++k) {
        std::vector<std::size_t> bb(p.branches.size());
        const auto shift = static_cast<std::size_t>(std::min(k, p.shifts));
        for (std::size_t i {}; i <= n; ++i)
            if (p.branches[i])
                bb.at(i < n ? i + shift : n) += p.branches[i];
        after.push_back(bb);
    }

    return after;
}


// Random transformations. =====================================================

using Splitter = trs::vertex_split::Functor<G, Planner>;
using Merger = trs::vertex_merger::Functor<G, Planner>;
using Extender = trs::edge_creation::InExistingChain<G, Planner>;
using Brancher2 = trs::edge_creation::InNewChain<Deg2, G, Planner>;
using Brancher3 = trs::edge_creation::InNewChain<Deg3, G, Planner>;
using Deleter = trs::edge_deletion::Functor<G, Planner>;
using EdgeMerger = trs::edge_merger::Functor<G, Planner>;
using EdgeSplitter = trs::edge_split::Functor<G, Planner>;
using Creator = trs::component_creation::Functor<G, Planner>;
using Remover = trs::component_deletion::Functor<G, Planner>;
using Puller = trs::pulling::Functor<G, Planner>;

/// Applies random transformations to a graph, checking them against their plans.
class Fuzzer {

public:

    explicit Fuzzer(const unsigned seed)
        : rng {seed}
    {
        create(6);
        create(4);
        create(3);
        // two unconnected cycles
        for (const auto n : {5, 4}) {
            create(n);
            const auto w = gr.ind_last_chain();
            trs::vertex_merger::From<Deg1, Deg1, G> {gr}(ESlot {w, End::A},
                                                         ESlot {w, End::B});
        }
    }

    G gr;
    std::shared_ptr<Planner> pp {std::make_shared<Planner>(gr)};

    Splitter split {gr, pp};
    Merger merge {gr, pp};
    Extender extend {gr, pp};
    Brancher2 branch2 {gr, pp};
    Brancher3 branch3 {gr, pp};
    Deleter remove {gr, pp};
    EdgeMerger mergeEdges {gr, pp};
    EdgeSplitter splitEdge {gr, pp};
    Creator create {gr, pp};
    Remover removeCmp {gr, pp};
    Puller pull {gr, pp};

    std::mt19937 rng;
    std::map<std::string, int> applied;  ///< Number per transformation.

    /// Applies a random transformation, if applicable; vetoes it at random.
    void step()
    {
        const auto veto = coin(0.2);
        pp->allow = !veto;
        pp->log = {};

        switch (pick(gr.num_edges() > 60 ? 3 : 11)) {
            case 0: return try_delete(veto);
            case 1: return try_merge_edges(veto);
            case 2: return try_merge(veto);
            case 3: return try_split(veto);
            case 4: return try_extend(veto);
            case 5: return try_branch(veto);
            case 6: return try_split_edge(veto);
            case 7: return try_pull(veto);
            case 8: return try_pull(veto);
            case 9: return coin(0.2) ? try_create(veto) : try_split(veto);
            default: return coin(0.1) ? try_remove_cmp(veto) : try_merge(veto);
        }
    }

private:

    auto pick(const std::size_t n) -> std::size_t
    {
        return std::uniform_int_distribution<std::size_t> {0, n - 1}(rng);
    }

    auto coin(const double p) -> bool
    {
        return std::bernoulli_distribution {p}(rng);
    }

    auto random_vertex() -> Vertex*
    {
        std::vector<Vertex*> vv;
        for (CmpId c {}; c < gr.num_compts(); ++c)
            for (auto& v : gr.compt(c).vertices().get())
                vv.push_back(v.get());

        return vv[pick(vv.size())];
    }

    auto random_edge() -> EgIdG { return EgIdG {pick(gr.num_edges())}; }

    /**
     * Applies \p f to \p args if applicable: vetoed, checks that the graph
     * is untouched; otherwise, checks the outcome by \p check_res.
     */
    template<typename F,
             typename C,
             typename... A>
    void run(const char* name, const bool veto, F& f, C check_res, A&&... args)
    {
        if (!f.is_applicable(args...))
            return;

        const auto before = dump(gr);
        const Snapshot snap {gr};
        if (veto) {
            const auto res = f.try_apply(std::forward<A>(args)...);
            ASSERT_FALSE(res) << name;
            EXPECT_EQ(res.error().reason, trs::vetoed) << name;
            EXPECT_EQ(pp->log.plans, 1) << name;
            EXPECT_EQ(pp->log.starts, 0) << name;
            EXPECT_EQ(dump(gr), before) << name;
            EXPECT_TRUE(Snapshot {gr} == snap) << name;
            applied[std::string {name} + " vetoed"]++;
            return;
        }

        const auto res = f(std::forward<A>(args)...);
        EXPECT_EQ(pp->log.plans, 1) << name;
        SCOPED_TRACE(name);
        check_res(res);
        applied[name]++;
    }

    void try_split(const bool veto)
    {
        auto& v = *random_vertex();
        const std::vector<Vertex::EgDescr> all {v.edges().begin(), v.edges().end()};
        std::vector<Vertex::EgDescr> ee;
        const auto i = pick(all.size());
        ee.push_back(all[i]);
        if (v.deg() == Deg4 && coin(0.5))
            ee.push_back(all[(i + 1 + pick(all.size() - 1)) % all.size()]);
        SCOPED_TRACE("split deg " + std::to_string(v.deg()) + " detaching " +
                     std::to_string(ee.size()));
        run("vertex split", veto, split,
            [&](const auto& res) { check(gr, *pp->log.split, res); }, v, ee);
    }

    void try_merge(const bool veto)
    {
        auto& v1 = *random_vertex();
        auto& v2 = *random_vertex();
        if (&v1 == &v2)
            return;
        SCOPED_TRACE("merger " + std::to_string(v1.deg()) + "+" +
                     std::to_string(v2.deg()));
        run("vertex merger", veto, merge,
            [&](const auto& res) { check(gr, *pp->log.merger, *res[0]); }, v1, v2);
    }

    void try_extend(const bool veto)
    {
        auto& v = *random_vertex();
        run("edge creation in existing chain", veto, extend,
            [&](const auto& res) { check(gr, *pp->log.creation, *res[0]); }, v);
    }

    void try_branch(const bool veto)
    {
        auto& v = *random_vertex();
        const auto check_res = [&](const auto& res) {
            check(gr, *pp->log.creation, *res[0]);
        };
        if (v.deg() == Deg2)
            run("edge creation in new chain at 2", veto, branch2, check_res, v);
        else if (v.deg() == Deg3)
            run("edge creation in new chain at 3", veto, branch3, check_res, v);
    }

    void try_delete(const bool veto)
    {
        run("edge deletion", veto, remove,
            [&](const auto&) { check(gr, *pp->log.deletion); }, random_edge());
    }

    void try_merge_edges(const bool veto)
    {
        const auto i1 = random_edge();
        const auto& eg = gr.edge(i1);
        const auto& v = *eg.vertex_at_outer(coin(0.5) ? End::A : End::B);
        const auto ee = trs::plans::edges_at_other_than(gr, v, std::vector {i1});
        if (ee.empty())
            return;
        run("edge merger", veto, mergeEdges,
            [&](const auto& res) { check(gr, *pp->log.edgeMerger, *res[0]); },
            i1, ee[pick(ee.size())]);
    }

    void try_split_edge(const bool veto)
    {
        run("edge split", veto, splitEdge,
            [&](const auto& res) { check(gr, *pp->log.creation, *res[0]); },
            random_edge());
    }

    void try_create(const bool veto)
    {
        run("component creation", veto, create, [](const auto&) {},
            pick(5) + 1);
    }

    void try_remove_cmp(const bool veto)
    {
        if (gr.num_compts() > 1)
            run("component deletion", veto, removeCmp, [](const auto&) {},
                CmpId {pick(gr.num_compts())});
    }

    void try_pull(const bool veto)
    {
        // driver: a random edge end; source: a random free end or bulk vertex
        // of its component
        const auto& d = gr.edge(random_edge());
        const auto ed = coin(0.5) ? End::A : End::B;
        const auto& cmp = gr.compt(d.c);
        const std::vector<ChIdG> ww {cmp.chids().begin(), cmp.chids().end()};
        const auto ws = ww[pick(ww.size())];
        const auto& m = gr.chain(ws);
        const auto es = coin(0.5) ? End::A : End::B;
        const auto bulk = m.length() > 1 && coin(0.3);
        const typename Path::Source s = bulk
            ? typename Path::Source {BSlot {ws, EgIdA {pick(m.length() - 1) + 1}}}
            : typename Path::Source {ESlot {ws, es}};
        if (!bulk && m.is_connected_at(es))
            return;
        const auto& vs = bulk ? *m.g[s.pos - 1].vertex_at_outer(End::B)
                              : *m.vertex_at_end(es);
        if (d.vertex_at_outer(ed) == &vs)
            return;

        Path path {&cmp, typename Path::Driver {&d, ed}, s};
        if (!path.is_contiguous())
            return;
        const auto steps = static_cast<int>(pick(3)) + 1;
        run("pulling", veto, pull,
            [&](const auto&) {
                EXPECT_EQ(pp->log.pulled, expected_pulled(*pp->log.pulling));
            }, path, steps);
    }
};


class PlanTest
    : public Test {};

// =============================================================================


/**
 * Random transformations, by their dispatching functors: the outcome of each
 * matches its plan, and a veto leaves the graph untouched.
 */
TEST_F(PlanTest, MatchesOutcome)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that the plans of random transformations match their ",
            "outcome, and that vetoes leave the graph untouched");

    std::map<std::string, int> applied;
    for (unsigned seed {1}; seed <= 20; ++seed) {
        Fuzzer f {seed};
        const IntegrityCheck integrityCheck {f.gr};
        for (int i {}; i < 400; ++i)
            f.step();
        for (const auto& [name, n] : f.applied)
            applied[name] += n;
    }

    if constexpr (verboseT)
        for (const auto& [name, n] : applied)
            std::cout << "  " << name << ": " << n << '\n';
    for (const auto* name : {"vertex split", "vertex merger",
                             "edge creation in existing chain",
                             "edge creation in new chain at 2",
                             "edge creation in new chain at 3", "edge deletion",
                             "edge merger", "edge split", "component creation",
                             "component deletion", "pulling"}) {
        EXPECT_GT(applied[name], 0) << name;
        EXPECT_GT(applied[std::string {name} + " vetoed"], 0) << name;
    }
}

}  // namespace graffine::tests::plan


namespace graffine::tests::plan {

/**
 * Merger of two internal vertices of an unconnected cycle: the vertices
 * merged are those asked for, and the plan tells which one is preserved.
 * \details The merger used to merge the boundary vertex of the cycle instead.
 */
TEST_F(PlanTest, MergerInUnconnectedCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the merger of two internal vertices of an unconnected cycle");

    static constexpr ChIdG w {0};

    for (const std::size_t len : {5, 6, 7})
        for (std::size_t a1 {1}; a1 < len; ++a1)
            for (std::size_t a2 {1}; a2 < len; ++a2) {
                G gr;
                const IntegrityCheck integrityCheck {gr};
                gr.add_single_chain_component(len);
                trs::vertex_merger::From<Deg1, Deg1, G> {gr}(ESlot {w, End::A},
                                                             ESlot {w, End::B});
                const BSlot b1 {w, EgIdA {a1}};
                const BSlot b2 {w, EgIdA {a2}};
                const auto pp = std::make_shared<Planner>(gr);
                trs::vertex_merger::From<Deg2, Deg2, G, Planner> merge {gr, pp};
                if (!merge.is_applicable(b1, b2))
                    continue;

                SCOPED_TRACE("cycle of " + std::to_string(len) + " at " +
                             std::to_string(a1) + ", " + std::to_string(a2));
                const auto res = merge(b1, b2);
                check(gr, *pp->log.merger, *res[0]);
            }
}

}  // namespace graffine::tests::plan
