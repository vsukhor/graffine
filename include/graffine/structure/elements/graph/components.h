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
 * \file components.h
 * \brief Creation, merger and split of the components of a graph, and moves of its chains and edges.
 * \details Part of the implementation of graph.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_COMPONENTS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_COMPONENTS_H

#include "graffine/structure/elements/graph.h"

namespace graffine::structure::detail {

template<typename CC>
void Graph<CC>::
generate_single_chain_components(
    const std::size_t num,
    const std::size_t len
)
{
    const auto n = num_chains();  // number of chains already present

    while (num_compts() < num + n)
        add_single_chain_component(len);
    if constexpr (verboseF)
        jot("Generated ", colorcodes::GREEN, num, colorcodes::RESET,
            " identical single-chain components, to ", numEdges, " edges total");
}


template<typename CC>
void Graph<CC>::
add_single_chain_component(const std::size_t len)
{
    ct.emplace_back(num_compts(), cn);  // empty
    EgIdG indIni {numEdges};
    ct.back().create_chain(len, indIni);
    numEdges += len;

    update({ind_last_chain()});
}


template<typename CC>
void Graph<CC>::
add_component(Compt&& cmp)
{
    numEdges += cmp.num_edges();
    ct.push_back(std::move(cmp));

    update();
}

template<typename CC>
auto Graph<CC>::
split_component(
    Compt& cmp,
    const PathsOverEndSlots& pp,
    const EndSlot& s
) -> Compt&
{
    auto [accessible, blocked] =
        pp.classify_chains_by_connectivity(s);

    if constexpr (verboseF)
        cmp.template print_classification<false>(accessible, blocked);

    return split_component(cmp, std::move(blocked));
}

template<typename CC>
auto Graph<CC>::
split_component(Compt& cmp, ChIdGs&& rm) -> Compt&
{
    const auto indOld = cmp.ind;  // reallocation of 'ct' may invalidate ref
    auto& newcmp = ct.emplace_back(num_compts(), cn);  // empty component

    ct[indOld].move_to(newcmp, std::move(rm));  // move the chids to it

    return ct.back();
}

template<typename CC>
auto Graph<CC>::
split_component(Compt& cmp, Chain& m) -> Compt&
{
    const auto c = cmp.ind;  // reallocation of 'ct' may invalidate ref

    auto& newcmp = ct.emplace_back(num_compts(), cn);  // empty component

    ct[c].move_to(newcmp, m);  // move the chain m to it

    return ct.back();
}


template<typename CC>
auto Graph<CC>::
merge_components(
    const CmpId acceptor,
    const CmpId donor
) -> Compt&
{
    return merge_components(ct[acceptor], ct[donor]);
}


template<typename CC>
auto Graph<CC>::
merge_components(
    Compt& acceptor,
    Compt& donor
) -> Compt&
{
    const auto indDon = donor.ind;
    const auto indAcc = acceptor.ind;

    GRAFFINE_ASSERT(indDon != indAcc, "Merging component ", indAcc, " into itself.");

    if constexpr (verboseF)
        jot("Merging cmp into ", indAcc , " <-- ", indDon);

    const auto acceptor_is_last = (indAcc == ind_last_cmpt());

    donor.set_ind(indAcc);
    donor.reind_chains(acceptor.num_chains(), acceptor.num_edges());
    acceptor.vv.append(donor.vertices().get());
    acceptor.ww.insert(acceptor.ww.end(), donor.ww.begin(), donor.ww.end());
    donor.ww.clear();
    acceptor.set_chis();
    donor.clear_chis();
    acceptor.set_egl();
    donor.clear_egl();

    if (indDon != ind_last_cmpt()) {
        ct.back().set_ind(indDon);
        donor = std::move(ct.back());
    }
    ct.pop_back();

    auto& res = acceptor_is_last ? donor : acceptor;

    if constexpr (verboseF)
        acceptor_is_last ? jot("result of the component merge is donor")
                         : jot("result of the component merge is acceptor");

    if constexpr (verboseF)
        jot("Component merge produces ind ", res.ind, '\n',
            "Total number of components: ", num_compts());

    return res;
}

template<typename CC>
void Graph<CC>::
remove_edge(EgIdG ind)
{
    const auto& eg = edge(ind);
    const auto w = eg.w;
    [[maybe_unused]] const auto a = eg.indw;

    ct[eg.c].remove_edge(eg.indc);

    const auto indLast = numEdges - 1;
    auto wLast = w;  // chain of the renumbered last edge
    if (ind < indLast) {
        wLast = egl[indLast].w;
        const auto aLast =
            (wLast == w && egl[indLast].a > a) ? egl[indLast].a - 1
                                               : egl[indLast].a;
        auto& egLast = cn[wLast].g[aLast];
        egLast.ind = ind;
        ct[egLast.c].get_egl(egLast.indc).i = ind;
    }
    numEdges--;
    update({w, wLast});
}


template<typename CC>
constexpr
void Graph<CC>::
rotate_chain(Chain& m, const std::size_t n)
{
    ct[m.c].rotate_chain(m, n);

    for (const auto& eg : m.g)
        egl[eg.ind].a = eg.indw;
}


template<typename CC>
constexpr
void Graph<CC>::
move_edges(
    Chain& fr,
    const std::size_t posFrBegin,
    const std::size_t posFrEnd,  // position after the last moved element
    Chain& to,
    const std::size_t posTo  // position of the first inserted element
)
{
    // Does not update chain connectivity and 'ngs'.
    // Does not remove source chain if it is emptied.

    GRAFFINE_ASSERT(fr.c == to.c,
            "Source chain component is different from that of the target");

    ct[fr.c].move_edges(fr, posFrBegin, posFrEnd, to, posTo);

    for (std::size_t i = posTo; i < to.length(); ++i) {
        const auto& tg = to.g[i];
        egl[tg.ind].set(to.idw, tg.indw);
    }
}


template<typename CC>
template<bool updateChis>
void Graph<CC>::
rename_chain(const ChIdG f, const ChIdG t)
{
    for (const auto e : End::Ids)
        copy_neigs(EndSlot{f, e}, EndSlot{t, e});

    cn[f].set_g_w(t);  // should be called before the move of g
    cn[t].g = std::move(cn[f].g);
    cn[f].g.clear();
    cn[t].c = cn[f].c;
    cn[t].idc = cn[f].idc;
    ct[cn[f].c].template rename_chain<updateChis>(f, t);
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_COMPONENTS_H
