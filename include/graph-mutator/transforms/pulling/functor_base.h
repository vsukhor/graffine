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
 * @file functor_base.h
 * @brief Base class for deriving degree-specific functors performing pulling.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_FUNCTOR_BASE_H
#define GRAPH_MUTATOR_PULLING_FUNCTOR_BASE_H

#include <memory>  // unique_ptr

#include "../../definitions.h"
#include "../../structure/graph.h"
#include "../edge_creation/in_existing_chain.h"
#include "../edge_deletion/deleting_host_chain.h"
#include "paths.h"


namespace graph_mutator::pulling {

/**
 * @brief Base for deriving functor classes for pulling a specific graph edge.
 * @tparam G Graph to which the transformation is applied.
 */
template<typename G>
struct FunctorBase {

    static_assert(std::is_base_of_v<graph_mutator::structure::GraphBase, G>);

    using Graph = G;
    using Cmpt = Graph::Cmpt;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using Components = Graph::Components;
    using Ps = Paths<Cmpt>;
    using Path = typename Ps::Path;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit FunctorBase(Graph& gr);


protected:

    Graph&         gr;  ///< Reference to the graph object
    Components&    ct;  ///< Reference to the graph components.
    Graph::Chains& cn;  ///< Reference to the graph edge chains.

    const ChIds& glm;
    const EgIds& gla;

    auto pull_free_end(Ps& pp, int s) -> bool;
    auto pull_free_end(Ps& pp) -> bool;

    auto chain_exit_path(
        Path::const_reverse_iterator& rit,
        const Path::const_reverse_iterator& rend
    ) const noexcept -> bool;

private:

    // Auxiliary functors

    // Creation of an edge at a free end of an existing chain.
    edge_creation::InExistingChain<Deg1, Graph> create_edge;

    // Deletion of a single-edge chain having one end free:
    // for chains having the other end connected via a vertex of degree 3
    edge_deletion::DeletingHostChain<Deg3, Graph> delete_edge_3;
    // for chains having the other end connected via a vertex of degree 4
    edge_deletion::DeletingHostChain<Deg4, Graph> delete_edge_4;

    void dissolve_single_edge_chain(Ps& pp);

