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
 * \file functor_base.h
 * \brief Base class for deriving degree-specific functors performing pulling.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_BASE_H
#define GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_BASE_H

#include "graffine/definitions.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_deletion/deleting_host_chain.h"
#include "graffine/transforms/pulling/path.h"

#include <utility>  // move
#include <memory>
#include <type_traits>

namespace graffine::transforms::pulling {

/**
 * Base for deriving functor classes pulling a specific graph edge.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct FunctorBase
{
    using Trait = pulling::Trait<G>;
    using Graph = Trait::Graph;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using Components = Graph::Components;
    using PropertyProcessor = PP;
    using Path = pulling::Path<Compt>;
    using PathW = typename Path::PathW;
    using Source = typename Path::Source;

    /**
     * Base class for deriving functors handling degree-specific vertex pulling.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit FunctorBase(Graph& gr,
                         std::shared_ptr<PP> pp = nullptr);

protected:

    /**
     * Checks that \p path refers to this graph, with valid chains, and that
     * \p steps is positive.
     */
    auto check_path_basics(const Path& path,
                           int steps) const -> Applicability;

    /**
     * Checks that the source end of \p path is free, unless the source is a
     * cycle, and that the source chain is long enough for \p steps.
     * \details A degree 2 source should end the path, which should cover at
     * least \p steps edges of the source chain, and the source chain should
     * be shrinkable by \p steps edges.
     */
    auto check_source(const Path& path,
                      int steps) const -> Applicability;

    /**
     * End of the last edge of \p path at the source vertex.
     * \details Is the end of the edge itself, so remains valid as the edge
     * moves between chains.
     */
    auto source_egEnd(const Path& path) const -> End::Id;

    /**
     * Source at end \p egeS of the edge \p iS, which is the last path edge.
     * \details A degree 2 source which is reached by the junction becomes the
     * chain end at this junction.
     * \param[in] iS Graph-wide index of the source edge.
     * \param[in] egeS End of the source edge at the source vertex.
     * \param[in] isBulk The source is a degree 2 vertex.
     */
    auto source_at(EgIdG iS,
                   End::Id egeS,
                   bool isBulk) const -> Source;

    Graph&         gr;  ///< Reference to the modified graph object
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    const Graph::EdgeDescriptors& egl;

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /**
     * Pull \p steps times a sequence of consecutive edges defined by \p path .
     * \note The driver vertex is expected to be a leaf node.
     */
    bool pull_leaf_vertex(Path& path, int steps);
    /**
     * Pull once a sequence of consecutive edges defined by \p path .
     * \overload
     * \tparam notify Calls pulling.on_end after the step. False for a step
     * which the caller completes with further topology changes, and so calls
     * pulling.on_end itself.
     */
    template<bool notify = true>
    bool pull_leaf_vertex(Path& path);

    bool chain_exit_path(
        PathW::const_reverse_iterator& rit,
        const PathW::const_reverse_iterator& rend
    ) const noexcept;

private:

    // Auxiliary functors:

    // Creation of an edge at a free end of an existing chain.
    edge_creation::InExistingChain<Graph, PP> create_edge;
    // Deletion of a single-edge chain having one end free:
    // for chains having the other end connected via a vertex of degree 3
    edge_deletion::DeletingHostChain<Deg3, Graph, PP> delete_edge_3;
    // for chains having the other end connected via a vertex of degree 4
    edge_deletion::DeletingHostChain<Deg4, Graph, PP> delete_edge_4;

    void dissolve_single_edge_chain(Path& path);

