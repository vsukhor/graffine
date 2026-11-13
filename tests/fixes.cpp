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

/// Regression tests of fixed defects in the structure and transformations.

#include "common.h"
#include "sample_graph.h"

#include "graffine/structure/analyzers/adjacency.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/paths/over_edges/generic.h"
#include "graffine/structure/paths/over_endslots/distance.h"
#include "graffine/transforms/component_creation/functor.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>


namespace graffine::tests::fixes {

using Compt = G::Compt;

class FixesTest
    : public Test {
};


// =============================================================================
// Paths
// =============================================================================

/// The search queues order the distance records by distance first.
TEST_F(FixesTest, DistanceOrdersByDistance)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that the distance records of the shortest ",
                          "path searches are ordered by distance");

    using DE = structure::paths::over_edges::Distance<Compt>;
    const DE near {EgIdC {5}, real(1)};
    const DE far  {EgIdC {1}, real(2)};
    EXPECT_TRUE(near < far);
    EXPECT_FALSE(far < near);

    // Equal distances: ordered by element, so that both fit in a set.
    const DE tie {EgIdC {7}, real(1)};
    EXPECT_TRUE(near < tie);

    using DS = structure::paths::over_endslots::Distance<Compt>;
    const DS nearS {ESlot {ChIdG {5}, End::A}, real(1)};
    const DS farS  {ESlot {ChIdG {1}, End::A}, real(2)};
    EXPECT_TRUE(nearS < farS);
    EXPECT_FALSE(farS < nearS);
}


/// Shortest distances over edges agree with an exhaustive relaxation.
TEST_F(FixesTest, ShortestPathsOverEdgesAreShortest)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the shortest distances over edges against ",
                          "Bellman-Ford, with non-uniform weights");

    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    // Weights making the fewest-edge paths not the shortest ones.
    for (std::size_t i {}; i < gr.num_edges(); ++i)
        gr.edge(EgIdG {i}).weight() = real(1 + (i * 7) % 5);

    for (const auto& c : gr.compts()) {

        const auto lg = structure::Adjacency<Compt>::list_edges(c);
        const auto n = c.num_edges();

        for (EgIdC src {}; src < n; ++src) {

            // Reference: Bellman-Ford over the edge adjacency list.
            std::vector<real> ref(n, std::numeric_limits<real>::infinity());
            ref[src()] = 0;
            for (std::size_t it {}; it < n; ++it)
                for (std::size_t u {}; u < n; ++u)
                    for (const auto v : lg[u])
                        ref[v()] = std::min(ref[v()],
                                            ref[u] + c.edge(v).weight());

            structure::paths::over_edges::Generic<Compt> pp {&c};
            pp.compute_from_source(src);
            const auto dd = pp.distances_to_global_ind();

            ASSERT_EQ(dd.size(), n);
            for (std::size_t i {}; i < n; ++i)
                EXPECT_FLOAT_EQ(dd[i].get_dist(), ref[i])
                    << "component " << c.ind << " from " << src << " to " << i;
        }
    }
}


/// The preceding edges reported by distances_to_global_ind() are graph-wide.
TEST_F(FixesTest, DistancesToGlobalInd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests distances_to_global_ind()");

    const auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    for (const auto& c : gr.compts()) {

        structure::paths::over_edges::Generic<Compt> pp {&c};
        pp.compute_from_source(EgIdC {});
        const auto dd = pp.distances_to_global_ind();

        for (EgIdC i {1}; i < c.num_edges(); ++i) {
            const auto path = pp.find_shortest_path<false>(EgIdC {}, i);
            ASSERT_GE(path.size(), 2);
            const auto prev = path[path.size() - 2];
            EXPECT_EQ(dd[i()].get_prev()(), c.edge(prev).ind());
        }
        EXPECT_FALSE(dd[0].get_prev().is_defined());
    }
}


// =============================================================================
// Elements
// =============================================================================

