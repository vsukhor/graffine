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
 * @file in_existing_chain.h
 * @brief Contains class performing creation of graph edges in existing chain.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_EDGE_CREATION_FUNCTOR_OLD_CHAIN_H
#define GRAPH_MUTATOR_EDGE_CREATION_FUNCTOR_OLD_CHAIN_H

#include <utility>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"

namespace graph_mutator::edge_creation {


/**
 * @brief Creates an edge adjoint to a vertex, putting it into an existing chain.
 * @details Creates a graph edge inserted into an existing chain. The edge will
 * have as one of its ends a vertex of degree \p D not affected by the addition
 * of the edge. Because the edge extends an existing chain, the other edge end
 * will have degree 2.
 * @tparam D Degree of one of the vertexes accepting the created edge.
 * @tparam G Graph to which the transformation is applied.
 */
template<Degree D,
         typename G>
struct InExistingChain {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);
    static_assert (is_implemented_degree<D>);

    static constexpr auto isNewChain = false;

    static constexpr auto I1 = undefined<Degree>;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = D;     ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Deg2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = D;     ///< The 2nd output vertex.

    static constexpr auto d = string_ops::str1<I2>;
    static constexpr auto shortName =
        string_ops::shName<d, 1, 'e', 'c', '_', 'o'>;
    static constexpr const char* fullName {"edge_creation_old_chain"};

    using Graph = G;
    using Chain = Graph::Chain;
    using Edge = Chain::Edge;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using Ends = Chain::Ends;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit InExistingChain(Graph& gr);

    /**
     * @brief Function call operator executing the transformation.
     * @param s Slot inside the chain where the new edge is placed.
     */
    template<typename S> requires structure::basic::slot<S>
    auto operator()(const S& s) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the graph object.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D,
         typename G>
InExistingChain<D, G>::
InExistingChain(Graph& gr)
    : gr {gr}
{}


template<Degree D,
         typename G>
template<typename S> requires structure::basic::slot<S>
auto InExistingChain<D, G>::
operator()(const S& s) noexcept -> Res
{
    const auto [w, ae] = s.we();

    auto& m = gr.cn[w];

    const auto a = std::is_same_v<S, EndSlot>
                 ? ae == Ends::A ? zero<EgId> : m.length()
                 : ae;

    if constexpr (verboseF) {
        const std::string at = std::is_same_v<S, EndSlot> ? " " : " at ";
        log_(colorcodes::GREEN, "Edge creation:", colorcodes::RESET,
                   " in existing chain: ",
                   colorcodes::BOLDYELLOW,  w, at, s.ea_str(),
                   colorcodes::RESET, '\n');
        m.print("before ", shortName, ": ", at, s.ea_str());
        log_("");
    }

    if constexpr (std::is_same_v<S, BulkSlot>)
        ASSERT(a && a <= m.length(), "vertex at ", a, " is not bulk");

    Edge eg {gr.edgenum++};
    const auto ep = m.insert_edge(std::move(eg), a);
    m.set_g_w();
    gr.ct[m.c].append(m.g[a]);

    ASSERT(ep == &m.g[a],
           "unsuccessful edge insert at slot ", s.w, " ", s.ea_str());

    gr.update();

    if constexpr (verboseF) {
        m.print(shortName, "  produces ");
        log_("");
    }

    return {m.c};
}


}  // namespace graph_mutator::edge_creation

#endif  // GRAPH_MUTATOR_EDGE_CREATION_FUNCTOR_OLD_CHAIN_H
