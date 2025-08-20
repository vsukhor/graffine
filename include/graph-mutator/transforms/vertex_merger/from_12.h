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
 * @file from_12.h
 * @brief Contains class encapsulating tip-to-side vertex merger.
 * @note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_12_H
#define GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_12_H

#include <array>
#include <vector>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "../vertex_split/to_11.h"
#include "common.h"
#include "log.h"


namespace graph_mutator::vertex_merger {


/**
 * @brief Template specialization for V1 + V2 -> V3 vertex merger.
 * @details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * @tparam G Graph to which the merger operation is applied.
 */
template<typename G>
struct From<Deg1, Deg2, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg2;  ///< Degree of the 2nd input vertex.
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
     * Merges a vertex of degree 1 to a vertex of degree 2.
     * @param s1 Slot on the parent degree 1 vertex.
     * @param s2 Slot on the parent degree 2 vertex.
     */
    auto operator()(const EndSlot& s1,
                    const BulkSlot& s2) noexcept -> Res;

private:

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    ///< Auxiliary functor producing a splitted intermediary.
    vertex_split::To<1, 1, Graph> split_to11;

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
From<Deg1, Deg2, G>::
From(Graph& gr)
    : gr {gr}
    , cn {gr.cn}
    , split_to11 {gr}
    , log {dd, gr}
{}


template<typename G>
auto From<Deg1, Deg2, G>::
operator()(
    const EndSlot& s1,
    const BulkSlot& s2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, a2] = s2.we();

    // For corresponding vertex merger of disconnected cycles, Functor<1, 0, G>
    // should be used. Therefore,
    ASSERT(!cn[w1].ngs[e1].num(),
           shortName, ": end ", s1.ea_str(), " is not a free end of ", w1);
    ASSERT(a2, shortName, " at the very beginning of w2 ", w2);
    ASSERT(a2 < cn[w2].length(), shortName, " at the very end of w2 ", w2);

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(s1, s2);

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    // cn[mi] is produced by the split:

    auto mi = cn[w2].is_disconnected_cycle() ? w2 : gr.chain_num();

    // Split w2 at a2 to produce independent chain ends A, B of cn[mi).

    split_to11(s2);

    // Connect all the ends involved:

    EndSlot w1A {w1, Ends::A};
    EndSlot w1B {w1, Ends::B};
    EndSlot w2A {w2, Ends::A};
    EndSlot w2B {w2, Ends::B};
    EndSlot miA {mi, Ends::A};
    EndSlot miB {mi, Ends::B};

    if (w1 == w2) {
        // This is not a cycle chain because the cycle requires neighbors
        // at both ends, while w1 is allowed to have a neig at only one end

        if (e1 == Ends::A) {
            if constexpr (verboseF)
                log.branch(1, "w1 == w2 and e1 == A");

            cn[w1].ngs = {Neigs{w1B, miA},
                          Neigs{w1A, miA}};

            gr.ngs_at(miA) = Neigs{w1A, w1B};
        }
        else {
            if constexpr (verboseF)
                log.branch(2, "w1 == w2 and e1 == B");

            gr.ngs_at(w1B) = Neigs{miA, miB};

            cn[mi].ngs = {Neigs{w1B, miB},
                          Neigs{w1B, miA}};
        }
    }
    else {
        if constexpr (verboseF)
            log.branch(3, "w1 != w2");

        gr.ngs_at(s1)  = Neigs{w2B, miA};
        gr.ngs_at(w2B) = Neigs{s1, miA};
        gr.ngs_at(miA) = Neigs{s1, w2B};
    }

    // Update internal records.

    cn[w2].c == cn[mi].c
        ? gr.ct[cn[mi].c].set_chis()
        : gr.merge_components(cn[w2].c, cn[mi].c);

    cn[w2].c == cn[w1].c
        ? gr.ct[cn[w1].c].set_chis()
        : gr.merge_components(cn[w1].c, cn[w2].c);

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(w1, {w2}, mi);

    return {c1, c2};
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_12_H
