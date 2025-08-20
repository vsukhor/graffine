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
 * @file from_22.h
 * @brief Contains implementation of merger between two bulk vertices.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_22_H
#define GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_22_H

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
 * @brief Template specialization for 2V2 -> V4 vertex merger.
 * @details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * @tparam G Graph to which the merger operation is applied.
 */
template<typename G>
struct From<Deg2, Deg2, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr auto I1 = Deg2;  ///< Degree of the 1st input vertex.
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
     * Merges two vertices of degree 2.
     * @param s1 Slot on the 1st parent vertex.
     * @param s2 Slot on the 2nd parent vertex.
     */
    auto operator()(const BulkSlot& s1,
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
From<Deg2, Deg2, G>::
From(Graph& gr)
    : gr {gr}
    , cn {gr.cn}
    , split_to11 {gr}
    , log {dd, gr}
{}


template<typename G>
auto From<2, 2, G>::
operator()(
    const BulkSlot& b1,
    const BulkSlot& b2
) noexcept -> Res
{
    const auto [w1, a1] = b1.we();
    const auto [w2, a2] = b2.we();

    // Print out summary before the operation.

    if constexpr (verboseF)
        log.before(b1, b2);

    // Define component, chain and end indexes of the merging ends.

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    EndSlot s1 {}, s2 {}, s3 {}, s4 {};

    if (w1 == w2) {
        szt aL, aS;
        if (a1 > a2) {
            aL = a1;
            aS = a2;
        } else {
            aL = a2;
            aS = a1;
        }

        if (!cn[w1].is_disconnected_cycle()) {

            if constexpr (verboseF)
                log.branch(1, "w1 == w2 && not cn[w1].is_disconnected_cycle()");

            // produce cn[w1].g[0...(aL-1)] + cn[chain_num()].g[aL...(size()-1)]
            // where indexes correspond to edges before the vertex split
            split_to11(BulkSlot{w1, aL});

            // produce cn[w1].g[0...(aS-1)] + cn[chain_num()+1].g[aS...(aL-1)] +
            // cn[chain_num()].g[aL...(size()-1)] where indexes correspond to edges
            // before the 1st split
            split_to11(BulkSlot{w1, aS});

            s1 = EndSlot{w1, Ends::B};
            s2 = EndSlot{gr.ind_last_chain(), Ends::A};
            s3 = EndSlot{gr.ind_last_chain(), Ends::B};
            s4 = EndSlot{gr.chain_num()-2, Ends::A};
        }
        else {

            if constexpr (verboseF)
                log.branch(2, "w1 == w2 && cn[w1].is_disconnected_cycle()");

            // produce cn[w1].g[aL...(size()-1),0...aL-1] where indexes
            // correspond to edges before the vertex split
            split_to11(BulkSlot{w1, aL});

            // produce cn[w1].g[aL...(size()-1),0...(aS-1)] +
            // cn[chain_num()].g[aS...(aL-1)] where indexes correspond to edges
            // before the split
            split_to11(BulkSlot{w1, aS + cn[w1].length() - aL});

            s1 = EndSlot{w1, Ends::A};
            s2 = EndSlot{w1, Ends::B};
            s3 = EndSlot{gr.ind_last_chain(), Ends::A};
            s4 = EndSlot{gr.ind_last_chain(), Ends::B};
        }
    }
    else {  // w1 != w2
        if (!cn[w1].is_disconnected_cycle() &&
            !cn[w2].is_disconnected_cycle()) {

            if constexpr (verboseF)
                log.branch(3, "w1 != w2 && "s +
                              "not cn[w1].is_disconnected_cycle() && " +
                              "not cn[w2].is_disconnected_cycle()");

            // produces cn[w1].g[0...(a1-1)] + cn[chain_num()].g[a1...(size()-1)]
            // where indexes correspond to edges before the vertex split
            split_to11(BulkSlot{w1, a1});

            // produces cn[w2].g[0...(a2-1)] + cn[chain_num()+1].g[a2...(size()-1)]
            // where indexes correspond to edges before the split
            split_to11(BulkSlot{w2, a2});

            s1 = EndSlot{w1, Ends::B};
            s2 = EndSlot{w2, Ends::B};
            s3 = EndSlot{gr.chain_num()-2, Ends::A};
            s4 = EndSlot{gr.ind_last_chain(), Ends::A};
        }
        else if (cn[w1].is_disconnected_cycle() &&
                 cn[w2].is_disconnected_cycle()) {

            if constexpr (verboseF)
                log.branch(4, "w1 != w2 && "s +
                              "cn[w1].is_disconnected_cycle() && " +
                              "cn[w2].is_disconnected_cycle()");

            // produce cn[w1].g[a1...(size()-1),0...a1-1] where indexes
            // correspond to edges before the vertexsplit
            split_to11(BulkSlot{w1, a1});

            // produce cn[w2].g[a2...(size()-1),0...a2-1] where indexes
            // correspond to edges before the split
            split_to11(BulkSlot{w2, a2});

            s1 = EndSlot{w1, Ends::A};
            s2 = EndSlot{w1, Ends::B};
            s3 = EndSlot{w2, Ends::A};
            s4 = EndSlot{w2, Ends::B};
        }
        else {
            if (cn[w1].is_disconnected_cycle()) {

                if constexpr (verboseF)
                    log.branch(5, "w1 != w2 && "s +
                                "cn[w1].is_disconnected_cycle() && " +
                                "not cn[w2].is_disconnected_cycle()");

                // produce cn[w1].g[a1...(size()-1),0...a1-1] where indexes
                // correspond to edges before the vertex split
                split_to11(BulkSlot{w1, a1});

                // produce cn[w2].g[0...(a2-1)] + cn[chain_num()].g[a2...(size()-1)]
                // where indexes correspond to edges before the split
                split_to11(BulkSlot{w2, a2});

                s1 = EndSlot{w1, Ends::A};
                s2 = EndSlot{w1, Ends::B};
                s3 = EndSlot{w2, Ends::B};
                s4 = EndSlot{gr.ind_last_chain(), Ends::A};
            }
            else {        // cn[w2] is a disconnected cycle

                if constexpr (verboseF)
                    log.branch(6, "w1 != w2 && "s +
                                "not cn[w1].is_disconnected_cycle() && " +
                                "cn[w2].is_disconnected_cycle()");

                // produce cn[w1].g[0...(a1-1)] + cn[chain_num()].g[a1...(size()-1)]
                // where indexes correspond to edges before the vertex split
                split_to11(BulkSlot{w1, a1});

                // produce cn[w2].g[a2...(size()-1),0...a2-1] where indexes
                // correspond to edges before the split
                split_to11(BulkSlot{w2, a2});

                s1 = EndSlot{w1, Ends::B};
                s2 = EndSlot{gr.ind_last_chain(), Ends::A};
                s3 = EndSlot{w2, Ends::A};
                s4 = EndSlot{w2, Ends::B};
            }
        }
    }

    // Join the ends:

    gr.ngs_at(s1) = Neigs{s2, s3, s4};
    gr.ngs_at(s2) = Neigs{s1, s3, s4};
    gr.ngs_at(s3) = Neigs{s2, s1, s4};
    gr.ngs_at(s4) = Neigs{s2, s3, s1};

    // Update internal records.

    const auto [u1, u2, u3, u4] = std::array{s1.w, s2.w, s3.w, s4.w};

    cn[u1].c == cn[u2].c
        ? gr.ct[cn[u2].c].set_chis()
        : gr.merge_components(cn[u1].c, cn[u2].c);

    cn[u1].c == cn[u3].c
        ? gr.ct[cn[u3].c].set_chis()
        : gr.merge_components(cn[u1].c, cn[u3].c);

    cn[u1].c == cn[u4].c
        ? gr.ct[cn[u4].c].set_chis()
        : gr.merge_components(cn[u1].c, cn[u4].c);

    gr.update();

    // Print out summary after the operation.

    if constexpr (verboseF)
        log.after(u1, {u2, u3, u4});

    return {c1, c2};
}


}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_FUNCTOR_22_H
