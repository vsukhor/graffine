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
 * \file to_13.h
 * \brief Contains class template enabling splits of specific graph vertices.
 * \details This Functor template specialization operates on vertices of
 * degree 4 to produce vertices of degrees 1 and 3.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_13_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_13_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/vertex_split/reporter.h"

#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::vertex_split {

/**
 * Template specialization for a functor generating degree 1 and 3 vertices.
 * \details Adds vertex type-specific split capability and updates the
 * graph for it. This Functor template specialization operates on degree 4
 * vertices to produce vertices of degree 1 and 3: V4 -> V1 + V3.
 * \tparam G Graph to which the split operation is applied.
 */
template<typename G,
         typename PP>
struct To<Deg1, Deg3, G, PP>
{
    using Trait = vertex_split::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Edge = Graph::Edge;
    using Vertex = Graph::Vertex;
    using EdgeInVertex = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Deg3;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = J1 + J2;  ///< Degree of the input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

    static_assert(areCompatibleDegrees<J1, J2>);

    const std::string dd {degrees_to_string(J1, J2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a Functor for vertex split operation acting on a Graph.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit To(Graph& gr,
                std::shared_ptr<PP> pp = nullptr);

    /**
     * Divides the graph chain at a vertex of degree 2.
     * \param[in] v Degree 3 vertex.
     * \param[in] s Descriptor of the incident edge to get disconnected.
     */
    auto operator()(Vertex& v,
                    const EdgeInVertex& s) -> Res;

    /**
     * Divides the chain at a vertex of degree 4 to produce 1+3.
     * \param[in] s Slot on a oundary chain vertex.
     */
    auto operator()(const EndSlot& s) -> Res;

private:

    Graph&  gr;  ///< Reference to the modified graph object.
    Chains& cn;  ///< Reference to the graph edge chains.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
To<Deg1, Deg3, G, PP>::
To(Graph& gr,
   std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , updateProperties {pp}
    , report {dd, gr}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto To<Deg1, Deg3, G, PP>::
operator()(Vertex& v,
           const EdgeInVertex& s) -> Res
{
    ASSERT_CALLING(v.deg() == I,
                   v.print("INCORRECT DEGREE"),
                   "Vertex must have degree", I, " actual degree is", v.deg());

    ASSERT_CALLING(v.is_conected_to(s),
                   v.print("NOT CONNECTED"),
                   "Vertex is not connected to ", s.str_short());

    const auto& cmp = gr.ct[v.get_c()];

    return (*this)(cmp.end_slot(s));
}


// cuts a 4-junction
template<typename G,
         typename PP>
auto To<Deg1, Deg3, G, PP>::
operator()(const EndSlot& s) -> Res
{
    const auto [w, e] = s.we();

    ASSERT(gr.ngs_at(s).num() == I - 1,
           shortName, ": number of connections ", gr.ngs_at(s).num(),
           " at slot ", s.w, " ", s.ea_str(), " is not as expected ", I - 1);

    if constexpr (verboseF)
        report.before(s);

    const auto c = cn[w].c;

    const auto ngs = gr.ngs_at(s).as_vector();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(s);

    std::array<Edge*, I> eg;
    eg[0] = &cn[w].end_edge(e);
    for (std::size_t i=1; i<I; ++i)
        eg[i] = &cn[ngs[i-1].w].end_edge(ngs[i-1].e);

    std::array<EgIdG, I> ind;
    ind[0] = gr.slot2ind(s);
    for (std::size_t i=1; i<I; ++i)
        ind[i] = gr.slot2ind(ngs[i-1]);

    auto& vv = gr.compt(c).vertices();
    auto v01 = eg[0]->vertex_at_outer(e);  // original vertex to be splitted
    eg[0]->disconnect_end_outer(e);
    auto vS = &vv.create_vertex(c);  // new free end of 'w'
    eg[0]->connect_to_outer(e, *vS);

    Res res {vS, v01};  // pair of vertices produced by the split

    gr.remove_slot_from_neigs(s);
    const auto isConnected = cn[w].is_connected_at(End::opp(e));
    if (isConnected) {

        typename Graph::PathsOverEndSlots pp {gr.compt(c)};
        bool isSeparable = not pp.are_connected(s, ngs[0]);
        if (isSeparable)
            gr.split_component(gr.compt(c), pp, s);
        else {
            gr.compt(c).set_egl();
            gr.compt(c).set_chis();
        }
    }
    else
        gr.split_component(gr.compt(c), cn[w]);

    gr.update_edge_descriptors();
    if constexpr (Graph::useAgl)
        for (const auto ii : ind)
            gr.update_adjacency_edges(ii);
    gr.update_vertex_descriptors();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    const auto w1 = gr.chid(ind[0]);
    [[maybe_unused]] const auto w2 = gr.chid(ind[1]);

    ASSERT(cn[w1].c == c || cn[w2].c == c,
           shortName, ": cn[w1].c ", cn[w1].c, " != c && cn[w2].c ",
           cn[w2].c, " != c ", c);

    // Print out summary after:

    if constexpr (verboseF)
        report.after(res, w1, {ind[1], ind[2], ind[3]});

    return res;
}


// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct To1<Deg3, G, PP>
    : public To<Deg1, Deg3, G, PP>
{
    using Base = To<Deg1, Deg3, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using Chains = Base::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Edge = Graph::Edge;
    using Vertex = Graph::Vertex;
    using EdgeInVertex = Vertex::EgDescr;
    using PropertyProcessor = Base::PropertyProcessor;
    using ResT = Base::ResT;
    using Res = Base::Res;
    using Base::operator();

    static constexpr auto J1 = Base::J1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Base::J2;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = Base::I1;  ///< Degree of the input vertex.
    static constexpr auto I2 = Base::I2;  ///< No 2nd input vertex.
    static constexpr auto I = Base::I;    ///< Input vertex degree.

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

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_13_H
