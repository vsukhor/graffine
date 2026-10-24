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
 * \file in_new_chain.h
 * \brief Contains class performing creation of graph edges in new chain.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_NEW_CHAIN_H
#define GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_NEW_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/neigs.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/edge_creation/common.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"

#include <memory>
#include <type_traits>

namespace graffine::transforms::edge_creation {


/**
 * Creates an edge adjoint to a vertex, branching a new chain.
 * \details Creates a graph edge together with its host chain. The edge will
 * have as one of its ends a vertex whose initial degree \p D will be
 * incremented by addition of the new chain branching from it. Because the new
 * chain has length 1, the other vertex of the new edge has degree 1.
 * \tparam D Degree of the source vertex, from which the chain is branched out.
 * \tparam G Graph to which the transformation is applied.
 */
template<Degree D,
         typename G,
         typename PP = void>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct InNewChain
    : public Applicable<InNewChain<D, G, PP>>
{
    static_assert(D == Deg2 || D == Deg3);

    using Trait = edge_creation::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using Edge = Chain::Edge;
    using Vertex = Graph::Vertex;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Slot = structure::descriptors::Slot<D>;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto isNewChain = true;

    static constexpr auto I1 = undefined<Degree>; // There is only one input vertex.
    static constexpr auto I2 = D;     ///< Degree of the input vetrex.
    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = I2 + Deg1;  ///< Degree of the 2nd output vertex.

    const std::string d {std::to_string(I2)};
    const std::string shortName {"ec_n"s + d};
    static constexpr const char* fullName  {"edge_creation_new_chain"};

    /**
     * Constructs a functor for creating an edge and its host chain an in Graph.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit InNewChain(Graph& gr,
                        std::shared_ptr<PP> pp = nullptr);

    auto apply(Vertex& v) -> Res;

    /**
     * Creates a new edge packed into a single-edge chain.
     * \details The source vertex may be either a chain internal vertex
     * (D == Deg2) or a three-way junction (D == Deg3).
     * Depending on the case, type Slot of the source corresponds to the
     * vertex_merger functor applied to connect the new chain to the source
     * vertex.
     * \tparam Slot Type of the source. One of the following:
     *    BulkSlot: at chain internal vertex (D == Deg2), or
     *    EndSlot: at branching vertex (D == Deg3).
     * \param[in] s Slot at the source vertex.
     */
    auto apply(const Slot& s) -> Res;

    /**
     * Creates a new edge packed into a single-edge chain.
     * \details The source vertex is a boundary vertex of an unconnected chain.
     * \overload
     * \param[in] w Id of the an unconnected cycle chain .
     */
    auto apply(ChIdG w) -> Res;

    /// Checks that a new edge can be attached to vertex \p v.
    auto applicability(const Vertex& v) const -> Applicability;

    /// Checks that a new edge can be attached to the vertex at \p s.
    auto applicability(const Slot& s) const -> Applicability;

    /// Checks that a new edge can be attached to the boundary vertex of the
    /// unconnected cycle \p w.
    auto applicability(ChIdG w) const -> Applicability;

protected:

    Graph& gr;  ///< Reference to the modified graph object.
    Chains& cn;

    /// Auxiliary transform merging vertexes of degrees Deg1 and I2.
    /// Is utilized for internal operation.
    vertex_merger::From<Deg1, I2, Graph, PP> merge_vertices;

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

private:

    template<typename S>
    void log_before(const S& s,
                    std::string&& at) const;
    void log_after(const EndSlot& ue) const;

    void check_slot(const Slot& s) const;

    auto pos_str(const Slot& s) const noexcept -> std::string;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
InNewChain<D, G, PP>::
InNewChain(Graph& gr,
           std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , merge_vertices {gr, pp}
    , updateProperties {pp}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}

template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
applicability(const Vertex& v) const -> Applicability
{
    if (!checks::is_in_graph(gr, v))
        return reject("vertex not in the graph");
    if (v.deg() != D)
        return reject("vertex degree does not match the transformation");

    if constexpr (D == Deg2) {
        const auto s = locate::bulk_slot(gr, v);
        return checks::is_valid(gr, s) ? applicability(s)
                                       : applicability(s.w);  // cycle boundary
    }
    else
        return applicability(locate::end_slot(gr, v));
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
applicability(const Slot& s) const -> Applicability
{
    if (!checks::is_valid(gr, s))
        return reject("invalid slot");

    if constexpr (D == Deg3)
        if (gr.ngs_at(s).num() != structure::containers::numNeigs<D>)
            return reject("chain end is not at a vertex of degree 3");

    return {};
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
applicability(const ChIdG w) const -> Applicability
{
    if constexpr (D != Deg2)
        return reject("boundary vertex of a cycle has degree 2");
    else {
        if (!checks::is_valid(gr, w))
            return reject("invalid chain");
        if (!cn[w].is_unconnected_cycle())
            return reject("chain is not an unconnected cycle");

        return {};
    }
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
apply(Vertex& v) -> Res
{
    if constexpr (D == Deg2) {
        const auto s = locate::bulk_slot(gr, v);
        return checks::is_valid(gr, s) ? apply(s)
                                       : apply(s.w);  // cycle boundary
    }
    else
        return apply(locate::end_slot(gr, v));
}

// Creates new single-edge chain attached to the original chain(s) at slot 's'.
template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
apply(const Slot& s) -> Res
{
    if constexpr (verboseF)
        log_before(s, pos_str(s));

    check_slot(s);

    gr.add_single_chain_component(1);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.template on_start<Slot>(s);

    constexpr auto e = End::A;  // end of the new chain to get connected

    merge_vertices(EndSlot{cn.back().idw, e}, s);

    const auto u = gr.edge(gr.num_edges()-1).w;  // ChIdG of the new chain
    const auto ep = &cn[u].g[0];                 // pointer to the new edge

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.template on_end<isNewChain>(*ep);

    if constexpr (verboseF)
        log_after(EndSlot{u, e});

    return {ep};
}


// Creates new single-edge chain attached to an unconnected cycle chain.
// For all all other cases the other overload should be called.
template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
apply(const ChIdG w) -> Res
{
    if constexpr (verboseF)
        log_before(w, ": unconnected cycle");

    // The new edge is attached to a boundary vertex of an unconnected cycle.
    GRAFFINE_ASSERT(cn[w].is_unconnected_cycle(),
           "chain", w, " is not an unconnected cycle.");

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.on_start(EndSlot{w, End::A});

    gr.add_single_chain_component(1);

    constexpr auto e = End::A;

    merge_vertices(EndSlot{cn.back().idw, e}, w);

    const auto u = gr.edge(gr.num_edges()-1).w;
    const auto ep = &cn[u].g[0];                 // pointer to the new edge

    if constexpr (!std::is_void_v<PP>)
        updateProperties->edgeCreation.template on_end<isNewChain>(*ep);

    if constexpr (verboseF)
        log_after(EndSlot{u, e});

    return {ep};
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
template<typename S>
void InNewChain<D, G, PP>::
log_before(const S& s, std::string&& at) const
{
    auto w = ChIdG::undefined;
    if constexpr (std::is_same_v<S, ChIdG>)
        w = s;
    else
        w = s.w;

    jot(
        colorcodes::GREEN, "Edge creation ", J1, I2, colorcodes::RESET,
        " with new chain: ", colorcodes::BOLDYELLOW,  w, at,
        colorcodes::RESET, '\n'
    );

    cn[w].print("before ", shortName);

    if constexpr (D == Deg3)
        for (const auto& u : gr.ngs_at(s)())
            cn[u.w].print("                  ");
    jot("");
}

template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
void InNewChain<D, G, PP>::
log_after(const EndSlot& ue) const
{
    cn[ue.w].print(shortName, " produces");
    const auto ngs = gr.ngs_at(ue)();

    for (const auto& ng : ngs)
        cn[ng.w].print("           and");
    jot("");
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
void InNewChain<D, G, PP>::
check_slot([[maybe_unused]] const Slot& s) const
{
    if constexpr (D == Deg2)

        // The new edge is attached to a three-way junction.
        GRAFFINE_ASSERT(s.a()() && s.a()() < cn[s.w].length(),
               "a ", s.a(), " is not internal edge of chain ", s.w);

    else if constexpr (D == Deg3)

        // The new edge is attached to a four-way junction.
        GRAFFINE_ASSERT(gr.ngs_at(s).num() == structure::containers::numNeigs<D>,
               "num of connections ", gr.ngs_at(s).num(), " at end ",
               s.ea_str(), " of chain ", s.w, " should be ",
               structure::containers::numNeigs<D>);

    else

        static_assert(false, "Degree not valid for edge_creation");
}



template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
pos_str(const Slot& s) const noexcept -> std::string
{
    if constexpr (D == Deg2)

        return " at "s + s.ea_str();

    else if constexpr (D == Deg3)

        return " "s + s.ea_str();

    else

        static_assert(false, "Degree not valid for edge_creation");

    return {};
}


}  // namespace graffine::transforms::edge_creation

#endif  // GRAFFINE_TRANSFORMS_EDGE_CREATION_IN_NEW_CHAIN_H