/// An edge whose inner end A is free is still a neighbour by its end B.
TEST_F(FixesTest, EdgeIsNeigOfThroughEndB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Edge::is_neig_of() for an edge having ",
                          "a free end A");

    // In a graph, chain ends carry degree 1 vertices, so every edge is
    // connected at both ends. Free ends occur only in edges assembled
    // outside a graph: here a path g0 - v1 - g1 - v2 - g2, with g0 free
    // at its end A and g2 free at its end B.
    using Vertex = Edge::Vertex;
    Vertex v1;
    Vertex v2;
    std::vector<Edge> g;
    for (std::size_t i {}; i < 3; ++i)
        g.emplace_back(EgIdG {i}, EgIdC {i}, EgIdA {i}, ChIdG {}, CmpId {});

    g[0].connect_to_inner(End::B, v1);
    g[1].connect_to_inner(End::A, v1);
    g[1].connect_to_inner(End::B, v2);
    g[2].connect_to_inner(End::A, v2);

    ASSERT_FALSE(g[0].is_connected_inner(End::A));
    ASSERT_FALSE(g[2].is_connected_inner(End::B));

    EXPECT_TRUE(g[0].is_neig_of(g[1]));  // through its end B
    EXPECT_TRUE(g[1].is_neig_of(g[0]));
    EXPECT_TRUE(g[1].is_neig_of(g[2]));
    EXPECT_TRUE(g[2].is_neig_of(g[1]));  // through its end A
    EXPECT_FALSE(g[0].is_neig_of(g[2]));
    EXPECT_FALSE(g[2].is_neig_of(g[0]));
}


/// is_unconnected_linear() requires both ends to be free.
TEST_F(FixesTest, ChainIsUnconnectedLinear)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Chain::is_unconnected_linear()");

    const auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    EXPECT_TRUE(gr.chain(ChIdG {0}).is_unconnected_linear());   // free chain

    const auto& m2 = gr.chain(ChIdG {2});  // free at A, at a junction at B
    ASSERT_FALSE(m2.is_connected_at(End::A));
    ASSERT_TRUE(m2.is_connected_at(End::B));
    EXPECT_FALSE(m2.is_unconnected_linear());

    EXPECT_FALSE(gr.chain(ChIdG {12}).is_unconnected_linear());  // cycle

    for (const auto& m : gr.chains())
        EXPECT_EQ(m.is_unconnected_linear(),
                  !m.is_connected_at(End::A) && !m.is_connected_at(End::B));
}


/// The internal vertex shared by two adjacent edges.
TEST_F(FixesTest, ChainVertexAtBulkOfEdgePair)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Chain::vertex_at_bulk() for a pair of edges");

    G gr;
    gr.add_single_chain_component(3);
    const IntegrityCheck integrityCheck {gr};

    const auto& m = gr.chain(ChIdG {});

    EXPECT_EQ(m.vertex_at_bulk(m.g[0].ind, m.g[1].ind), m.vertex_at_bulk(EgIdA {1}));
    EXPECT_EQ(m.vertex_at_bulk(m.g[1].ind, m.g[0].ind), m.vertex_at_bulk(EgIdA {1}));
    EXPECT_EQ(m.vertex_at_bulk(m.g[2], m.g[1]), m.vertex_at_bulk(EgIdA {2}));
}


/// Moving edges into the interior of another chain renumbers its edges.
TEST_F(FixesTest, MoveEdgesToInnerPosition)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Graph::move_edges() to a position inside ",
                          "the target chain");

    auto gr = create_sample_graph<G>();

    // Chains 2 and 3 share a component (see create_sample_graph()).
    auto& fr = gr.chain(ChIdG {3});
    auto& to = gr.chain(ChIdG {2});
    ASSERT_EQ(fr.c, to.c);
    const auto lenFr = fr.length();
    const auto lenTo = to.length();
    ASSERT_GE(lenFr, 2);
    ASSERT_GE(lenTo, 2);

    gr.move_edges(fr, 0, 1, to, 1);

    ASSERT_EQ(fr.length(), lenFr - 1);
    ASSERT_EQ(to.length(), lenTo + 1);

    const auto& cmp = gr.compt(to.c);
    for (const auto* m : {&fr, &to})
        for (std::size_t a {}; a < m->length(); ++a) {
            const auto& eg = m->g[a];
            EXPECT_EQ(eg.indw, a) << "chain " << m->idw;
            EXPECT_EQ(eg.w, m->idw);
            EXPECT_EQ(cmp.get_egl(eg.indc).w, m->idw);
            EXPECT_EQ(cmp.get_egl(eg.indc).a, a);
            EXPECT_EQ(gr.get_egl(eg.ind).w, m->idw);
            EXPECT_EQ(gr.get_egl(eg.ind).a, a);
        }
}


