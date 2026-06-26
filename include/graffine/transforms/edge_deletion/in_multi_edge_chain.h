/* =============================================================================

This file is part of graffine, a lightweight graph transformation library.

Copyright (c) 2021-2026 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * \file in_multi_edge_chain.h
 * \brief Contains class performing edge deletion in chains consisting of
 * multiple edges.
 * \note This case is discriminated because deletion of an edge in multi-edge
 * chains does not trigger automatically deletion of the host chain itself
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_MULTI_EDGE_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_MULTI_EDGE_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_deletion/preserving_host_chain.h"

#include <memory>
#include <type_traits>

/**
 * Contains operations handling edge deletion in a graph.
 * These are functors for deleting edges based on chain lengths and vertex
 * degrees.
 */
namespace graffine::transforms::edge_deletion {

/**
 * Vertex degree-independent wrapper class for edge deletion in chains
 * consisting of multiple edges.
 * \details Handles the case of multi-edge chains. Notably, because the chain
 * contains more than a single edge, at least one of the edge ends is a vertex
 * of degree 2.
 * \note This case is discriminated because deletion of an edge in multi-edge
 * chains does not trigger automatically deletion of the host chain itself
 * \tparam Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
struct InMultiEdgeChain
{
    using Trait = edge_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Edge = Chain::Edge;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    /// This is a single-edge chain, so one of the edge ends has degree 1.
    static constexpr auto I = Deg1;

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit constexpr InMultiEdgeChain(Graph& gr,
                                        std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the deletion using edge index.
     * \param[in] ind Graph-wide index of the deleted edge.
     */
    auto operator()(EgIdG ind) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    PreservingHostChain<1, G, PP> deg1;
    PreservingHostChain<2, G, PP> deg2;
    PreservingHostChain<3, G, PP> deg3;
    PreservingHostChain<4, G, PP> deg4;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
constexpr
InMultiEdgeChain<G, PP>::
InMultiEdgeChain(Graph& gr,
                 std::shared_ptr<PP> pp)
    : gr {gr}
    , deg1 {gr, pp}
    , deg2 {gr, pp}
    , deg3 {gr, pp}
    , deg4 {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto InMultiEdgeChain<G, PP>::
operator()(const EgIdG ind) noexcept -> Res
{
    const auto& m = gr.chain(ind);
    const auto& eg = m.g[gr.egl[ind].a];

    ASSERT_CALLING(m.length() > 1,
                   m.print("InMultiEdgeChain"),
                   "incorrect chain for edge deletion in multi-edge chain: ",
                   m.length());
    ASSERT(m.is_shrinkable(), "chain ", m.idw, " is not shrinkable.");

    const auto& vA = *eg.vertex_at_outer(End::A);
    const auto& vB = *eg.vertex_at_outer(End::B);

    const auto bA = m.is_unconnected_cycle_boundary(vA);
    const auto bB = m.is_unconnected_cycle_boundary(vB);

    ASSERT(not(bA || bB),
           "edge is not deleatable: boundary of disconnected cycle chain");

    const auto d = vA.is_bulk() ? vB.deg() : vA.deg();

    if (d == Deg1) return deg1(ind);
    if (d == Deg2) return deg2(ind);
    if (d == Deg3) return deg3(ind);
    if (d == Deg4) return deg4(ind);

    return {ResT::undefined};  // unreachable
}


}  // namespace graffine::transforms::edge_deletion

#endif  // GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_MULTI_EDGE_CHAIN_H
