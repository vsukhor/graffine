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
 * \file on_2.h
 * \brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 2.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_ON_2_H
#define GRAFFINE_TRANSFORMS_PULLING_ON_2_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_split/to_11.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/pulling/common.h"
#include "graffine/transforms/pulling/functor_base.h"
#include "graffine/transforms/pulling/path.h"
#include "graffine/transforms/pulling/reporter.h"

#include <array>
#include <memory>
#include<string>
#include <type_traits>

namespace graffine::transforms::pulling {


/**
 * Functor for the edge pulling initiated from a vertex of degree 2.
 * \tparam Dir Direction of the pull operation.
 * \tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G,
         typename PP>
struct On<Deg2, Dir, G, PP>
    : public FunctorBase<G, PP>
{
    using Base = FunctorBase<G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Compt = Base::Compt;
    using Chain = Base::Chain;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using Path = Base::Path;
    using PropertyProcessor = Base::PropertyProcessor;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg2;

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
     * Pulls a vertex of degree 2.
     */
    auto operator()(
        Path& path,
        int steps
    ) noexcept -> Res;

protected:

    using Base::gr;
    using Base::cn;
    using Base::egl;
    using Base::updateProperties;

    /**
     * Pulls over path length 1 edge.
     * \details Pulls a vertex of degree 2 over a path of length 1 edge.
     * The path is expected to be a valid path of length 1 edge over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * \param[inout] path Path containing the string of edges to get pulled.
     */
    void pull_1(Path& path);

    /**
     * Pulls over path of length more than 1 edge.
     * \details Pulls a vertex of degree 2 over a path of length more than 1 edge.
     * The path is expected to be a valid path over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * \param[inout] path Path containing the string of edges to get pulled.
     */
    void pull_N(Path& path);

    /**
     * Checks the validity of the path before pulling.
     * \param[inout] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps intended for pulling.
     * \details Checks that the path is valid for pulling, i.e., it starts with
     * a driver edge and ends with a source edge, and that the source edge is
     * connected to the driver edge.
     * \note The path is expected to be a valid path over the graph.
     */
    void check_path(
        const Path& path,
        const int steps
    ) const noexcept;

private:

    // Auxiliary functors for vertex transformations
    vertex_split::To<Deg1, Deg1, G, PP>    vertex_split_11;  // degree 2 -> 1 + 1
    vertex_merger::From<Deg1, Deg1, G, PP> vertex_merge_11;  // degree 1 + 1 -> 2
    vertex_merger::From<Deg1, Deg2, G, PP> vertex_merge_12;  // degree 1 + 2 -> 3

    /// Pretty-printer of the initial and final configurations.
    Reporter<On<D, Dir, G, PP>> report {*this};

