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
 * \file graph_factory.cpp
 * \brief Tests that GraphFactory assembles exactly the graph assembled by
 * its original implementation, kept in graph_factory_reference.h.
 * \details Users rely on the numbering and ordering produced by the factory
 * (chain ids, edge order in chains, components and their chain order,
 * component-wide indices, vertex order), so these are compared element for
 * element on random graphs.
 */

#include "common.h"
#include "graph_factory_reference.h"
#include "graffine/structure/analyzers/integral_tests.h"
#include "graffine/structure/modifiers/graph_factory.h"

#include <algorithm>  // shuffle, min
#include <cstddef>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace graffine::tests::graph_factory {

using Vertex = G::Vertex;
using Factory = structure::modifiers::GraphFactory<G>;
using Reference = structure::modifiers::reference::ReferenceGraphFactory<G>;


/// Parameters of a random graph.
struct Params {
    std::size_t numJunctions {};  ///< Vertices of degree 1, 3 or 4.
    std::size_t maxLen {1};       ///< Maximal number of edges in a link.
    std::size_t numRings {};      ///< Free cycles of degree-2 vertices.
    std::size_t numLinear {};     ///< Isolated linear chains.
    bool shuffleEdges {};         ///< Edges listed in random order.
    bool shuffleVertices {};      ///< Vertices listed in random order.
    bool swapEnds {};             ///< Paths randomly reversed.
};


/// Input of the factory.
struct Input {
    std::vector<std::unique_ptr<Vertex>> vv;
    std::vector<Edge> egs;
    std::size_t baseId {};  ///< Smallest edge handle, for the comparison.
};


/** Random graph: junctions of degree 1, 3 and 4 linked pairwise at random
 * (configuration model, yielding loops at junctions, parallel links and
 * many components) by paths of degree-2 vertices, plus free rings and
 * isolated linear chains.
 */
auto generate(const unsigned seed, const Params& p) -> Input
{
    std::mt19937 rng {seed};
    auto uniform = [&rng](const std::size_t lo, const std::size_t hi) {
        return std::uniform_int_distribution<std::size_t> {lo, hi}(rng);
    };

    Input in;
    auto new_vertex = [&in]() {
        const auto i = in.vv.size();
        in.vv.push_back(std::make_unique<Vertex>(VIdH {i}, false));
        return i;
    };

    // Outer ends are chain coordinates: along a path through degree-2
    // vertices, the end B of each edge meets the end A of the next one, as
    // in any saved graph. Hence a path is reversed only as a whole.
    std::vector<std::pair<std::size_t, std::size_t>> links;
    auto path = [&](const std::size_t u, const std::size_t v,
                    const std::size_t len) {
        const auto first = links.size();
        auto prev = u;
        for (std::size_t k {1}; k < len; ++k) {
            const auto x = new_vertex();
            links.emplace_back(prev, x);
            prev = x;
        }
        links.emplace_back(prev, v);
        if (p.swapEnds && uniform(0, 1)) {
            std::reverse(links.begin() + first, links.end());
            for (auto i {first}; i < links.size(); ++i)
                std::swap(links[i].first, links[i].second);
        }
    };

    std::vector<std::size_t> stubs;
    for (std::size_t j {}; j < p.numJunctions; ++j) {
        const auto r = uniform(0, 9);
        const std::size_t deg = r < 3 ? 1 : r < 7 ? 3 : 4;
        const auto v = new_vertex();
        stubs.insert(stubs.end(), deg, v);
    }
    if (stubs.size() % 2)
        stubs.push_back(new_vertex());  // a leaf
    std::shuffle(stubs.begin(), stubs.end(), rng);
    for (std::size_t i {}; i + 1 < stubs.size(); i += 2) {
        const auto u = stubs[i];
        const auto v = stubs[i+1];
        auto len = uniform(1, p.maxLen);
        if (u == v)  // a loop at a junction: a connected cycle
            len = std::max(len, Chain::minCycleLength);
        path(u, v, len);
    }

    for (std::size_t r {}; r < p.numRings; ++r) {
        const auto v = new_vertex();
        path(v, v, uniform(Chain::minCycleLength, p.maxLen + 1));
    }

    for (std::size_t r {}; r < p.numLinear; ++r) {
        const auto u = new_vertex();
        path(u, new_vertex(), uniform(1, p.maxLen));
    }

    if (p.shuffleEdges)
        std::shuffle(links.begin(), links.end(), rng);

    in.egs.reserve(links.size());
    for (std::size_t i {}; i < links.size(); ++i) {
        const auto [u, v] = links[i];
        const auto o = uniform(0, 1) ? Orientation::Forwards
                                     : Orientation::Backwards;
        auto& eg = in.egs.emplace_back(EgIdG {i}, o, Edge::weight_t {1});
        eg.connect_to_outer(End::A, *in.vv[u]);
        eg.connect_to_outer(End::B, *in.vv[v]);
    }
    in.baseId = in.egs.empty() ? 0 : in.egs.front().id();
    for (const auto& eg : in.egs)
        in.baseId = std::min(in.baseId, eg.id());

    if (p.shuffleVertices)
        std::shuffle(in.vv.begin(), in.vv.end(), rng);

    return in;
}


/** Complete description of an assembled graph. Edge handles are written
 * relative to \p baseId, so that the graphs assembled from two identical
 * inputs, generated one after the other, compare equal.
 */
