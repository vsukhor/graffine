/* =============================================================================

This file is part of graffine, a lightweight graph transformation library.

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
 * \file core.h
 * \brief Contains low-level vertex merging routines for joining linear chains.
 * \note Implementation for the case of max vertex degree = 3 is available at
 * https://github.com/vsukhor/mitoSim/blob/master/include_transformer.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::vertex_merger {

/**
 * Encapsulates core vertex merger operations.
 * \details Elementary dynamics and updates the graph for it.
 * Forms base for clases adding more specific types of dynamics.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
struct Core
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    const std::string dd {degrees_to_string(Deg1, Deg1)};

    /**
     * Constructs a Core object from the Graph class instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     * \param[in] shortName Human-readable name, shoer form.
     */
    explicit constexpr Core(Graph& gr,
                            std::shared_ptr<PP> pp = nullptr,
                            std::string_view shortName = "");

    /**
     * Performs merger of free ends of two antiparallel oriented chains.
     * \details Antiparallel orientation is defined as:
     * merging end 1 of the 1st chain with to end 1 of the 2nd chain, or
     * merging end 2 of the 1st chain with to end 2 of the 2nd chain.
     * \attention Important:
     * if \p end is A, reverses g of \p w1:
     * end == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>);
     * if \p end is B, reverses g of \p w2:
     * end == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
     * \param[in] end Index of chain boundary at the vertex merger position.
     * \param[in] w1 Index of the 1st chain.
     * \param[in] w2 Index of the 2nd chain.
     */
    auto antiparallel(
        Chain::EndId end,
        ChIdG w1,
        ChIdG w2
    ) noexcept -> Res;

    /**
     * Performs merger of free ends of two parallel oriented chains.
     * \details Parallel orientation is defined as:
     * merging end 1 of the 1st chain to to end 2 of the 2nd chain, or
     * merging end 2 of the 1st chain to to end 1 of the 2nd chain.
     * \note: edge indexes are combined as 234 + 01-> 23401 (>>>> + >> -> >>>>>>)
     * \param[in] w1 Index of the 1st chain.
     * \param[in] w2 Index of the 2nd chain.
     */
    auto parallel(ChIdG w1, ChIdG w2) noexcept -> Res;

    /**
     * Merges end vertices to form a linear chain.
     * \param[in] s1 Chain boundary slot at the 1st parent vertex.
     * \param[in] s2 Chain boundary slot on the 2nd parent vertex.
     */
    auto to_linear(
        const EndSlot& s1,
        const EndSlot& s2
    ) noexcept -> Res;

    /**
     * Merges end vertices an unconnected chain to form a cycle.
     * \param[in] w Chain index.
     */
    auto to_cycle(ChIdG w) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    const std::string_view shortName;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
constexpr
Core<G, PP>::
Core(
    Graph& gr,
    std::shared_ptr<PP> pp,
    std::string_view shortName
)
    : gr {gr}
    , cn {gr.chains()}
    , updateProperties {pp}
    , shortName {shortName}
    , report {gr, dd}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


// Important:
// if \p end is A, reverses g of w1, if \p end is B, reverses g of w2, i.e.:
// end == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>)
// end == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
template<typename G,
         typename PP>
auto Core<G, PP>::
antiparallel(
    const Chain::EndId end,
    const ChIdG w1,
    const ChIdG w2
) noexcept -> Res
{
    auto& m1 = cn[w1];
    auto& m2 = cn[w2];

    // Indexes of the participating components:

    const auto c1 = m1.c;
    const auto c2 = m2.c;

    // Slots to be connected:

    EndSlot s1 {w1, end};
    EndSlot s2 {w2, end};

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(s1, s2, "a");

    ASSERT(w1 != w2, shortName, " antiparallel: same chain ", w1, " w1 == w2 ",
           "merge_to_cycle should be used instead");
    ASSERT(!m1.is_connected_at(end), shortName,
           " antiparallel: end ", end, " of w1 ", w1, " is not free");
    ASSERT(!m2.is_connected_at(end),
           " antiparallel: end ", end, " of w2 ", w2, " is not free");
    ASSERT(m1.length(), shortName,
           " antiparallel: chain w1 ", w1, " has no edges");
    ASSERT(m2.length(), shortName,
           " antiparallel: chain w2 ", w2, " has no edges");

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2, s1);  // (preserved, removed)

    auto& eg1 = m1.end_edge(end);
    auto ind1 = eg1.ind;

    gr.compt(eg1.c).remove_vertex_at_edge_outer(eg1, end);
    auto v2 = m2.vertex_at_end(end);

    // Update the merging component, chain and end indexes:

    if (end == End::A)
        // copy w1's B-end neigs to its A-end
        gr.copy_neigs(EndSlot{w1, End::B},
                      EndSlot{w1, End::A});
    // copy w2's end-end neigs to w1's B-end
    gr.copy_neigs(EndSlot{w2, End::opp(end)},
                  EndSlot{w1, End::B});

    if (end == End::A)
        // Edge::a takes values [1:length()];
        // for w1 reverse positions if A-ends are joined;
        m1.reverse_g();
    else
        // Edge::a takes values [1:length()];
        // for w2 reverse positions if B-ends are joined;
        m2.reverse_g();

    // transfer edges from m2 to m1
    std::move(m2.g.begin(),
              m2.g.end(), std::back_inserter(m1.g));
    m2.g.clear();

    m1.set_g_w();  // because of the edges just inserted
    gr.compt(m1.c).set_edges();
    gr.compt(m1.c).set_egl();  // correct egl.a of reversed edges

    gr.compt(c2).remove(m2);  // removes from component internal recs only

    if (w2 != gr.ind_last_chain())
        gr.rename_chain(gr.ind_last_chain(), w2);
    cn.pop_back();

    // If c1 != c2, acceptor doesn't necessary remain c1, hence the new cmp
    c1 == c2
        ? gr.compt(c1).set_chis()
        : gr.merge_components(c1, c2);

    // Update internal records.
    // Vertex data is not yet ready, so withVertices is false.

    gr.template update<false>();

    // Connect to opposite of 'end' if 'end' == A because in this case reversal
    // of 'm1' did not swap vertices in edge connectivity. If if end == B, it
    // was 'm2' which was reversed, so just use 'end' when connecting 'm1'.
    const auto activeE = end == End::A ? End::opp(end) : end;
    gr.edge(ind1).connect_to_outer(activeE, *v2);

    gr.update();  // final update

    const auto w = w1 == gr.num_chains() ? w2 : w1;

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(v2, w);

    return {v2};
}


