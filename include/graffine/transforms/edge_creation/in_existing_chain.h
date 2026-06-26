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
 * \file in_existing_chain.h
 * \brief Contains class performing creation of graph edges in existing chain.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_EXISTING_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_EXISTING_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/edge_creation/common.h"

#include <memory>
#include <utility>

#include <memory>
#include <type_traits>

namespace graffine::transforms::edge_creation {

/**
 * Creates an edge adjoint to a vertex, putting it into an existing chain.
 * \details Creates a graph edge inserted into an existing chain. The edge will
 * have as one of its ends a vertex of degree , not affected by the addition
 * of the edge. Because the edge extends an already existing chain, vertex at
 * the other end of the new edge will have degree 2.
 * \tparam D Degree of one of the vertexes accepting the created edge.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
struct InExistingChain
{
    using Trait = edge_creation::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Edge = Chain::Edge;
    using Vertex = Graph::Vertex;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using End = Chain::End;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto isNewChain = false;

    static constexpr auto I1 = undefined<Degree>;  ///< There is only one input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< Is determined by the parameter.
    static constexpr auto J1 = Deg2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = I2;    ///< Degree of the 2nd output vertex.

    static constexpr const char* shortName {"ec_oc"};
    static constexpr const char* fullName  {"edge_creation_old_chain"};

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit InExistingChain(Graph& gr,
                             std::shared_ptr<PP> pp = nullptr);

    auto operator()(Vertex& v) -> Res;

    /**
     * Executs the transformation.
     * The new edge is appended/inserted at vertex defined by slot \p s.
     * If the vertex is internal, the new edge will become spanned between its
     * incident edges. Otherwise the new dge is appended to the chain, becoming
     * its boundary edge.
     * \param[in] s Slot inside a chain where the new edge is placed.
     */
    template<structure::descriptors::slot S>
    auto operator()(const S& s) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
InExistingChain<G, PP>::
InExistingChain(Graph& gr,
                std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto InExistingChain<G, PP>::
operator()(Vertex& v) -> Res
{
    const auto d = v.deg();
    const auto& cmp = gr.ct[v.get_c()];
    const auto& egv = *v.edges().begin();  // EdgeInVertex
    const auto& egl = cmp.get_egl(egv.w);  // EdgeEnComponent 'egv.w' is EgIdC
    const auto w = egl.w;

    if (d == Deg2) {
        const auto a = egl.a;
        return (*this)(a == 0 ? BulkSlot{w, a+1} : BulkSlot{w, a});
    }

    const auto& m = gr.cn[w];
    return (*this)(m.end_slot(egl.a, egv.e));
}

template<typename G,
         typename PP>
template<structure::descriptors::slot S>
auto InExistingChain<G, PP>::
operator()(const S& s) noexcept -> Res
{
    constexpr auto atChainEnd = std::is_same_v<S, EndSlot>;

    const auto [w, ae] = s.we();
    auto& m = gr.chain(w);

    auto a = EgIdA::undefined;
    if constexpr (atChainEnd)
        a = ae == End::A ? 0 : m.length();  // at chain end
    else
        a = ae;                             // in chain interior

    if constexpr (verboseF) {

        const std::string at = atChainEnd ? " " : " at ";
        jot(colorcodes::GREEN, "Edge creation:", colorcodes::RESET,
            " in existing chain: ",
            colorcodes::BOLDYELLOW,  w, at, s.ea_str(),
            colorcodes::RESET, '\n');
        m.print("before ", shortName, ": ", at, s.ea_str());
        jot("");
    }

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.on_start(w, a, atChainEnd);

    if constexpr (!atChainEnd) {
        ASSERT(a > 0 && a <= m.length(),
               "vertex at ", a, " is not in chain interior");
    }

    // Insert the new edge at 'a', moving forwards original 'a' and the rest
    // of the head portion. In the spatial formulation, the positions anf other
    // properties are set or adjusted by respective property processors.

    auto& cmp = gr.compt(m.c);
    Edge eg{gr.num_edges()};
    gr.set_num_edges(gr.num_edges()+1);
    Edge* ep {};
    if constexpr (atChainEnd)
        ep = ae == End::A
            ? &cmp.prepend_edge_to_chain(std::move(eg), m)
            : &cmp.append_edge_to_chain(std::move(eg), m);
    else
        ep = &cmp.insert_edge_into_chain(std::move(eg), m, a);

    ASSERT(ep == &m.g[a],
           "unsuccessful edge insert at slot ", s.w, " ", s.ea_str());

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.template on_end<isNewChain>(*ep);

    if constexpr (verboseF) {
        m.print(shortName, "  produces ");
        ep->print("result: ");
        jot("");
    }

    return {ep};
}


}  // namespace graffine::transforms::edge_creation

#endif  // GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_EXISTING_CHAIN_H
