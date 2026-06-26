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
 * \file to_12.h
 * \brief Contains class template enabling splits of specific graph vertices.
 * \details This Functor template specialization operates on vertices of
 * degree 3 to produce a degree 1 vertex and a degree 2 vertex.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_12_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_12_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/neigs.h"
#include "graffine/structure/analyzers/connectivity.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/vertex_split/reporter.h"

#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::vertex_split {


/**
 * Generator of degree 1 and 2 vertices from split of a vertex deg 2.
 * \details Adds vertex type-specific split capability and updates the
 * graph for it. This Functor template specialization operates on degree 3
 * vertices to produce vertices of degree 1 and 2: V3 -> V1 + V2.
 * \tparam G Graph to which the split operation is applied.
 */
template<typename G,
         typename PP>
struct To<Deg1, Deg2, G, PP>
{
    using Trait = vertex_split::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using EdgeInVertex = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Deg2;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = J1 + J2;  ///< Degree of the input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

    static_assert(areCompatibleDegrees<J1, J2>);

    const std::string dd {degrees_to_string(J1, J2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a Functor object based on the Graph instance.
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
     * Divides the graph at a vertex of degree 3.
     * \param[in] s End slot disconnected vrom the divided vertex.
     */
    auto operator()(const EndSlot& s) -> Res;

private:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Auxiliary functor for intermediate vertex merger.
    vertex_merger::Core<Graph, PP> merge;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
To<Deg1, Deg2, G, PP>::
To(Graph& gr,
   std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
    , merge {gr, pp, "vm_core called from"s + shortName}
    , report {dd, gr}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto To<Deg1, Deg2, G, PP>::
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


template<typename G,
         typename PP>
auto To<Deg1, Deg2, G, PP>::
operator()(const EndSlot& s) -> Res
{
    if constexpr (verboseF)
        report.before(s);

    auto& cn = gr.chains();

    const auto [w, e] = s.we();
    const auto c = cn[w].c;
    const auto ngs = gr.ngs_at(s).as_vector(); // copy

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(s);

    ASSERT(ngs.size() == structure::containers::numNeigs<I>,
           shortName, ": input vertex degree ", ngs.size() + 1, " != I ", I);

    const auto& ng0 = ngs[0];
    const auto& ng1 = ngs[1];

    const auto [n0, e0] = ng0.we();
    const auto [n1, e1] = ng1.we();

    auto& egS = cn[w].end_edge(e);
    auto& eg0 = cn[n0].end_edge(e0);
    auto& eg1 = cn[n1].end_edge(e1);

    const auto indS = egS.ind;
    const auto ind0 = eg0.ind;

    auto& vv = gr.compt(c).vertices();

    auto v01 = egS.vertex_at_outer(e);  // original vertex to be splitted

    egS.disconnect_end_outer(e);
    auto vS = &vv.create_vertex(c);  // new free end of chain 'w'
    egS.connect_to_outer(e, *vS);

    gr.remove_slot_from_neigs(s);  // chain 'w' is now disconnected, but is
                                   // still part of the component

    Res res {vS, nullptr};  // pair of vertices produced by the split

    if (n0 == n1) {  // extract a cycle chain

        if constexpr (verboseF)
            report.branch(1, " creates unconnected cycle ");

        gr.split_component(gr.compt(c), cn[n0]);
        gr.update();

        res[1] = v01;
    }
    else {

        if (w == n0 || w == n1) {  // remove an end of a connected cycle chain

            if constexpr (verboseF)
                report.branch(2, " an end of a connected cycle chain");

            eg1.disconnect_end_outer(e1);  // v01 is now coonected to eg0:e0 only
           auto v1 = &vv.create_vertex(c);
            eg1.connect_to_outer(e1, *v1);  // v1 is connected to eg1:e1

            gr.remove_slot_from_neigs(s.opp());

            gr.split_component(gr.compt(c), cn[w]);

            res[1] = merge.to_linear(ng0, ng1)[0];
        }
        else if (!cn[w].is_connected()) {

            if constexpr (verboseF)
                report.branch(2, " extracts unconnected linear chain");

            eg1.disconnect_end_outer(e1);  // v01 is now coonected to eg0:e0 only
            auto v1 = &vv.create_vertex(c);
            eg1.connect_to_outer(e1, *v1);  // v1 is connected to eg1:e1

            gr.remove_slot_from_neigs(ng0);
            gr.compt(c).set_chis();

            gr.split_component(gr.compt(c), cn[w]);

            res[1] = merge.to_linear(ng0, ng1)[0];
        }
        else {

            eg1.disconnect_end_outer(e1);  // v01 is now coonected to eg0:e0 only
            auto v1 = &vv.create_vertex(c);
            eg1.connect_to_outer(e1, *v1);  // v1 is connected to eg1:e1

            typename Graph::PathsOverEndSlots pp {gr.compt(c)};
            bool isSeparable = not pp.are_connected(s, ng0);

            if constexpr (verboseF)
                isSeparable
                    ? report.branch(3, " in generic separable component")
                    : report.branch(4, " in generic unseparable component");

            gr.remove_slot_from_neigs(ng0);
            gr.compt(c).set_chis();

            if (isSeparable)
                gr.split_component(gr.compt(c), pp, s);
            else {
                gr.compt(c).set_egl();
                gr.compt(c).set_chis();
            }

            auto& cmp = gr.compt(cn[ng0.w].c);

            structure::Connectivity conty {cn};

            const auto sideIsSeparable =
                not conty.dfs(ng0, ng1, cmp.num_chains());

            if (sideIsSeparable)
                gr.split_component(cmp, conty.find_connected_chains(ng1));

            if (isSeparable)
                gr.compt_last().set_chis();

            res[1] = merge.to_linear(ng0, ng1)[0];
        }
    }

    gr.update_vertex_descriptors();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    const auto w1 = gr.chid(indS);
    [[maybe_unused]] const auto w2 = gr.chid(ind0);

    ASSERT(cn[w1].c == c ||
           cn[w2].c == c,
           shortName, ": cn[w1].c ", cn[w1].c, " != c && cn[w2].c ",
           cn[w2].c, " != c ", c);

    // Print out summary after:

    if constexpr (verboseF)
        report.after(res, w1, {ind0});

    return res;
}

// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct To1<Deg2, G, PP>
    : public To<Deg1, Deg2, G, PP>
{
    using Base = To<Deg1, Deg2, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using Chains = Base::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
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

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_12_H
