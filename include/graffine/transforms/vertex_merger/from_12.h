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
 * \file from_12.h
 * \brief Contains class encapsulating tip-to-side vertex merger.
 * \note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_12_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_12_H

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
 * Template specialization for V1 + V2 -> V3 vertex merger.
 * \details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * \tparam G Graph to which the merger operation is applied.
 */
template<typename G,
         typename PP>
struct From<Deg1, Deg2, G, PP>
    : public Applicable<From<Deg1, Deg2, G, PP>>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;
    using Vertex = Graph::Vertex;
    using EgDescr = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg2;  ///< Degree of the 2nd input vertex.
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
     * Merges a vertex of degree 1 to a chain internal vertex.
     * \details Unchecked: see Applicable. The vertices may be given in any
     * order. The degree 2 vertex may also be the boundary vertex of an
     * unconnected cycle.
     * \param[in] v1 The 1st parent vertex.
     * \param[in] v2 The 2nd parent vertex.
     */
    auto apply(Vertex& v1,
               Vertex& v2) -> Res;

    /**
     * Merges a vertex of degree 1 to a chain internal vertex.
     * \details Unchecked: see Applicable.
     * \param[in] s1 Slot on the parent degree 1 vertex.
     * \param[in] s2 Slot on the parent degree 2 vertex.
     */
    auto apply(const EndSlot& s1,
               const BulkSlot& s2) -> Res;

    /**
     * Merges a vertex of degree 1 to a boundary vertex in an unconnected cycle.
     * \details Unchecked: see Applicable.
     * \param[in] s1 Slot on the merged degree 1 vertex.
     * \param[in] w2 Chain index of the cycle chain.
     */
    auto apply(const EndSlot& s1,
               ChIdG w2) -> Res;

    /// Checks that vertices \p v1 and \p v2 can be merged.
    /// \details Rejects vertices not belonging to the graph, or whose
    /// degrees are not 1 and 2, in either order, then checks their slots as
    /// the slot overloads do.
    auto applicability(const Vertex& v1,
                       const Vertex& v2) const -> Applicability;

    /// Checks that the vertices at \p s1 and \p s2 can be merged.
    /// \details Rejects \p s1 failing check_free_end(), an invalid \p s2 ,
    /// and, if both are in the same chain, a merger closing a loop shorter
    /// than Chain::minCycleLength.
    auto applicability(const EndSlot& s1,
                       const BulkSlot& s2) const -> Applicability;

    /// Checks that the vertex at \p s1 can be merged to the boundary vertex
    /// of the unconnected cycle \p w2.
    /// \details Rejects \p s1 failing check_free_end(), and a chain \p w2
    /// which does not exist or is not an unconnected cycle.
    auto applicability(const EndSlot& s1,
                       ChIdG w2) const -> Applicability;

private:

    /// Checks of the free end \p s1.
    /// \details Rejects an invalid slot, a chain having no edges, and a
    /// chain end which is not free.
    auto check_free_end(const EndSlot& s1) const -> Applicability;

    Graph& gr;  ///< Reference to the modified graph object.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
From<Deg1, Deg2, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , updateProperties {pp}
    , report {gr, dd}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
