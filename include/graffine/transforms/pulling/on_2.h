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

#include <algorithm>  // ranges::sort, ranges::adjacent_find
#include <array>
#include <memory>
#include<string>
#include <type_traits>

namespace graffine::transforms::pulling {


/** Functor for the edge pulling initiated from a vertex of degree 2.
 * \details The first step uses vertex splits and mergers to turn the driver
 * into a free chain end; the remaining steps then pull it as a leaf.
 * \tparam G Graph to which the pull operation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<typename G,
         typename PP>
struct On<Deg2, G, PP>
    : public FunctorBase<G, PP>
    , public Applicable<On<Deg2, G, PP>>
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

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg2;

    static_assert(is_pullable_degree<D>);

    const std::string d {std::to_string(D)};
    const std::string shortName {d + "pu"};
    const std::string fullName  {"Pulling from Vertex Deg "s + d};

    /** Constructs this functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit On(Graph& gr,
                std::shared_ptr<PP> pp = nullptr);

    /** Pulls a vertex of degree 2 along \p path by \p steps.
     * \details Corrects the driver of \p path first, if the path has more
     * than one edge. Unchecked: see Applicable.
     * \param[inout] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps to pull over.
     * \return Index of the component hosting the path after the pull.
     */
    auto apply(
        Path& path,
        int steps
    ) -> Res;

    /** Checks that \p path can be pulled by \p steps.
     * \details Checks a copy of \p path with the driver corrected as by
     * apply(). Rejects a path failing check_path_basics(), a driver on an
     * unconnected cycle if such are omitted, and a degree 2 source next to
     * the driver. If the driver and the source are on the same chain, only
     * rejects a source end which is not free, unless the chain is a cycle;
     * otherwise, rejects a source failing check_source().
     */
    auto applicability(
        const Path& path,
        int steps
    ) const -> Applicability;

protected:

    using Base::gr;
    using Base::cn;
    using Base::check_path_basics;
    using Base::check_source;
    using Base::egl;
    using Base::source_at;
    using Base::source_egEnd;
    using Base::frame;
    using Base::updateProperties;

    /** Pulls over path length 1 edge.
     * \details Pulls a vertex of degree 2 over a path of length 1 edge.
     * The path is expected to be a valid path of length 1 edge over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * \param[inout] path Path containing the string of edges to get pulled.
     */
    void pull_1(Path& path);

    /** Pulls over path of length more than 1 edge.
     * \details Pulls a vertex of degree 2 over a path of length more than 1 edge.
     * The path is expected to be a valid path over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * \param[inout] path Path containing the string of edges to get pulled.
     */
    void pull_N(Path& path);

    /** Checks the validity of the path before pulling.
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
    ) const;

private:

    // Auxiliary functors for vertex transformations
    vertex_split::To<Deg1, Deg1, G, PP>    vertex_split_11;  // degree 2 -> 1 + 1
    vertex_merger::From<Deg1, Deg1, G, PP> vertex_merge_11;  // degree 1 + 1 -> 2
    vertex_merger::From<Deg1, Deg2, G, PP> vertex_merge_12;  // degree 1 + 2 -> 3

    /// Pretty-printer of the initial and final configurations.
    Reporter<On<D, G, PP>> report {*this};

    void correct_driver(Path& path) const;

    /** Applicability of a pull driven from a degree 2 vertex of an
     * unconnected cycle.
     * \details The split at the driver opens the cycle, and the merger after
     * the first step closes it into a loop of one edge less, the driver edge
     * becoming a tail at the next path vertex; the remaining steps pull the
     * tail into the loop. Rejects a source off the cycle, a cycle which
     * cannot shrink by \p steps, a path which is not a simple part of the
     * cycle, ends elsewhere than at the source, or, for the boundary vertex
     * as source, reaches it over the edge at the other end; also rejects a
     * corrected path shorter than \p steps, or of a single edge.
     */
    auto check_unconnected_cycle(
        const Path& path,
        int steps
    ) const -> Applicability;

    /** For a driver at the boundary vertex of an unconnected cycle, the
     * position of the edge at the other end; undefined otherwise.
     * \param[in] m Chain of the driver.
     * \param[in] a Position of the driver edge.
     * \param[in] egEnd Inner end of the driver edge at the driver vertex.
     */
    static auto across_boundary(
        const Chain& m,
        EgIdA a,
        End::Id egEnd
    ) noexcept -> EgIdA;

