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
 * @file from_10.h
 * @brief Contains class for a merger between a tip vertex and a cycle boundary vertex.
 * @note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_10_H
#define GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_10_H

#include <array>
#include <vector>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "common.h"
#include "log.h"


namespace graph_mutator::vertex_merger {


/**
 * @brief Merger of a degree 1 vertex to a boundary vertex belonging cycle chain.
 * @details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * @tparam G Graph to which the merger operation is applied.
 */
template<typename G>
struct From<Deg1, Deg0, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg0;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Deg3;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = undefined<Degree>;  ///< No 2nd output vertex.

    static constexpr auto dd = string_ops::str2<I1, I2>;
    static constexpr auto shortName = string_ops::concat<shortNameStem, dd, 2>;
    static constexpr auto fullName  = string_ops::concat<fullNameStem, dd, 2>;

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;
    using ResT = CmpId;
    using Res = std::array<ResT, 2>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit From(Graph& gr);

    /**
     * Merges a vertex of degree 1 to a boundary vertex in a disconnected cycles.
     * @param s1 Slot on the merged degree 1 vertex.
     * @param w2 Chain index of the cycle chain.
     */
    auto operator()(
        const EndSlot& s1,
        ChId w2
    ) noexcept -> Res;

private:

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
From<Deg1, Deg0, G>::
From(Graph& gr)
    : gr {gr}
    , cn {gr.cn}
    , log {dd, gr, {}, "a CYCLE "}
{}


template<typename G>
auto From<Deg1, Deg0, G>::
operator()(
    const EndSlot& s1,
    const ChId w2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();

    // Indexes of the participating components:

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(s1, EndSlot{w2, Ends::Undefined});

    ASSERT(!cn[w1].ngs[e1].num(), shortName,
           ": end ", s1.ea_str(), " of chain ", s1.w, " is not a free end");
    ASSERT(cn[w2].is_disconnected_cycle(),
           shortName, "w2 ", w2, " is not a disconnected cycle");

    // Connect slots:

    const EndSlot s2A {w2, Ends::A};
    const EndSlot s2B {w2, Ends::B};

    gr.ngs_at(s1)  = Neigs{s2A, s2B};
    gr.ngs_at(s2A) = Neigs{s2B, s1};
    gr.ngs_at(s2B) = Neigs{s2A, s1};

    // Update internal records:

    gr.merge_components(c1, c2);

    gr.update();

    // Print out summary after the operation.

   if constexpr (verboseF)
        log.after(w1, {w2});

    return {c1, c2};
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_10_H
