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
 * \file preserving_host_chain.h
 * \brief Contains classes performing deletion of specific graph edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_PRESERVING_HOST_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_PRESERVING_HOST_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/edge_deletion/common.h"

#include <memory>
#include <string>
#include <type_traits>

/**
 * Contains operations handling edge deletion in a graph.
 * These are functors for deleting edges based on chain lengths and vertex
 * degrees.
 */
namespace graffine::transforms::edge_deletion {


/**
 * Template for deletion of graph edges without deleting their host chain.
 * \details Handles the case of multi-edge chains. Notably, because the chain
 * contains more than a single edge, at least one of the edge ends is a vertex
 * of degree 2.
 * \tparam D Vertex degree at the non-trivial edge end (the other one has D==2).
 * \tparam Graph hosting the edge to be deleted.
 */
template<Degree D,
         typename G,
         typename PP = void>
struct PreservingHostChain
{
    static_assert(isImplementedDegree<D>);

    using Trait = edge_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Edge = Chain::Edge;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto isChainDeleteded = false;

    /**
     * Vertex degree at the first end of the deleted edge.
     * \note This is a multi-edge chain, so this verex has degree 2.
     */
    static constexpr auto I1 = Deg2;

    /**
     * Vertex degree at the second end of the deleted edge.
     * \note This verex can have arbitrary implemented degree.
     */
    static constexpr auto I2 = D;

    static constexpr auto J1 = undefined<Degree>;
    static constexpr auto J2 = undefined<Degree>;

    const std::string dd {std::to_string(I2)};

    /**
     * Short form of human-readable name of edge deletion functors.
     * \note Is used for naming conventions in the code.
     */
    const std::string shortName {make_name("edph_", dd)};

    /**
     * Full form of human-readable name of edge deletion functors.
     * \note Is used for more descriptive naming conventions in the code.
     */
    const std::string fullName {
        make_name("edge_deletion_without_host_chain_", dd)
    };

// public methods

    /**
     * Constructs the Functor object based on a Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties. If null, no
     *               processing is intended
     */
    constexpr explicit PreservingHostChain(Graph& gr,
                                           std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the deletion using edge index.
     * \param[in] ind Graph-wide index of the deleted edge.
     */
    auto operator()(EgIdG ind) noexcept -> Res;

    /**
     * Delete edge using its position.
     * \param[in] s Slot incident to a chain internal vertex.
     */
    auto operator()(const BulkSlot& s) noexcept -> Res;

protected:  // Data

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D,
         typename G,
         typename PP>
constexpr
PreservingHostChain<D, G, PP>::
PreservingHostChain(Graph& gr,
                    std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<Degree D,
         typename G,
         typename PP>
auto PreservingHostChain<D, G, PP>::
operator()(const BulkSlot& s) noexcept -> Res
{
    const auto [w, a] = s.we();

    auto& m = gr.chain(w);

    if constexpr (verboseF) {

        jot(colorcodes::GREEN, "Edge_deletion ", dd, ": ",
            colorcodes::BOLDYELLOW, "chain ", w, " at ", a, colorcodes::RESET,
            '\n');
        m.g[a].print(std::string(colorcodes::MAGENTA) + "To delete: "s +
                     colorcodes::RESET);
        jot("");
        m.print("before ed at ", s.ea_str());
        jot("");
    }

    ASSERT(w < gr.num_chains(),
           shortName, ": w ", w, "exceeds number of chains ", gr.num_chains());
    ASSERT(a < m.length(), shortName,
           ": a ", a, " exceeds chain length ", m.length(), "of chain ", w);
    ASSERT(m.is_shrinkable(), shortName, ": chain ", w, "is not shrinkable");

    const auto c = m.c;

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeDeletion.template on_start<isChainDeleteded>(m.g[a]);

    // Topology is not affected because the chain is not destroyed

    // Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
    // These then become g[a], g[a+1], ... .
    gr.remove_edge(m.g[a].ind);  // gr.update() functionality is included

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeDeletion.template on_end<isChainDeleteded>(
            a < m.length() ? *m.g[a].vertex_at_outer(End::A)
                           : *m.g[a-1].vertex_at_outer(End::B)
        );

    if constexpr (verboseF) {
        m.print(shortName, " produces");
        jot("");
    }

    return {c};
}


template<Degree D,
         typename G,
         typename PP>
auto PreservingHostChain<D, G, PP>::
operator()(const EgIdG ind) noexcept -> Res
{
    return (*this)(gr.ind2bslot(ind));
}



}  // namespace graffine::transforms::edge_deletion

#endif  // GRAFFINE_TRANSFORMS_EDGE_DELETION_PRESERVING_HOST_CHAIN_H
