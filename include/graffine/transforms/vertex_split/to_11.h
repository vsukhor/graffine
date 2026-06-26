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
 * \file to_11.h
 * \brief Contains class template enabling splits of specific graph vertices.
 * \details This Functor template specialization operates on vertices of
 * degree 2 to produce two degree 1 vertices.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_11_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_11_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/vertex_split/reporter.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::vertex_split {

/**
 * Template specialization for a functor generating 2 degree 1 vertices.
 * \details Adds vertex type-specific split capability and updates
 * the graph for it. This implementation operates on degree 2 vertices
 * at specific position to produce two degree 1 vertices: V2 -> 2V1.
 * \tparam G Graph to which the split operation is applied.
 */
template<typename G,
         typename PP>
struct To<Deg1, Deg1, G, PP>
{
    using Trait = vertex_split::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using EdgeInVertex = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Deg1;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = J1 + J2;  ///< Degree of the input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

    static_assert(areCompatibleDegrees<J1, J2>);

    const std::string dd {degrees_to_string(J1, J2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a functor for vertex split operation acting on a Graph.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit To(Graph& gr,
                std::shared_ptr<PP> pp = nullptr);

    /**
     * Divides the graph chain at a vertex of degree 2.
     * \param[in] v Internal chain vertex.
     */
    auto operator()(Vertex& v) -> Res;

    /**
     * Divides the graph chain at a vertex of degree 2.
     * \param[in] s Slot on internal chain vertex. Location parameter \a s.a is
     *              counted from 1 and designates the last edge to remain in the
     *              original chain.
     */
    auto operator()(const BulkSlot& s) -> Res;

protected:

    /**
     * Divides an unconnected cycle chain at its boundary vertex.
     * \param[in] w Graph-wide chain index.
     */
    auto disconnected_cycle(ChIdG w) -> Res;

    /**
     * Divides an unconnected cycle chain at its internal vertex.
     * \param[in] s Slot on internal chain vertex. Location parameter \a s.a is
     *              counted from 1 and designates the last edge to remain in the
     *              original chain.
     */
    auto disconnected_cycle(const BulkSlot& s) -> Res;

    /**
     * Divides a linear chain at its internal vertex.
     * \param[in] s Slot on internal chain vertex. Location parameter \a s.a is
     *              counted from 1 and designates the last edge to remain in the
     *              original chain.
     */
    auto linear(const BulkSlot& s) -> Res;

private:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;

    /// Auxiliary functor for intermediate vertex merger.
    vertex_merger::Core<Graph, PP> merge;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
To<Deg1, Deg1, G, PP>::
To(Graph& gr,
   std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
    , report {dd, gr}
    , merge {gr, pp, "vm_core called from"s + shortName}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto To<Deg1, Deg1, G, PP>::
operator()(Vertex& v) -> Res
{
    ASSERT_CALLING(v.deg() == I1,
                   v.print("INCORRECT DEGREE"),
                   "Vertex must have degree", I1, " actual degree is", v.deg());

    const auto d = v.deg();
    const auto& cmp = gr.ct[v.get_c()];
    const auto& egd = *v.edges().begin();
    const auto w = cmp.get_egl(egd.indc()).w;
    const auto& m = gr.cn[w];

    if (m.is_unconnected_cycle_boundary(v))

        return disconnected_cycle(w);

    return (*this)(cmp.bulk_slot(egd));
}

template<typename G,
         typename PP>
auto To<Deg1, Deg1, G, PP>::
operator()(const BulkSlot& s) -> Res
{
    const auto [w, a] = s.we();
    const auto& m = gr.chain(w);

    return a == 0 || a == m.length()
        ? disconnected_cycle(w)  // boundary vetex of an unconnected cycle chain
        : m.is_unconnected_cycle()
        ? disconnected_cycle(s)  // internal vetex of an unconnected cycle chain
        : linear(s);      // internal vetex of a linear or connected cycle chain
}


// -- protected functions ------------------------------------------------------


// Divide an unconnected cycle chain at its boundary vertex.
template<typename G,
         typename PP>
auto To<Deg1, Deg1, G, PP>::
disconnected_cycle(const ChIdG w) -> Res
{
    auto& m = gr.chain(w);

    if constexpr (verboseF)
        // Print out summary before the split
        report.before(EndSlot{w, End::undefined}, "_cyc");

    ASSERT(m.is_unconnected_cycle(),
           "vs11_cyc used with non-cycle chain ", w);

    ASSERT(m.length() >= Chain::minCycleLength,
           "vs11_cyc: length of chain ", w, " is below minCycleLength");

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(w);

    const auto ind1 = m.g.front().ind;
    const auto ind2 = m.g.back().ind;

    for (const auto e : End::Ids)
        m.ngs[e].clear();

    auto& cmp = gr.compt(m.c);

    auto& egB = m.end_edge(End::B);

    auto vA = m.vertex_at_end(End::A);
    egB.disconnect_end_outer(End::B);
    auto vB = &cmp.vertices().create_vertex(m.c);
    egB.connect_to_outer(End::B, *vB);

    cmp.chis.insert(Deg1, w);
    cmp.chis.clear(Deg2);

    gr.update_edge_descriptors();
    if constexpr (Graph::useAgl) {
        gr.update_adjacency_edges(ind1);
        gr.update_adjacency_edges(ind2);
    }
    gr.update_vertex_descriptors();

    Res res {vA, vB};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    if constexpr (verboseF)
        // Print out summary after the split:
        report.after(res, w, {});

    return res;
}


// Divide an unconnected cycle chain at its internal vertex.
template<typename G,
         typename PP>
auto To<Deg1, Deg1, G, PP>::
disconnected_cycle(const BulkSlot& s) -> Res
{
    // 'w' is a global chain index
    // 'a' is the vertex position inside the chain 'w'
    // 'a' is counted from 1 and is the last edge to remain in the original chain
    const auto [w, a] = s.we();

    auto& m = gr.chain(w);

    if constexpr (verboseF)
        // Print out summary before the split
        report.before(s, "_dic");

    ASSERT(m.length() > 1, "vs11_chint: chain ", w, " has less than 2 edges");
    ASSERT(a > 0 && a < m.length(),
           "vs11_dic: 'a' ", a, " at the chain end: use cycle() instead");

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(s);

    const auto ind2 = m.g[a].ind;

    gr.rotate_chain(m, a());

    auto res = disconnected_cycle(w);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    if constexpr (verboseF)
        // Print out summary after the split
        report.after(res, w, {ind2}, true);

    return res;
}


// Divide at a vertex of degree 2 in chains othre than unconnected cycles.
// 'w' is a global chain index
// 'a' is the vertex position inside the chain 'w'
// 'a' is counted from 1 and is the last edge to remain in the original chain
template<typename G,
         typename PP>
auto To<Deg1, Deg1, G, PP>::
linear(const BulkSlot& s) -> Res
{
    const auto [w, a] = s.we();

    auto& mm = gr.chain(w);

    if constexpr (verboseF)
        // Print out summary before the split
        report.before(s, "_lin");

    ASSERT(mm.length() > 1, "vs11_lin: chain ", w, " has less than 2 edges");
    ASSERT(a > 0 && a < mm.length(),
           "vs11_lin: 'a' ", a, " at the chain end: use cycle() instead");

    const auto c = mm.c;

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(s);

    auto& eg1 = mm.g[a-1];
    auto& eg2 = mm.g[a];

    auto vB = eg1.vertex_at_outer(End::B);
    eg2.disconnect_end_outer(End::A);
    auto vA = &gr.compt(c).vertices().create_vertex(c);
    eg2.connect_to_outer(End::A, *vA);

    auto& n = gr.compt(c).create_chain();
    // reset the chain ref after possible reallocation by the container holding
    // the chains
    auto& m = gr.chain(w);

    constexpr auto e = End::B;
    const EndSlot we {w, e};
    const EndSlot ne {n.idw, e};
    const auto isConnected = m.is_connected_at(e);

    gr.copy_neigs(we, ne);
    m.ngs[e].clear();

    std::move(m.g.begin() + static_cast<long>(a()),
              m.g.end(),
              std::back_inserter(n.g));
    m.g.erase(m.g.begin() + static_cast<long>(a()),
              m.g.end());
    n.set_g_w();

    if (isConnected) {

        typename Graph::PathsOverEndSlots pp {gr.compt(c)};
        bool isSeparable = not pp.are_connected(we, ne.opp());
        if (isSeparable)
            gr.split_component(gr.compt(c), pp, we);
        else {
            gr.compt(c).set_egl();
            gr.compt(c).set_chis();
        }
    }
    else
        gr.split_component(gr.compt(c), n);

    gr.update_edge_descriptors();
    gr.update_vertex_descriptors();

    Res res {vA, vB};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    if constexpr (verboseF)
        // Print out summary after the split
        report.after(res, m.idw, {n.g[0].ind}, false);

    return res;
}


// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct To1<Deg1, G, PP>
    : public To<Deg1, Deg1, G, PP>
{
    using Base = To<Deg1, Deg1, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using Chains = Base::Chains;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using BulkSlot = Base::BulkSlot;
    using Vertex = Base::Vertex;
    using EdgeInVertex = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Base::ResT;
    using Res = Base::Res;
    using Base::operator();
    using Base::disconnected_cycle;
    using Base::linear;

    static constexpr auto J1 = Base::J1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Base::J2;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = Base::I1;  ///< Degree of the input vertex.
    static constexpr auto I2 = Base::I2;  ///< No 2nd input vertex.
    static constexpr auto I = Base::I;    ///< Input vertex degree.

    static_assert(PropertyProcessor::TransformType == Trait::Type);
    static_assert(PropertyProcessor::Element::Type == G::Type);

    using Base::dd;
    using Base::shortName;
    using Base::fullName;

    explicit constexpr To1(Graph& gr,
                           std::shared_ptr<PP> pp = nullptr)
        : Base {gr, pp}
    {
        ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};

}  // namespace graffine::transforms::vertex_split

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_11_H