    /** Inner end of edge \p b at the vertex it shares with edge \p a, both in
     * chain \p m, also across the boundary vertex of an unconnected cycle.
     */
    static auto shared_egEnd(
        const Chain& m,
        EgIdA a,
        EgIdA b
    ) noexcept -> End::Id;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
On<Deg2, G, PP>::
On(Graph& gr,
   std::shared_ptr<PP> pp)
    : Base {gr, pp, D}
    , vertex_split_11 {gr, pp}
    , vertex_merge_11 {gr, pp}
    , vertex_merge_12 {gr, pp}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto On<Deg2, G, PP>::
applicability(
    const Path& path,
    const int steps
) const -> Applicability
{
    if (const auto ok = check_path_basics(path, steps); !ok)
        return ok;

    if (cn[path.d.w].is_unconnected_cycle()) {
        if constexpr (omitUnconnectedCycles)
            return reject("pulling deg 2 from an unconnected cycle chain");
        else
            return check_unconnected_cycle(path, steps);
    }

    // apply() corrects the driver before pulling: check the corrected path.
    auto corrected = path;
    if (corrected.length() > 1)
        correct_driver(corrected);

    if (corrected.src().is_bulk()) {
        if (corrected.length() == 1)
            return reject("driver vertex is next to a degree 2 source");
        return check_source(corrected, steps);
    }

    if (corrected.drv().w == corrected.src().w) {
        const auto& m = cn[corrected.s.w];
        if (!m.is_cycle()) {
            if (m.ngs[corrected.s.e].num())
                return reject("source end is not free");
            return {};
        }

        // An end of the connected cycle of the driver, at its junction: the
        // split at the driver opens the cycle, the source chain then runs
        // from the next path vertex to the junction, and cannot be dissolved
        // there, its end not being free.
        const auto vv = corrected.vertices();
        if (vv.empty() || !(*vv.back() == *m.vertex_at_end(corrected.s.e)) ||
            corrected.pthw.back() != m.end2ind(corrected.s.e))
            return reject("path does not end at the source");
        if (corrected.length() <= static_cast<std::size_t>(steps))
            return reject("path to the junction of the cycle is too short for the steps");
        return {};
    }

    return check_source(corrected, steps);
}


template<typename G,
         typename PP>
auto On<Deg2, G, PP>::
check_unconnected_cycle(
    const Path& path,
    const int steps
) const -> Applicability
{
    const auto& m = cn[path.d.w];
    const auto len = m.length();

    if (path.s.w != path.d.w)
        return reject("source is not on the cycle of the driver");

    // the merger closing the opened cycle needs a loop of minCycleLength,
    // and each further step shortens the loop by one edge
    if (!m.is_shrinkable_by(static_cast<std::size_t>(steps)))
        return reject("cycle is not shrinkable by the steps");

    auto corrected = path;
    if (corrected.length() > 1)
        correct_driver(corrected);

    if (!corrected.is_contiguous() || corrected.length() >= len)
        return reject("path is not a contiguous part of the cycle");
    {
        auto pv = corrected.vertices();
        std::ranges::sort(pv);
        if (std::ranges::adjacent_find(pv) != pv.end())
            return reject("path passes a vertex twice");
    }

    // pull_1() expects a free source end
    if (corrected.length() == 1)
        return reject("driver vertex is next to the source on an unconnected cycle");

    if (corrected.length() < static_cast<std::size_t>(steps))
        return reject("path is too short for the steps");

    const auto vv = corrected.vertices();
    const auto& s = corrected.s;
    const auto* vS = s.is_bulk() ? m.vertex_at_bulk(s.pos)
                                 : m.vertex_at_end(s.e);
    if (vv.empty() || !vS || !(*vv.back() == *vS))
        return reject("path does not end at the source");

    // an end source is reached over its end edge, so as to be found from it
    if (!s.is_bulk() && corrected.pthw.back() != m.end2ind(s.e))
        return reject("path does not reach the source over its end edge");

    return {};
}


template<typename G,
         typename PP>
auto On<Deg2, G, PP>::
apply(
    Path& path,
    const int steps
) -> Res
{
    if constexpr (verboseF)
        report.before(path, steps);

    if (path.length() > 1)
        correct_driver(path);

    check_path(path, steps);

    if constexpr (!std::is_void_v<PP>)
        hooks::on_start(updateProperties->pulling, path);

    // pull on a chain internal node: special procedures producing a free end
    (path.length() == 1) ? pull_1(path)
                         : pull_N(path);

    this->pull_leaf_vertex(path, steps - 1);

    if constexpr (verboseF)
        report.after(path);

    return {path.cmp->ind};
}


template<typename G,
         typename PP>
void On<Deg2, G, PP>::
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

