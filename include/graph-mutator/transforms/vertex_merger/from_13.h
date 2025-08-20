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
 * @file from_13.h
 * @brief Contains implementation of the for 1V + 3V -> 4V merger type.
 * @details Class encapsulating merger of tip vertex to a 3-way junction.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_13_H
#define GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_13_H

#include <array>
#include <vector>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "common.h"
#include "log.h"


namespace graph_mutator::vertex_merger {


/**
 * @brief Template specialization for V1 + V3 -> V4 vertex merger.
 * @details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * @tparam G Graph to which the merger operation is applied.
 */
template<typename G>
struct From<Deg1, Deg3, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg3;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = I1 + I2;                 ///< Degree of the 1st output vertex.
    static constexpr auto J2 = undefined<Degree>;       ///< No 2nd output vertex.

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
     * Merges a vertex of degree 1 to a vertex of degree 3.
     * @param s1 Slot on degree 1 parent vertex.
     * @param s2 Slot on degree 3 parent vertex.
     */
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) noexcept -> Res;

private:

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
From<Deg1, Deg3, G>::
From(Graph& gr)
    : gr {gr}
    , cn {gr.cn}
//    , merge {gr, shortName}
    , log {dd, gr}
{}


template<typename G>
auto From<Deg1, Deg3, G>::
operator()(
    const EndSlot& s1,
    const EndSlot& s2
) noexcept -> Res
{

    const auto w1 = s1.w;
    const auto w2 = s2.w;

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    const auto ngs = gr.ngs_at(s2);  // segments connected to w2 at e2

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(s1, s2);

    // Update connection to s1:

    gr.ngs_at(s1) = Neigs{s2, ngs[0], ngs[1]};

    // Update connection to s2 and its connected segments:

    gr.ngs_at(s2).insert(s1);
    gr.ngs_at(ngs[0]).insert(s1);
    gr.ngs_at(ngs[1]).insert(s1);

    // Update internal records:

    c1 == c2
        ? gr.ct[c1].set_chis()
        : gr.merge_components(c1, c2);

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(w1, {w2, ngs[0].w, ngs[1].w});

    return {c1, c2};
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_13_H
