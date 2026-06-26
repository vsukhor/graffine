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
 * \file deleting_host_chain.h
 * \brief Struct performing deletion of graph edges along with host chain.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_DELETING_HOST_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_DELETING_HOST_CHAIN_H

#include "graffine/definitions.h"
//#include "graffine/properties/processors.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"

#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::edge_deletion {

/**
 * Functor template for deletion of a graph edge along with its host chain.
 * \details This template is specialized for different vertex degrees.
 * Handles the case of single-edge chains. Importantly, a single-edge chain is
 * suitable for deletion upon the edge removal only if one of its ends is
 * unconnected, while the other end is at a junction.
 * \tparam D Vertex degree at the connected end of the deleted chain.
 * \tparam Graph hosting the edge to be deleted.
 */
template<Degree D,
         typename G,
         typename PP = void>
struct DeletingHostChain
{
    static_assert(isJunctionDegree<D>);

    using Trait = edge_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto isChainDeleteded = true;

    /// This is a single-edge chain, so one of the edge ends has degree 1.
    static constexpr auto I1 = Deg1;

    /**
     * This is a single-edge connected chain,
     * \details One of the edge ends is a (3-way or 4-way) junction,
     * because single-edge loops are not allowed.
     */
    static constexpr auto I2 = D;  // Deg3 or Deg4

    // Output degrees.

    /// There is only one vertex of interest after the deletion.
    static constexpr auto J1 = undefined<Degree>;
    /// Degree of the connection vertex after the chain deletion.
    static constexpr auto J2 = I2 - 1;

    const std::string dd {std::to_string(I2)};

    /**
     * Short name for edge deletion functors.
     * \note Are used for naming conventions in the code.
     */
    const std::string shortName {make_name("edrh_", dd)};

    /**
     *  Full name for edge deletion functors.
     * \note Are used for more descriptive naming conventions in the code.
     */
    const std::string fullName {make_name("edge_deletion_with_host_chain_", dd)};

// public methods

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    constexpr explicit DeletingHostChain(Graph& gr,
                                         std::shared_ptr<PP> pp = nullptr);

    /**
     * Deletion using edge index.
     * \param[in] ind Graph-wide index of the deleted edge.
     */
    auto operator()(EgIdG ind) noexcept -> Res;

    /**
     * Dletion using edge position.
     * \param[in] s End slot connecting the deleted edge.
     */
    auto operator()(const EndSlot& s) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
     std::shared_ptr<PP> updateProperties;

    // Auxiliary functors.

    /// Component deletion.
    component_deletion::Functor<Graph, PP> clrem;
    /// Vertex split (1, D-1).
    vertex_split::To<Deg1, J2, Graph, PP> split;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D,
         typename G,
         typename PP>
constexpr
DeletingHostChain<D, G, PP>::
DeletingHostChain(Graph& gr,
                  std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
    , clrem {gr, pp}
    , split {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<Degree D,
         typename G,
         typename PP>
auto DeletingHostChain<D, G, PP>::
operator()(const EndSlot& s) noexcept -> Res
{
    const auto [w, e] = s.we();

    auto& m = gr.chain(w);
    const auto c = m.c;
    const auto& ngs = gr.ngs_at(s);
    const auto ind = m.end_edge(e).ind;

    if constexpr (verboseF) {
        jot(colorcodes::GREEN, "Edge_deletion ", dd, ": ",
                   colorcodes::BOLDYELLOW, "chain ", w, " ", s.ea_str(),
                   colorcodes::RESET, '\n');
        m.g[0].print(std::string(colorcodes::MAGENTA) + "To delete: "s +
                     colorcodes::RESET);
        jot("");
        m.print("before ", shortName, " ", s.ea_str());
        for (const auto& ng : ngs())
            gr.chain(ng.w).print("              ", ng.ea_str());

        jot("");
    }

    ASSERT(w < gr.num_chains(),
           "w ", w, " exceeds number of chains ", gr.num_chains());
    ASSERT(m.length() == 1, "inncorrect functor for edge deletion: length > 1");
    ASSERT(m.is_shrinkable(), "chain ", w, " is not shrinkable.");
    ASSERT(m.has_single_leaf_vertex(),
            "chain ", w, " does not a have single connected end");
    ASSERT(ngs.num() == structure::containers::numNeigs<D>, "slot ", s.w, " ",
           s.ea_str(), " has incorrect degree ", ngs.num() + 1, " != D ", D);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeDeletion.template on_start<isChainDeleteded>(m.g[0]);

    // The deleted chain has a connected end because othrwise a component
    // deletion rather than edge deletion should have been active.
    // The connected end is single because if both ends were connected,
    // edge (and hence, chain) removal would induce topology
    // change by vertex merger on the chain ends.

    const auto [v1, v2] = split(s);

    // index of the connected component holding the edge
    const auto vRemove = gr.compt(v1->get_c()).get_egl(0).i == ind
                       ? v1
                       : v2;
    [[maybe_unused]] const auto vStay = vRemove == v1 ? v2 : v1;
    const auto cr = vRemove->get_c();

    ASSERT(gr.compt(cr).num_edges() == 1, "component to remove is too big");

    clrem(cr);    // calls gr.update() inside

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeDeletion.template on_end<isChainDeleteded>(*vStay);

    if constexpr (verboseF)
        jot(shortName, " ends with success \n");

    // 'c' is not necessarily the index of the component actually removed,
    // it is 'cr' which was.
    return {c};
}


template<Degree D,
         typename G,
         typename PP>
auto DeletingHostChain<D, G, PP>::
operator()(const EgIdG ind) noexcept -> Res
{
    const auto& m = gr.chain(gr.egl[ind].w);

    ASSERT(m.length() == 1, "inncorrect functor for edge deletion: length > 1");
    ASSERT(m.is_shrinkable(), "chain ", m.idw, " is not shrinkable.");
    ASSERT(m.has_single_leaf_vertex(),
            "chain ", m.idw, " does not a have single connected end");

    return (*this)(EndSlot{m.idw, m.get_single_connected_end()});
}


}  // namespace graffine::transforms::edge_deletion

#endif // GRAFFINE_TRANSFORMS_EDGE_DELETION_DELETING_HOST_CHAIN_H