    // the boundary vertex of an unconnected cycle is the other vertex of
    // degree 2 at a chain end; the split at it opens the cycle in place
    const auto aX = across_boundary(cnD, aD, egeD);

    GRAFFINE_ASSERT(aX.is_defined() ||
           (bsDe.a()() && bsDe.a()() < cnD.length()),
           "pulling deg 2 from a chain end");
    GRAFFINE_ASSERT(!cnD.is_unconnected_cycle() || !omitUnconnectedCycles,
           "pulling deg 2 from an unconnected cycle chain");

    // source
    const auto iS = p.back();  // ind of the source edge
    const auto egeS = source_egEnd(path);
    // The split at the driver opens an unconnected cycle, and the merger
    // closes it at the next path vertex, so the boundary vertex of the cycle,
    // as a source, moves inside the chain: it is found from its edge as a
    // degree 2 source is.
    const auto isBulk = path.s.is_bulk() || cnD.is_unconnected_cycle();

    // non-path edge in front of the driver:
    // because driver end is at degree 2, there should be at least one such edge
    auto aQ = aX.is_defined() ? aX : cnD.neig_a_of_egEnd(aD, egeD);

    GRAFFINE_ASSERT(aQ.is_defined(), "Vertex degree at the pulled edge is not 2.");

    const auto egeQ = shared_egEnd(cnD, aD, aQ);
    const auto icQ = cnD.g[aQ].indc;
    const auto iQ = path.cmp->get_egl(icQ).i;

    // the driver keeps its vertex object: the split leaves it on the driver
    // edge, which is the first resulting vertex if it follows the slot; the
    // merger below then keeps the vertex next to it on the path
    const auto keepD = cnD.g[bsDe.a()() % cnD.length()].ind == iD ? Keep::First
                                                                 : Keep::Second;
    nested<PP>(frame, [&] { vertex_split_11(bsDe, keepD); });

    auto wS = egl[iS].w;

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&gr.edge(iD), egeD},
                 source_at(iS, egeS, isBulk),
                 path.pthw};

    // on_end is called below, once the step is completed by the merger
    [[maybe_unused]] const auto source_was_dissolved =
        this->template pull_leaf_vertex<false>(path);

    const auto wQ = egl[iQ].w;
    const auto eQ = cn.edge(egl[iQ]).points_forwards() ? egeQ
                                                       : End::opp(egeQ);
    auto wD = egl[iD].w;  // index of the chain containing the driver edge

    nested<PP>(frame, [&] {
        vertex_merge_12(EndSlot{wQ, eQ},
                        cn[wD].egEnd_to_bulkslot(End::opp(egeD), egl[iD].a));
    });

    wS = egl[iS].w;

    GRAFFINE_ASSERT(isBulk || source_was_dissolved || (!source_was_dissolved &&
           (cn[wS].is_headind(iS) || cn[wS].is_tailind(iS))),
           "Source is not a chain end");

    // the source is found from its edge, which also holds for a cycle source
    path = Path {&gr.compt(wS),
                 typename Path::Driver {&gr.edge(iD), egeD},
                 source_at(iS, egeS, isBulk),
                 path.pthw};

    if constexpr (!std::is_void_v<PP>)
        hooks::on_end(updateProperties->pulling, path);
}


template<typename G,
         typename PP>
void On<Deg2, G, PP>::
pull_1(Path& path)
{
    GRAFFINE_ASSERT(path.length() == 1, "Path is longer than 1 edge.");

    const auto& pc = path.pthc;  // path over component-wide edge indexes (indc)

    // driver
    auto& cnD = cn[path.d.w];  // pulled chain
    const auto icD = pc[0];    // indc of the driver edge
    const auto aD = path.cmp->get_egl(icD).a;
    auto& egD = cnD.g[aD];

    GRAFFINE_ASSERT(!cnD.is_unconnected_cycle() || !omitUnconnectedCycles,
           "pulling deg 2 from an unconnected cycle chain");

    // the driver becomes the free end, and the source vertex joins the chain
    egD.template reverse<true>();

    // the vertices exchange their places, so that each keeps its path index
    gr.exchange_vertices(*egD.vertex_at_inner(End::A),
                         *egD.vertex_at_inner(End::B));

    if constexpr (!std::is_void_v<PP>)
        hooks::on_end(updateProperties->pulling, path);
}


