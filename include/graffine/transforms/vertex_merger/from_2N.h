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
 * \file from_2N.h
 * \brief Contains implementation of the for 2 + N merger type.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_2N_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_2N_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <array>
#include <memory>
#include <vector>
#include <type_traits>


namespace graffine::transforms::vertex_merger {


/**
 * Template specialization for V2 + V2 -> V4 vertex merger.
 * \details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * \tparam G Graph to which the merger operation is applied.
 */
template<typename G,
         typename PP>
struct FromN<Deg2, G, PP>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using Components = Graph::Components;
    using Edge = Graph::Edge;
    using End = Chain::End;
    using EndId = End::Id;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;
    using Vertex = Graph::Vertex;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto I1 = Deg2;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg2;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = I1 + I2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = undefined<Degree>;  ///< No 2nd output vertex.
    static constexpr auto ee = UnconnectedCycleEnd;
    static constexpr auto oe = structure::descriptors::End::opp(UnconnectedCycleEnd);

    const std::string dd {degrees_to_string(I1, I2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit FromN(Graph& gr,
                   std::shared_ptr<PP> pp = nullptr);

    /**
     * Merges two degree 2 vertices.
     * \param[in] v1 1st input vertex.
     * \param[in] v2 2nd input vertex.
     */
    auto operator()(const Vertex& v1,
                    const Vertex& v2) noexcept -> Res;

    /**
     * Merges a boundary vertex on a cycle chain to a junction vertex.
     * \param[in] w1 Chain index of the cycle chain.
     * \param[in] s2 Slot incident to the junction vertex.
     */
    auto operator()(ChIdG w1,
                    const EndSlot& s2) noexcept -> Res;

    /**
     * Merges a chain inner vertex to a junction vertex.
     * \param[in] b1 Slot connected to a chain inner vertex.
     * \param[in] s2 Slot connected to the junction vertex.
     */
    auto operator()(const BulkSlot& b1,
                    const EndSlot& s2) noexcept -> Res;

private:

    Graph&      gr;  ///< Reference to the modified graph object.
    Chains&     cn;  ///< Reference to the graph edge chains.
    Components& ct;  ///< Reference to the graph components.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;

    constexpr auto deg4_vertex(
        Chain& m, EndId em,
        Chain& p, EndId ep,
        Chain& q, EndId eq
    ) noexcept -> Vertex*;

    constexpr auto deg4_vertex(
        const EndSlot& s,
        const EndSlot& p,
        const EndSlot& q
    ) noexcept -> Vertex*;

    constexpr void join_ends(const EndSlot& s1,
                             const EndSlot& s2,
                             const EndSlot& s3,
                             const EndSlot& s4) noexcept;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
FromN<Deg2, G, PP>::
FromN(Graph& gr,
      std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , ct {gr.compts()}
    , updateProperties {pp}
    , report {gr}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto FromN<Deg2, G, PP>::
operator()(
    const Vertex& v1,
    const Vertex& v2
) noexcept -> Res
{
    ASSERT(v1.deg() == Deg2 ||
           v2.deg() == Deg2,
           "Incorrect degrees of input vertices");

    const auto egs1 = &v1.edges.begin();
    const auto egs2 = &v2.edges.begin();

    const auto egl1 = ct[v1.get_c()].get_egl(egs1.w);
    const auto egl2 = ct[v2.get_c()].get_egl(egs1.w);

    const auto w1 = egl1.w;
    const auto w2 = egl2.w;

    const auto isCB1 = v1.is_cycle_boundary();
    const auto isCB2 = v2.is_cycle_boundary();

    if (isCB1)  // a bulk vertex to a boundary of an unconnected cycle w1

        return (*this)(BulkSlot{w2, egl2.a},
                       EndSlot{w1, gr.edge(egl1.i).outer_endId(egs1.e)});

    else if (isCB2)  // a bulk vertex to a boundary of an unconnected cycle w2

        return (*this)(BulkSlot{w1, egl1.a},
                       EndSlot{w2, gr.edge(egl2.i).outer_endId(egs2.e)});

    else if (v1.deg() == Deg2)  // two chain internal vertices

        return (*this)(BulkSlot{w1, egl1.a},
                       BulkSlot{w2, egl2.a});

    else if (v2.deg() == Deg2)  // two chain internal vertices

        return (*this)(BulkSlot{w1, egl1.a},
                       BulkSlot{w2, egl2.a});
}


// Merges a bulk vertex to a boundary vertex of an unconnected cycle.
template<typename G,
         typename PP>
auto FromN<Deg2, G, PP>::
operator()(
    const BulkSlot& b1,
    const EndSlot& s2
) noexcept -> Res
{

}

// Merges a bulk vertex to a boundary vertex of an unconnected cycle.
template<typename G,
         typename PP>
auto FromN<Deg2, G, PP>::
operator()(
    const ChIdG w1,
    const EndSlot& s2
) noexcept -> Res
{
    ASSERT(cn[w1].is_unconnected_cycle(),
           shortName, ": w1 ", w1, " is not an unconnected cycle");

    const auto [w2, e2] = s2.we();

    // Print out summary before the operation.

    if constexpr (verboseF)
//        report.before(b1, EndSlot{w2, End::undefined},
//                      {}, {}, "an unconnected CYCLE ");

    // Define component, chain and end indexes of the merging ends:

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    std::vector<EndSlot> slots1 {EndSlot {w1, End::A},
                                 EndSlot {w1, End::B}};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2, slots1[0]);  // (preserved, removed)

    auto [eg11, eg12] = ct[c1].rem_unconnected_cycle_boindary_vetrex();

    const auto ngs = gr.ngs_at(s2);  // chains connected to w2 at e2

    std::vector<EndSlot> slots2 {ngs()};
    slots2.push_back(s2);

    // Create mutual connections of slots1 to s2 and their connected chains:

    for (const auto ss1 : slots1) {
        auto& ngs1 = gr.ngs_at(ss1);
        for (const auto ss2 : slots2)
            ngs1.insert(std::move(ss2));
    }
    for (const auto ss2 : slots2) {
        auto& ngs2 = gr.ngs_at(ss2);
        for (const auto ss1 : slots1)
            ngs2.insert(std::move(ss1));
    }


    // Update internal records:

    if (c1 == c2)
        gr.compt(c1).set_chis();
    else {
        auto& cmp = gr.merge_components(c1, c2);
        cmp.set_chains();
    }

    auto v2 = cn[w2].vertex_at_end(e2);
    eg11->connect_to_outer(End::A, *v2);
    eg12->connect_to_outer(End::B, *v2);

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(v2, w1, gr.ngs_at(s2).ww());

    return {v2};
}



template<typename G,
         typename PP>
constexpr
void FromN<Deg2, G, PP>::
join_ends(const EndSlot& s1,
          const EndSlot& s2,
          const EndSlot& s3,
          const EndSlot& s4) noexcept
{
    gr.ngs_at(s1) = Neigs{s2, s3, s4};
    gr.ngs_at(s2) = Neigs{s1, s3, s4};
    gr.ngs_at(s3) = Neigs{s2, s1, s4};
    gr.ngs_at(s4) = Neigs{s2, s3, s1};

}

template<typename G,
         typename PP>
constexpr
auto FromN<Deg2, G, PP>::
deg4_vertex(
    Chain& m, const EndId em,
    Chain& p, const EndId ep,
    Chain& q, const EndId eq
) noexcept -> Vertex*
{
    auto v = m.vertex_at_end(em);  // will become junction

    p.connect_end_vertex(ep, *v);
    q.connect_end_vertex(eq, *v);

    return v;
}

template<typename G,
         typename PP>
constexpr
auto FromN<Deg2, G, PP>::
deg4_vertex(
    const EndSlot& s,
    const EndSlot& p,
    const EndSlot& q
) noexcept -> Vertex*
{
    auto v = cn[s.w].vertex_at_end(s.e);  // will become junction

    cn[p.w].connect_end_vertex(p.e, *v);
    cn[q.w].connect_end_vertex(q.e, *v);

    return v;
}

}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_2N_H
