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

/**
 * \file vertex_identity.cpp
 * \brief Tests of the vertex objects kept by the transformations: by the
 * vertex splits and mergers given a Keep, and by pulling along its path.
 */

#include "common.h"
#include "graffine/transforms/plan.h"
#include "graffine/transforms/property_processor.h"
#include "graffine/transforms/pulling/functor.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"
#include "graffine/transforms/vertex_split/to_11.h"
#include "graffine/transforms/vertex_split/to_12.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace graffine::tests::vertex_identity {

namespace trs = graffine::transforms;

using Vertex = G::Vertex;
using VId = decltype(std::declval<Vertex>().get_id());
using Path = trs::pulling::Path<G::Compt>;
using Keep = trs::Keep;
using Edges = std::set<EgIdG>;

/// Builds a graph.
using Shape = std::function<void(G&)>;

/// Small graphs, with all kinds of chains and junctions.
inline auto shapes() -> std::vector<std::pair<std::string, Shape>>
{
    using Merger11 = trs::vertex_merger::From<Deg1, Deg1, G>;
    using Merger12 = trs::vertex_merger::From<Deg1, Deg2, G>;
    using Merger13 = trs::vertex_merger::From<Deg1, Deg3, G>;

    return {
        {"linear chain", [](G& gr) { gr.add_single_chain_component(6); }},
        {"unconnected cycle", [](G& gr) {
            gr.add_single_chain_component(6);
            Merger11 {gr}(ESlot {ChIdG {0}, End::A}, ESlot {ChIdG {0}, End::B});
        }},
        {"junction of 3", [](G& gr) {
            gr.add_single_chain_component(7);
            gr.add_single_chain_component(3);
            Merger12 {gr}(ESlot {ChIdG {1}, End::B}, BSlot {ChIdG {0}, EgIdA {3}});
        }},
        {"two junctions of 3", [](G& gr) {
            gr.add_single_chain_component(8);
            gr.add_single_chain_component(2);
            gr.add_single_chain_component(2);
            Merger12 {gr}(ESlot {ChIdG {1}, End::B}, BSlot {ChIdG {0}, EgIdA {2}});
            Merger12 {gr}(ESlot {ChIdG {2}, End::B}, BSlot {ChIdG {3}, EgIdA {3}});
        }},
        {"junction of 4", [](G& gr) {
            gr.add_single_chain_component(7);
            gr.add_single_chain_component(3);
            gr.add_single_chain_component(2);
            Merger12 {gr}(ESlot {ChIdG {1}, End::B}, BSlot {ChIdG {0}, EgIdA {3}});
            Merger13 {gr}(ESlot {ChIdG {2}, End::B}, ESlot {ChIdG {0}, End::B});
        }},
        {"lollipop", [](G& gr) {
            gr.add_single_chain_component(8);
            Merger12 {gr}(ESlot {ChIdG {0}, End::B}, BSlot {ChIdG {0}, EgIdA {3}});
        }},
        {"chain and cycle", [](G& gr) {
            gr.add_single_chain_component(5);
            gr.add_single_chain_component(5);
            Merger11 {gr}(ESlot {ChIdG {1}, End::A}, ESlot {ChIdG {1}, End::B});
        }},
    };
}

/// Edges at vertex \p v.
inline auto edges_at(const G& gr, const Vertex& v) -> Edges
{
    const auto ii = trs::plans::edges_at(gr, v);
    return {ii.begin(), ii.end()};
}

/// Topology of graph \p gr up to vertex identity: the edges of each chain,
/// in order, and the edges at each vertex.
inline auto topology(const G& gr)
    -> std::pair<std::vector<std::vector<EgIdG>>, std::multiset<Edges>>
{
    std::vector<std::vector<EgIdG>> chains;
    for (ChIdG w {}; w < gr.num_chains(); ++w) {
        auto& ii = chains.emplace_back();
        for (const auto& eg : gr.chain(w).g)
            ii.push_back(eg.ind);
    }
    std::multiset<Edges> vertices;
    for (CmpId c {}; c < gr.num_compts(); ++c)
        for (const auto& v : gr.compt(c).vertices().get())
            vertices.insert(edges_at(gr, *v));

    return {chains, vertices};
}

/// Whether graph \p gr has a vertex of id \p id.
inline auto has_vertex(const G& gr, const VId id) -> bool
{
    for (CmpId c {}; c < gr.num_compts(); ++c)
        for (const auto& v : gr.compt(c).vertices().get())
            if (v->get_id() == id)
                return true;

    return false;
}