template<typename G,
         typename PP>
auto On<Deg2, G, PP>::
across_boundary(
    const Chain& m,
    const EgIdA a,
    const End::Id egEnd
) noexcept -> EgIdA  // static
{
    if (!m.is_unconnected_cycle())
        return EgIdA::undefined;

    const auto b = m.egEnd_to_bulkslot(egEnd, a).a()();

    return b == 0            ? EgIdA {m.length() - 1}
         : b == m.length()   ? EgIdA {0}
                             : EgIdA::undefined;
}


template<typename G,
         typename PP>
auto On<Deg2, G, PP>::
shared_egEnd(
    const Chain& m,
    const EgIdA a,
    const EgIdA b
) noexcept -> End::Id  // static
{
    if (a == b + 1 || b == a + 1)
        return m.internal_egEnd(a, b);

    // the end edges of an unconnected cycle, sharing its boundary vertex
    return m.g[b].inner_endId(b == 0 ? End::A : End::B);
}


template<typename G,
         typename PP>
void On<Deg2, G, PP>::
correct_driver(Path& path) const
{
    const auto& m = cn[path.d.w];
    const auto a0 = path.cmp->get_egl(path.pthc[0]).a;
    const auto aX = across_boundary(m, a0, path.d.egEnd);

    GRAFFINE_ASSERT(path.length() > 1,
           "Attempting to correct driver in a single-edge path.");
    GRAFFINE_ASSERT(aX.is_defined() || !m.egEnd_is_head(path.d.ind, path.d.egEnd),
           "Driver in pull 2 is at chain head end.");
    GRAFFINE_ASSERT(aX.is_defined() || !m.egEnd_is_tail(path.d.ind, path.d.egEnd),
           "Driver in pull 2 is at chain tail end.");

    const auto ce = aX.is_defined() ? &m.g[aX]
                                    : m.connected_edge(a0, path.d.egEnd);
    GRAFFINE_ASSERT(ce, "Driver in pull 2 multi-edge path is at chain end.");

    if (ce->ind == path.pthw[1]) {

        if constexpr (verboseF)
            jot<false>(colorcodes::BOLDCYAN, "Correcting driver. ",
                       colorcodes::RESET);

        const auto w1 = path.cmp->get_egl(path.pthc[1]).w;
        const auto a1 = path.cmp->get_egl(path.pthc[1]).a;
        const auto egeD = shared_egEnd(cn[w1], a0, a1);
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


template<typename G,
         typename PP>
void On<Deg2, G, PP>::
check_path(
    const Path& path,
    [[maybe_unused]] const int steps
) const
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
            GRAFFINE_ASSERT(End::opp(shared_egEnd(cn[wD], g1.a, g0.a)) == path.d.egEnd,
                   "opp of driver's internal_egEnd is not d.egEnd");
        }
        const auto gS = c->get_egl(pc.back());
        const auto gT = c->get_egl(pc[pc.size()-2]);
        if (!path.s.is_bulk() && gS.w == gT.w) {
            GRAFFINE_ASSERT(End::opp(shared_egEnd(cn[wS], gT.a, gS.a)) ==
                   cn[wS].g[gS.a].inner_endId(eS),
                   "opp of source's internal_egEnd is not its inner_endId");
        }
    }

    [[maybe_unused]] const auto sourceIsCycle = cn[wS].is_cycle();
    [[maybe_unused]] const auto sourceIsBulk = path.s.is_bulk();

    GRAFFINE_ASSERT(sourceIsBulk || sourceIsCycle ||
           (!sourceIsCycle && cn[wS].ngs[eS].num() == 0),
           "Source end is not disconnected");

    [[maybe_unused]] const auto lenS = path.length_over_source_chain();

    GRAFFINE_ASSERT((sourceIsBulk && lenS >= static_cast<size_t>(steps)) ||
           (!sourceIsBulk && path.drv().w == path.src().w) ||
           (!sourceIsCycle &&
            lenS >= static_cast<size_t>(steps)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<size_t>(steps)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_ON_2_H
