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
 * @file on_3.h
 * @brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 3.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_FUNCTOR_3_H
#define GRAPH_MUTATOR_PULLING_FUNCTOR_3_H

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "../vertex_split/to_1B.h"
#include "../vertex_merger/from_10.h"
#include "../vertex_merger/from_12.h"
#include "../vertex_merger/from_20.h"
#include "../vertex_merger/from_22.h"
#include "common.h"
#include "functor_base.h"
#include "log.h"
#include "paths.h"


namespace graph_mutator::pulling {


/**
 * @brief Functor for the edge pulling initiated from a vertex of degree 3.
 * @tparam Dir Direction of the pull operation.
 * @tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G>
struct On<Deg3, Dir, G>
    : public FunctorBase<G> {

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg3;
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
    using BulkSlot = Chain::BulkSlot;
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
     * @brief Pulls a vertex of degree 3.
     * @details Pulls the vertex at the end of the path, converting it to a 4-node.
     * @param pp Path to the vertex to pull.
     * @param n Number of steps to pull over.
     * @return Result containing the component ID after the pull operation.
     */
    auto operator()(
        Ps& pp,
        int n
    ) noexcept -> Res;

protected:

    void check_path(
        const Ps& pp,
        const int n
    ) const noexcept;

private:

    using Base::cn;
    using Base::ct;
    using Base::gla;
    using Base::glm;

    // Auxiliary functors for vertex transformations
    vertex_split::To<1, 2, Graph>  vertex_split_12;  // degree 3 -> 1 + 2
    vertex_split::To<1, 0, Graph>  vertex_split_10;  // degree 3 -> 1 + 0
    vertex_merger::From<1, 0, Graph> vertex_merge_10;  // degree 1 + 0 -> 2
    vertex_merger::From<1, 2, Graph> vertex_merge_12;  // degree 1 + 2 -> 3
    vertex_merger::From<2, 0, Graph> vertex_merge_20;  // degree 2 + 0 -> 4
    vertex_merger::From<2, 2, Graph> vertex_merge_22;  // degree 2 + 2 -> 4

    L log;

    void pull(Ps& pp);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G>
On<Deg3, Dir, G>::
On(Graph& gr)
    : Base {gr}
    , vertex_split_12 {gr}
    , vertex_split_10 {gr}
    , vertex_merge_10 {gr}
    , vertex_merge_12 {gr}
    , vertex_merge_20 {gr}
    , vertex_merge_22 {gr}
    , log {*this}
{}


template<Orientation Dir,
         typename G>
auto On<Deg3, Dir, G>::
operator()(
    Ps& pp,
    const int n
) noexcept -> Res
{
    if constexpr (verboseF)
        log.before(pp, n);

    check_path(pp, n);

    pull(pp);  // pull at a chain end connected via a 3-way junction

    this->pull_free_end(pp, n - 1);

    if constexpr (verboseF)
        log.after(pp);

    return {pp.cmp->ind};
}


/// Pulls at a connected chain end (converting a 3-node into 4-node)
template<Orientation Dir,
         typename G>
void On<Deg3, Dir, G>::
pull(Ps& pp)
{
    const auto iD = pp.d.ind;
    const auto egeD = pp.d.egEnd;
    auto wD = pp.d.w;
    auto eD = cn[wD].ind2end(iD, egeD);

    ASSERT(is_defined(eD), "driver of pull<3> is not a chain end.");

    const auto iS = pp.pth.back();
    auto& cnS = pp.cmp->chain(pp.s.w);
    const auto egeS = cnS.g[gla[iS]].oriented_end(pp.s.e);

    // cn[wD].ngs[eD].num() == D - 1,
    // since it is a connected chain end with nn < D, and the
    // pure loop is excluded by the availability of free ends in the component
    ASSERT(cn[wD].ngs[eD].num() == 2, "nn != ", 2);

    const std::array ns {cn[wD].ngs[eD][0],
                         cn[wD].ngs[eD][1]};

    std::array ni {cn[ns[0].w].end2ind(ns[0].e),
                   cn[ns[1].w].end2ind(ns[1].e)};

    // disconnect iD at the branching (i.e. wD at eD)
    // in the case if the chain neighboring to wD at eD is a cycle, disconnecting
    // the driver will result in a disconnected cycle: so use vertex_split_10
    if (cn[ns[0].w].is_connected_cycle() && ns[0].w == ns[1].w)
        vertex_split_10(EndSlot{wD, eD});
    else
        vertex_split_12(EndSlot{wD, eD});

    wD = glm[iD];
    if (iD != iS)
        eD = cn[wD].ind2end(iD, egeD);
    // otherwise (iD == iS) eD is not changed by the vertex_split_1B above.

    auto wS = glm[iS];
    auto eS = cn[wS].g[gla[iS]].points_forwards()
            ? egeS
            : Ends::opp(egeS);

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], egeD},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};
    this->pull_free_end(pp, 1);

    wD = glm[iD];
    if (iD != iS)
        eD = cn[wD].ind2end(iD, egeD);

    wS = glm[iS];

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], egeD},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};

    const auto a0 = eD == Ends::A
                  ? 1
                  : cn[wD].length() - 1;
    const auto w1 = glm[ni[0]];

    // driver neigbour slots over the branching are part of the path
    if (const auto pi =
            std::find(pp.pth.begin(), pp.pth.end(), ni[0]) - pp.pth.begin();
        pi < static_cast<long int>(pp.pth.size()) - 1)
        ni[0] = pp.pth[pi + 1];

    if (const auto pi =
        std::find(pp.pth.begin(), pp.pth.end(), ni[1]) - pp.pth.begin();
        pi < static_cast<long int>(pp.pth.size()) - 1)
        ni[1] = pp.pth[pi + 1];

    const auto a1 = std::max(gla[ni[0]], gla[ni[1]]);

    if (iD == iS)
        cn[w1].is_disconnected_cycle()
            ? vertex_merge_10(EndSlot{wD, Ends::opp(eD)}, w1)
            : vertex_merge_12(EndSlot{wD, Ends::opp(eD)}, BulkSlot{w1, a1});
    else
        cn[w1].is_disconnected_cycle()
            ? vertex_merge_20(BulkSlot{wD, a0}, w1)
            : vertex_merge_22(BulkSlot{wD, a0}, BulkSlot{w1, a1});

    wS = glm[iS];
    eS = cn[wS].g[gla[iS]].points_forwards()
       ? egeS
       : Ends::opp(egeS);

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], egeD},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};
}


template<Orientation Dir,
         typename G>
void On<Deg3, Dir, G>::
check_path(
    const Ps& pp,
    const int n
) const noexcept
{
    const auto c = pp.cmp;
    const auto pc = pp.pthc;
    const auto wS = pp.s.w;
    const auto eS = pp.s.e;
    const auto wD = pp.d.w;
    const auto eD = pp.d.end_slot(c->chain(wD)).e;

    ASSERT(c->chain(wD).ngs[eD].num() == 2,
           "Driver end is not at a three-way junction");

    const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && c->chain(wS).ngs[eS].num() == 0),
           "Source end is not disconnected");

    const auto lenS = pp.length_over_source_chain();

    ASSERT((!sourceIsCycle &&
            lenS >= static_cast<EgId>(n)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<EgId>(n)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_FUNCTOR_3_H
