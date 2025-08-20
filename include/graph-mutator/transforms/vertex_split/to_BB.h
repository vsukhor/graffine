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
 * @file to_BB.h
 * @brief Contains class template enabling splits of specific graph vertices.
 * @details This Functor template specialization operates on vertices of
 * degree 4 to produce two vertices whose degrees sum up to 4, except 1 and 3,
 * implemented in a separate functor.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_BB_H
#define GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_BB_H

#include <array>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../vertex_merger/from_11.h"
#include "common.h"
#include "to_1B.h"
#include "to_13.h"
#include "log.h"


namespace graph_mutator::vertex_split {

/**
 * @brief Template specialization for producing a pair of degree 2 vertices.
 * @details Adds vertex type-specific split capability and updates
 * the graph for it. This Functor template specialization operates
 * on vertices of degree 4 to produce two vertices of degree 2: V4 -> 2V2.
 * The V2 vertices may be either internal chain vertices or boundary vertices of
 * one or two disconnected cycle chains.
 * @tparam G Graph to which the split operation is applied.
 */
template<Degree J1_,
         Degree J2_,
         typename G> requires (BulkDegree<J1_> and
                               BulkDegree<J2_>)
struct To<J1_, J2_, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);
    static_assert(are_compatible_degrees<J1_, J2_>);

    static constexpr auto J1 = J1_;        ///< Degree of the 1st output vertex.
    static constexpr auto J2 = J2_;        ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = Deg4;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = undefined<Degree>;       ///< No 2nd input vertex.
    static constexpr auto I = I1;                       ///< Input vertex degree.

    static constexpr auto dd = string_ops::str2<J1, J2>;
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
    explicit To(Graph& gr);

    /**
     * @brief Divides the component at a vertex of degree 4.
     * @param s Slots defining the vertex to split.
     */
    auto operator()(const std::array<EndSlot, 2>& s) -> Res;

    /**
     * @brief Divides the component at a vertex of degree 4.
     * @param s1 Slot defining the vertex to split and its 1st edge.
     * @param s2 Slot defining the vertex to split and its 2nd edge.
     */
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) -> Res;

private:

    ///< Auxiliary low-level routine producing an intermediate state.
    vertex_merger::From<Deg1, Deg1, Graph> merge;

    To<Deg1, Deg3, Graph> split_to_13;  ///< Auxiliary functor.
    To<Deg1, Deg2, Graph> split_to_12;  ///< Auxiliary functor.
    To<Deg1, Deg0, Graph> split_to_10;  ///< Auxiliary functor.

    Graph& gr;  ///< Reference to the graph object.

    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    Log<Graph> log;

    void check_validity(
        const EndSlot& s1,
        const EndSlot& s2
    ) const noexcept;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree J1_,
         Degree J2_,
         typename G> requires (BulkDegree<J1_> and
                               BulkDegree<J2_>)
To<J1_, J2_, G>::
To(Graph& gr)
    : merge {gr}
    , split_to_13 {gr}
    , split_to_12 {gr}
    , split_to_10 {gr}
    , gr {gr}
    , cn {gr.cn}
    , log {dd, gr}
{}


// cuts a 4-junction
template<Degree J1_,
         Degree J2_,
         typename G> requires (BulkDegree<J1_> and
                               BulkDegree<J2_>)
auto To<J1_, J2_, G>::
operator()(const std::array<EndSlot, 2>& s) -> Res
{
    return (*this)(s[0], s[1]);
}


// cuts a 4-junction
template<Degree J1_,
         Degree J2_,
         typename G> requires (BulkDegree<J1_> and
                               BulkDegree<J2_>)
