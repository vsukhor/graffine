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
 * \file from_22.h
 * \brief Contains implementation of merger between two chain internal vertices.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_22_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_22_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <algorithm>
#include <array>
#include <memory>
#include <tuple>
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
struct From<Deg2, Deg2, G, PP>
    : public Applicable<From<Deg2, Deg2, G, PP>>
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
    explicit From(Graph& gr,
                  std::shared_ptr<PP> pp = nullptr);

    /**
     * Merges two degree 2 vertices.
     * \details Unchecked: see Applicable.
     * \param[in] v1 1st input vertex.
     * \param[in] v2 2nd input vertex.
     */
    auto apply(const Vertex& v1,
               const Vertex& v2) -> Res;

    /**
     * Merges two chain internal vertices.
     * \details Unchecked: see Applicable.
     * \param[in] s1 Slot on the 1st input vertex.
     * \param[in] s2 Slot on the 2nd input vertex.
     */
    auto apply(const BulkSlot& s1,
               const BulkSlot& s2) -> Res;

    /**
     * Merges a chain internal vertex to a boundary vertex on a cycle chain.
     * \details Unchecked: see Applicable.
     * \param[in] b1 Slot incident to degree 2 vertex.
     * \param[in] w2 Chain index of the cycle chain.
     */
    auto apply(const BulkSlot& b1,
               ChIdG w2) -> Res;

    /**
     * Merges two unconnected cycles on their boundary vertices.
     * \details Unchecked: see Applicable.
     * \param[in] w1 Chain index of the 1st cycle chain.
     * \param[in] w2 Chain index of the 2nd cycle chain.
     */
    auto apply(ChIdG w1,
               ChIdG w2) -> Res;

    /// Checks that vertices \p v1 and \p v2 can be merged.
    auto applicability(const Vertex& v1,
                       const Vertex& v2) const -> Applicability;

    /// Checks that the vertices at \p b1 and \p b2 can be merged.
    auto applicability(const BulkSlot& b1,
                       const BulkSlot& b2) const -> Applicability;

    /// Checks that the vertex at \p b1 can be merged to the boundary vertex
    /// of the unconnected cycle \p w2.
    auto applicability(const BulkSlot& b1,
                       ChIdG w2) const -> Applicability;

    /// Checks that the boundary vertices of unconnected cycles \p w1 and
    /// \p w2 can be merged.
    auto applicability(ChIdG w1,
                       ChIdG w2) const -> Applicability;

private:

    /**
     * Checks the loops closed by merging two vertices of the same chain.
     * \param[in] m The chain.
     * \param[in] d Number of edges between the vertices along the chain.
     */
    static auto check_loops(const Chain& m,
                            std::size_t d) -> Applicability;

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
    ) -> Vertex*;

    constexpr void join_ends(const EndSlot& s1,
                             const EndSlot& s2,
                             const EndSlot& s3,
                             const EndSlot& s4);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
From<Deg2, Deg2, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , ct {gr.compts()}
    , updateProperties {pp}
    , report {gr, dd}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
