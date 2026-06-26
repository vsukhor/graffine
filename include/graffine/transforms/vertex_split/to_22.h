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
 * \file to_22.h
 * \brief Contains class template enabling splits of specific graph vertices.
 * \details This Functor template specialization operates on vertices of
 * degree 4 to produce two vertices whose degrees sum up to 4, except 1 and 3,
 * implemented in a separate functor.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_22_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_22_H

#include "graffine/definitions.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/vertex_split/reporter.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"

#include <array>
#include <memory>
#include <type_traits>

namespace graffine::transforms::vertex_split {

/**
 * Template specialization for producing a pair of degree 2 vertices.
 * \details Adds vertex type-specific split capability and updates
 * the graph for it. This Functor template specialization operates
 * on vertices of degree 4 to produce two vertices of degree 2: V4 -> 2V2.
 * The V2 vertices may be either internal chain vertices or boundary vertices of
 * one or two unconnected cycle chains.
 * \tparam G Graph to which the split operation is applied.
 */
template<typename G,
         typename PP>
struct To<Deg2, Deg2, G, PP>
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

    static constexpr auto J1 = Deg2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Deg2;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = J1 + J2;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

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
     * \param[in] s Descriptors of incident edges to get disconnected.
     */
    auto operator()(Vertex& v,
                    const std::vector<EdgeInVertex>& s) -> Res;

    /**
     * Divides the component at a vertex of degree 4.
     * Template parameters are used for testing of output chain types, their
     * default values may be accepted in most cases.
     * \param[in] s Slots defining the vertex to split.
     */
    template<bool checkValidity = false,
             bool toDisconnectedCycle1 = false,
             bool toDisconnectedCycle2 = false>
    auto operator()(const std::array<EndSlot, J1>& s) -> Res;

    /**
     * Divides the component at a vertex of degree 4.
     * Template parameters are used for testing of output chain types, their
     * default values may be accepted in most cases.
     * \param[in] s1 Slot defining the vertex to split and its 1st edge.
     * \param[in] s2 Slot defining the vertex to split and its 2nd edge.
     */
    template<bool checkValidity = false,
             bool toDisconnectedCycle1 = false,
             bool toDisconnectedCycle2 = false>
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) -> Res;

private:

    // Auxiliary low-level functors for intermediate states.
    vertex_merger::From<Deg1, Deg1, Graph, PP> merge;
    To<Deg1, Deg3, Graph, PP> split_to_13;
    To<Deg1, Deg2, Graph, PP> split_to_12;

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;

    /**
     * Do explicit check of the expected chain types.
     * Is not necessary in most cases.
     */
    template<bool toDisconnectedCycle1,
             bool toDisconnectedCycle2>
     void check_validity(
        const EndSlot& s1,
        const EndSlot& s2
    ) const noexcept;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
To<Deg2, Deg2, G, PP>::
To(Graph& gr,
   std::shared_ptr<PP> pp)
    : merge {gr, pp}
    , split_to_13 {gr, pp}
    , split_to_12 {gr, pp}
    , gr {gr}
    , updateProperties {pp}
    , report {dd, gr}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto To<Deg2, Deg2, G, PP>::
operator()(Vertex& v,
           const std::vector<EdgeInVertex>& s) -> Res
{
    ASSERT_CALLING(v.deg() == I,
                   v.print("INCORRECT DEGREE"),
                   "Vertex must have degree", I, " actual degree is", v.deg());

    ASSERT(s.size() == J1,
           "Incorrect number of incident slots for ", shortName);

    ASSERT_CALLING(v.is_conected_to(s[0]),
                   v.print("NOT CONNECTED"),
                   "Vertex is not connected to ", s[0].str_short());
    ASSERT_CALLING(v.is_conected_to(s[1]),
                   v.print("NOT CONNECTED"),
                   "Vertex is not connected to ", s[1].str_short());

    const auto& cmp = gr.ct[v.get_c()];
    const auto s1 = cmp.end_slot(s[0]);
    const auto s2 = cmp.end_slot(s[1]);

    return (*this)(s1, s2);
}


// cuts a 4-junction
template<typename G,
         typename PP>
template<bool checkValidity,
         bool toDisconnectedCycle1,
         bool toDisconnectedCycle2>
auto To<Deg2, Deg2, G, PP>::
operator()(const std::array<EndSlot, J1>& s) -> Res
{
    return (*this)<checkValidity,
                   toDisconnectedCycle1,
                   toDisconnectedCycle2>(s[0], s[1]);
}


// cuts a 4-junction
template<typename G,
         typename PP>
template<bool checkValidity,
         bool toDisconnectedCycle1,
         bool toDisconnectedCycle2>
