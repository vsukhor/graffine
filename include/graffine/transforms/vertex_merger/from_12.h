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
     * \param[in] v1 The 1st parent vertex.
     * \param[in] v2 The 2nd parent vertex.
     */
    auto operator()(Vertex& v1,
                    Vertex& v2) noexcept -> Res;

    /**
     * Merges a vertex of degree 1 to a chain internal vertex.
     * \param[in] s1 Slot on the parent degree 1 vertex.
     * \param[in] s2 Slot on the parent degree 2 vertex.
     */
    auto operator()(const EndSlot& s1,
                    const BulkSlot& s2) noexcept -> Res;

    /**
     * Merges a vertex of degree 1 to a boundary vertex in an unconnected cycle.
     * \param[in] s1 Slot on the merged degree 1 vertex.
     * \param[in] w2 Chain index of the cycle chain.
     */
    auto operator()(const EndSlot& s1,
                    ChIdG w2) noexcept -> Res;

private:

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
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
operator()(
    Vertex& v1,
    Vertex& v2
) noexcept -> Res
{
    ASSERT(v1.is_leaf() || v2.is_leaf(), "No leaf vertex found in input.");

    const auto& egs1 = *v1.edges().begin();
    const auto& egs2 = *v2.edges().begin();

    const auto egl1 = gr.ct[v1.get_c()].get_egl(egs1.w);
    const auto egl2 = gr.ct[v2.get_c()].get_egl(egs1.w);


    if (v1.is_leaf()) {

        ASSERT(v2.is_bulk(), "Vertex 2 is not BULK");

        const EndSlot es = gr.cn[egl1.w].end_slot(egl1.a, egs1.e);
        ASSERT(es.is_defined(), "Vertex 1 is not at chain end of", egl1.w);

        const BulkSlot bs {egl2.w, egl2.a};

        return (*this)(es, bs);
    }
    else {  // v2.is_leaf()

        ASSERT(v1.is_bulk(), "Vertex 1 is not BULK");

        const EndSlot es = gr.cn[egl2.w].end_slot(egl2.a, egs2.e);
        ASSERT(es.is_defined(), "Vertex 2 is not at chain end of", egl2.w);

        const BulkSlot bs {egl1.w, egl1.a};

        return (*this)(es, bs);
    }
}


// Merges a vertex of degree 1 to a chain internal vertex.
template<typename G,
         typename PP>
auto From<Deg1, Deg2, G, PP>::
operator()(
    const EndSlot& s1,
    const BulkSlot& s2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();
    const auto [w2, a2] = s2.we();

    // For corresponding vertex merger of unconnected cycles, "From<1, 1, G>"
    // should be used. Therefore,
    ASSERT(!cn[w1].ngs[e1].num(),
           shortName, ": end ", s1.ea_str(), " is not a free end of ", w1);
    ASSERT(a2 > 0, shortName, " at the very beginning of w2 ", w2);
    ASSERT(a2 < cn[w2].length(), shortName, " at the very end of w2 ", w2);

    if (cn[w2].is_unconnected_cycle()) {
        gr.rotate_chain(cn[w2], a2());
        return (*this)(s1, w2);
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
        // at both ends. In contrast, 'w1' is allowed to have a neigbours at one
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
operator()(
    const EndSlot& s1,
    const ChIdG w2
) noexcept -> Res
{
    const auto [w1, e1] = s1.we();

    // Indexes of the participating components:

    const auto c1 = gr.chain(w1).c;
    const auto c2 = gr.chain(w2).c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(s1, EndSlot{w2, End::undefined}, {}, {}, "a CYCLE ");

    ASSERT(!gr.chain(w1).ngs[e1].num(), shortName,
           ": end ", s1.ea_str(), " of chain ", s1.w, " is not a free end");
    ASSERT(gr.chain(w2).is_unconnected_cycle(),
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
        ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};


}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_12_H
