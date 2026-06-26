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
 * \file on_3.h
 * \brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 3.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_ON_3_H
#define GRAFFINE_TRANSFORMS_PULLING_ON_3_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_22.h"
#include "graffine/transforms/pulling/common.h"
#include "graffine/transforms/pulling/functor_base.h"
#include "graffine/transforms/pulling/path.h"
#include "graffine/transforms/pulling/reporter.h"

#include <algorithm>  // find, max
#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::pulling {


/**
 * Functor doing edge pulling initiated from a vertex of degree 3.
 * \tparam Dir Direction of the pull operation.
 * \tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G,
         typename PP>
struct On<Deg3, Dir, G, PP>
    : public FunctorBase<G, PP>
{
    using Base = FunctorBase<G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Compt = Base::Compt;
    using Chain = Base::Chain;
    using End = Base::End;
    using BulkSlot = Base::BulkSlot;
    using EndSlot = Base::EndSlot;
    using Path = Base::Path;
    using PropertyProcessor = Base::PropertyProcessor;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg3;

    static_assert(is_pullable_degree<D>);

    const std::string d {std::to_string(D)};
    const std::string dir {std::to_string(Dir)};
    const std::string shortName {d + "pu_" + dir};
    const std::string fullName  {"Pulling "s + dir + " from Vertex Deg " + d};

    /**
     * Constructs this functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit On(Graph& gr,
                std::shared_ptr<PP> pp = nullptr);

    /**
     * Pulls a vertex of degree 3.
     * \details Pulls vertex at the end of the path, converting it to a 4-node.
     * \param[in] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps to pull over.
     * \return Result containing the component ID after the pull operation.
     */
    auto operator()(
        Path& path,
        int steps
    ) noexcept -> Res;

protected:

    /**
     * \param[inout] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps to pull over.
     */
    void check_path(
        const Path& path,
        const int steps
    ) const noexcept;

private:

    using Base::cn;
    using Base::egl;
    using Base::gr;
    using Base::updateProperties;

    // Auxiliary functors for vertex transformations
    vertex_split::To<Deg1, Deg2, G, PP>    vertex_split_12;  // degree 3 -> 1 + 2
    vertex_merger::From<Deg1, Deg2, G, PP> vertex_merge_12;  // degree 1 + 2 -> 3
    vertex_merger::From<Deg2, Deg2, G, PP> vertex_merge_22;  // degree 2 + 2 -> 4

    /// Pretty-printer of the initial and final configurations.
    Reporter<On<D, Dir, G, PP>> report {*this};

    void pull(Path& path);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G,
         typename PP>