// Example: 2345 + 01-> 234501 (>>>> + >> -> >>>>>>)
template<typename G,
         typename PP>
auto Core<G, PP>::
parallel(
    const ChIdG w1,
    const ChIdG w2
) noexcept -> Res
{
    // connects A_w2_B-A_w1_B  -->  A_w1_B

    constexpr typename End::Id e1 {End::A};
    constexpr typename End::Id e2 {End::B};

    auto& m1 = cn[w1];
    auto& m2 = cn[w2];

    const auto c1 = m1.c;
    const auto c2 = m2.c;

    EndSlot s1 {w1, e1};
    EndSlot s2 {w2, e2};

    auto& eg1 = m1.end_edge(e1);
    auto& eg2 = m2.end_edge(e2);

    const auto ind1 = eg1.ind;

    auto v2 = eg2.vertex_at_outer(e2);

    ASSERT(v2, "end B of ", eg2.indw, " is not connected.");

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(s2, s1, "p");

    ASSERT(w1 != w2,
           shortName, " parallel: w1 == w2 == ", w1, ": ",
            "merge_to_cycle should be used instead");
    ASSERT(!cn[w1].is_connected_at(End::A),
            shortName, " parallel: end A of w1 ", w1, " is not free");
    ASSERT(!cn[w2].is_connected_at(End::B),
            shortName, " parallel: end B of w2 ", w2, " is not free");
    ASSERT(cn[w1].length(),
            shortName, " parallel: chain w1 ", w1, " has no edges");
    ASSERT(cn[w2].length(),
            shortName, " parallel: chain w2 ", w2, " has no edges");

    if constexpr(!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2, s1);  // (preserved, removed)

    gr.compt(c1).remove_vertex_at_edge_outer(eg1, e1);

    // Indexes of the participating components:
    // Update the merging component, chain and end indexes:

    gr.copy_neigs(EndSlot{w2, End::A},
                  EndSlot{w1, End::A});

    gr.compt(c2).remove(m2);

    std::move(m1.g.begin(),
              m1.g.end(), std::back_inserter(m2.g));
    m1.g = std::move(m2.g);
    m1.set_g_w();
    gr.compt(c1).set_edges();
    gr.compt(c1).set_egl();

    if (w2 != gr.ind_last_chain())
        gr.rename_chain(gr.ind_last_chain(), w2);

    cn.pop_back();

    [[maybe_unused]] auto& cmp = c1 == c2
        ? gr.compt(c1).set_chis()
        : gr.merge_components(c1, c2);

    // Update internal records.
    // Vertex data is not yet ready, so withVertices is false:

    gr.template update<false>();

    // Connect edge to vertex.
    // This does not disrupt graph internal records.

    gr.edge(ind1).connect_to_outer(e1, *v2);

    gr.update();  // final update

    if constexpr(!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation.

    const auto w = w1 == gr.num_chains() ? w2 : w1;

    ASSERT(gr.chain(w).c == cmp.ind, "cmp is not correct");

    if constexpr (verboseF)
        report.after(v2, w);

    return {v2};
}

template<typename G,
         typename PP>
auto Core<G, PP>::
to_linear(
    const EndSlot& s1,
    const EndSlot& s2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, e2] = s2.we();

    if (e1 == e2)     return antiparallel(e1, w1, w2);
    if (e1 == End::A) return parallel(w1, w2);
    /* else */        return parallel(w2, w1);
}

template<typename G,
         typename PP>
auto Core<G, PP>::
to_cycle(const ChIdG w) noexcept -> Res
{
    ASSERT(!cn[w].is_unconnected_cycle(),
           shortName, ": attempt to merge_to_cycle a separate cycle chain ", w);
    ASSERT(!cn[w].is_connected_at(End::A) &&
           !cn[w].is_connected_at(End::B),
           shortName, ": attempt to merge_to_cycle a not separate chain ", w);
    ASSERT(cn[w].length() >= Chain::minCycleLength,
           shortName, " cycle: chain w ", w, "length < minCycleLength");

    const EndSlot sA {w, End::A};
    const EndSlot sB {w, End::B};

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(sA, sB, "c");

    if constexpr(!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(sA, sB);  // (preserved, removed)

    // Update the neighbor information of the merging chain:

    gr.connect(sA, sB);

    const auto c = cn[w].c;
    auto& cmp = gr.compt(c);

    cmp.chis.clear(Deg1);
    cmp.chis.insert(Deg2, w);

    // Update vertices:

    auto& vv = cmp.vertices();

    auto& egB = cn[w].g.back();
    vv.remove_connected_to_outer(egB, End::B);

    auto vA = cn[w].vertex_at_end(End::A);
    egB.connect_to_outer(End::B, *vA);

    // Update internal records:

    gr.update();

    if constexpr(!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*vA);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(vA, w);

    return {vA};
}

}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_H