auto To<J1_, J2_, G>::
operator()(const EndSlot& _s1,
           const EndSlot& _s2) -> Res
{
    const auto s1 = _s1;  // copy
    const auto s2 = _s2;  // copy

    if constexpr (verboseF)
        log.before({s1, s2});

//    check_validity(s1, s2);

    std::array<EgId, I> ind;               // indexes of edges connected to s
    std::array<typename Ends::Id, I> end;  // chain ends connected to s
    ind[0] = gr.slot2ind(s1);              // edge ind at s1
    ind[1] = gr.slot2ind(s2);              // edge ind at s2
    end[0] = gr.edge_end(s1);
    end[1] = gr.edge_end(s2);
    for (szt i=1, j=2; i<I; ++i) {
        const auto& u = gr.ngs_at(s1)[i-1];
        if (u != s2) {
            ind[j] = gr.slot2ind(u);
            end[j] = gr.edge_end(u);
            ++j;
        }
    }

    split_to_13(s1);

    const auto& ngs2 = gr.ngs_at(s2);
    ngs2[0].w == ngs2[1].w ? split_to_10(s2) :
                             split_to_12(s2);

    const auto ww0 = gr.glm[ind[0]];
    const auto ww1 = gr.glm[ind[1]];

    merge(EndSlot{ww0, cn[ww0].ind2end(ind[0], end[0])},
          EndSlot{ww1, cn[ww1].ind2end(ind[1], end[1])});

    if constexpr (verboseF)
        log.after({ind[0],ind[1]}, {ind[2], ind[3]});

    std::vector cc {gr.edge(ind[0]).c, gr.edge(ind[1]).c,
                    gr.edge(ind[2]).c, gr.edge(ind[3]).c};
    std::sort(cc.begin(),cc.end());
    cc.erase(std::unique(cc.begin(), cc.end()), cc.end());

    return {cc.front(), cc.back()};
}


template<Degree J1_,
         Degree J2_,
         typename G> requires (BulkDegree<J1_> and
                               BulkDegree<J2_>)
void To<J1_, J2_, G>::
check_validity(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept
{
    [[maybe_unused]] const auto& m1 = gr.cn[s1.w];
    [[maybe_unused]] const auto& m2 = gr.cn[s2.w];
    [[maybe_unused]] const auto& ngs1 = gr.ngs_at(s1);
    [[maybe_unused]] const auto& ngs2 = gr.ngs_at(s2);

    auto cycle_is_selected = [&]() -> bool
    {
        return gr.is_same_cycle(s1, s2);
    };

    auto cycle_is_excluded = [&]() -> bool
    {
        const auto& other = ngs1.other_than({s2});

        return gr.is_same_cycle(other.front(), other.back());
    };

    ASSERT(s1 != s2, shortName, ": slots s1 and s2 are identical:", s1);

    ASSERT(ngs1.num() == I - 1,
           shortName, ": ngs1.num ", ngs1.num(), " != I-1 ", I - 1);
    ASSERT(ngs2.num() == I - 1,
           shortName, ": ngs2.num ", ngs2.num(), " != I-1 ", I - 1);

    ASSERT(ngs1.has(s2),
           shortName, ": s2 ", s2, "not found among connections at s1 ", s1);
    ASSERT(ngs2.has(s1),
           shortName, ": s1 ", s1, "not found among connections at s2 ", s2 );

    if constexpr (J1_ == Deg2 && J2_ == Deg2)
        ASSERT(!gr.is_same_cycle(s1, s2),
               shortName,  ": disconnecting a cycle chain");

    if constexpr (J1_ == Deg0 && J2_ == Deg0)
        ASSERT(cycle_is_selected() && cycle_is_excluded(),
               shortName, ": both selected and excluded ",
               "pairs of slots should belong to a cycle chain");

    if constexpr ((J1_ == Deg0 && J2_ == Deg2) ||
                  (J1_ == Deg2 && J2_ == Deg0))
        ASSERT(( cycle_is_selected() && !cycle_is_excluded()) ||
               (!cycle_is_selected() &&  cycle_is_excluded()),
               shortName,  ": only either selected ",
               "or excluded pair of slots may belong to a cycle chain");
}


}  // namespace graph_mutator::vertex_split

#endif  // GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_BB_H
