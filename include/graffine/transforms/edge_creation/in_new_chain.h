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
 * \brief Creation of an edge forming a new chain, branching from a vertex.
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


/** Creates an edge at a vertex, forming a new chain branching from it.
 * \details The new edge forms a single-edge chain, joining the source
 * vertex, whose degree \p D increases by one, to a new free end (degree 1).
 * The edge and its chain are first created as a new component, which is
 * then merged into the source vertex by a vertex merger 1+D: the hooks of
 * that merger run inside this transformation (see context.h). Usage:
 * \code
 * edge_creation::InNewChain<Deg2, G> branch {gr};
 * const auto [eg] = branch(G::BulkSlot {w, 3});  // a junction at position 3 of w
 * \endcode
 * \tparam D Degree of the source vertex, from which the chain is branched
 * out: 2 (an internal vertex, or the boundary vertex of an unconnected
 * cycle) or 3 (a junction).
 * \tparam G Graph to which the transformation is applied.
 * \tparam PP Processor of non-topological properties, or void.
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

    static constexpr auto isNewChain = true;  ///< The edge forms a new chain.

    static constexpr auto I1 = undefined<Degree>; ///< There is only one input vertex.
    static constexpr auto I2 = D;     ///< Degree of the input vertex.
    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = I2 + Deg1;  ///< Degree of the 2nd output vertex.

    /// Context of the hooks of the inner vertex merger.
    static constexpr Frame frame {Trait::Type, D};

    const std::string d {std::to_string(I2)};       ///< Source degree, as text.
    const std::string shortName {"ec_n"s + d};      ///< Abbreviated name.
    static constexpr const char* fullName  {"edge_creation_new_chain"};  ///< Name in messages.

    /** Constructs a functor creating an edge and its host chain in a
     * graph.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit InNewChain(Graph& gr,
                        std::shared_ptr<PP> pp = nullptr);

    /** Creates a new single-edge chain branching from vertex \p v.
     * \details Calls the overload taking the slot of \p v, or the cycle id
     * if \p v is the boundary vertex of an unconnected cycle.
     * Unchecked: see Applicable.
     * \return Pointer to the new edge.
     */
    auto apply(Vertex& v) -> Res;

    /** Creates a new single-edge chain branching from the vertex at \p s.
     * \details The source vertex is a chain internal vertex (D == Deg2,
     * Slot is BulkSlot) or a three-way junction (D == Deg3, Slot is
     * EndSlot). Unchecked: see Applicable.
     * \param[in] s Slot at the source vertex.
     * \return Pointer to the new edge.
     */
    auto apply(const Slot& s) -> Res;

    /** Creates a new single-edge chain branching from the boundary vertex
     * of the unconnected cycle \p w, which has no slot of its own.
     * \details Only for D == Deg2. Unchecked: see Applicable.
     * \param[in] w Id of an unconnected cycle chain.
     * \return Pointer to the new edge.
     */
    auto apply(ChIdG w) -> Res;

    /** Checks that a new edge can be attached to vertex \p v.
     * \details Rejects \p v if it does not belong to the graph or its degree
     * is not D, then checks its slot as the slot overload does.
     */
    auto applicability(const Vertex& v) const -> Applicability;

    /** Checks that a new edge can be attached to the vertex at \p s.
     * \details Rejects an invalid slot and, for D == 3, a chain end not at a
     * vertex of degree 3.
     */
    auto applicability(const Slot& s) const -> Applicability;

    /** Checks that a new edge can be attached to the boundary vertex of the
     * unconnected cycle \p w.
     * \details Rejects every \p w unless D == 2, and a chain \p w which
     * does not exist or is not an unconnected cycle.
     */
    auto applicability(ChIdG w) const -> Applicability;

    /** Outcome of the creation of a new chain at vertex \p v (see plan.h).
     */
    auto plan(const Vertex& v) const -> Plan<G> { return {&v}; }

    /** Outcome of the creation of a new chain at slot \p s (see plan.h).
     */
    auto plan(const Slot& s) const -> Plan<G>
    {
        return {&plans::vertex_at(gr, s)};
    }

    /** Outcome of the creation of a new chain at the boundary vertex of
     * cycle \p w (see plan.h).
     */
    auto plan(ChIdG w) const -> Plan<G>
    {
        return {&plans::vertex_at(gr, w)};
    }

    /** Processor of non-topological properties.
     */
    auto processor() const noexcept -> PP* { return updateProperties.get(); }

    /** The graph transformed.
     */
    auto graph() const noexcept -> const Graph& { return gr; }

protected:

    Graph& gr;  ///< Reference to the modified graph object.
    Chains& cn; ///< The chains of \a gr.

    /// Inner transformation merging the free end of the new chain (degree
    /// 1) with the source vertex (degree I2).
    vertex_merger::From<Deg1, I2, Graph, PP> merge_vertices;

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

private:

    /** Calls \p f with the location of vertex \p v.
     * \details For D == 2 the location is the bulk slot of \p v, or the
     * chain id if \p v is the boundary vertex of an unconnected cycle;
     * otherwise it is a chain end slot at \p v.
     */
    template<typename F>
    auto with_slot(const Vertex& v, F&& f) const
    {
        if constexpr (D == Deg2) {
            const auto s = locate::bulk_slot(gr, v);
            return checks::is_valid(gr, s) ? f(s)
                                           : f(s.w);  // cycle boundary
        }
        else
            return f(locate::end_slot(gr, v));
    }

    /** Completes the creation, once the new chain is attached at its end A.
     * \return Pointer to the new edge.
     */
    auto finish() -> Res;

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
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor given, while PP is void");
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

    return with_slot(v, [this](const auto& s) { return applicability(s); });
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
    return with_slot(v, [this](const auto& s) { return apply(s); });
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
        hooks::on_start<Slot>(updateProperties->edgeCreation, s);

    constexpr auto e = End::A;  // end of the new chain to get connected

    nested<PP>(frame, [&] { merge_vertices(EndSlot{cn.back().idw, e}, s); });

    return finish();
}


// Creates new single-edge chain attached to an unconnected cycle chain.
// For all other cases the other overload should be called.
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

    gr.add_single_chain_component(1);

    // As in the slot overload, the hook sees the new chain already added.
    if constexpr (!std::is_void_v<PP>)
        hooks::on_start(updateProperties->edgeCreation, w);

    constexpr auto e = End::A;

    nested<PP>(frame, [&] { merge_vertices(EndSlot{cn.back().idw, e}, w); });

    return finish();
}


template<Degree D,
         typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto InNewChain<D, G, PP>::
finish() -> Res
{
    const auto u = gr.edge(gr.num_edges()-1).w;  // ChIdG of the new chain
    const auto ep = &cn[u].g[0];                 // pointer to the new edge

    if constexpr (!std::is_void_v<PP>)
        hooks::on_end<isNewChain>(updateProperties->edgeCreation, *ep);

    if constexpr (verboseF)
        log_after(EndSlot{u, End::A});

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