/// Component members previously failing to compile, or misbehaving.
TEST_F(FixesTest, ComponentLookups)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Component::operator==, edge(EgIdG), ",
                          "chain(ChIdC), contains/find(Vertex), find_vertex()");

    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    const auto& c = gr.compt(gr.chain(ChIdG {2}).c);
    const auto& other = gr.compt(gr.chain(ChIdG {0}).c);

    EXPECT_TRUE(c == c);
    EXPECT_FALSE(c == other);

    for (const auto w : c.ww) {
        const auto& m = gr.chain(w);
        EXPECT_EQ(&c.chain(m.idc), &m);
        for (const auto& eg : m.g)
            EXPECT_EQ(c.edge(eg.ind).id, eg.id);
    }
    EXPECT_THROW(c.chain(ChIdC {99}), precondition_error);
    EXPECT_THROW(c.edge(gr.chain(ChIdG {0}).g[0].ind), precondition_error);

    for (const auto& vp : c.vertices().get()) {
        EXPECT_TRUE(c.contains(*vp));
        EXPECT_EQ(c.find(*vp), vp.get());
        EXPECT_EQ(c.find_vertex(vp->get_id()), vp.get());
        EXPECT_FALSE(other.contains(*vp));
        EXPECT_EQ(other.find(*vp), nullptr);
    }
}


/// Neighbour vertices, and removal of a slot from the neighbour lists.
TEST_F(FixesTest, ComponentNeighbours)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Component::ngs_of(Vertex) and ",
                          "Component::remove_slot_from_neigs()");

    auto gr = create_sample_graph<G>();
    auto& c = gr.compt(gr.chain(ChIdG {2}).c);

    // The 3-way junction at end B of chain 2.
    const ESlot s {ChIdG {2}, End::B};
    const auto* v = gr.vertex(s);
    ASSERT_EQ(v->deg(), Deg3);

    const auto nn = c.ngs_of(*v);
    ASSERT_EQ(nn.size(), Deg3);
    for (const auto* n : nn) {
        EXPECT_NE(n, v);
        EXPECT_TRUE(v->common_edge(*n).is_defined());
    }

    const auto partners = gr.ngs_at(s).as_vector();
    c.remove_slot_from_neigs(s);
    EXPECT_EQ(gr.ngs_at(s).num(), 0);
    for (const auto& p : partners)
        EXPECT_FALSE(gr.ngs_at(p).contains(s));
}


/// Rebuilding the component records from the chains changes nothing.
TEST_F(FixesTest, ComponentUpdateEdgeDescriptors)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Component::update_edge_descriptors()");

    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    for (auto& c : gr.compts()) {

        const auto egl = c.get_egl();  // copy
        auto ww = c.ww;

        c.update_edge_descriptors();

        EXPECT_TRUE(c.ww_is_sorted());
        auto ww2 = c.ww;
        std::ranges::sort(ww);
        std::ranges::sort(ww2);
        EXPECT_EQ(ww, ww2);
        ASSERT_EQ(c.get_egl().size(), egl.size());
        for (std::size_t i {}; i < egl.size(); ++i)
            EXPECT_TRUE(c.get_egl()[i] == egl[i]) << "component " << c.ind;
    }
}


// =============================================================================
// Graph
// =============================================================================

