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
 * \tparam G Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct InMultiEdgeChain
    : public Applicable<InMultiEdgeChain<G, PP>>
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
    auto apply(EgIdG ind) -> Res;

    /// Checks that edge \p ind can be deleted from its multi-edge chain.
    /// \details Rejects an invalid edge index, an edge of a single-edge
    /// chain, and an edge at the boundary vertex of an unconnected cycle,
    /// then checks it as the PreservingHostChain functor for the degree at
    /// its non-bulk end does.
    auto applicability(EgIdG ind) const -> Applicability;

private:

    /// Degree of the end vertex of edge \p ind other than of degree 2.
    auto end_degree(EgIdG ind) const -> Degree;

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
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
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
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InMultiEdgeChain<G, PP>::
end_degree(const EgIdG ind) const -> Degree
{
    const auto& eg = gr.edge(ind);
    const auto& vA = *eg.vertex_at_outer(End::A);
    const auto& vB = *eg.vertex_at_outer(End::B);

    return vA.is_bulk() ? vB.deg() : vA.deg();
}


template<typename G,
         typename PP>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InMultiEdgeChain<G, PP>::
applicability(const EgIdG ind) const -> Applicability
{
    if (!checks::is_valid_edge(gr, ind))
        return reject("invalid edge index");

    const auto& m = gr.chain(ind);
    const auto& eg = gr.edge(ind);
    if (m.length() < 2)
        return reject("chain has a single edge");
    if (m.is_unconnected_cycle_boundary(*eg.vertex_at_outer(End::A)) ||
        m.is_unconnected_cycle_boundary(*eg.vertex_at_outer(End::B)))
        return reject("edge is at the boundary of an unconnected cycle");

    switch (end_degree(ind)) {
        case Deg1: return deg1.applicability(ind);
        case Deg2: return deg2.applicability(ind);
        case Deg3: return deg3.applicability(ind);
        case Deg4: return deg4.applicability(ind);
        default:   return reject("vertex degree at the edge end is not implemented");
    }
}


template<typename G,
         typename PP>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InMultiEdgeChain<G, PP>::
apply(const EgIdG ind) -> Res
{
    switch (end_degree(ind)) {
        case Deg1: return deg1.apply(ind);
        case Deg2: return deg2.apply(ind);
        case Deg3: return deg3.apply(ind);
        default:   return deg4.apply(ind);
    }
}


}  // namespace graffine::transforms::edge_deletion

#endif  // GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_MULTI_EDGE_CHAIN_H