check_loops(
    const Chain& m,
    const std::size_t d
) -> Applicability
{
    // Merging two vertices of a linear chain closes the part between them
    // in a loop. On an unconnected cycle, both parts become loops.
    const auto shortest = m.is_unconnected_cycle() ? std::min(d, m.length() - d)
                                                   : d;
    if (shortest < Chain::minCycleLength)
        return reject("loop would be shorter than the minimal cycle length");

    return {};
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
applicability(
    const Vertex& v1,
    const Vertex& v2
) const -> Applicability
{
    if (!checks::is_in_graph(gr, v1) || !checks::is_in_graph(gr, v2))
        return reject("vertex not in the graph");
    if (!v1.is_bulk() || !v2.is_bulk())
        return reject("vertex degree is not 2");
    if (&v1 == &v2)
        return reject("merging a vertex with itself");

    const auto b1 = locate::bulk_slot(gr, v1);
    const auto b2 = locate::bulk_slot(gr, v2);
    const auto isCB1 = !checks::is_valid(gr, b1);  // cycle boundary
    const auto isCB2 = !checks::is_valid(gr, b2);

    if (isCB1 && isCB2) return applicability(b1.w, b2.w);
    if (isCB1)          return applicability(b2, b1.w);
    if (isCB2)          return applicability(b1, b2.w);
    /* else */          return applicability(b1, b2);
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
applicability(
    const BulkSlot& b1,
    const BulkSlot& b2
) const -> Applicability
{
    if (!checks::is_valid(gr, b1) || !checks::is_valid(gr, b2))
        return reject("invalid bulk slot");
    if (b1 == b2)
        return reject("merging a vertex with itself");

    if (b1.w == b2.w) {
        const std::size_t a1 = std::get<1>(b1.we())();
        const std::size_t a2 = std::get<1>(b2.we())();
        return check_loops(gr.chain(b1.w), a1 > a2 ? a1 - a2 : a2 - a1);
    }

    return {};
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
applicability(
    const BulkSlot& b1,
    const ChIdG w2
) const -> Applicability
{
    if (!checks::is_valid(gr, b1))
        return reject("invalid bulk slot");
    if (!checks::is_valid(gr, w2))
        return reject("invalid chain");
    if (!gr.chain(w2).is_unconnected_cycle())
        return reject("chain is not an unconnected cycle");

    // The boundary vertex is at position 0 of the cycle.
    if (b1.w == w2)
        return check_loops(gr.chain(w2), std::get<1>(b1.we())());

    return {};
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
applicability(
    const ChIdG w1,
    const ChIdG w2
) const -> Applicability
{
    if (!checks::is_valid(gr, w1) || !checks::is_valid(gr, w2))
        return reject("invalid chain");
    if (w1 == w2)
        return reject("merging a vertex with itself");
    if (!gr.chain(w1).is_unconnected_cycle() ||
        !gr.chain(w2).is_unconnected_cycle())
        return reject("chain is not an unconnected cycle");

    return {};
}


template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
apply(
    const Vertex& v1,
    const Vertex& v2
) -> Res
{
    const auto b1 = locate::bulk_slot(gr, v1);
    const auto b2 = locate::bulk_slot(gr, v2);
    const auto isCB1 = !checks::is_valid(gr, b1);  // cycle boundary
    const auto isCB2 = !checks::is_valid(gr, b2);

    if (isCB1 && isCB2) return apply(b1.w, b2.w);
    if (isCB1)          return apply(b2, b1.w);
    if (isCB2)          return apply(b1, b2.w);
    /* else */          return apply(b1, b2);
}


// Merges two chain internal vertices.
template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
apply(
    const BulkSlot& b1,
    const BulkSlot& b2
) -> Res
{
    const auto [w1, a1] = b1.we();
    const auto [w2, a2] = b2.we();

    const EndSlot w1A {w1, End::A};
    const EndSlot w1B {w1, End::B};
    const EndSlot w2A {w2, End::A};
    const EndSlot w2B {w2, End::B};

    const auto isCycle1 = cn[w1].is_unconnected_cycle();
    const auto isCycle2 = cn[w2].is_unconnected_cycle();

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(b1, b2);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(b1, b2);  // not always (preserved, removed)

    // Define slots to get merged.

    ResT res0 {};
    EndSlot s1 {}, s2 {}, s3 {}, s4 {};

    if (w1 == w2) {

        EgIdA aL, aS;
        if (a1 > a2) {
            aL = a1;
            aS = a2;
        }
        else {
            aL = a2;
            aS = a1;
        }
        const auto w = w1;
        const auto isCycle = isCycle1;

        const EndSlot wA {w, End::A};
        const EndSlot wB {w, End::B};

        if (!isCycle) {

            if constexpr (verboseF)
                report.branch(1,
                    "w1 == w2 : NOT unconnected cycle: \n", report.indent,
                    "two internal vertices of the same linear chain");

            ct[c1].rem_chain_internal_vetrex(w, aL);

            // make nA..nB:mA..mB:pA..pB
            ct[c1].create_chain();            // to connect end A of w
            auto& p = ct[c1].create_chain();  // to connect end B of w
            auto& n = cn[gr.num_chains()-2];  // assigned AFTER create_chain
            auto& m = cn[w];                  // assigned AFTER create_chain

            const EndSlot nA {n.idw, End::A};
            const EndSlot nB {n.idw, End::B};
            const EndSlot pA {p.idw, End::A};
            const EndSlot pB {p.idw, End::B};

            gr.move_edges(m, aL(), m.length(), p, 0);  // move head of m to p
            gr.move_edges(m, 0, aS(), n, 0);           // move tail of m to n

            gr.copy_neigs(wA, nA);  // set neigs at n.A
            gr.copy_neigs(wB, pB);  // set neigs at p.B

            res0 = deg4_vertex(wA, wB, pA);

            s1 = nB;
            s2 = wA;
            s3 = wB;
            s4 = pA;
        }
        else {

            if constexpr (verboseF)
                report.branch(2,
                    "w1 == w2 : w is unconnected cycle: \n", report.indent,
                    "two internal vertices of the same unconnected chain");

            ct[c1].rem_chain_internal_vetrex(w, aS);

            auto& n = ct[c1].create_chain();  // to hold w[aS...aL-1]
            auto& m = cn[w];                  // assigned AFTER create_chain

            const EndSlot nA {n.idw, End::A};
            const EndSlot nB {n.idw, End::B};

            auto u = cn[w].vertex_at_bulk(aL);
            m.g[aL-1].disconnect_end_outer(End::B);
            m.g[aS-1].connect_to_outer(End::B, *u);

            gr.move_edges(m, aS(), aL(), n, 0);   // move internal part of m to n

            res0 = deg4_vertex(wA, nA, nB);

            s1 = wA;
            s2 = wB;
            s3 = nA;
            s4 = nB;
        }
    }
    else {  // w1 != w2
        if (!isCycle1 &&
            !isCycle2) {

            if constexpr (verboseF)
                report.branch(3,
                    "w1 != w2 : internal vertices of linear chains: \n",
                    report.indent, "w1 is NOT unconnected cycle \n",
                    report.indent, "w2 is NOT unconnected cycle");

            ct[c1].rem_chain_internal_vetrex(w1, a1);

            const auto u1 = ct[c1].create_chain().idw;  // to move w1 head
            auto& n2 = ct[c2].create_chain();           // to move w2 head
            auto& n1 = cn[u1];  // assigned AFTER create_chain
            auto& m1 = cn[w1];  // assigned AFTER create_chain
            auto& m2 = cn[w2];  // assigned AFTER create_chain

            const EndSlot n1A {n1.idw, End::A};
            const EndSlot n1B {n1.idw, End::B};
            const EndSlot n2A {n2.idw, End::A};
            const EndSlot n2B {n2.idw, End::B};

            gr.move_edges(m1, a1(), m1.length(), n1, 0);  // move head of m1 to n1
            gr.move_edges(m2, a2(), m2.length(), n2, 0);  // move head of m2 to n2

            gr.copy_neigs(w1B, n1B);  // set neigs at n1.B
            gr.copy_neigs(w2B, n2B);  // set neigs at n2.B
            if (c1 != c2)
                gr.merge_components(c1, c2);

            res0 = deg4_vertex(w2B, w1B, n1A);

            s1 = w1B;
            s2 = w2B;
            s3 = n1A;
            s4 = n2A;
        }
        else if (isCycle1 &&
                 isCycle2) {

            if constexpr (verboseF)
                report.branch(4, "w1 != w2 : ",
                    "internal vertices of distinct unconnected cycles\n",
                    report.indent, "w1 IS unconnected cycle \n",
                    report.indent, "w2 IS unconnected cycle");

            auto& m1 = cn[w1];
            auto& m2 = cn[w2];

            gr.rotate_chain(m1, a1());
            ct[c2].rem_chain_internal_vetrex(w2, a2);
            gr.rotate_chain(m2, a2());
            gr.merge_components(c1, c2);

            res0 = deg4_vertex(w1A, w2A, w2B);

            s1 = w1A;
            s2 = w1B;
            s3 = w2A;
            s4 = w2B;
        }
        else {
            if (isCycle1) {

                if constexpr (verboseF)
                    report.branch(5, "w1 != w2 : ",
                    "internal vertices of unconnected cycle and a linear chain\n",
                    report.indent, "w1 is unconnected cycle \n",
                    report.indent, "w2 is not unconnected cycle");

                auto& n2 = ct[c2].create_chain();  // to move w2 head
                auto& m1 = cn[w1];                 // assigned AFTER create_chain
                auto& m2 = cn[w2];                 // assigned AFTER create_chain

                const EndSlot n2A {n2.idw, End::A};
                const EndSlot n2B {n2.idw, End::B};

                gr.rotate_chain(m1, a1());

                ct[c2].rem_chain_internal_vetrex(w2, a2);

                gr.move_edges(m2, a2(), m2.length(), n2, 0);  // move head of m2 to n2
                gr.copy_neigs(w2B, n2B);  // set neigs at n2.B

                gr.merge_components(c1, c2);

                res0 = deg4_vertex(w1A, w2B, n2A);

                s1 = w1A;
                s2 = w1B;
                s3 = w2B;
                s4 = n2A;
            }
            else {  // isCycle2

                if constexpr (verboseF)
                    report.branch(6, "w1 != w2 : internal vertices of ",
                        "a linear chain w1, and an unconnected cycle w2 \n",
                        report.indent, "w1 is a linear chain\n",
                        report.indent, "w2 is unconnected cycle chain");

                auto& n1 = ct[c1].create_chain();  // to move w1 head
                auto& m1 = cn[w1];                 // assigned AFTER create_chain
                auto& m2 = cn[w2];                 // assigned AFTER create_chain

                const EndSlot n1A {n1.idw, End::A};
                const EndSlot n1B {n1.idw, End::B};

                gr.rotate_chain(m2, a2());

                ct[c1].rem_chain_internal_vetrex(w1, a1);

                gr.move_edges(m1, a1(), m1.length(), n1, 0);  // move head of m1 to n1
                gr.copy_neigs(w1B, n1B);  // set neigs at n1.B

                gr.merge_components(c1, c2);

                res0 = deg4_vertex(w2A, w1B, n1A);

                s1 = w1B;
                s2 = n1A;
                s3 = w2A;
                s4 = w2B;
            }
        }
    }
    join_ends(s1, s2, s3, s4);

    ct[res0->get_c()].set_chis();

    // Update internal records.

    const auto [u1, u2, u3, u4] = std::array{s1.w, s2.w, s3.w, s4.w};

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*res0);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(res0, u1, {u2, u3, u4});

    return {res0};
}


// Merges a bulk vertex to a boundary vertex of an unconnected cycle.
template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
apply(
    const BulkSlot& b1,
    const ChIdG w2
) -> Res
{
    const auto [w1, a1] = b1.we();

    const EndSlot w1A {w1, End::A};
    const EndSlot w1B {w1, End::B};
    const EndSlot w2A {w2, End::A};
    const EndSlot w2B {w2, End::B};

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    // Print out summary before the operation.

    if constexpr (verboseF)
        report.before(b1, EndSlot{w2, End::undefined},
                      {}, {}, "an unconnected CYCLE ");

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(b1, w2A);  // (preserved, removed)

    // Define component, chain and end indexes of the merging ends:

    ResT res0 {};
    EndSlot s1 {}, s2 {}, s3 {}, s4 {};

    ct[c2].rem_unconnected_cycle_boindary_vetrex();

    if (w1 == w2) {

        const auto w = w1;

        const EndSlot wA {w, End::A};
        const EndSlot wB {w, End::B};

        if constexpr (verboseF)
            report.branch(7,
                "w1 == w2 and is an unconnected CYCLE merged with itself: \n",
                report.indent, "boundary vertex with an internal one"
            );

        auto& n = ct[c1].create_chain();
        auto& m = cn[w];  // assigned after create_chain()
        gr.move_edges(m, a1(), m.length(), n, 0);

        const EndSlot nA {n.idw, End::A};
        const EndSlot nB {n.idw, End::B};

        res0 = deg4_vertex(wB, wA, nB);

        // Slots to join:
        s1 = wB;
        s2 = wA;
        s3 = nB;
        s4 = nA;
    }
    else {
        if (!cn[w1].is_unconnected_cycle()) {

            if constexpr (verboseF)
                report.branch(8, "w1 != w2 \n",
                    report.indent, "w1 is NOT an unconnected CYCLE : \n",
                    report.indent, "w2 = ", w2, " is merged at boundary vertex"
                );

            auto& n1 = ct[c1].create_chain();
            auto& m1 = cn[w1];  // assigned AFTER create_chain()
            gr.move_edges(m1, a1(), m1.length(), n1, 0);  // move head of m to n

            const EndSlot n1A {n1.idw, End::A};
            const EndSlot n1B {n1.idw, End::B};

            gr.copy_neigs(w1B, n1B);  // set neigs at n1.B
            gr.merge_components(c1, c2);
            res0 = deg4_vertex(w1B, w2A, w2B);

            // Slots to join:
            s1 = w1B;
            s2 = w2B;
            s3 = w2A;
            s4 = n1A;
        }
        else {

            if constexpr (verboseF)
                report.branch(9,
                    "w1 != w2 & both w1 and w2 are unconnected CYCLEs : \n",
                    report.indent, "w1 = ", w1, " is merged to an internal vertex, \n",
                    report.indent, "w2 = ", w2, " is merged at boundary vertex");

            gr.rotate_chain(cn[w1], a1());
            gr.merge_components(c1, c2);

            res0 = deg4_vertex(w1A, w2A, w2B);

            // Slots to join:
            s1 = w1A;
            s2 = w1B;
            s3 = w2A;
            s4 = w2B;
        }
    }

    join_ends(s1, s2, s3, s4);

    ct[res0->get_c()].set_chis();

    // Update internal records:

    const auto [u1, u2, u3, u4] = std::array{s1.w, s2.w, s3.w, s4.w};

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*res0);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(res0, u1, {u2, u3, u4});

    return {res0};
}


// Merges two unconnected cycles on their boundary vertices.
template<typename G,
         typename PP>
auto From<Deg2, Deg2, G, PP>::
apply(
    const ChIdG w1,
    const ChIdG w2
) -> Res
{
    // Indexes of the participating components:

    const auto c1 = cn[w1].c;
    const auto c2 = cn[w2].c;

    // Print out summary before the operation.

    if constexpr (verboseF) {
        report.before(EndSlot{w1, End::undefined},
                      EndSlot{w2, End::undefined},
                      {},
                      "an unconnected CYCLE ",
                      "an unconnected CYCLE ");
        report.branch(10, "two unconnected cycles on their boundary vertices");
    }

    GRAFFINE_ASSERT(cn[w1].is_unconnected_cycle(),
           shortName, ": w1 ", w1, " is not an unconnected cycle");
    GRAFFINE_ASSERT(cn[w2].is_unconnected_cycle(),
           shortName, ": w2 ", w2, " is not an unconnected cycle");

    const EndSlot a1 {w1, End::A};
    const EndSlot b1 {w1, End::B};
    const EndSlot a2 {w2, End::A};
    const EndSlot b2 {w2, End::B};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_start(a1, a2);  // (preserved, removed)

    // Produce an unconnected intermediary:

    auto [eg11, eg12] = ct[c1].rem_unconnected_cycle_boindary_vetrex();

    join_ends(a1, b1, a2, b2);

    // Update the graph components:

    gr.merge_components(c1, c2);

    auto& eg22 = cn[w2].end_edge(oe);
    auto v2 = eg22.vertex_at_outer(oe);
    eg11->connect_to_outer(ee, *v2);
    eg12->connect_to_outer(oe, *v2);

    gr.update();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexMerger.on_end(*v2);

    // Print out summary after the operation.

    if constexpr (verboseF)
        report.after(v2, w1, {w2});

    return {v2};
}


template<typename G,
         typename PP>
constexpr
void From<Deg2, Deg2, G, PP>::
join_ends(const EndSlot& s1,
          const EndSlot& s2,
          const EndSlot& s3,
          const EndSlot& s4)
{
    gr.ngs_at(s1) = Neigs{s2, s3, s4};
    gr.ngs_at(s2) = Neigs{s1, s3, s4};
    gr.ngs_at(s3) = Neigs{s2, s1, s4};
    gr.ngs_at(s4) = Neigs{s2, s3, s1};

}

template<typename G,
         typename PP>
constexpr
auto From<Deg2, Deg2, G, PP>::
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
auto From<Deg2, Deg2, G, PP>::
deg4_vertex(
    const EndSlot& s,
    const EndSlot& p,
    const EndSlot& q
) -> Vertex*
{
    auto v = cn[s.w].vertex_at_end(s.e);  // will become junction

    cn[p.w].connect_end_vertex(p.e, *v);
    cn[q.w].connect_end_vertex(q.e, *v);

    return v;
}

}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_22_H