/// Generation of single-chain components next to multi-chain ones.
TEST_F(FixesTest, GenerateSingleChainComponentsAddsNum)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that generate_single_chain_components() ",
                          "adds the requested number of components");

    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    const auto nc = gr.num_compts();
    const auto nw = gr.num_chains();
    const auto ne = gr.num_edges();
    ASSERT_NE(nc, nw);  // some components hold several chains

    gr.generate_single_chain_components(2, 3);

    EXPECT_EQ(gr.num_compts(), nc + 2);
    EXPECT_EQ(gr.num_chains(), nw + 2);
    EXPECT_EQ(gr.num_edges(), ne + 6);
}


/// One-line summary of the graph covers the degrees 1 to 4.
TEST_F(FixesTest, GraphReport)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Graph::report()");

    G gr;
    gr.add_single_chain_component(3);
    const IntegrityCheck integrityCheck {gr};

    std::ostringstream os;
    gr.report(os);
    EXPECT_EQ(os.str(), " v 2 2 0 0 egn 3 cnn 1 ctn 1");

    const auto grS = create_sample_graph<G>();
    std::ostringstream osS;
    grS.report(osS);
    std::ostringstream expected;
    expected << " v";
    for (const auto d : degrees)
        expected << ' ' << grS.num_vertices(d);
    expected << " egn " << grS.num_edges() << " cnn " << grS.num_chains()
             << " ctn " << grS.num_compts();
    EXPECT_EQ(osS.str(), expected.str());
    EXPECT_NE(grS.num_vertices(Deg4), 0);
}


/// Graph-wide neighbour indexes and adjacency lists.
TEST_F(FixesTest, GraphNeighboursAndAdjacency)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Graph::ng_inds_at(), update_adjacency() ",
                          "and print_adjacency_lists()");

    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    const ESlot s {ChIdG {2}, End::B};
    std::set<EgIdG> expected;
    for (const auto& n : gr.ngs_at(s))
        expected.insert(gr.chain(n.w).end2ind(n.e));
    const auto inds = gr.ng_inds_at(s);
    EXPECT_EQ(std::set<EgIdG>(inds.begin(), inds.end()), expected);
    EXPECT_EQ(inds.size(), 2);

    gr.update_adjacency();
    for (const auto& c : gr.compts())
        EXPECT_EQ(c.adj.lg, structure::Adjacency<Compt>::list_edges(c));

    auto& c2 = gr.compt(gr.chain(ChIdG {2}).c);
    c2.adj.clear();
    gr.update_adjacency_edges(gr.chain(ChIdG {2}).g[0].ind);
    EXPECT_EQ(c2.adj.lg, structure::Adjacency<Compt>::list_edges(c2));

    EXPECT_NO_THROW(gr.print_adjacency_lists("adj "));
}


// =============================================================================
// Containers and descriptors
// =============================================================================

/// Chain classes report their sizes.
TEST_F(FixesTest, ChainClassesReport)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests ChainClasses::report()");

    G gr;
    gr.add_single_chain_component(2);
    const IntegrityCheck integrityCheck {gr};

    std::ostringstream os;
    gr.compt(CmpId {}).chis.report(os);
    EXPECT_EQ(os.str(), " s_1_1 1");

    const auto grS = create_sample_graph<G>();
    std::ostringstream osS;
    grS.compt(grS.chain(ChIdG {2}).c).chis.report(osS);
    EXPECT_NE(osS.str().find(" s_1_3 3"), std::string::npos) << osS.str();
}


/// Neighbour set as an array of its exact size.
TEST_F(FixesTest, NeigsAsArray)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Neigs::as_array()");

    const auto gr = create_sample_graph<G>();
    const auto& ngs = gr.ngs_at(ESlot {ChIdG {2}, End::B});
    ASSERT_EQ(ngs.num(), 2);

    const auto arr = ngs.as_array<2>();
    const auto vec = ngs.as_vector();
    EXPECT_TRUE(std::ranges::equal(arr, vec));

    EXPECT_THROW(ngs.as_array<3>(), precondition_error);
    EXPECT_THROW(ngs.as_array<1>(), precondition_error);
}


