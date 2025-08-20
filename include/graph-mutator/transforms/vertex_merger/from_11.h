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
 * @file from_11.h
 * @brief Contains class performing merger between vertices of degree 1.
 * @note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_11_H
#define GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_11_H

#include <array>
#include <vector>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "common.h"
#include "core.h"
#include "log.h"

namespace graph_mutator::vertex_merger {


/**
 * @brief Merges two vertices of degree 1 to produce a vertex of degree 2.
 * @details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * @tparam G Graph to which the merger operation is applied.
 */
template<typename G>
struct From<Deg1, Deg1, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg1;  ///< Degree of the 2nd input vertex.
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
    using ResT = CmpId;
    using Res = std::array<ResT, 2>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit From(Graph& gr);

    /**
     * Merges two vertices of degree 1 to produce a vertex of degree 2.
     * @attention Important: when merge.antiparallel() is called, one of the
     * chains is reversed:
     * then, if \p e1 is A, reverses g of w1, if \p e1 is B, reverses g of w2:
     * e1 == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>)
     * e1 == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
     * @param w1 Chain index of the 1st parent vertex.
     * @param e1 Chain end of the 1st parent vertex.
     * @param w2 Chain index of the 2nd parent vertex.
     * @param e2 Chain end of the 2nd parent vertex.
     */
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) noexcept -> Res;

private:

    Graph& gr;  ///< Reference to the graph object.

    Core<Graph> merge;  ///< Low-level free-end connector.

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
From<Deg1, Deg1, G>::
From(Graph& gr)
    : gr {gr}
    , merge {gr, shortName}
    , log {dd, gr}
{}

// Important: when merge.antiparallel is called:
// if \p e1 is A, reverses g of w1, if \p e1 is B, reverses g of w2
template<typename G>
auto From<Deg1, Deg1, G>::
operator()(
    const EndSlot& s1,
    const EndSlot& s2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, e2] = s2.we();

    // Use low-level vertex merging functions directly
    // gr.update() is called in each of these:

    if (w2 == w1)      return merge.to_cycle(w1);
    if (e1 == e2)      return merge.antiparallel(e1, w1, w2);
    if (e1 == Ends::A) return merge.parallel(w1, w2);
    /* else */         return merge.parallel(w2, w1);
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_11_H