check_free_end(const EndSlot& s1) const -> Applicability
{
    if (!checks::is_valid(gr, s1))
        return reject("invalid end slot");
    if (!gr.chain(s1.w).length())
        return reject("chain has no edges");
    if (gr.chain(s1.w).is_connected_at(s1.e))
        return reject("chain end is not free");

    return {};
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
applicability(
    const Vertex& v1,
    const Vertex& v2
) const -> Applicability
{
    if (!checks::is_in_graph(gr, v1) || !checks::is_in_graph(gr, v2))
        return reject("vertex not in the graph");

    const auto& leaf = v1.is_leaf() ? v1 : v2;
    const auto& bulk = v1.is_leaf() ? v2 : v1;
    if (!leaf.is_leaf() || !bulk.is_bulk())
        return reject("vertex degrees are not 1 and 2");

    const auto s1 = locate::end_slot(gr, leaf);
    const auto s2 = locate::bulk_slot(gr, bulk);
    if (checks::is_valid(gr, s2))
        return applicability(s1, s2);

    return applicability(s1, s2.w);  // boundary vertex of a cycle
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
applicability(
    const EndSlot& s1,
    const BulkSlot& s2
) const -> Applicability
{
    if (const auto ok = check_free_end(s1); !ok)
        return ok;
    if (!checks::is_valid(gr, s2))
        return reject("invalid bulk slot");

    // Merging an end to the bulk of its own chain closes a loop between them.
    if (s1.w == s2.w) {
        const auto [w, a] = s2.we();
        const auto loop = s1.e == End::A ? a() : gr.chain(w).length() - a();
        if (loop < Chain::minCycleLength)
            return reject("loop would be shorter than the minimal cycle length");
    }

    return {};
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
applicability(
    const EndSlot& s1,
    const ChIdG w2
) const -> Applicability
{
    if (const auto ok = check_free_end(s1); !ok)
        return ok;
    if (!checks::is_valid(gr, w2))
        return reject("invalid chain");
    if (!gr.chain(w2).is_unconnected_cycle())
        return reject("chain is not an unconnected cycle");

    return {};
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
apply(
    Vertex& v1,
    Vertex& v2
) -> Res
{
    const auto& leaf = v1.is_leaf() ? v1 : v2;
    const auto& bulk = v1.is_leaf() ? v2 : v1;

    const auto s1 = locate::end_slot(gr, leaf);
    const auto s2 = locate::bulk_slot(gr, bulk);
    if (checks::is_valid(gr, s2))
        return apply(s1, s2);

    return apply(s1, s2.w);  // boundary vertex of a cycle
}


// Merges a vertex of degree 1 to a chain internal vertex.
template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
apply(
    const EndSlot& s1,
    const BulkSlot& s2
) -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, a2] = s2.we();

    // For corresponding vertex merger of unconnected cycles, "From<1, 1, G>"
    // should be used. Therefore,
    GRAFFINE_ASSERT(!cn[w1].ngs[e1].num(),
           shortName, ": end ", s1.ea_str(), " is not a free end of ", w1);
    GRAFFINE_ASSERT(a2 > 0, shortName, " at the very beginning of w2 ", w2);
    GRAFFINE_ASSERT(a2 < cn[w2].length(), shortName, " at the very end of w2 ", w2);

    if (cn[w2].is_unconnected_cycle()) {
        gr.rotate_chain(cn[w2], a2());
        return apply(s1, w2);
    }

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(s1, s2);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2, s1);  // (preserved, removed)

    // cn[mi] is produced by the split:

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    auto& m = gr.compt(c2).create_chain();
    auto& mi = m.idw;

    // Connect all the ends involved:

    EndSlot w1A {w1, End::A};
    EndSlot w1B {w1, End::B};
    EndSlot w2A {w2, End::A};
    EndSlot w2B {w2, End::B};
    EndSlot miA {mi, End::A};
    EndSlot miB {mi, End::B};

    Vertex* v2 {};
    if (w1 == w2) {
        // This is not a cycle chain because the cycle requires neighbors
        // at both ends. In contrast, 'w1' is allowed to have neighbours at one
        // end only
        const auto w = w1;
        const auto wA = w1A;
        const auto wB = w1B;

        if (e1 == End::A) {

            if constexpr (verboseF)
                report.branch(1, "w1 == w2 and e1 == A");

            auto& eg1 = cn[w].end_edge(e1);
            gr.compt(c1).remove_vertex_at_edge_outer(eg1, e1);

            // position 'mi' at the B-end of "w2"

            gr.move_edges(cn[w], a2(), cn[w].length(), m, 0);

            gr.ngs_at(miA) = Neigs{wA, wB};  // set neigs at miA
            gr.copy_neigs(wB, miB);          // set neigs at miB
            cn[w].ngs = {Neigs{wB, miA},     // set neigs at wA
                         Neigs{wA, miA}};    // set neigs at wB

            v2 = m.vertex_at_end(End::A);
            eg1.connect_to_outer(e1, *v2);
        }
        else {  // i.e. (e1 == End::B)

            if constexpr (verboseF)
                report.branch(2, "w1 == w2 and e1 == B");

            auto& eg1 = cn[w].end_edge(e1);
            gr.compt(c1).remove_vertex_at_edge_outer(eg1, e1);

            // position 'mi' at the A-end of "w2"

            gr.move_edges(cn[w], 0, a2(), m, 0);

            gr.copy_neigs(wA, miA);          // set neigs at miA
            gr.ngs_at(miB) = Neigs{wA, wB};  // set neigs at miB
            cn[w].ngs = {Neigs{miB, wB},     // set neigs at wA
                         Neigs{miB, wA}};    // set neigs at wB

            v2 = m.vertex_at_end(End::B);
            cn[w].connect_end_vertex(e1, *v2);  // ref changed after mov_edges()
        }
        gr.compt(c1).set_chis();
    }
    else {

        if constexpr (verboseF)
            report.branch(3, "w1 != w2");

        auto& eg1 = cn[w1].end_edge(e1);
        gr.compt(eg1.c).remove_vertex_at_edge_outer(eg1, e1);

        // position 'mi' at the B-end of "w2"

        gr.move_edges(cn[w2], a2(), cn[w2].length(), m, 0);

        gr.ngs_at(miA) = Neigs{s1, w2B};   // set neigs at miA
        gr.copy_neigs(w2B, miB);           // set neigs at miB
        gr.ngs_at(s1)  = Neigs{w2B, miA};  // set neigs at s
        gr.ngs_at(w2B) = Neigs{s1, miA};   // set neigs at w2B

        if (c1 != c2) {
            gr.compt(c2).set_chis();
            auto& cmp = gr.merge_components(c1, c2);
            cmp.set_chains();
            cmp.set_egl();  // set_chains() renumbers the edges
        }
        else
            gr.compt(c1).set_chis();

        v2 = cn.back().vertex_at_end(End::A);
        eg1.connect_to_outer(e1, *v2);
    }

    // Update internal records.

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(v2, w1, {w2}, mi);

    return {v2};
}


// Merges a vertex of deg.1 into a boundary vertex of an unconnected cycle 'w2'
template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
apply(
    const EndSlot& s1,
    const ChIdG w2
) -> Res
{
    const auto [w1, e1] = s1.we();

    // Indexes of the participating components:

    const auto c1 = gr.chain(w1).c;
    const auto c2 = gr.chain(w2).c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(s1, EndSlot{w2, End::undefined}, {}, {}, "a CYCLE ");

    GRAFFINE_ASSERT(!gr.chain(w1).ngs[e1].num(), shortName,
           ": end ", s1.ea_str(), " of chain ", s1.w, " is not a free end");
    GRAFFINE_ASSERT(gr.chain(w2).is_unconnected_cycle(),
           shortName, "w2 ", w2, " is not an unconnected cycle");

    const EndSlot s2A {w2, End::A};
    const EndSlot s2B {w2, End::B};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(s2A, s1);  // (preserved, removed)

    auto& eg1 = cn[w1].end_edge(e1);
    gr.compt(c1).remove_vertex_at_edge_outer(eg1, e1);

    // Connect slots:

    gr.ngs_at(s1)  = Neigs{s2A, s2B};
    gr.ngs_at(s2A) = Neigs{s2B, s1};
    gr.ngs_at(s2B) = Neigs{s2A, s1};

    gr.merge_components(c1, c2);

    // Update internal records.
    // Vertex data is not yet ready, so withVertices is false.
    gr.template update<false>();

    auto v2 = cn[w2].vertex_at_end(UnconnectedCycleEnd);
    eg1.connect_to_outer(e1, *v2);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    gr.update();  // final update

    // Print out summary after the operation.

   if constexpr (verboseF)
        report.after(v2, w1, {w2});

    return {v2};
}

// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct From1<Deg2, G, PP>
    : public From<Deg1, Deg2, G, PP>
{
    using Base = From<Deg1, Deg2, G, PP>;
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

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_12_H