On<Deg3, Dir, G, PP>::
On(Graph& gr,
   std::shared_ptr<PP> pp)
    : Base {gr, pp}
    , vertex_split_12 {gr, pp}
    , vertex_merge_12 {gr, pp}
    , vertex_merge_22 {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<Orientation Dir,
         typename G,
         typename PP>
auto On<Deg3, Dir, G, PP>::
operator()(
    Path& path,
    const int steps
) noexcept -> Res
{
    if constexpr (verboseF)
        report.before(path, steps);

    check_path(path, steps);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->pulling.on_start(path);

    // single step at a chain end connected via a 3-way junction;
    // forms a leaf node
    pull(path);

    this->pull_leaf_vertex(path, steps - 1);

    if constexpr (verboseF)
        report.after(path);

    return {path.cmp->ind};
}


/// Pulls at a connected chain end (converting a 3-node into 4-node)
template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg3, Dir, G, PP>::
pull(Path& path)
{
    const auto iD = path.d.ind;
    const auto egeD = path.d.egEnd;
    auto wD = path.d.w;
    auto eD = cn[wD].ind2end(iD, egeD);  // chain end at the driving vertex.

    ASSERT(End::is_defined(eD), "driver of pull<", D, "> is not a chain end.");

    const auto iS = path.pthw.back();
    auto& cnS = path.cmp->chain(path.s.w);
    const auto egeS = cnS.g[egl[iS].a].inner_endId(path.s.e);

    // cn[wD].ngs[eD].num() == D - 1,
    // since it is a connected chain end with nn < D, and the
    // pure loop is excluded by the availability of free ends in the component
    ASSERT(cn[wD].ngs[eD].num() == 2, "nn != ", 2);

    const auto nww = cn[wD].ngs[eD].ww();
    const auto nee = cn[wD].ngs[eD].ee();

    std::array ni {cn[nww[0]].end2ind(nee[0]),
                   cn[nww[1]].end2ind(nee[1])};

    // disconnect iD at the branching (i.e. wD at eD)
    vertex_split_12(EndSlot{wD, eD});

    wD = egl[iD].w;
    if (iD != iS)
        eD = cn[wD].ind2end(iD, egeD);
    // otherwise (iD == iS) eD is not changed by the vertex split above.

    auto wS = egl[iS].w;
    auto eS = cn.edge(egl[iS]).points_forwards()
            ? egeS
            : End::opp(egeS);

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&cn.edge(egl[iD]), egeD},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};
    this->pull_leaf_vertex(path, 1);

    wD = egl[iD].w;
    if (iD != iS)
        eD = cn[wD].ind2end(iD, egeD);

    wS = egl[iS].w;

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&cn.edge(egl[iD]), egeD},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};

    const auto a0 = eD == End::A
                  ? 1
                  : cn[wD].length() - 1;
    const auto w1 = egl[ni[0]].w;

    // driver neigbour slots over the branching are part of the path
    if (const auto pi =
            std::find(path.pthw.begin(), path.pthw.end(), ni[0]) - path.pthw.begin();
        pi < static_cast<long int>(path.pthw.size()) - 1)
        ni[0] = path.pthw[pi + 1];

    if (const auto pi =
        std::find(path.pthw.begin(), path.pthw.end(), ni[1]) - path.pthw.begin();
        pi < static_cast<long int>(path.pthw.size()) - 1)
        ni[1] = path.pthw[pi + 1];

    const auto a1 = std::max(egl[ni[0]].a,
                             egl[ni[1]].a);

    if (iD == iS)
        cn[w1].is_unconnected_cycle()
            ? vertex_merge_12(EndSlot{wD, End::opp(eD)}, w1)
            : vertex_merge_12(EndSlot{wD, End::opp(eD)}, BulkSlot{w1, a1});
    else
        cn[w1].is_unconnected_cycle()
            ? vertex_merge_22(BulkSlot{wD, a0}, w1)
            : vertex_merge_22(BulkSlot{wD, a0}, BulkSlot{w1, a1});

    wS = egl[iS].w;
    eS = cn.edge(egl[iS]).points_forwards()
       ? egeS
       : End::opp(egeS);

    // recreate path for eventual further steps
    path = Path {&gr.compt(wS),
                 typename Path::Driver {&cn.edge(egl[iD]), egeD},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg3, Dir, G, PP>::
check_path(
    const Path& path,
    [[maybe_unused]] const int steps
) const noexcept
{
    const auto c = path.cmp;
    const auto pc = path.pthc;
    const auto wS = path.s.w;
    [[maybe_unused]] const auto eS = path.s.e;
    const auto wD = path.d.w;
    [[maybe_unused]] const auto eD = path.d.end_slot(c->chain(wD)).e;

    ASSERT(c->chain(wD).ngs[eD].num() == 2,
           "Driver end is not at a three-way junction");

    [[maybe_unused]] const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && c->chain(wS).ngs[eS].num() == 0),
           "Source end is not disconnected");

    [[maybe_unused]] const auto lenS = path.length_over_source_chain();

    ASSERT((!sourceIsCycle &&
            lenS >= static_cast<size_t>(steps)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<size_t>(steps)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_ON_3_H