    void shift_edges_to_target_chain(const Ps& pp);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
FunctorBase<G>::
FunctorBase(Graph& gr)
    : gr {gr}
    , ct {gr.ct}
    , cn {gr.cn}
    , glm {gr.glm}
    , gla {gr.gla}
    , create_edge {gr}
    , delete_edge_3 {gr}
    , delete_edge_4 {gr}
{}


template<typename G>
auto FunctorBase<G>::
pull_free_end(
    Ps& pp,
    int s
) -> bool
{
    bool source_was_dissolved {};

    while (s-- > 0) {
        source_was_dissolved = pull_free_end(pp);

        if (s > 0 && source_was_dissolved) {
            const auto& d = pp.drv();

            ENSURE(d.w == pp.src().w, "s > 0 && source_was_dissolved");

            const auto eS = Ends::opp(cn[d.w].ind2end(d.ind, d.egEnd));
            pp.set_src(EndSlot {d.w, eS});
        }
    }

    return source_was_dissolved;
}


template<typename G>
auto FunctorBase<G>::
pull_free_end(Ps& pp) -> bool
{
    const auto wS = pp.s.w;  // source chid

    if (pp.d.w == wS)   // driver and source belong to the same chain
        return false;

    if constexpr (verboseF) {
        log_(colorcodes::GREEN, "---Pulling free end over path: ",
             colorcodes::RESET);
        pp.template print_short<true>("");
//        pp.template print_detailed("");
    }

    bool is_dissolved {};

    if (cn[wS].length() == 1) {  // the whole source chain will disappear
        dissolve_single_edge_chain(pp);
        is_dissolved = true;
    }

    shift_edges_to_target_chain(pp);

    if constexpr (verboseF) {
        log_<false>(colorcodes::GREEN, "---Finised pulling free end.",
             colorcodes::RESET);
        const auto txt = is_dissolved ? "Source was dissolved"s : ""s;
        log_(colorcodes::YELLOW, txt, colorcodes::RESET);
    }

    return is_dissolved;
}


template<typename G>
void FunctorBase<G>::
dissolve_single_edge_chain(Ps& pp)
{
    const auto iD = pp.d.ind;
    auto wS = pp.s.w;

    const auto& pth = pp.pth;

    log_(colorcodes::GREEN, "------Consuming chain ", colorcodes::YELLOW, wS,
         ":", colorcodes::RESET);

    ASSERT(cn[wS].length() == 1, "trailing chain is too long: ", wS);

    const auto iS = cn[wS].g[0].ind;  // since cn[wS].length() == 1
    auto& cnS = pp.cmp->chain(wS);
    const auto egeS = cnS.g[gla[iS]].oriented_end(pp.s.e);

    create_edge(pp.s);  // create an edge at the free end of the source chain

    const auto connectedSlot = pp.s.opp();  // the only connected source slot

    const auto iN = pth[pth.size()-2];    // edge next to the source
    const auto wN = glm[iN];
    const auto eN = cn[wN].ind2end(iN, connectedSlot);

    const EgId a = eN == Ends::A
                 ? 0
                 : cn[wN].length();  // !!! not cn[wN].length()-1 because of
                                     // how insert_edge() works

    auto eg = cn[wS].g[gla[iS]];  // copy
    cn[wN].insert_edge(std::move(eg), a);
    cn[wS].remove_edge(cn[wS].end2a(connectedSlot.e));
    gr.ct[pp.cmp->ind].set_gl();
    gr.update_books();

    const auto& ns = cn[wS].ngs[connectedSlot.e];
    const auto nnS = ns.num();

    ASSERT(nnS == 2 || nnS == 3,
           "trailing chain connection degree is not incorrect: ", nnS);

    nnS == 2 ? delete_edge_3(connectedSlot)
             : delete_edge_4(connectedSlot);

    // Update the path based on the original edge index of the source
    wS = glm[iS];

    auto eS = cn[wS].g[gla[iS]].points_forwards()
            ? egeS
            : Ends::opp(egeS);

    pp = Ps {&ct[cn[wS].c],
             typename Ps::Driver {&cn[glm[iD]].g[gla[iD]], pp.d.egEnd},
             typename Ps::Source {EndSlot{wS, eS}},
             pp.pth};
}


template<typename G>
void FunctorBase<G>::
shift_edges_to_target_chain(const Ps& pp)
{
    const auto& d = pp.d;
    const auto& pth = pp.pth;

    ASSERT(d.ind == pth[0],
           "driver edge is not the first one in the path");
    ASSERT(is_defined(cn[d.w].ind2end(d.ind, pp.driver_end())),
           "driver is not a chain end");
    ASSERT(cn[d.w].end2ind(pp.driver_end()) == d.ind,
           "driver chain end ind differs from driver ind ");

    // path.front() is a pulling edge which is alwais a free end
    const auto w0 = d.w;

    // cn[w2].length()>1 must be ensured before, because shift_end_edge()
    //will shift an end edge to the next chain upwards in the path
    const auto w2 = pp.s.w;

    if (w0 != w2) {
        // rit points to the last element and moves backwards upon incrementing
        auto rit = pth.rbegin();
        const auto rend = pth.rend();
        // on return from mitoExit_path() 'rit' points to the last edge
        // of the current chain in 'path'; returns true iff *rit==path[0]
        while(!chain_exit_path(rit, rend)) {
            auto leadingInd = *rit;
            auto indN = *(++rit);  // here rit switches to a consecutive chain

            const auto [f, t] = gr.inds_to_chain_link(leadingInd, indN);

            ASSERT(f.is_defined(),
                   "leadingInd = ", leadingInd, " and indN = ", indN,
                   " are not ends of connected chains");

            gr.ct[pp.cmp->ind].shift_end_edge(f, t);
            gr.update_books();
        }
    }
}


/// on return, \p rit points to the last edge of the current chain in \p path
template<typename G>
auto FunctorBase<G>::
chain_exit_path(
    Path::const_reverse_iterator& rit,
    const Path::const_reverse_iterator& rend
) const noexcept -> bool
{
    const auto w = glm[*rit];

    bool firstElement;

    // upon execution, firstElement == true iff rit reached the rend() of path,
    // i.e. the pulling edge
    do firstElement = ++rit == rend;
    while (!firstElement && glm[*rit] == w);

    --rit;   // point rit back to the ending edge in path belonging to w

    // the last edge of the current chain to which 'it' now points
    // is the first path element
    return firstElement;
}


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_FUNCTOR_BASE_H