/// Edge descriptor containers and descriptors.
TEST_F(FixesTest, EdgeDescriptors)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests EdgeDescriptorsInGraph::append(Edge), ",
                          "EdgeDescriptorsInComponent::check() and ",
                          "EdgeInComponent::is_defined()");

    auto gr = create_sample_graph<G>();

    G::EdgeDescriptors egl;
    const auto& eg = gr.chain(ChIdG {3}).g[1];
    egl.append(eg);
    ASSERT_EQ(egl.size(), 1);
    EXPECT_EQ(egl[0].w, eg.w);
    EXPECT_EQ(egl[0].a, eg.indw);
    EXPECT_EQ(egl[0].h, eg.id);

    using Descr = Compt::EdgeDescr;
    EXPECT_FALSE(Descr {}.is_defined());
    EXPECT_TRUE(Descr {eg}.is_defined());

    auto& c = gr.compt(eg.c);
    EXPECT_NO_THROW(c.get_egl().check(eg, c.ind));

    auto& d = c.get_egl(eg.indc);
    const auto saved = d;
    d.a = EgIdA {99};
    EXPECT_THROW(c.get_egl().check(eg, c.ind), invariant_error);
    d = saved;
    EXPECT_NO_THROW(c.get_egl().check(eg, c.ind));
}


/// Vertex lookup by an attached edge end.
TEST_F(FixesTest, VerticesFindByEdgeEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Vertices::find(EdgeSlotC) and the free ",
                          "function find_vertex()");

    const auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    for (const auto& c : gr.compts())
        for (const auto w : c.ww)
            for (const auto& eg : gr.chain(w).g)
                for (const auto e : End::Ids) {
                    const structure::descriptors::EdgeSlotC s {eg.indc, e};
                    const auto* v = eg.vertex_at_inner(e);
                    EXPECT_EQ(c.vertices().find(s), v);
                    EXPECT_EQ(structure::containers::find_vertex(s, c.vertices()),
                              v->get_id());
                }

    const auto& c0 = gr.compt(CmpId {});
    const structure::descriptors::EdgeSlotC missing {EgIdC {999}, End::A};
    EXPECT_EQ(c0.vertices().find(missing), nullptr);
    EXPECT_FALSE(structure::containers::find_vertex(missing, c0.vertices())
                     .is_defined());
    structure::containers::print<Deg2>(c0.vertices(), "deg2 ");
}


/// Sequence forwards the arguments and moves the inserted values.
TEST_F(FixesTest, SequenceOfMoveOnlyElements)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Sequence::emplace_back() and insert() ",
                          "with move-only elements");

    structure::containers::Sequence<std::vector<std::unique_ptr<int>>> s;
    s.emplace_back(std::make_unique<int>(1));
    s.insert(s.begin(), std::make_unique<int>(2));
    auto p = std::make_unique<int>(3);
    s.emplace_back(std::move(p));

    ASSERT_EQ(s.size(), 3);
    EXPECT_EQ(*s[0], 2);
    EXPECT_EQ(*s[1], 1);
    EXPECT_EQ(*s[2], 3);
    EXPECT_EQ(p, nullptr);
}


// =============================================================================
// Transformations
// =============================================================================

/// With GRAFFINE_DO_INTEGRAL_TESTS, operator() and try_apply() check the graph.
TEST_F(FixesTest, IntegrityChecksAfterTransformations)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the integrity checks run after the ",
                          "transformations if GRAFFINE_DO_INTEGRAL_TESTS is ON");

    if (!doIntegralTests)
        GTEST_SKIP() << "GRAFFINE_DO_INTEGRAL_TESTS is OFF";

    G gr;
    gr.add_single_chain_component(2);
    transforms::component_creation::Functor<G> create {gr};

    // A valid graph passes the checks.
    EXPECT_NO_THROW(create(2));
    EXPECT_TRUE(create.try_apply(2).has_value());

    // An out-of-range weight on an edge the transformation does not touch
    // is reported once the transformation is done.
    gr.edge(EgIdG {}).weight() = std::numeric_limits<real>::quiet_NaN();
    EXPECT_THROW(create(2), invariant_error);
    EXPECT_THROW((void)create.try_apply(2), invariant_error);

    // apply() skips the checks.
    EXPECT_NO_THROW(create.apply(2));
}


}  // namespace graffine::tests::fixes
