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
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/edge_split/functor.h"

#include <array>
#include <cstddef>
#include <vector>


namespace graffine::tests::edge_split_merger {

using namespace graffine;
namespace elements = structure;
namespace trs = transforms;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

using EdgeSplit = trs::edge_split::Functor<G>;
using EdgeMerger = trs::edge_merger::Functor<G>;


/// Topology-level summary of a graph, used to compare graph states.
struct Snapshot {

    std::size_t numEdges {};
    std::size_t numChains {};
    std::size_t numCompts {};
    std::vector<std::size_t> lengths;               ///< Per chain.
    std::vector<std::array<std::size_t, 2>> ngs;    ///< Per chain end.
    std::array<std::size_t, 4> numVertices {};      ///< Per degree 1 to 4.

    explicit Snapshot(const G& gr)
        : numEdges {gr.num_edges()}
        , numChains {gr.num_chains()}
        , numCompts {gr.num_compts()}
        , numVertices {gr.num_vertices(Deg1), gr.num_vertices(Deg2),
                       gr.num_vertices(Deg3), gr.num_vertices(Deg4)}
    {
        for (ChIdG w {}; w < gr.num_chains(); ++w) {
            const auto& m = gr.chain(w);
            lengths.push_back(m.length());
            ngs.push_back({m.ngs[End::A].num(), m.ngs[End::B].num()});
        }
    }

    bool operator==(const Snapshot&) const = default;
};


/// Checks graph-wide edge indexes to be consistent with the edge positions.
void check_edge_indexes(const G& gr)
{
    std::vector<bool> seen(gr.num_edges());

    for (ChIdG w {}; w < gr.num_chains(); ++w) {
        const auto& m = gr.chain(w);
        for (EgIdA a {}; a < m.length(); ++a) {
            const auto& eg = m.g[a];
            ASSERT_LT(eg.ind, gr.num_edges());
            ASSERT_FALSE(seen[eg.ind()]);
            seen[eg.ind()] = true;
            ASSERT_EQ(eg.w, w);
            ASSERT_EQ(eg.indw, a);
            ASSERT_EQ(&gr.edge(eg.ind), &eg);
        }
    }
}


class EdgeSplitMergerTest
    : public Test
{};

// =============================================================================


/// Tests split of an internal edge of a linear chain, and merger reversing it.
TEST_F(EdgeSplitMergerTest, LineInternalEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of an internal edge of a linear chain, ",
            "and merger reversing it"
        );

    constexpr std::size_t len {4};
    constexpr ChIdG w {0};
    constexpr EgIdG ind {1};  // source edge, at position 1 in the chain

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(len);
    const Snapshot before {gr};

    EdgeSplit split {gr};
    EdgeMerger merge {gr};

    const auto res = split(ind);
    const auto& m = gr.chain(w);

    ASSERT_TRUE(res[0]);
    ASSERT_EQ(gr.num_edges(), len + 1);
    ASSERT_EQ(gr.num_chains(), 1);
    ASSERT_EQ(gr.num_compts(), 1);
    ASSERT_EQ(m.length(), len + 1);
    ASSERT_EQ(res[0]->ind, len);       // new edge gets the next free index
    ASSERT_EQ(res[0], &m.g[2]);        // it follows the source edge
    ASSERT_EQ(m.g[1].ind, ind);
    ASSERT_EQ(m.g[3].ind, 2);
    ASSERT_EQ(m.ngs[End::A].num(), 0);
    ASSERT_EQ(m.ngs[End::B].num(), 0);
    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len);
    check_edge_indexes(gr);
    gr.compt(CmpId{}).template check<profuse>();

    // Merge the new edge into the source edge.
    const auto mr = merge(EgIdG{len}, ind);

    ASSERT_TRUE(mr[0]);
    ASSERT_EQ(mr[0]->ind, ind);
    ASSERT_EQ(Snapshot{gr}, before);
    check_edge_indexes(gr);
    gr.compt(CmpId{}).template check<profuse>();
}


