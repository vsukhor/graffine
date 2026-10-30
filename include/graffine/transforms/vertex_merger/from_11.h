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
 * \file from_11.h
 * \brief Contains class performing merger between vertices of degree 1.
 * \note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <array>
#include <memory>
#include <vector>
#include <type_traits>

namespace graffine::transforms::vertex_merger {


/**
 * Merges two vertices of degree 1 to produce a vertex of degree 2.
 * \details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * \tparam G Graph to which the merger operation is applied.
 */
template<typename G,
         typename PP>
struct From<Deg1, Deg1, G, PP>
    : public Applicable<From<Deg1, Deg1, G, PP>>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg1;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Deg2;  ///< Degree of the 1st output vertex.
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
     * Merges two vertices of degree 1 to produce a vertex of degree 2.
     * \details Calls its overload using chain free end slots.
     * Unchecked: see Applicable.
     * \param[in] v1 First merged vertex.
     * \param[in] v2 Second merged vertex.
     * \return The vertex resulting from the merger.
     */
    auto apply(Vertex& v1,
               Vertex& v2) -> Res;

    /**
     * Merges two vertices of degree 1 to produce a vertex of degree 2.
     * \details If \p s1 and \p s2 are the two ends of the same chain, the
     * chain is closed into a cycle.
     * \attention Important: when used for antiparallel merger (i.e. on equal
     * ends of the merged chains), one of the chains is reversed:
     * if \p s1.e is A, reverses chain \p s1.w, if \p s1.e is B, reverses
     * chain \p s2.w:
     * s1.e == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>)
     * s1.e == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
     * Unchecked: see Applicable.
     * \param[in] s1 Chain boundary slot at the 1st parent vertex.
     * \param[in] s2 Chain boundary slot on the 2nd parent vertex.
     * \return The vertex resulting from the merger.
     */
    auto apply(const EndSlot& s1,
               const EndSlot& s2) -> Res;

    /**
     * Checks that vertices \p v1 and \p v2 can be merged.
     * \details Rejects vertices not belonging to the graph, or not of degree
     * 1, then checks their slots as the slot overload does.
     */
    auto applicability(const Vertex& v1,
                       const Vertex& v2) const -> Applicability;

    /**
     * Checks that the vertices at slots \p s1 and \p s2 can be merged.
     * \details Rejects:
     * - invalid slots, or \p s1 equal to \p s2 ;
     * - slots of chains having no edges;
     * - slots at chain ends which are not free;
     * - the two ends of a chain shorter than Chain::minCycleLength, which
     *   would form a too short cycle.
     */
    auto applicability(const EndSlot& s1,
                       const EndSlot& s2) const -> Applicability;

private:

    /// Free end slot of a vertex of degree 1 belonging to the graph.
    auto slot_of(const Vertex& v) const -> EndSlot;


    Graph& gr;  ///< Reference to the modified graph object.

    Core<Graph, PP> merge;  ///< Low-level free-end connector.

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
From<Deg1, Deg1, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp
)
    : gr {gr}
    , merge {gr, pp, shortName}
    , report {gr, dd}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
slot_of(const Vertex& v) const -> EndSlot
{
    const auto& egs = *v.edges().begin();
    const auto& egl = gr.compt(v.get_c()).get_egl(egs.indc());

    return gr.chain(egl.w).end_slot(egl.a, egs.e);
}


template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
applicability(
    const Vertex& v1,
    const Vertex& v2
) const -> Applicability
{
    if (!checks::is_in_graph(gr, v1) || !checks::is_in_graph(gr, v2))
        return reject("vertex not in the graph");
    if (!v1.is_leaf() || !v2.is_leaf())
        return reject("vertex degree is not 1");

    return applicability(slot_of(v1), slot_of(v2));
}


template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
applicability(
    const EndSlot& s1,
    const EndSlot& s2
) const -> Applicability
{
    if (!checks::is_valid(gr, s1) || !checks::is_valid(gr, s2))
        return reject("invalid end slot");
    if (s1 == s2)
        return reject("merging a vertex with itself");

    const auto& m1 = gr.chain(s1.w);
    const auto& m2 = gr.chain(s2.w);
    if (!m1.length() || !m2.length())
        return reject("chain has no edges");
    if (m1.is_connected_at(s1.e) || m2.is_connected_at(s2.e))
        return reject("chain end is not free");
    if (s1.w == s2.w && m1.length() < Chain::minCycleLength)
        return reject("cycle would be shorter than the minimal cycle length");

    return {};
}


template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
apply(
    Vertex& v1,
    Vertex& v2
) -> Res
{
    return apply(slot_of(v1), slot_of(v2));
}


// Important: for an antiparallel merger, if s1.e is A, reverses chain s1.w;
// if s1.e is B, reverses chain s2.w.
template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
apply(
    const EndSlot& s1,
    const EndSlot& s2
) -> Res
{
    // Use low-level vertex merging functions directly
    // gr.update() is called in each of these:

    if (s2.w == s1.w)  return merge.to_cycle(s1.w);
    /* else */         return merge.to_linear(s1, s2);
}


// /////////////////////////////////////////////////////////////////////////////

/**
 * Vertex merger of vertices of degrees 1 and 1.
 * \details The same functor as From<Deg1, Deg1, G, PP>, addressed
 * by the degree of the second vertex only.
 */
template<typename G,
         typename PP>
struct From1<Deg1, G, PP>
    : public From<Deg1, Deg1, G, PP>
{
    using Base = From<Deg1, Deg1, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using BulkSlot = Base::BulkSlot;
    using Vertex = Base::Vertex;
    using PropertyProcessor = PP;
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

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H