auto dump(const G& gr, const std::size_t baseId) -> std::string
{
    std::ostringstream o;
    auto h = [baseId](const EgIdH id) { return id() - baseId; };

    o << "edges " << gr.num_edges() << " chains " << gr.num_chains()
      << " compts " << gr.num_compts() << '\n';

    for (ChIdG w {}; w < gr.num_chains(); ++w) {
        const auto& m = gr.chain(w);
        o << "chain " << m.idw() << " c " << m.c() << " idc " << m.idc()
          << " len " << m.length() << '\n';
        for (const auto e : End::Ids) {
            o << "  ngs" << int(e);
            for (const auto& s : m.ngs[e]())
                o << " (" << s.w() << ',' << int(s.e) << ')';
            o << '\n';
        }
        for (const auto& eg : m.g)
            o << "  edge " << h(eg.id) << " ind " << eg.ind()
              << " indw " << eg.indw() << " indc " << eg.indc()
              << " w " << eg.w() << " c " << eg.c()
              << " ornt " << int(eg.orientation())
              << " vA " << eg.vid_at(End::A)()
              << " vB " << eg.vid_at(End::B)() << '\n';
    }

    for (CmpId c {}; c < gr.num_compts(); ++c) {
        const auto& cmp = gr.compt(c);
        o << "compt " << cmp.ind() << " ww";
        for (const auto& w : cmp.ww)
            o << ' ' << w();
        o << '\n';
        for (const auto& v : cmp.vv.get()) {
            o << "  vertex " << v->get_id()() << " c " << v->get_c()()
              << " deg " << v->deg();
            for (const auto& s : v->edges())
                o << " (" << h(s.idh) << ',' << int(s.e) << ','
                  << s.indc()() << ')';
            o << '\n';
        }
        const auto& egl = cmp.get_egl();
        o << "  egl";
        for (std::size_t k {}; k < egl.size(); ++k)
            o << " (" << egl[EgIdC {k}].w() << ',' << egl[EgIdC {k}].a()
              << ',' << h(egl[EgIdC {k}].h) << ')';
        o << '\n';
    }

    const auto& egl = gr.get_egl();
    o << "graph egl";
    for (std::size_t k {}; k < egl.size(); ++k)
        o << " (" << egl[EgIdG {k}].w() << ',' << egl[EgIdG {k}].a()
          << ',' << h(egl[EgIdG {k}].h) << ')';
    o << '\n';

    return o.str();
}


struct GraphFactoryTest
    : public Test {};


/// Assembles the graph from seed and parameters with both factories and
/// compares the results.
void compare(const unsigned seed, const Params& p)
{
    SCOPED_TRACE("seed " + std::to_string(seed));

    auto ref = generate(seed, p);
    const auto grRef = Reference {}.create(std::move(ref.vv),
                                           std::move(ref.egs));
    auto in = generate(seed, p);
    G gr;
    Factory {}.create(gr, std::move(in.vv), std::move(in.egs));

    ASSERT_EQ(dump(gr, in.baseId), dump(grRef, ref.baseId));

    const auto res = structure::IntegralTests<G, profuse> {gr}.validate();
    EXPECT_TRUE(res) << (res ? "" : res.error().message);
}


TEST_F(GraphFactoryTest, MatchesReferenceOnRandomGraphs)
{
    if constexpr (verboseT)
        print_description(
            "Tests that the factory assembles random graphs exactly as its "
            "original implementation: chain numbering and edge order, chain "
            "connections, components, indices and vertex order");

    unsigned seed {};
    for (const std::size_t numJunctions : {0, 1, 2, 5, 20, 100, 400})
        for (const std::size_t maxLen : {1, 2, 5, 20})
            for (const bool shuffled : {false, true})
                for (const bool rings : {false, true}) {
                    Params p {.numJunctions = numJunctions,
                              .maxLen = maxLen,
                              .numRings = rings ? 1 + numJunctions / 10 : 0,
                              .numLinear = rings ? 1 + numJunctions / 20 : 0,
                              .shuffleEdges = shuffled,
                              .shuffleVertices = shuffled,
                              .swapEnds = shuffled};
                    if (!numJunctions && !rings)
                        continue;  // empty graph: tested separately
                    for (unsigned r {}; r < 3; ++r)
                        compare(++seed, p);
                }
}


TEST_F(GraphFactoryTest, MatchesReferenceOnLargeGraph)
{
    if constexpr (verboseT)
        print_description(
            "Tests that the factory assembles a large random graph exactly "
            "as its original implementation");

    compare(12345, Params {.numJunctions = 1500, .maxLen = 8,
                           .numRings = 20, .numLinear = 20,
                           .shuffleEdges = true, .shuffleVertices = true,
                           .swapEnds = true});
}


TEST_F(GraphFactoryTest, EmptyInputGivesEmptyGraph)
{
    if constexpr (verboseT)
        print_description(
            "Tests that the factory assembles an empty graph from no edges");

    G gr;
    Factory {}.create(gr, {}, {});
    EXPECT_EQ(gr.num_chains(), 0);
    EXPECT_EQ(gr.num_compts(), 0);
    EXPECT_EQ(gr.num_edges(), 0);
}


}  // namespace graffine::tests::graph_factory
