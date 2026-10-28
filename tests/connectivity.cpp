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
#include "sample_graph.h"
#include "vertex_split_common.h"
#include "graffine/structure/analyzers/connectivity.h"
#include "graffine/structure/modifiers/graph_factory.h"

#include <deque>
#include <set>
#include <utility>
#include <vector>

namespace graffine::tests::connectivity {

using vertex_split::VertexMerger;
using vertex_split::VertexSplit;

using Conty = structure::Connectivity<Chain>;
using ChainSet = std::set<std::size_t>;

constexpr auto eA = End::A;
constexpr auto eB = End::B;


/**
 * Reference reachability by breadth-first search over the end slots.
 * \details Slots are adjacent if they share a junction or bound the same
 * chain. The step from \p src across its own chain is excluded, so that
 * the search proceeds from the junction at \p src only.
 */
bool reference_reach(const G& gr, const ESlot src, const ESlot tgt)
{
    std::set<std::pair<std::size_t, int>> seen;
    auto key = [](const ESlot& s) { return std::pair {s.w(), int(s.e)}; };

    std::deque<ESlot> q {src};
    seen.insert(key(src));
    while (!q.empty()) {
        const auto s = q.front();
        q.pop_front();
        if (s == tgt)
            return true;
        std::vector<ESlot> next {gr.chain(s.w).ngs[s.e]().begin(),
                                 gr.chain(s.w).ngs[s.e]().end()};
        if (s != src)
            next.push_back(s.opp());
        for (const auto& n : next)
            if (seen.insert(key(n)).second)
                q.push_back(n);
    }
    return false;
}

/// Reference connected set of chains containing chain \p w.
ChainSet reference_chains(const G& gr, const ChIdG w)
{
    ChainSet res {w()};
    for (ChIdG u {}; u < gr.num_chains(); ++u)
        for (const auto e : End::Ids)
            if (reference_reach(gr, ESlot{w, eA}, ESlot{u, e}) ||
                reference_reach(gr, ESlot{w, eB}, ESlot{u, e}))
                res.insert(u());
    return res;
}

ChainSet as_set(const ChIdGs& ww)
{
    ChainSet res;
    for (const auto w : ww)
        res.insert(w());
    return res;
}


/// Two junctions joined by a bridge, two leaves at each: a,b >-w-< c,d.
G make_dumbbell()
{
    G gr;
    gr.add_single_chain_component(9);
    gr.add_single_chain_component(3);
    gr.add_single_chain_component(3);

    VertexMerger<Deg1, Deg2, G> merge12 {gr};
    merge12(ESlot{ChIdG{1}, eA}, BSlot{ChIdG{0}, 3});
    merge12(ESlot{ChIdG{2}, eA}, BSlot{gr.ind_last_chain(), 3});

    return gr;
}

/// A loop split into two chains by two junctions, with a tail at one
/// junction and a leaf at the other.
G make_theta()
{
    G gr;
    gr.add_single_chain_component(12);
    gr.add_single_chain_component(3);

    VertexMerger<Deg1, Deg2, G> merge12 {gr};
    merge12(ESlot{ChIdG{0}, eA}, BSlot{ChIdG{0}, 4});
    const auto loop = gr.chain(ChIdG{0}).is_connected_cycle()
                    ? ChIdG{0}
                    : gr.ind_last_chain();
    merge12(ESlot{ChIdG{1}, eA}, BSlot{loop, 3});

    return gr;
}

/// Three junctions joined by two bridges in a row: a,b >-w1-< c >-w2-< d,e.
G make_bridges()
{
    G gr;
    gr.add_single_chain_component(15);
    for (int i {}; i < 3; ++i)
        gr.add_single_chain_component(2);

    VertexMerger<Deg1, Deg2, G> merge12 {gr};
    merge12(ESlot{ChIdG{1}, eA}, BSlot{ChIdG{0}, 3});
    merge12(ESlot{ChIdG{2}, eA}, BSlot{gr.ind_last_chain(), 4});
    merge12(ESlot{ChIdG{3}, eA}, BSlot{gr.ind_last_chain(), 4});

    return gr;
}


class ConnectivityTest
    : public Test {

protected:

    /// Compares Connectivity against the reference on all pairs of slots.
    static void expect_matches_reference(const G& gr)
    {
        Conty conty {gr.chains()};

        for (ChIdG w1 {}; w1 < gr.num_chains(); ++w1)
            for (const auto e1 : End::Ids)
                for (ChIdG w2 {}; w2 < gr.num_chains(); ++w2)
                    for (const auto e2 : End::Ids) {
                        const ESlot s {w1, e1};
                        const ESlot t {w2, e2};
                        if (s == t)
                            continue;
                        EXPECT_EQ(conty.dfs(s, t), reference_reach(gr, s, t))
                            << "from {" << w1 << " " << End::str(e1)
                            << "} to {" << w2 << " " << End::str(e2) << "}";
                    }

        for (ChIdG w {}; w < gr.num_chains(); ++w)
            EXPECT_EQ(as_set(conty.find_connected_chains(w)),
                      reference_chains(gr, w)) << "seed " << w;
    }

    /**
     * Applies a 1+2 split at every slot of a degree 3 junction, each to a
     * fresh copy of the graph, and checks that every resulting component
     * is a complete connected set of chains.
     */
    template<typename Make>
    static void split12_everywhere(Make make)
    {
        const auto proto = make();
        for (ChIdG w {}; w < proto.num_chains(); ++w)
            for (const auto e : End::Ids) {
                if (proto.chain(w).ngs[e].num() != 2)
                    continue;

                G gr = make();
                const IntegrityCheck integrityCheck {gr};
                VertexSplit<Deg1, Deg2, G> split {gr};
                split(ESlot{w, e});

                for (CmpId c {}; c < gr.num_compts(); ++c) {
                    const auto& cmp = gr.compt(c);
                    EXPECT_EQ(as_set(cmp.ww), reference_chains(gr, cmp.ww[0]))
                        << "split at {" << w << " " << End::str(e)
                        << "}, component " << c;
                }
            }
    }
};


/// A slot at the same junction is reachable even if its chain continues to
/// another junction.
TEST_F(ConnectivityTest, AdjacentChainConnectedAtBothEnds)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that a junction neighbour connected at both ",
                          "ends is found by dfs");

    const auto gr = make_dumbbell();
    Conty conty {gr.chains()};

    // The bridge is the chain connected at both ends.
    ChIdG bridge {};
    for (ChIdG w {}; w < gr.num_chains(); ++w)
        if (gr.chain(w).ngs[eA].num() && gr.chain(w).ngs[eB].num())
            bridge = w;

    for (const auto e : End::Ids)
        for (const auto& s : gr.chain(bridge).ngs[e]()) {
            EXPECT_TRUE(conty.dfs(s, ESlot{bridge, e}));
            EXPECT_TRUE(conty.dfs(s, ESlot{bridge, End::opp(e)}));
        }
}