/// Graph made by \p shape, checked when going out of scope.
struct Graph {
    G gr;
    IntegrityCheck<G> integrityCheck {gr};

    explicit Graph(const Shape& shape) { shape(gr); }
};

/// Index of resulting vertex \p k.
constexpr auto index(const Keep k) -> std::size_t { return k == Keep::First ? 0 : 1; }


/**
 * Applies the split of a vertex functor \p F to \p s on two graphs made by
 * \p shape, without and with each Keep, and checks that the topologies
 * agree, and that the vertex object is the resulting vertex asked for.
 * \return Whether the split is applicable.
 */
template<typename F,
         typename S>
auto check_split(const Shape& shape, const S& s) -> bool
{
    for (const auto k : {Keep::First, Keep::Second}) {
        Graph a {shape};
        if (!F {a.gr}.is_applicable(s))
            return false;
        const auto res = F {a.gr}(s);

        Graph b {shape};
        const auto id = trs::plans::vertex_at(b.gr, s).get_id();
        const auto resk = F {b.gr}(s, k);

        EXPECT_EQ(topology(b.gr), topology(a.gr));
        EXPECT_EQ(resk[index(k)]->get_id(), id);
        for (std::size_t i {}; i < 2; ++i)
            EXPECT_EQ(edges_at(b.gr, *resk[i]), edges_at(a.gr, *res[i]))
                << "resulting vertex " << i;
    }

    return true;
}

/**
 * Applies the merger functor \p F to \p s1 and \p s2 on two graphs made by
 * \p shape, without and with each Keep, and checks that the topologies
 * agree, that the vertex kept is that asked for, and that the plan tells it.
 * \return Whether the merger is applicable.
 */
template<typename F,
         typename S1,
         typename S2>
auto check_merger(const Shape& shape, const S1& s1, const S2& s2) -> bool
{
    for (const auto k : {Keep::First, Keep::Second}) {
        Graph a {shape};
        if (!F {a.gr}.is_applicable(s1, s2))
            return false;
        F {a.gr}(s1, s2);

        Graph b {shape};
        const auto id1 = trs::plans::vertex_at(b.gr, s1).get_id();
        const auto id2 = trs::plans::vertex_at(b.gr, s2).get_id();
        const auto [kept, removed] = k == Keep::First ? std::pair {id1, id2}
                                                      : std::pair {id2, id1};
        F f {b.gr};
        const auto p = f.plan(s1, s2, k);
        EXPECT_EQ(p.preserved->get_id(), kept);
        EXPECT_EQ(p.removed->get_id(), removed);
        const auto res = f(s1, s2, k);

        EXPECT_EQ(topology(b.gr), topology(a.gr));
        EXPECT_EQ(res[0]->get_id(), kept);
        EXPECT_FALSE(has_vertex(b.gr, removed));
    }

    return true;
}


class VertexIdentityTest
    : public Test {};

// =============================================================================


/**
 * Vertex splits and mergers given a Keep: the topology is that without it,
 * and the vertex object kept is the one asked for.
 */