    void correct_driver(Path& path) const noexcept;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G,
         typename PP>
On<Deg2, Dir, G, PP>::
On(Graph& gr,
   std::shared_ptr<PP> pp)
    : Base {gr, pp}
    , vertex_split_11 {gr, pp}
    , vertex_merge_11 {gr, pp}
    , vertex_merge_12 {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<Orientation Dir,
         typename G,
         typename PP>
auto On<Deg2, Dir, G, PP>::
operator()(
    Path& path,
    const int steps
) noexcept -> Res
{
    if constexpr (verboseF)
        report.before(path, steps);

    if (path.length() > 1)
        correct_driver(path);

    check_path(path, steps);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->pulling.on_start(path);

    // pull on a chain internal node: special procedures producing a free end
    (path.length() == 1) ? pull_1(path)
                         : pull_N(path);

    this->pull_leaf_vertex(path, steps - 1);

    if constexpr (verboseF)
        report.after(path);

    return {path.cmp->ind};
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg2, Dir, G, PP>::
pull_N(Path& path)
{
    const auto& p = path.pthw;   // path over graph-wide edge indexes (ind)
    const auto& pc = path.pthc;  // path over component-wide edge indexes (indc)

    // driver
    const auto& cnD = cn[path.d.w];  // pulled chain
    const auto egeD = path.d.egEnd;  // edge end of the iD
    const auto icD = pc[0];          // indc of the driver edge
    const auto iD = p[0];            // ind of the driver edge
    auto aD = path.cmp->get_egl(icD).a;

    // BulkSlot to split for creating the branching vertex
    auto bsDe = cnD.egEnd_to_bulkslot(egeD, aD);

    ASSERT(bsDe.a()() && bsDe.a()() < cnD.length(),
           "pulling deg 2 from a chain end");
    ASSERT(!cnD.is_unconnected_cycle() || !omitUnconnectedCycles,
           "pulling deg 2 from an unconnected cycle chain");

    // source
    const auto iS = p.back();  // ind of the source edge
    auto& cnS = path.cmp->chain(path.s.w);
    const auto egeS = cnS.g[egl[iS].a].inner_endId(path.s.e);

    // non-path edge in front of the driver:
    // because driver end is at degree 2, there should be at least one such edge
    auto aQ = cnD.neig_a_of_egEnd(aD, egeD);

    ASSERT(aQ.is_defined(), "Vertex degree at the pulled edge is not 2.");

    const auto egeQ = cnD.internal_egEnd(aD, aQ);
    const auto icQ = cnD.g[aQ].indc;
    const auto iQ = path.cmp->get_egl(icQ).i;

    vertex_split_11(bsDe);

    auto wS = egl[iS].w;
    auto eS = cn.edge(egl[iS]).points_forwards()
            ? egeS
            : End::opp(egeS);

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&gr.edge(iD), egeD},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};

    [[maybe_unused]] const auto source_was_dissolved = this->pull_leaf_vertex(path, 1);

    const auto wQ = egl[iQ].w;
    const auto eQ = cn.edge(egl[iQ]).points_forwards() ? egeQ
                                                       : End::opp(egeQ);
    auto wD = egl[iD].w;  // index of the chain containing the driver edge

    vertex_merge_12(EndSlot{wQ, eQ},
                    cn[wD].egEnd_to_bulkslot(End::opp(egeD), egl[iD].a));

    wS = egl[iS].w;

    ASSERT(source_was_dissolved || (!source_was_dissolved &&
           (cn[wS].is_headind(iS) || cn[wS].is_tailind(iS))),
           "Source is not a chain end");

    eS = cn[wS].has_single_leaf_vertex()
       ? cn[wS].get_single_leaf_end()
       : End::undefined;

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&gr.edge(iD), egeD},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg2, Dir, G, PP>::
pull_1(Path& path)
{
    ASSERT(path.length() == 1, "Path is longer than 1 edge.");

    const auto& pc = path.pthc;  // path over component-wide edge indexes (indc)

    // driver
    auto& cnD = cn[path.d.w];  // pulled chain
    const auto icD = pc[0];    // indc of the driver edge
    const auto aD = path.cmp->get_egl(icD).a;
    auto& egD = cnD.g[aD];

    ASSERT(!cnD.is_unconnected_cycle() || !omitUnconnectedCycles,
           "pulling deg 2 from an unconnected cycle chain");

    egD.template reverse<true>();

    if constexpr (!std::is_void_v<PP>)
        updateProperties->pulling.on_end(path);
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg2, Dir, G, PP>::
correct_driver(Path& path) const noexcept
{
    ASSERT(path.length() > 1,
           "Attempting to correct driver in a single-edge path.");
    ASSERT(!cn[path.d.w].egEnd_is_head(path.d.ind, path.d.egEnd),
           "Driver in pull 2 is at chain head end.");
    ASSERT(!cn[path.d.w].egEnd_is_tail(path.d.ind, path.d.egEnd),
           "Driver in pull 2 is at chain tail end.");

    const auto a0 = path.cmp->get_egl(path.pthc[0]).a;

    const auto ce = cn[path.d.w].connected_edge(a0, path.d.egEnd);
    ASSERT(ce, "Driver in pull 2 multi-edge path is at chain end.");

    if (ce->ind == path.pthw[1]) {

        if constexpr (verboseF)
            jot<false>(colorcodes::BOLDCYAN, "Correcting driver. ",
                       colorcodes::RESET);

        const auto w1 = path.cmp->get_egl(path.pthc[1]).w;
        const auto a1 = path.cmp->get_egl(path.pthc[1]).a;
        const auto egeD = cn[w1].internal_egEnd(a0, a1);
        path.pthw.pop_front();
        path = Path {&gr.compt(w1),
                     typename Path::Driver {&cn[w1].g[a1], egeD},
                     path.s,
                     path.pthw};

        if constexpr (verboseF) {
            path.print_detailed("After correction: ");
            jot("");
        }
    }
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg2, Dir, G, PP>::
check_path(
    const Path& path,
    [[maybe_unused]] const int steps
) const noexcept
{
    const auto c = path.cmp;
    const auto pc = path.pthc;
    const auto wS = path.s.w;
    [[maybe_unused]] const auto eS = path.s.e;

    if (pc.size() > 1) {
        [[maybe_unused]] const auto wD = path.d.w;

        // if the first two edges of the path are on the same chain, their
        // common vertex should be on the driver edge end opposite to 'egEnd'
        const auto& g0 = c->get_egl(pc[0]);
        const auto& g1 = c->get_egl(pc[1]);
        if (g0.w == g1.w) {
            ASSERT(End::opp(cn[wD].internal_egEnd(g1.a, g0.a)) == path.d.egEnd,
                   "opp of driver's internal_egEnd is not d.egEnd");
        }
        const auto gS = c->get_egl(pc.back());
        const auto gT = c->get_egl(pc[pc.size()-2]);
        if (gS.w == gT.w) {
            ASSERT(End::opp(cn[wS].internal_egEnd(gT.a, gS.a)) ==
                   cn[wS].g[gS.a].inner_endId(eS),
                   "opp of source's internal_egEnd is not its inner_endId");
        }
    }

    [[maybe_unused]] const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && cn[wS].ngs[eS].num() == 0),
           "Source end is not disconnected");

    [[maybe_unused]] const auto lenS = path.length_over_source_chain();

    ASSERT(path.drv().w == path.src().w ||
           (!sourceIsCycle &&
            lenS >= static_cast<size_t>(steps)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<size_t>(steps)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_ON_2_H