/// Leaves at opposite ends of a bridge are connected.
TEST_F(ConnectivityTest, CrossesBridges)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that dfs proceeds beyond chains connected ",
                          "at both ends");

    const auto gr = make_bridges();
    Conty conty {gr.chains()};

    std::vector<ESlot> leaves;
    for (ChIdG w {}; w < gr.num_chains(); ++w)
        if (gr.chain(w).has_single_leaf_vertex())
            leaves.emplace_back(w, End::opp(gr.chain(w).the_only_leaf_end()));

    ASSERT_EQ(leaves.size(), 5);
    for (const auto& s : leaves)
        for (const auto& t : leaves)
            if (s != t) {
                EXPECT_TRUE(conty.dfs(s, t));
                EXPECT_TRUE(conty.dfs(s, t.opp()));
            }
}


/// The source chain is not entered through the source slot, even when
/// a connected cycle at the same junction leads back to it.
TEST_F(ConnectivityTest, SourceChainNotReentered)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that dfs does not re-enter the source chain ",
                          "through its own junction");

    const auto gr = create_sample_graph<G>();
    Conty conty {gr.chains()};

    // Chain 16 hangs off the connected cycle 6 (see sample_graph.h).
    constexpr ChIdG tail {16};
    ASSERT_TRUE(gr.chain(ChIdG{6}).is_connected_cycle());
    ASSERT_FALSE(gr.chain(tail).is_connected_at(eB));

    EXPECT_FALSE(conty.dfs(ESlot{tail, eA}, ESlot{tail, eB}));
    EXPECT_TRUE(conty.dfs(ESlot{ChIdG{6}, eA}, ESlot{tail, eB}));

    // Nothing is reachable from a free end.
    EXPECT_FALSE(conty.dfs(ESlot{tail, eB}, ESlot{tail, eA}));
}