    void shift_edges_to_target_chain(const Path& path);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto FunctorBase<G, PP>::
check_path_basics(
    const Path& path,
    const int steps
) const -> Applicability
{
    const auto c = path.cmp;
    if (!c || c->ind >= gr.num_compts() || &gr.compt(c->ind) != c)
        return reject("path does not refer to a component of the graph");
    if (path.d.w >= gr.num_chains() || path.s.w >= gr.num_chains())
        return reject("path refers to invalid chains");
    if (path.s.is_bulk() ? !Path::is_bulk_position(cn[path.s.w], path.s.pos)
                         : !End::is_valid(path.s.e))
        return reject("path refers to an invalid source");
    if (steps < 1)
        return reject("number of steps is not positive");

    return {};
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto FunctorBase<G, PP>::
check_source(
    const Path& path,
    const int steps
) const -> Applicability
{
    const auto& m = cn[path.s.w];
    const auto sourceIsCycle = m.is_cycle();

    if (path.s.is_bulk()) {
        const auto vv = path.vertices();
        const auto& vS = *m.vertex_at_bulk(path.s.pos);
        if (vv.empty() || !(*vv.back() == vS))
            return reject("path does not end at the source");
        if (path.length_over_source_chain() < static_cast<std::size_t>(steps))
            return reject("path over the source chain is too short for the steps");
        if (!m.is_shrinkable_by(static_cast<std::size_t>(steps)))
            return reject("source chain is not shrinkable by the steps");

        return {};
    }

    if (!sourceIsCycle && m.ngs[path.s.e].num())
        return reject("source end is not free");

    const auto need = static_cast<std::size_t>(steps) +
                      (sourceIsCycle ? Chain::minCycleLength : 0);
    if (path.length_over_source_chain() < need)
        return reject("path over the source chain is too short for the steps");

    return {};
}

template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto FunctorBase<G, PP>::
source_egEnd(const Path& path) const -> End::Id
{
    const auto iS = path.pthw.back();
    const auto& m = cn[path.s.w];
    const auto a = egl[iS].a;

    // the chain end of the source edge at the source vertex
    const auto e = !path.s.is_bulk() ? path.s.e
                 : a() + 1 == path.s.pos() ? End::B
                                           : End::A;

    return m.g[a].inner_endId(e);
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto FunctorBase<G, PP>::
source_at(
    const EgIdG iS,
    const End::Id egeS,
    const bool isBulk
) const -> Source
{
    const auto wS = egl[iS].w;
    const auto& m = cn[wS];

    if (!isBulk)
        return EndSlot {wS, cn.edge(egl[iS]).points_forwards() ? egeS
                                                             : End::opp(egeS)};

    const auto bs = m.egEnd_to_bulkslot(egeS, egl[iS].a);

    return Path::is_bulk_position(m, bs.a())
         ? Source {bs}
         : Source {EndSlot {wS, bs.a()() ? End::B : End::A}};
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
FunctorBase<G, PP>::
FunctorBase(Graph& gr,
            std::shared_ptr<PP> pp)
    : gr {gr}
    , cn {gr.chains()}
    , egl {gr.get_egl()}
    , updateProperties {pp}
    , create_edge {gr, pp}
    , delete_edge_3 {gr, pp}
    , delete_edge_4 {gr, pp}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
bool FunctorBase<G, PP>::
pull_leaf_vertex(
    Path& path,
    int steps)
{
    bool source_was_dissolved {};

    while (steps-- > 0) {
        source_was_dissolved = pull_leaf_vertex(path);

        if (steps > 0 && source_was_dissolved) {
            const auto& d = path.drv();

            GRAFFINE_ENSURE(d.w == path.src().w, "steps > 0 && source_was_dissolved");

            const auto eS = End::opp(cn[d.w].ind2end(d.ind, d.egEnd));
            path.set_src(EndSlot {d.w, eS});
        }

        GRAFFINE_ASSERT(!path.cmp->template check<verboseF>(), "Intermediate cmp check.");
        GRAFFINE_ASSERT(path.is_contiguous(), "Path is broken by the pull step.");
    }

    return source_was_dissolved;
}

// Pull one step.
template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
template<bool notify>
bool FunctorBase<G, PP>::
pull_leaf_vertex(Path& path)
{
    const auto wS = path.s.w;  // source chid

    if (path.d.w == wS) {  // driver and source belong to the same chain

        if constexpr (notify && !std::is_void_v<PP>)
            updateProperties->pulling.on_end(path);

        return false;      // no dissolving of the source edge
    }
    if constexpr (verboseF) {

        jot(colorcodes::GREEN, "---Pulling free end over path: ",
            colorcodes::RESET);
        path.template print_short<true>("");
//        path.template print_detailed("");
    }

    bool is_dissolved {};

    // a degree 2 source is kept track of by its edge
    const auto iS = path.pthw.back();
    const auto egeS = path.s.is_bulk() ? source_egEnd(path) : End::undefined;

    if (!path.s.is_bulk() && cn[wS].length() == 1) {  // the whole source chain will disappear

        dissolve_single_edge_chain(path);
        is_dissolved = true;
    }

    shift_edges_to_target_chain(path);

    // edge descriptors were updated in 'dissolve_single_edge_chain()' and in
    // 'shift_edges_to_target_chain()', so here vertex descriptors are to update
    gr.update_vertex_descriptors();

    if (path.s.is_bulk())
        path.set_src(source_at(iS, egeS, true));

    if constexpr (notify && !std::is_void_v<PP>)
        updateProperties->pulling.on_end(path);

    if constexpr (verboseF) {

        jot<false>(colorcodes::GREEN, "---Finised pulling free end.",
                   colorcodes::RESET);
        const auto txt = (is_dissolved ? "Source was dissolved"s : ""s);
        jot(colorcodes::YELLOW, txt, colorcodes::RESET);
    }

    return is_dissolved;
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
void FunctorBase<G, PP>::
dissolve_single_edge_chain(Path& path)
{
    const auto iD = path.d.ind;
    auto wS = path.s.w;

    const auto& pthw = path.pthw;

    if constexpr (verboseF)
        jot(colorcodes::GREEN, "------Consuming chain ", colorcodes::YELLOW, wS,
            ":", colorcodes::RESET);

    GRAFFINE_ASSERT(cn[wS].length() == 1, "trailing chain is too long: ", wS);
    GRAFFINE_ASSERT(!path.s.is_bulk(), "dissolving the chain of a degree 2 source");

    const auto iS = cn[wS].g[0].ind;  // since cn[wS].length() == 1
    auto& cnS = path.cmp->chain(wS);
    const auto egeS = cnS.g[egl[iS].a].inner_endId(path.s.e);

    create_edge(path.s.end_slot());  // create an edge at the free end of the source chain

    const auto connectedSlot = path.s.opp();  // the only connected source slot

    const auto iN = pthw[pthw.size()-2];    // edge next to the source
    const auto wN = egl[iN].w;
    const auto eN = cn[wN].ind2end(iN, connectedSlot);

    auto& cmp = gr.compt(path.cmp->ind);

    cmp.jShifter(cmp, connectedSlot, EndSlot{wN, eN});

    gr.update_edge_descriptors();

    const auto& ns = cn[wS].ngs[connectedSlot.e];
    const auto nnS = ns.num();

    GRAFFINE_ASSERT(nnS == 2 || nnS == 3,
           "trailing chain connection degree is not correct: ", nnS);

    nnS == 2 ? delete_edge_3(connectedSlot)
             : delete_edge_4(connectedSlot);

    // Update the path based on the original edge index of the source
    wS = egl[iS].w;

    auto eS = cn.edge(egl[iS]).points_forwards()
            ? egeS
            : End::opp(egeS);

    path = Path {&gr.compt(wS),
                 typename Path::Driver {&gr.edge(iD), path.d.egEnd},
                 typename Path::Source {EndSlot{wS, eS}},
                 path.pthw};
}


template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
void FunctorBase<G, PP>::
shift_edges_to_target_chain(const Path& path)
{
    const auto& d = path.d;
    const auto& pthw = path.pthw;

    GRAFFINE_ASSERT(d.ind == pthw[0],
           "driver edge is not the first one in the path");
    GRAFFINE_ASSERT(End::is_defined(cn[d.w].ind2end(d.ind, path.d.egEnd)),
           "driver is not a chain end");
    GRAFFINE_ASSERT(cn[d.w].end2ind(path.driver_chain_end()) == d.ind,
           "driver chain end ind differs from driver ind ");

    // path.front() is a pulling edge which is alwais a free end
    const auto w0 = d.w;

    // cn[w2].length()>1 must be ensured before, because shift_chain_junction()
    // will move an end edge to the adjoint chain upwards in the path
    const auto w2 = path.s.w;

    if (w0 != w2) {
        // 'rit' points to the last element and moves backwards upon incrementing
        auto rit = pthw.rbegin();
        const auto rend = pthw.rend();
        // on return from chain_exit_path() 'rit' points to the last edge
        // of the current chain in 'path'; returns true iff *rit==path[0]
        while(!chain_exit_path(rit, rend)) {
            auto leadingInd = *rit;
            auto indN = *(++rit);  // here rit switches to a consecutive chain

            const auto [f, t] = gr.inds_to_chain_link(leadingInd, indN);

            GRAFFINE_ASSERT(f.is_defined(),
                   "leadingInd = ", leadingInd, " and indN = ", indN,
                   " are not ends of connected chains");

//            gr.ct[path.cmp->ind].shift_chain_junction(f, t);
            auto& cmp = gr.compt(path.cmp->ind);
            cmp.jShifter(cmp, f, t);
            gr.update_edge_descriptors();
        }
    }
}


/// on return, \p rit points to the last edge of the current chain in \p path
template<typename G,
         typename PP>
    requires pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
bool FunctorBase<G, PP>::
chain_exit_path(
    PathW::const_reverse_iterator& rit,
    const PathW::const_reverse_iterator& rend
) const noexcept
{
    const auto w = egl[*rit].w;

    bool firstElement;

    // upon execution, firstElement == true iff rit reached the rend() of path,
    // i.e. the pulling edge
    do firstElement = ++rit == rend;
    while (!firstElement && egl[*rit].w == w);

    --rit;   // point rit back to the ending edge in path belonging to w

    // the last edge of the current chain to which 'it' now points
    // is the first path element
    return firstElement;
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_BASE_H