auto To<Deg2, Deg2, G, PP>::
operator()(const EndSlot& _s1,
           const EndSlot& _s2) -> Res
{
    const auto s1 = _s1;  // copy
    const auto s2 = _s2;  // copy

    if constexpr (verboseF)
        report.before({s1, s2});

    if constexpr (checkValidity)
        check_validity<toDisconnectedCycle1, toDisconnectedCycle2>(s1, s2);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_start(s1, s2);

    const auto ngs = gr.ngs_at(s1).as_vector();

    std::array<EgIdG, I> ind;             // indexes of edges connected to s
    std::array<typename End::Id, I> end;  // chain ends connected to s
    ind[0] = gr.slot2ind(s1);             // edge ind at s1
    ind[1] = gr.slot2ind(s2);             // edge ind at s2
    end[0] = gr.edge_end(s1);
    end[1] = gr.edge_end(s2);
    for (std::size_t i=1, j=2; i<I; ++i) {
        const auto& u = ngs[i-1];
        if (u != s2) {
            ind[j] = gr.slot2ind(u);
            end[j] = gr.edge_end(u);
            ++j;
        }
    }

    [[maybe_unused]] auto [va, vb] = split_to_13(s1);

    ASSERT(!gr.compt(va->get_c()).template check<verboseF>(),
           "split_to_13 failed on component ", va->get_c());
    ASSERT(!gr.compt(vb->get_c()).template check<verboseF>(),
           "split_to_13 failed on component ", vb->get_c());

    auto [vc, v2] = split_to_12(s2);

    ASSERT(!gr.compt(vc->get_c()).template check<verboseF>(),
           "split_to_12 failed on component ", vc->get_c());
    ASSERT(!gr.compt(v2->get_c()).template check<verboseF>(),
           "split_to_12 failed on component ", v2->get_c());

    const auto ww0 = gr.chid(ind[0]);
    const auto ww1 = gr.chid(ind[1]);

    auto v1 = merge(EndSlot{ww0, gr.chain(ww0).ind2end(ind[0], end[0])},
                    EndSlot{ww1, gr.chain(ww1).ind2end(ind[1], end[1])});

    Res res {v1[0], v2};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->vertexSplit.on_end(res);

    if constexpr (verboseF)
        report.after(res, {ind[0],ind[1]}, {ind[2], ind[3]});

    std::vector cc {gr.edge(ind[0]).c, gr.edge(ind[1]).c,
                    gr.edge(ind[2]).c, gr.edge(ind[3]).c};
    std::sort(cc.begin(),cc.end());
    cc.erase(std::unique(cc.begin(), cc.end()), cc.end());

    return res;
}


template<typename G,
         typename PP>
template<bool toDisconnectedCycle1,
         bool toDisconnectedCycle2>
void To<Deg2, Deg2, G, PP>::
check_validity(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept
{
    [[maybe_unused]] const auto& m1 = gr.chain(s1.w);
    [[maybe_unused]] const auto& m2 = gr.chain(s2.w);
    [[maybe_unused]] const auto& ngs1 = gr.ngs_at(s1);
    [[maybe_unused]] const auto& ngs2 = gr.ngs_at(s2);

    [[maybe_unused]] auto cycle_is_selected = [&]() -> bool
    {
        return gr.is_same_cycle(s1, s2);
    };

    [[maybe_unused]] auto cycle_is_excluded = [&]() -> bool
    {
        const auto& other = ngs1.other_than({s2});

        return gr.is_same_cycle(*other.begin(), *other.rbegin());
    };

    ASSERT(s1 != s2, shortName,
           ": slots s1 and s2 are identical:", s1.str_short());

    using namespace structure::containers;
    ASSERT(ngs1.num() == numNeigs<I>,
           shortName, ": ngs1.num ", ngs1.num(), " != numNeigs ", numNeigs<I>);
    ASSERT(ngs2.num() == I - 1,
           shortName, ": ngs2.num ", ngs2.num(), " != numNeigs ", numNeigs<I>);

    ASSERT(ngs1.contains(s2),
           shortName, ": s2 ", s2.str_short(),
           "not found among connections at s1 ", s1.str_short());
    ASSERT(ngs2.contains(s1),
           shortName, ": s1 ", s1.str_short(),
           "not found among connections at s2 ", s2.str_short());

    if constexpr (!toDisconnectedCycle1 && !toDisconnectedCycle2) {
        ASSERT(!gr.is_same_cycle(s1, s2),
               shortName,  ": disconnecting a cycle chain");
    }

    else if constexpr (toDisconnectedCycle1 && toDisconnectedCycle2) {
        ASSERT(cycle_is_selected() && cycle_is_excluded(),
               shortName, ": both selected and excluded ",
               "pairs of slots should belong to a cycle chain");
    }

    else if constexpr ((toDisconnectedCycle1 && !toDisconnectedCycle2) ||
                       (!toDisconnectedCycle1 && toDisconnectedCycle2)) {
        ASSERT(( cycle_is_selected() && !cycle_is_excluded()) ||
               (!cycle_is_selected() &&  cycle_is_excluded()),
               shortName,  ": only either selected ",
               "or excluded pair of slots may belong to a cycle chain");
    }
}

// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct To2<Deg2, G, PP>
    : public To<Deg2, Deg2, G, PP>
{
    using Base = To<Deg2, Deg2, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using Chains = Base::Chains;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using Vertex = Base::Vertex;
    using EdgeInVertex = Base::EgDescr;
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

    explicit constexpr To2(Graph& gr,
                           std::shared_ptr<PP> pp = nullptr)
        : Base {gr, pp}
    {
        ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};


}  // namespace graffine::transforms::vertex_split

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_TO_22_H
