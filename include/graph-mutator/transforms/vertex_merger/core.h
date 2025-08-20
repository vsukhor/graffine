/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file core.h
 * @brief Contains low-level vertex merging routines for joining linear chains.
 * @note Implementation for the case of max vertex degree = 3 is available at
 * https://github.com/vsukhor/mitoSim/blob/master/include/core_transformer.h
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_CORE_H
#define GRAPH_MUTATOR_VERTEX_MERGER_CORE_H

#include <algorithm>
#include <array>
#include <string>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "log.h"


namespace graph_mutator::vertex_merger {

/**
 * @brief Encapsulates core vertex merger operations.
 * @details Elementary dynamics and updates the graph for it.
 * Forms base for clases adding more specific types of dynamics.
 * @tparam G Graph to which the transformation is applied.
 */
template<typename G>
struct Core {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;

    static constexpr auto dd = string_ops::str2<1, 1>;

    /**
     * @brief Constructs a Core object from the Graph class instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit constexpr Core(Graph& gr,
                            std::string&& shortName);

    /**
     * @brief Performs merger of free ends of two antiparallel oriented chains.
     * @details Antiparallel orientation is defined as:
     * merging end 1 of the 1st chain with to end 1 of the 2nd chain, or
     * merging end 2 of the 1st chain with to end 2 of the 2nd chain.
     * @attention Important:
     * if \p end is A, reverses g of \p w1:
     * end == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>);
     * if \p end is B, reverses g of \p w2:
     * end == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
     * @param end End index at the vertex merger position.
     * @param w1 Index of the 1st chain.
     * @param w2 Index of the 2nd chain.
     */
    auto antiparallel(
        Chain::EndId end,
        ChId w1,
        ChId w2
    ) noexcept -> std::array<CmpId, 2>;

    /**
     * @brief Performs merger of free ends of two parallel oriented chains.
     * @details Parallel orientation is defined as:
     * merging end 1 of the 1st chain to to end 2 of the 2nd chain, or
     * merging end 2 of the 1st chain to to end 1 of the 2nd chain.
     * @note: edge indexes are combined as 234 + 01-> 23401 (>>>> + >> -> >>>>>>)
     * @param w1 Index of the 1st chain.
     * @param w2 Index of the 2nd chain.
     */
    auto parallel(ChId w1, ChId w2) noexcept -> std::array<CmpId, 2>;

    /**
     * @brief Merges end vertices a disconnected chain to form a cycle.
     * @param w Chain index.
     */
    auto to_cycle(ChId w) noexcept -> std::array<CmpId, 2>;

    /**
     * @brief Updates the structure of the output disconnected components.
     * @param w1 Index of the 1st participant chain.
     * @param w2 Index of the 2nd participant chain.
     */
//    constexpr void update_chain_cmpt(CmpId c1, CmpId c2) noexcept;

    /**
     * @brief Updates the structure of the output disconnected components.
     * @param c1 Index of the 1st participant component.
     * @param c2 Index of the 2nd participant component.
     */
//    constexpr void update_cmpt(CmpId c1, CmpId c2) noexcept;

protected:

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn;       ///< Reference to the graph edge chains.

