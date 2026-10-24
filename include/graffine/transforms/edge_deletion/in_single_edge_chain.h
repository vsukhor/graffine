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
 * \file in_single_edge_chain.h
 * \brief Struct performing deletion of graph edges along with host chain.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_SINGLE_EDGE_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_SINGLE_EDGE_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_deletion/deleting_host_chain.h"

#include <memory>
#include <type_traits>

/**
 * Contains utilities for handling edge deletion operations in a graph.
 * \details These are functors for deleting edges based on chain lengths and
 * vertex degrees.
 */
namespace graffine::transforms::edge_deletion {

/**
 * Vertex degree-independent wrapper class for edge deletion in chains
 * consisting of single edge.
 * \details Handles the case of single-edge chains. Notably, because the chain
 * contains only one edge, at least one of the edge ends is a vertex
 * of degree 1. So, a single-edge chain is suitable for deletion upon the edge
 * removal only if one of its ends is unconnected, while the other end is at a
 * junction between chains.
 * \note This case is discriminated because deletion of an edge in single-edge
 * chains does trigger automatically deletion of the host chain itself
 * \tparam Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct InSingleEdgeChain
    : public Applicable<InSingleEdgeChain<G, PP>>
{
    using Trait = edge_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using EndSlot = Chain::EndSlot;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    /// This is a single-edge chain, so one of the edge ends has degree 1.
    static constexpr auto I = Deg1;

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit constexpr InSingleEdgeChain(Graph& gr,
                                         std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the deletion using edge index.
     * \param[in] ind Graph-wide index of the deleted edge.
     */
    auto apply(EgIdG ind) -> Res;

    /// Checks that edge \p ind can be deleted with its single-edge chain.
    auto applicability(EgIdG ind) const -> Applicability;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    DeletingHostChain<3, G, PP> deg3;
    DeletingHostChain<4, G, PP> deg4;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
constexpr
InSingleEdgeChain<G, PP>::
InSingleEdgeChain(Graph& gr,
                  std::shared_ptr<PP> pp)
    : gr {gr}
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
auto InSingleEdgeChain<G, PP>::
applicability(const EgIdG ind) const -> Applicability
{
    if (!checks::is_valid_edge(gr, ind))
        return reject("invalid edge index");

    const auto& m = gr.chain(ind);
    if (m.length() != 1 || !m.has_single_leaf_vertex())
        return reject("edge is not a chain attached by a single end");

    const auto d = m.ngs[m.get_single_connected_end()].num() + 1;
    if (d == Deg3) return deg3.applicability(ind);
    if (d == Deg4) return deg4.applicability(ind);

    return reject("vertex degree at the chain end is not 3 or 4");
}


template<typename G,
         typename PP>
    requires edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InSingleEdgeChain<G, PP>::
apply(const EgIdG ind) -> Res
{
    const auto& m = gr.chain(ind);
    const auto d = m.ngs[m.get_single_connected_end()].num() + 1;

    return d == Deg3 ? deg3.apply(ind)
                     : deg4.apply(ind);
}


}  // namespace graffine::transforms::edge_deletion

#endif // GRAFFINE_TRANSFORMS_EDGE_DELETION_IN_SINGLE_EDGE_CHAIN_H