/// Tests split of the boundary edges of a linear chain.
TEST_F(EdgeSplitMergerTest, LineBoundaryEdges)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of the boundary edges of a linear chain"
        );

    constexpr std::size_t len {3};
    constexpr ChIdG w {0};

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(len);
    const Snapshot before {gr};

    EdgeSplit split {gr};
    EdgeMerger merge {gr};

    // Split the head edge: the new edge is appended at chain end B.
    const auto rB = split(EgIdG{len - 1});

    ASSERT_TRUE(rB[0]);
    ASSERT_EQ(gr.chain(w).length(), len + 1);
    ASSERT_EQ(rB[0], &gr.chain(w).g[len]);
    ASSERT_EQ(rB[0]->ind, len);
    ASSERT_EQ(gr.chain(w).g[len - 1].ind, len - 1);

    // Split the tail edge: the new edge is inserted after it.
    const auto rA = split(EgIdG{0});

    ASSERT_TRUE(rA[0]);
    ASSERT_EQ(gr.chain(w).length(), len + 2);
    ASSERT_EQ(rA[0], &gr.chain(w).g[1]);
    ASSERT_EQ(rA[0]->ind, len + 1);
    ASSERT_EQ(gr.chain(w).g[0].ind, 0);

    ASSERT_EQ(gr.num_edges(), len + 2);
    ASSERT_EQ(gr.num_vertices(Deg1), 2);
    ASSERT_EQ(gr.num_vertices(Deg2), len + 1);
    check_edge_indexes(gr);
    gr.compt(CmpId{}).template check<profuse>();

    // Revert both splits.
    merge(EgIdG{len + 1}, EgIdG{0});
    merge(EgIdG{len}, EgIdG{len - 1});

    ASSERT_EQ(Snapshot{gr}, before);
    check_edge_indexes(gr);
    gr.compt(CmpId{}).template check<profuse>();
}


/// Tests merger deleting an edge that is not the last one graph-wide.
TEST_F(EdgeSplitMergerTest, MergerReindexesLastEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests merger deleting an edge that is not the last one graph-wide"
        );

    constexpr std::size_t len {4};
    constexpr ChIdG w {0};
    constexpr EgIdG ind {1};

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(len);
    const Snapshot before {gr};

    EdgeSplit split {gr};
    EdgeMerger merge {gr};

    split(ind);  // creates edge 'len' at position 2

    // Delete the source edge: the new edge has the last graph-wide index,
    // and should be reindexed to take the index of the deleted one.
    const auto mr = merge(ind, EgIdG{len});

    ASSERT_TRUE(mr[0]);
    ASSERT_EQ(mr[0]->ind, ind);
    ASSERT_EQ(mr[0], &gr.edge(ind));
    ASSERT_EQ(mr[0], &gr.chain(w).g[1]);
    ASSERT_EQ(Snapshot{gr}, before);
    check_edge_indexes(gr);
    gr.compt(CmpId{}).template check<profuse>();
}


/// Tests split of an edge adjacent to a junction, and merger reversing it.
TEST_F(EdgeSplitMergerTest, JunctionAdjacentEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests split of an edge adjacent to a junction, ",
            "and merger reversing it"
        );

    constexpr std::size_t len {4};

    constexpr ChIdG w0 {0};
    constexpr ChIdG w1 {1};
    constexpr ChIdG w2 {2};

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(len);

    // Branch a new chain at position 1: w0 (1 edge), w1 (new), w2 (len-1).
    trs::edge_creation::InNewChain<2, G> branch {gr};
    branch(BSlot{w0, EgIdA{1}});

    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.chain(w0).length(), 1);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);

    const Snapshot before {gr};
    const auto& c = gr.compt(CmpId{});

    EdgeSplit split {gr};
    EdgeMerger merge {gr};

    // Split the only edge of w0, whose end B is at the junction.
    const auto src = gr.chain(w0).g[0].ind;
    const auto res = split(src);

    ASSERT_TRUE(res[0]);
    ASSERT_EQ(gr.num_edges(), len + 2);
    ASSERT_EQ(gr.num_chains(), 3);
    ASSERT_EQ(gr.chain(w0).length(), 2);
    ASSERT_EQ(gr.chain(w1).length(), 1);
    ASSERT_EQ(gr.chain(w2).length(), len - 1);
    ASSERT_EQ(res[0], &gr.chain(w0).g[1]);

    // The junction is preserved.
    ASSERT_EQ(gr.chain(w0).ngs[End::A].num(), 0);
    ASSERT_EQ(gr.chain(w0).ngs[End::B].num(), 2);
    ASSERT_EQ(gr.chain(w1).ngs[End::A].num(), 2);
    ASSERT_EQ(gr.chain(w2).ngs[End::A].num(), 2);
    ASSERT_EQ(gr.num_vertices(Deg3), 1);
    ASSERT_EQ(c.chis.num({1, 3}), 3);
    check_edge_indexes(gr);
    c.template check<profuse>();

    // Merge the new edge back into the source edge.
    const auto mr = merge(res[0]->ind, src);

    ASSERT_TRUE(mr[0]);
    ASSERT_EQ(mr[0]->ind, src);
    ASSERT_EQ(Snapshot{gr}, before);
    ASSERT_EQ(c.chis.num({1, 3}), 3);
    check_edge_indexes(gr);
    c.template check<profuse>();
}


}  // namespace graffine::tests::edge_split_merger
