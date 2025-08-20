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
 * @file preserving_host_chain.h
 * @brief Contains classes performing deletion of specific graph edges.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_EDGE_DELETION_FUNCTOR_2_H
#define GRAPH_MUTATOR_EDGE_DELETION_FUNCTOR_2_H

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"


/**
 * @brief Contains utilities for handling edge deletion operations in a graph.
 * These are functors for deleting edges based on chain lengths and vertex
 * degrees.
 */
namespace graph_mutator::edge_deletion {


/**
 * @brief Functor template for deletion of graph edges without deleting their host chain.
 * @details Handles the case of multi-edge chains. Notably, because the chain
 * contains more than a single edge, at least one of the edge ends is a vertex
 * of degree 2.
 * @tparam D Vertex degree at one of the edge ends.
 * @tparam Graph hosting the edge to be deleted.
 */
template<Degree D,
         typename G>
struct PreservingHostChain {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);
    static_assert(is_implemented_degree<D>);

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using BulkSlot = Chain::BulkSlot;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Vertex degree at the 1st end of the deleted edge.
     * @note This is a multi-edge chain, so this verex has degree 2.
     */
    static constexpr auto I1 = Deg2;

    /**
     * @brief Vertex degree at the 2nd end of the deleted edge.
     * @note This verex can have arbitrary implemented degree.
     */
    static constexpr auto I2 = D;

    static constexpr auto J1 = undefined<Degree>;
    static constexpr auto J2 = undefined<Degree>;

    static constexpr auto dd = string_ops::str1<I2>;
    /**
     * @brief Short name stem for edge deletion functors.
     * Used for naming conventions in the code.
     */
    static constexpr std::array shortNameStem {'e', 'd', 'p', 'h', '_'};

    /**
     * @brief Full name stem for edge deletion functors.
     * Used for more descriptive naming conventions in the code.
     */
    static constexpr std::array fullNameStem {
        'e', 'd', 'g', 'e', '_', 'd', 'e', 'l', 'e', 't', 'i', 'o', 'n', '_',
        'w', 'i', 't', 'h', 'o', 'u', 't', '_', 'h', 'o', 's', 't', '_',
        'c', 'h', 'a', 'i', 'n', '_'
    };

    static constexpr auto shortName = string_ops::concat<shortNameStem, dd, 1>;
    static constexpr auto fullName  = string_ops::concat<fullNameStem, dd, 1>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    constexpr explicit PreservingHostChain(Graph& gr);

    /**
     * @brief Function call operator executing the deletion using edge index.
     * @param ind Graph-wide index of the deleted edge.
     */
    auto operator()(EgId ind) noexcept -> Res;

    /**
     * @brief Function call operator executing the deletion using edge position.
     * @param s Slot in the chain internal.
     */
    auto operator()(const BulkSlot& s) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the graph object.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D,
         typename G>
constexpr
PreservingHostChain<D, G>::
PreservingHostChain(Graph& gr)
    : gr {gr}
{}


template<Degree D,
         typename G>
auto PreservingHostChain<D, G>::
operator()(const BulkSlot& s) noexcept -> Res
{
    const auto [w, a] = s.we();

    auto& m = gr.cn[w];

    if constexpr (verboseF) {
        log_(colorcodes::GREEN, "Edge_deletion ", dd, ": ",
                   colorcodes::BOLDYELLOW, w, " at ", a, colorcodes::RESET, '\n');
        m.print("before ed at ", s.ea_str());
        log_("");
    }

    ASSERT(w < gr.chain_num(),
           shortName, ": w ", w, "exceeds number of chains ", gr.chain_num());
    ASSERT(a < m.length(), shortName,
           ": a ", a, " exceeds chain length ", m.length(), "of chain ", w);
    ASSERT(m.is_shrinkable(), shortName, ": chain ", w, "is not shrinkable");
    ASSERT(
        (D == 2 && !m.is_tail(a) && !m.is_head(a)) ||
        (!D && (m.is_tail(a) || m.is_head(a)) && m.is_disconnected_cycle()) ||
        (D && D != 2 && ((m.is_tail(a) && m.ngs[Ends::A].num() == D-1) ||
                         (m.is_head(a)  && m.ngs[Ends::B].num() == D-1))),
        shortName, ": incorrect a ", a, " for chain ", w
    );

    const auto c = m.c;
    const auto ind = m.g[a].ind;

    // topology is not affected because the chain is not destroyed

    if (const auto& b = gr.ct[c].gl.back();
        m.g[a].ind != b.i)
        gr.cn[b.w].g[b.a].indc = m.g[a].indc;

    if (ind < gr.edgenum-1) {
        auto& elast = gr.edge(gr.edgenum-1);
        elast.ind = ind;
        gr.ct[elast.c].set_gl();
    }

    gr.edgenum--;
    m.g.erase(m.g.begin() + static_cast<long>(a));
    m.set_g_w();

    gr.ct[c].set_gl();

    gr.update();

    if constexpr (verboseF) {
        m.print(shortName, " produces");
        log_("");
    }

    return {c};
}


template<Degree D,
         typename G>
auto PreservingHostChain<D, G>::
operator()(const EgId ind) noexcept -> Res
{
    return (*this)(gr.ind2bslot(ind));
}



}  // namespace graph_mutator::edge_deletion

#endif // GRAPH_MUTATOR_EDGE_DELETION_FUNCTOR_2_H