TEST_F(VertexIdentityTest, KeepPlacesTheVertexObjects)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that the vertex splits and mergers given a Keep keep the ",
            "vertex object asked for, in the topology without it");

    using Split11 = trs::vertex_split::To<Deg1, Deg1, G>;
    using Split12 = trs::vertex_split::To<Deg1, Deg2, G>;
    using Merger12 = trs::vertex_merger::From<Deg1, Deg2, G>;
    using Merger22 = trs::vertex_merger::From<Deg2, Deg2, G>;

    std::map<std::string, int> applied;
    for (const auto& [name, shape] : shapes()) {
        SCOPED_TRACE(name);

        // slots of the graph made by the shape
        std::vector<ESlot> ends;
        std::vector<BSlot> bulks;
        std::vector<ChIdG> cycles;
        {
            Graph g {shape};
            for (ChIdG w {}; w < g.gr.num_chains(); ++w) {
                const auto& m = g.gr.chain(w);
                for (const auto e : End::Ids)
                    ends.push_back({w, e});
                for (std::size_t a {}; a <= m.length(); ++a)
                    bulks.push_back({w, EgIdA {a}});
                if (m.is_unconnected_cycle())
                    cycles.push_back(w);
            }
        }

        for (const auto& b : bulks) {
            SCOPED_TRACE("split 1+1 at " + b.str_short());
            applied["split 1+1"] += check_split<Split11>(shape, b);
        }
        for (const auto& s : ends) {
            SCOPED_TRACE("split 1+2 at " + s.str_short());
            applied["split 1+2"] += check_split<Split12>(shape, s);
        }
        for (const auto& s1 : ends) {
            for (const auto& b2 : bulks) {
                SCOPED_TRACE("merger 1+2 of " + s1.str_short() + " " + b2.str_short());
                applied["merger 1+2"] += check_merger<Merger12>(shape, s1, b2);
            }
            for (const auto w2 : cycles) {
                SCOPED_TRACE("merger 1+2 of " + s1.str_short() + " cycle " +
                             w2.as_string());
                applied["merger 1+2 to a cycle"] +=
                    check_merger<Merger12>(shape, s1, w2);
            }
        }
        for (const auto& b1 : bulks) {
            for (const auto& b2 : bulks) {
                SCOPED_TRACE("merger 2+2 of " + b1.str_short() + " " + b2.str_short());
                applied["merger 2+2"] += check_merger<Merger22>(shape, b1, b2);
            }
            for (const auto w2 : cycles) {
                SCOPED_TRACE("merger 2+2 of " + b1.str_short() + " cycle " +
                             w2.as_string());
                applied["merger 2+2 to a cycle"] +=
                    check_merger<Merger22>(shape, b1, w2);
            }
        }
    }

    for (const auto* name : {"split 1+1", "split 1+2", "merger 1+2",
                             "merger 1+2 to a cycle", "merger 2+2",
                             "merger 2+2 to a cycle"})
        EXPECT_GT(applied[name], 0) << name;
}

}  // namespace graffine::tests::vertex_identity


namespace graffine::tests::vertex_identity {

/// Ids of the vertices of \p path, from the driver to the source.
inline auto ids(const Path& path) -> std::vector<VId>
{
    std::vector<VId> ii;
    for (const auto* v : path.vertices())
        ii.push_back(v->get_id());

    return ii;
}

/**
 * Property processor recording the ids of the path vertices seen by the
 * pulling hooks, and of the vertices resulting from the inner vertex splits
 * and mergers.
 */
struct Recorder {

    std::set<VId> splits;   ///< Vertices resulting from inner splits.
    std::set<VId> mergers;  ///< Vertices resulting from inner mergers.

    struct Hooks {
        Recorder* r {};
    };

    struct : Hooks {
        void on_start(const ESlot&, const ESlot&) {}
        void on_start(const BSlot&, const ESlot&) {}
        void on_start(const BSlot&, const BSlot&) {}
        void on_end(Vertex& v) { r->mergers.insert(v.get_id()); }
    } vertexMerger;

    struct : Hooks {
        void on_start(ChIdG) {}
        void on_start(const ESlot&) {}
        void on_start(const BSlot&) {}
        void on_start(const ESlot&, const ESlot&) {}
        void on_end(const std::array<Vertex*, 2>& vv)
        {
            r->splits.insert({vv[0]->get_id(), vv[1]->get_id()});
        }
    } vertexSplit;

    struct : Hooks {
        template<typename S> void on_start(const S&) {}
        void on_start(ChIdG, EgIdA, bool) {}
        template<bool isNewChain> void on_end(Edge&) {}
    } edgeCreation;

    struct : Hooks {
        template<bool isChainDeleted> void on_start(Edge&) {}
        template<bool isChainDeleted> void on_end(Vertex&) {}
    } edgeDeletion;

    struct : Hooks {
        void on_start() {}
        void on_end() {}
    } componentCreation, componentDeletion;

    struct : Hooks {
        std::vector<VId> start;
        std::vector<std::vector<VId>> ends;
        void on_start(Path& path) { start = ids(path); }
        void on_end(Path& path) { ends.push_back(ids(path)); }
    } pulling;

    Recorder()
    {
        for (Hooks* h : std::initializer_list<Hooks*> {
                 &vertexMerger, &vertexSplit, &edgeCreation, &edgeDeletion,
                 &componentCreation, &componentDeletion, &pulling})
            h->r = this;
    }
};

static_assert(trs::pulling::Processor<Recorder, G>);


/**
 * Pullings of every path of small graphs, by every number of steps up to 3:
 * each path vertex keeps its object after each step.
 */
TEST_F(VertexIdentityTest, PullingKeepsThePathVertices)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that pulling keeps the vertex objects of the path");

