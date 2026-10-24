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
 * \file from_13.h
 * \brief Contains implementation of the for 1V + 3V -> 4V merger type.
 * \details Class encapsulating merger of a tip vertex to a 3-way junction.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_13_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_13_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <array>
#include <memory>
#include <vector>
#include <type_traits>

namespace graffine::transforms::vertex_merger {

/**
 * Template specialization for V1 + V3 -> V4 vertex merger.
 * \details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * \tparam G Graph to which the merger operation is applied.
 */
template<typename G,
         typename PP>
struct From<Deg1, Deg3, G, PP>
    : public Applicable<From<Deg1, Deg3, G, PP>>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;
    using Vertex = Graph::Vertex;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg3;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = I1 + I2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = undefined<Degree>;  ///< No 2nd output vertex.

    const std::string dd {degrees_to_string(I1, I2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit From(Graph& gr,
                  std::shared_ptr<PP> pp = nullptr);

    /**
     * Merges a vertex of degree 1 to a vertex of degree 3.
     * \details Unchecked: see Applicable. The vertices may be given in any
     * order.
     * \param[in] v1 Degree 1 input vertex.
     * \param[in] v2 Degree 3 input vertex.
     */
    auto apply(const Vertex& v1,
               const Vertex& v2) -> Res;

    /**
     * Merges a vertex of degree 1 to a vertex of degree 3.
     * \details Unchecked: see Applicable.
     * \param[in] s1 Slot on degree 1 input vertex.
     * \param[in] s2 Slot on degree 3 input vertex.
     */
    auto apply(const EndSlot& s1,
               const EndSlot& s2) -> Res;

    /// Checks that vertices \p v1 and \p v2 can be merged.
    auto applicability(const Vertex& v1,
                       const Vertex& v2) const -> Applicability;

    /// Checks that the vertices at \p s1 and \p s2 can be merged.
    auto applicability(const EndSlot& s1,
                       const EndSlot& s2) const -> Applicability;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
applicability(
    const Vertex& v1,
    const Vertex& v2
) const -> Applicability
{
    if (!checks::is_in_graph(gr, v1) || !checks::is_in_graph(gr, v2))
        return reject("vertex not in the graph");

    const auto& leaf = v1.deg() == I1 ? v1 : v2;
    const auto& junction = v1.deg() == I1 ? v2 : v1;
    if (leaf.deg() != I1 || junction.deg() != I2)
        return reject("vertex degrees are not 1 and 3");

    return applicability(locate::end_slot(gr, leaf),
                         locate::end_slot(gr, junction));
}


template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
applicability(
    const EndSlot& s1,
    const EndSlot& s2
) const -> Applicability
{
    if (!checks::is_valid(gr, s1) || !checks::is_valid(gr, s2))
        return reject("invalid end slot");

    const auto& m1 = gr.chain(s1.w);
    if (!m1.length())
        return reject("chain has no edges");
    if (m1.is_connected_at(s1.e))
        return reject("chain end is not free");
    if (gr.chain(s2.w).num_neigs(s2.e) != I2 - 1)
        return reject("chain end is not at a 3-way junction");

    // If the chain of s1 is at the junction, the merger closes it in a loop.
    const auto other = EndSlot {s1.w, End::opp(s1.e)};
    if ((other == s2 || gr.ngs_at(s2).contains(other)) &&
        m1.length() < Chain::minCycleLength)
        return reject("loop would be shorter than the minimal cycle length");

    return {};
}

template<typename G,
         typename PP>
From<Deg1, Deg3, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp
)
    : gr {gr}
    , updateProperties {pp}
    , report {gr, dd}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
apply(
    const Vertex& v1,
    const Vertex& v2
) -> Res
{
    const auto& leaf = v1.deg() == I1 ? v1 : v2;
    const auto& junction = v1.deg() == I1 ? v2 : v1;

    return apply(locate::end_slot(gr, leaf),
                 locate::end_slot(gr, junction));
}

template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
apply(
    const EndSlot& s1,
    const EndSlot& s2
) -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, e2] = s2.we();

    auto& cn = gr.chains();

    GRAFFINE_ASSERT(!cn[w1].is_connected_at(e1),
           "Slot ", s1.str_short(), " is connected." );
    GRAFFINE_ASSERT(cn[w2].num_neigs(e2) == I2 - 1,
           "Slot ", s2.str_short(), " is not a 3-way junction.");

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    const auto ngs = gr.ngs_at(s2).as_vector();  // chains connected to w2 at e2

    // Print out summary before the operation:

    if constexpr (verboseF)
        report.before(s1, s2);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2, s1);  // (preserved, removed)

    auto& eg1 = cn[w1].end_edge(e1);
    gr.compt(c1).remove_vertex_at_edge_outer(eg1, e1);

    // Update connection to s1:

    gr.ngs_at(s1) = Neigs{s2, ngs[0], ngs[1]};

    // Update connection to s2 and its connected chains:

    gr.ngs_at(s2).insert(s1);
    gr.ngs_at(ngs[0]).insert(s1);
    gr.ngs_at(ngs[1]).insert(s1);

    // Update internal records:

    if (c1 == c2)
        gr.compt(c1).set_chis();
    else {
        auto& cmp = gr.merge_components(c1, c2);
        cmp.set_chains();
    }

    auto v2 = cn[w2].vertex_at_end(e2);
    eg1.connect_to_outer(e1, *v2);

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation:

    if constexpr (verboseF)
        report.after(v2, w1, {w2, ngs[0].w, ngs[1].w});

    return {v2};
}

// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct From1<Deg3, G, PP>
    : public From<Deg1, Deg3, G, PP>
{
    using Base = From<Deg1, Deg3, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using BulkSlot = Base::BulkSlot;
    using Vertex = Base::Vertex;
    using PropertyProcessor = Base::PropertyProcessor;
    using ResT = Base::ResT;
    using Res = Base::Res;
    using Base::operator();

    static constexpr auto I1 = Base::I1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Base::I2;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Base::J1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Base::J2;  ///< No 2nd output vertex.

    using Base::dd;
    using Base::shortName;
    using Base::fullName;

    constexpr From1(Graph& gr,
                    std::shared_ptr<PP> pp = nullptr)
        : Base {gr, pp}
    {
        GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};


}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_13_H
