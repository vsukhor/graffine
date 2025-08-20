/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file on_2.h
 * @brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 2.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_FUNCTOR_2_H
#define GRAPH_MUTATOR_PULLING_FUNCTOR_2_H

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "../vertex_split/to_11.h"
#include "../vertex_merger/from_11.h"
#include "../vertex_merger/from_12.h"
#include "common.h"
#include "functor_base.h"
#include "log.h"
#include "paths.h"


namespace graph_mutator::pulling {


/**
 * @brief Functor for the edge pulling initiated from a vertex of degree 2.
 * @tparam Dir Direction of the pull operation.
 * @tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G>
struct On<Deg2, Dir, G>
    : public FunctorBase<G> {

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg2;
    static_assert(is_pullable_degree<D>);

    static constexpr auto d = string_ops::str1<D>;

    static constexpr auto shortName
        {string_ops::shName<d, 1, 'p', 'u', '_', signchar<Dir>>};
    static constexpr auto fullName
        {string_ops::shName<d, 1,
         'P', 'u', 'l', 'l', 'i', 'n', 'g', ' ', 'f', 'r', 'o', 'm', ' ',
         'V', 'e', 'r', 't', 'e', 'x', ' ', 'D', 'e', 'g', ' ', signchar<Dir>>};

    using Base = FunctorBase<G>;
    using Graph = G;
    using Cmpt = Graph::Cmpt;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using Ps = Paths<Cmpt>;
    using L = Log<On<D, Dir, Graph>>;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Constructs this functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit On(Graph& gr);

    /**
     * @brief Pulls a vertex of degree 2.
     */
    auto operator()(
        Ps& pp,
        int n
    ) noexcept -> Res;

protected:

    using Base::gr;
    using Base::cn;
    using Base::ct;
    using Base::gla;
    using Base::glm;

    /**
     * @brief  Pulls over path of length 1 edge.
     * @param pp Path
     * @details Pulls a vertex of degree 2 over a path of length 1 edge.
     * The path is expected to be a valid path of length 1 edge over the graph.
     * The path starts with a driver edge and ends with a source edge.
     */
    void pull_1(Ps& pp);

    /**
     * @brief Pulls over path of length more than 1 edge.
     * @details Pulls a vertex of degree 2 over a path of length more than 1 edge.
     * The path is expected to be a valid path over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * @param pp Path
     */
    void pull_N(Ps& pp);

    /**
     * @brief Checks the validity of the path before pulling.
     * @param pp Path to check.
     * @param n Number of steps intended for pulling.
     * @details Checks that the path is valid for pulling, i.e., it starts with
     * a driver edge and ends with a source edge, and that the source edge is
     * connected to the driver edge.
     * @note The path is expected to be a valid path over the graph.
     */
    void check_path(
        const Ps& pp,
        const int n
    ) const noexcept;

private:

    // Auxiliary functors for vertex transformations
    vertex_split::To<1, 1, Graph>  vertex_split_11;  // degree 2 -> 1 + 1
    vertex_merger::From<1, 1, Graph> vertex_merge_11;  // degree 1 + 1 -> 2
    vertex_merger::From<1, 2, Graph> vertex_merge_12;  // degree 1 + 2 -> 3

    L log;

    void correct_driver(Ps& pp) const noexcept;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G>
On<Deg2, Dir, G>::
On(Graph& gr)
    : Base {gr}
    , vertex_split_11 {gr}
    , vertex_merge_11 {gr}
    , vertex_merge_12 {gr}
    , log {*this}
{}


template<Orientation Dir,
         typename G>
auto On<Deg2, Dir, G>::
operator()(
    Ps& pp,
    const int n
) noexcept -> Res
{
    if constexpr (verboseF)
        log.before(pp, n);

    if (pp.length() > 1)
        correct_driver(pp);

    check_path(pp, n);

    // pull at a bulk node
    pp.length() == 1 ? pull_1(pp)
                     : pull_N(pp);

    this->pull_free_end(pp, n - 1);

    if constexpr (verboseF)
        log.after(pp);

    return {pp.cmp->ind};
}


template<Orientation Dir,
         typename G>
void On<Deg2, Dir, G>::
pull_N(Ps& pp)
{
    const auto& p = pp.pth;       // path over graph-wide edge indexes (ind)
    const auto& pc = pp.pthc;     // path over component-wide indexes (indc)

    // driver
    const auto& cnD = cn[pp.d.w];  // pulled chain
    const auto egeD = pp.d.egEnd;  // edge end of the iD
    const auto icD = pc[0];        // indc of the driver edge
    const auto iD = p[0];          // ind of the driver edge
    auto aD = pp.cmp->gl[icD].a;

    // BulkSlot to split for creating the branching vertex
    auto bsDe = cnD.egEnd_to_bulkslot(egeD, aD);

    ASSERT(bsDe.a() && bsDe.a() < cnD.length(),
           "pulling deg 2 from a chain end");
    ASSERT(!cnD.is_disconnected_cycle(),
           "pulling deg 2 from a disconnected cycle chain");

    // source
    const auto iS = p.back();  // ind of the source edge
    auto& cnS = pp.cmp->chain(pp.s.w);
    const auto egeS = cnS.g[gla[iS]].oriented_end(pp.s.e);

    // non-path edge in front of the driver:
    // because driver end is at degree 2, there should be at least one such edge
    auto aQ = cnD.neig_a_of_egEnd(aD, egeD);

    ASSERT(is_defined(aQ), "Vertex degree at the pulled edge is not 2.");

    const auto egeQ = cnD.internal_egEnd(aD, aQ);
    const auto icQ = cnD.g[aQ].indc;
    const auto iQ = pp.cmp->gl[icQ].i;

    vertex_split_11(bsDe);

    auto wS = glm[iS];
    auto eS = cn[wS].g[gla[iS]].points_forwards()
            ? egeS
            : Ends::opp(egeS);

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], egeD},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};


    const auto source_was_dissolved = this->pull_free_end(pp, 1);


    const auto wQ = glm[iQ];
    const auto eQ = cn[wQ].g[gla[iQ]].points_forwards() ? egeQ
                                                        : Ends::opp(egeQ);
    auto wD = glm[iD];  // index of the chain containing the driver edge

    vertex_merge_12(EndSlot{wQ, eQ},
                    cn[wD].egEnd_to_bulkslot(Ends::opp(egeD), gla[iD]));

    wS = glm[iS];

    ASSERT(source_was_dissolved || (!source_was_dissolved &&
           (cn[wS].is_headind(iS) || cn[wS].is_tailind(iS))),
           "Source is not a chain end");

    eS = cn[wS].has_one_free_end()
       ? cn[wS].get_single_free_end()
       : Ends::Undefined;

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], egeD},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};
}


