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
     * \param[in] v1 Degree 1 input vertex.
     * \param[in] v2 Degree 3 input vertex.
     */
    auto operator()(const Vertex& v1,
                    const Vertex& v2) noexcept -> Res;

    /**
     * Merges a vertex of degree 1 to a vertex of degree 3.
     * \param[in] s1 Slot on degree 1 input vertex.
     * \param[in] s2 Slot on degree 3 input vertex.
     */
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) noexcept -> Res;

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
From<Deg1, Deg3, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp
)
    : gr {gr}
    , updateProperties {pp}
    , report {gr, dd}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
operator()(
    const Vertex& v1,
    const Vertex& v2
) noexcept -> Res
{
    ASSERT((v1.deg() == I1 && v2.deg() == I2) ||
           (v1.deg() == I2 && v2.deg() == I1),
           "Incorrect degrees of input vertices");

    auto slot = [&](const Vertex& v) -> EndSlot
    {
        const auto& egs = *v.edges().begin();
        const auto& egd = gr.ct[v.get_c()].get_egl(egs.w);
        const auto w = egd.w;
        const auto& eg = gr.cn[w].g[egd.a];
        const auto s = EndSlot{w, eg.outer_endId(egs.e)};

        ASSERT_CALLING(gr.cn[w].template vertex_at_slot<EndSlot>(s)->operator==(v),
                       v.print("ERROR"),
                       "Vertex is not at chain end of", w);

        return s;
    };

    const auto s1 = slot(v1);
    const auto s2 = slot(v2);

    return (v1.deg() == Deg1)   ? (*this)(s1, s2)
         /*(v1.deg() == Deg3)*/ : (*this)(s2, s1);
}

template<typename G,
         typename PP>
auto From<Deg1, Deg3, G, PP>::
operator()(
    const EndSlot& s1,
    const EndSlot& s2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, e2] = s2.we();

    auto& cn = gr.chains();

    ASSERT(!cn[w1].is_connected_at(e1),
           "Slot ", s1.str_short(), " is connected." );
    ASSERT(cn[w2].num_neigs(e2) == I2 - 1,
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
        ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};


}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_13_H