    const std::string shortName;

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
Core<G>::
Core(
    Graph& gr,
    std::string&& shortName
)
    : gr {gr}
    , cn {gr.cn}
    , shortName {shortName}
    , log {dd, gr}
{}


// Important:
// if \p end is A, reverses g of w1, if \p end is B, reverses g of w2, i.e.:
// end == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>)
// end == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
template<typename G>
auto Core<G>::
antiparallel(
    const Chain::EndId end,
    const ChId w1,
    const ChId w2
) noexcept -> std::array<CmpId, 2>
{
    auto& m1 = cn[w1];
    auto& m2 = cn[w2];

    // Indexes of the participating components:

    const auto c1 = m1.c;
    const auto c2 = m2.c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(EndSlot{w1, end},
                   EndSlot{w2, end}, "a");

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

    // Update the merging component, chain and end indexes:

    if (end == Ends::A)
        // copy w1's B-end neigs to its A-end
        gr.copy_neigs(EndSlot{w1, Ends::B},
                      EndSlot{w1, Ends::A});
    // copy w2's end-end neigs to w1's B-end
    gr.copy_neigs(EndSlot{w2, Ends::opp(end)},
                  EndSlot{w1, Ends::B});

    gr.ct[c2].remove(m2);

    if (end == Ends::A)
        // Edge::a takes values [1:length()];
        // for w1 reverse positions if A-ends are joined;
        m1.reverse_g();
    else
        // Edge::a takes values [1:length()];
        // for w2 reverse positions if B-ends are joined;
        m2.reverse_g();

    std::move(m2.g.begin(),
              m2.g.end(), std::back_inserter(m1.g));
    m2.g.clear();
    m1.set_g_w();  // because of the edges just inserted

    if (w2 != gr.ind_last_chain())
        gr.rename_chain(gr.ind_last_chain(), w2);
    cn.pop_back();

    c1 == c2
        ? gr.ct[c1].set_chis()
        : gr.merge_components(c1, c2);

    // Update internal records:
    gr.ct[c1].set_edges();
    gr.ct[c1].set_gl();
    if (c1 != c2 && c2 < gr.cmpt_num()) {
        gr.ct[c2].set_edges();
        gr.ct[c2].set_gl();
    }

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(w1 == gr.chain_num() ? w2 : w1);

    return {c1, c2};
}


// Example: 2345 + 01-> 234501 (>>>> + >> -> >>>>>>)
template<typename G>
auto Core<G>::
parallel(
    const ChId w1,
    const ChId w2
) noexcept -> std::array<CmpId, 2>
{
    // connects A_w2_B-A_w1_B  -->  A_w1_B

    auto& m1 = cn[w1];
    auto& m2 = cn[w2];

    const auto c1 = m1.c;
    const auto c2 = m2.c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(EndSlot{w2, Ends::B},
                   EndSlot{w1, Ends::A}, "p");

    ASSERT(w1 != w2,
           shortName, " parallel: w1 == w2 == ", w1, ": ",
            "merge_to_cycle should be used instead");
    ASSERT(!cn[w1].is_connected_at(Ends::A),
            shortName, " parallel: end A of w1 ", w1, " is not free");
    ASSERT(!cn[w2].is_connected_at(Ends::B),
            shortName, " parallel: end B of w2 ", w2, " is not free");
    ASSERT(cn[w1].length(),
            shortName, " parallel: chain w1 ", w1, " has no edges");
    ASSERT(cn[w2].length(),
            shortName, " parallel: chain w2 ", w2, " has no edges");

    // Indexes of the participating components:
    // Update the merging component, chain and end indexes:

    gr.copy_neigs(EndSlot{w2, Ends::A},
                  EndSlot{w1, Ends::A});

    gr.ct[c2].remove(m2);

    std::move(m1.g.begin(),
              m1.g.end(), std::back_inserter(m2.g));
    m1.g = std::move(m2.g);
    m1.set_g_w();
    gr.ct[c1].set_edges();
    gr.ct[c1].set_gl();

    if (w2 != gr.ind_last_chain())
        gr.rename_chain(gr.ind_last_chain(), w2);

    cn.pop_back();

    c1 == c2
        ? gr.ct[c1].set_chis()
        : gr.merge_components(c1, c2);

    if (c2 < gr.cmpt_num() && c1 != c2) {
        gr.ct[c2].set_edges();
        gr.ct[c2].set_gl();
    }
    // Update internal records:

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(w1 == gr.chain_num() ? w2 : w1);

    return {c1, c2};
}


template<typename G>
auto Core<G>::
to_cycle(const ChId w) noexcept -> std::array<CmpId, 2>
{
//    auto& m = cn[w];

    ASSERT(!cn[w].is_disconnected_cycle(),
           shortName, ": attempt to merge_to_cycle a separate cycle chain ", w);
    ASSERT(!cn[w].is_connected_at(Ends::A) &&
           !cn[w].is_connected_at(Ends::B),
           shortName, ": attempt to merge_to_cycle a not separate chain ", w);
    ASSERT(cn[w].length() >= Chain::minCycleLength,
           shortName, " cycle: chain w ", w, "length < minCycleLength");

    const EndSlot sA {w, Ends::A};
    const EndSlot sB {w, Ends::B};

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(sA, sB, "c");

    // Update the neighbor informathin of the merging chain:

    gr.ngs_at(sA).insert(sB);
    gr.ngs_at(sB).insert(sA);

    gr.ct[cn[w].c].chis.cn11 = undefined<ChId>;
    gr.ct[cn[w].c].chis.cn22 = w;

    // Update internal records:

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(w);

    return {cn[w].c,
            cn[w].c};
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_CORE_H