template<Orientation Dir,
         typename G>
void On<Deg2, Dir, G>::
pull_1(Ps& pp)
{
    ASSERT(pp.length() == 1, "Path is longer than 1 edge.");

    const auto& pc = pp.pthc;   // path over component-wide indexes (indc)

    // driver
    auto& cnD = cn[pp.d.w];  // pulled chain
    const auto icD = pc[0];  // indc of the driver edge
    auto aD = pp.cmp->gl[icD].a;
    auto& egD = cnD.g[aD];

    ASSERT(!cnD.is_disconnected_cycle(),
           "pulling deg 2 from a disconnected cycle chain");

    egD.reverse();
}


template<Orientation Dir,
         typename G>
void On<Deg2, Dir, G>::
correct_driver(Ps& pp) const noexcept
{
    ASSERT(pp.length() > 1,
           "Attempting to correct driver in a single-edge path.");
    ASSERT(!cn[pp.d.w].egEnd_is_head(pp.d.ind, pp.d.egEnd),
           "Driver in pull 2 is at chain head end.");
    ASSERT(!cn[pp.d.w].egEnd_is_tail(pp.d.ind, pp.d.egEnd),
           "Driver in pull 2 is at chain tail end.");

    const auto a0 = pp.cmp->gl[pp.pthc[0]].a;

    const auto ce = cn[pp.d.w].connected_edge(a0, pp.d.egEnd);
    ASSERT(ce, "Driver in pull 2 multi-edge path is at chain end.");

    if (ce->ind == pp.pth[1]) {

        if constexpr (verboseF)
            log_(colorcodes::BOLDCYAN, "Correcting driver: ", colorcodes::RESET);

        const auto w1 = pp.cmp->gl[pp.pthc[1]].w;
        const auto a1 = pp.cmp->gl[pp.pthc[1]].a;
        const auto egeD = cn[w1].internal_egEnd(a0, a1);
        pp.pth.pop_front();
        pp = Ps {&ct[cn[w1].c],
                 typename Ps::Driver {&cn[w1].g[a1], egeD},
                 pp.s,
                 pp.pth};

        if constexpr (verboseF) {
            pp.print_detailed("after correction ");
            log_("");
        }
    }
}


template<Orientation Dir,
         typename G>
void On<Deg2, Dir, G>::
check_path(
    const Ps& pp,
    const int n
) const noexcept
{
    const auto c = pp.cmp;
    const auto pc = pp.pthc;
    const auto wS = pp.s.w;
    const auto eS = pp.s.e;

    if (pc.size() > 1) {
        const auto wD = pp.d.w;

        if (c->gl[pc[0]].w == c->gl[pc[1]].w)
            ASSERT(Ends::opp(cn[wD].internal_egEnd(c->gl[pc[1]].a,
                                                   c->gl[pc[0]].a)) ==
                   pp.d.egEnd,
                "opp of driver's internal_egEnd is not d.egEnd");

        if (c->gl[pc.back()].w == c->gl[pc[pc.size()-2]].w)
            ASSERT(Ends::opp(cn[wS].internal_egEnd(
                        c->gl[pc[pc.size()-2]].a,
                        c->gl[pc.back()].a
                   )) == cn[wS].g[c->gl[pc.back()].a].oriented_end(eS),
                   "opp of source's internal_egEnd is not its oriented_end");
    }

    const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && cn[wS].ngs[eS].num() == 0),
           "Source end is not disconnected");

    const auto lenS = pp.length_over_source_chain();

    ASSERT(pp.drv().w == pp.src().w ||
           (!sourceIsCycle &&
            lenS >= static_cast<EgId>(n)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<EgId>(n)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_FUNCTOR_2_H
