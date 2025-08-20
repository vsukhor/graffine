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
 * @file to_12.h
 * @brief Contains class template enabling splits of specific graph vertices.
 * @details This Functor template specialization operates on vertices of
 * degree 3 to produce a degree 1 vertex and a degree 2 vertex.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_1B_H
#define GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_1B_H

#include <array>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "../vertex_merger/core.h"
#include "common.h"
#include "log.h"


namespace graph_mutator::vertex_split {


/**
 * @brief Generator of degree 1 and 2 vertices upon split of a vertex deg 2.
 * @details Adds vertex type-specific split capability and updates the
 * graph for it. This Functor template specialization operates on degree 3
 * vertices to produce vertices of degree 1 and 2: V3 -> V1 + V2.
 * @tparam G Graph to which the split operation is applied.
 */
template<Degree J2_,
         typename G> requires BulkDegree<J2_>
struct To<Deg1, J2_, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);
    static_assert(are_compatible_degrees<Deg1, J2_>);

    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = J2_;   ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = Deg3;  ///< Degree of the input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

    static constexpr auto dd = string_ops::str2<J1, J2>;
    static constexpr auto shortName = string_ops::concat<shortNameStem, dd, 2>;
    static constexpr auto fullName  = string_ops::concat<fullNameStem, dd, 2>;

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using ResT = CmpId;
    using Res = std::array<ResT, 2>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit To(Graph& gr);

    /**
     * @brief Divides the graph at a vertex of degree 3.
     * @param w Global chain index.
     * @param end The chain end.
     */
    auto operator()(const EndSlot& s) -> Res;

private:

    ///< Auxiliary low-level routine producing an intermediary state.
    vertex_merger::Core<Graph> merge;

    Graph& gr;  ///< Reference to the graph object.

    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    Log<Graph> log;

};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree J2_,
         typename G> requires BulkDegree<J2_>
To<Deg1, J2_, G>::
To(Graph& gr)
    : merge {gr, "vm_core called from"s + shortName}
    , gr {gr}
    , cn {gr.cn}
    , log {dd, gr}
{}


template<Degree J2_,
         typename G> requires BulkDegree<J2_>
auto To<Deg1, J2_, G>::
operator()(const EndSlot& s) -> Res
{
    if constexpr (verboseF)
        log.before(s);

    const auto clini = cn[s.w].c;

    const auto& ngs = gr.ngs_at(s);

    ASSERT(ngs.num() == I - 1,
           shortName, ": input vertex degree ", ngs.num() + 1, " != I ", I);

    const auto ng0 = ngs.front();  // copy

    const auto ind1 = gr.slot2ind(s);
    const auto ind2 = gr.slot2ind(ng0);

    if constexpr (J2 == 2) {

        const auto ng1 = ngs.back();   // copy

        const auto [n0, e0] = ng0.we();
        const auto [n1, e1] = ng1.we();

    // If not a cycle, this forms a new component
    // from s.w's neigs and beyond (excluding s.w itself).
        const auto isCycle = gr.cut_component_at(s);

        gr.remove_slot_from_neigs(s);
        gr.remove_slot_from_neigs(ng0);

        gr.ct[clini].set_chis();

        auto& cmp = gr.ct[cn[ng0.w].c];
        const auto is_cycle1 = cmp.template dfs<true>(ng0, ng1.opp());

        if (!is_cycle1)
            gr.split_component(cmp, cmp.find_chains(ng1));

        if (!isCycle)
            gr.ct.back().set_chis();

        if (e0 == e1)
            // if e0 is A, g of n0 becomes reversed, else g of n1:
            merge.antiparallel(e0, n0, n1);
        else
            e0 == Ends::A && e1 == Ends::B
            ? merge.parallel(n0, n1)
            : merge.parallel(n1, n0);
    }
    else {
        gr.remove_slot_from_neigs(s);

        auto& newcmp = gr.ct.emplace_back(gr.cmpt_num(), cn);
        gr.ct[clini].move_to(newcmp, cn[ng0.w]);
    }

//    gr.update_books();
//    if constexpr (Graph::useAgl) {
//        gr.update_adjacency_edges(ind1);
//        gr.update_adjacency_edges(ind2);
//    }
    gr.update();

    const auto w1 = gr.glm[ind1];
    const auto w2 = gr.glm[ind2];

    ASSERT(cn[w1].c == clini ||
           cn[w2].c == clini,
           shortName, ": cn[w1].c ", cn[w1].c, " != clini && cn[w2].c ",
           cn[w2].c, " != clini ", clini);

    // Print out summary after:

    if constexpr (verboseF)
        log.after(w1, {ind2});

    return {cn[w1].c,
            cn[w2].c};
}


}  // namespace graph_mutator::vertex_split

#endif  // GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_1B_H