    using Puller = trs::pulling::Functor<G, Recorder>;

    // Number of pullings by driver degree and kind of path.
    std::map<std::pair<Degree, std::string>, int> applied;

    for (const auto& [name, shape] : shapes()) {
        SCOPED_TRACE(name);

        // drivers and sources of the graph made by the shape
        std::vector<std::pair<EgIdG, End::Id>> drivers;
        std::vector<typename Path::Source> sources;
        {
            Graph g {shape};
            for (EgIdG i {}; i < g.gr.num_edges(); ++i)
                for (const auto e : End::Ids)
                    drivers.emplace_back(i, e);
            for (ChIdG w {}; w < g.gr.num_chains(); ++w) {
                const auto& m = g.gr.chain(w);
                for (const auto e : End::Ids)
                    if (!m.is_connected_at(e))
                        sources.emplace_back(ESlot {w, e});
                for (std::size_t a {1}; a < m.length(); ++a)
                    sources.emplace_back(BSlot {w, EgIdA {a}});
            }
        }

        for (const auto& [i, ed] : drivers)
            for (const auto& s : sources)
                for (int steps {1}; steps <= 3; ++steps) {
                    Graph g {shape};
                    auto& gr = g.gr;
                    const auto& d = gr.edge(i);
                    if (gr.chain(s.w).c != d.c)
                        continue;
                    const auto& vs = s.is_bulk()
                        ? *gr.chain(s.w).vertex_at_bulk(s.pos)
                        : *gr.chain(s.w).vertex_at_end(s.e);
                    if (d.vertex_at_inner(ed) == &vs)
                        continue;
                    Path path {&gr.compt(d.c), typename Path::Driver {&d, ed}, s};
                    if (!path.is_contiguous())
                        continue;

                    const auto pp = std::make_shared<Recorder>();
                    Puller pull {gr, pp};
                    if (!pull.is_applicable(path, steps))
                        continue;

                    // the kind of path, before the pulling
                    const auto vv = path.vertices();
                    const auto deg = vv[0]->deg();
                    std::vector<std::string> kinds {"any"};
                    if (steps > 1)
                        kinds.push_back("several steps");
                    if (s.is_bulk())
                        kinds.push_back("bulk source");
                    if (gr.chain(s.w).is_cycle())
                        kinds.push_back("source in a cycle");
                    if (!s.is_bulk() && gr.chain(s.w).length() <= std::size_t(steps) &&
                        path.drv().w != s.w)
                        kinds.push_back("source chain dissolved");
                    if (std::ranges::any_of(vv.begin() + 1, vv.end() - 1,
                                            [](const auto* v) { return v->deg() > Deg2; }))
                        kinds.push_back("over a junction");

                    SCOPED_TRACE("driver " + std::to_string(i()) + " " + End::str(ed) +
                                 ", source " + s.str_short() + ", steps " +
                                 std::to_string(steps) + ", degree " +
                                 std::to_string(deg));

                    pull(path, steps);

                    const auto& h = pp->pulling;
                    ASSERT_EQ(h.ends.size(), static_cast<std::size_t>(steps));
                    for (std::size_t k {}; k < h.ends.size(); ++k)
                        EXPECT_EQ(h.ends[k], h.start) << "after step " << k + 1;
                    EXPECT_EQ(ids(path), h.start);

                    // the inner split detaching the driver, and the inner
                    // merger joining the vertex next to it, pass their objects
                    // to the hooks
                    if (deg == Deg3 || (deg == Deg2 && vv.size() > 2 &&
                                        path.length() > 1)) {
                        EXPECT_TRUE(pp->splits.contains(h.start[0]));
                        EXPECT_TRUE(pp->mergers.contains(h.start[1]));
                    }

                    for (const auto& kind : kinds)
                        applied[{deg, kind}]++;
                }
    }

    for (const auto deg : {Deg1, Deg2, Deg3})
        for (const auto* kind : {"any", "several steps", "bulk source",
                                 "source in a cycle", "source chain dissolved",
                                 "over a junction"})
            EXPECT_GT((applied[{deg, kind}]), 0)
                << "degree " << deg << ", " << kind;

    if constexpr (verboseT)
        for (const auto& [key, n] : applied)
            std::cout << "  degree " << key.first << ", " << key.second << ": "
                      << n << '\n';
}

}  // namespace graffine::tests::vertex_identity