/// Exhaustive comparison against the reference search.
TEST_F(ConnectivityTest, MatchesReference)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dfs and find_connected_chains against a ",
                          "reference search on all pairs of chain ends");

    expect_matches_reference(create_sample_graph<G>());
    expect_matches_reference(make_dumbbell());
    expect_matches_reference(make_theta());
    expect_matches_reference(make_bridges());
}


/// Searches reuse the object state without leaking marks between them.
TEST_F(ConnectivityTest, RepeatedSearches)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests repeated searches on the same object");

    const auto gr = create_sample_graph<G>();
    Conty conty {gr.chains()};

    for (int i {}; i < 3; ++i)
        for (ChIdG w {}; w < gr.num_chains(); ++w) {
            const auto ww = conty.find_connected_chains(w);
            EXPECT_EQ(as_set(ww), reference_chains(gr, w));
            EXPECT_TRUE(std::is_sorted(ww.begin(), ww.end()));
        }
}


/// The 1+2 split yields complete components in every branch.
TEST_F(ConnectivityTest, Split12Components)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that the 1+2 vertex split yields complete ",
                          "connected components at every degree 3 slot");

    split12_everywhere(make_dumbbell);
    split12_everywhere(make_theta);
    split12_everywhere(make_bridges);
    split12_everywhere([] { return create_sample_graph<G>(); });
}


/// The graph factory groups chains into components across junctions.
TEST_F(ConnectivityTest, FactoryComponents)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that the graph factory assembles components ",
                          "spanning several junctions");

    using Vertex = G::Vertex;
    using Factory = structure::modifiers::GraphFactory<G>;

    // Dumbbell 0-1-2(-3) 2-4-5(-6)(-7); theta 8 =(9|10|11)= 12; chain 13-14.
    constexpr std::array<std::pair<uint, uint>, 15> links {{
        {0, 1}, {1, 2}, {3, 2}, {2, 4}, {4, 5}, {5, 6}, {5, 7},
        {8, 9}, {9, 12}, {8, 10}, {10, 12}, {8, 11}, {11, 12},
        {13, 14}, {14, 15}}};

    std::vector<std::unique_ptr<Vertex>> vv;
    for (uint i {}; i < 16; ++i)
        vv.push_back(std::make_unique<Vertex>(VIdH {i}, false));

    std::vector<Edge> egs;
    for (std::size_t i {}; i < links.size(); ++i) {
        auto& eg = egs.emplace_back(EgIdG {i}, Orientation::Forwards,
                                    Edge::weight_t {1});
        eg.connect_to_outer(End::A, *vv[links[i].first]);
        eg.connect_to_outer(End::B, *vv[links[i].second]);
    }

    G gr;
    const IntegrityCheck integrityCheck {gr};
    Factory {}.create(gr, std::move(vv), std::move(egs));

    ASSERT_EQ(gr.num_compts(), 3);
    std::multiset<std::size_t> sizes;
    for (CmpId c {}; c < gr.num_compts(); ++c) {
        const auto& cmp = gr.compt(c);
        sizes.insert(cmp.num_chains());
        EXPECT_EQ(as_set(cmp.ww), reference_chains(gr, cmp.ww[0]));
    }
    EXPECT_EQ(sizes, (std::multiset<std::size_t> {1, 3, 5}));
}


}  // namespace graffine::tests::connectivity
