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
 * \file neighbours.h
 * \brief Connections between the chains of a graph.
 * \details Part of the implementation of graph.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_NEIGHBOURS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_NEIGHBOURS_H

#include "graffine/structure/elements/graph.h"

namespace graffine::structure::detail {

template<typename CC>
void Graph<CC>::
copy_neigs(
    const EndSlot& f,
    const EndSlot& t)
{
    ngs_at(t) = ngs_at(f);

    // Substitute f in f's neig's neigs for t:
    replace_slot_in_neigs(f, t);
}


template<typename CC>
void Graph<CC>::
remove_slot_from_neigs(const EndSlot& s)
{
    const auto sc = ngs_at(s);  // copy
    auto&      sr = ngs_at(s);  // ref

    for (auto& ne : sc()) {

        auto& ngs = ngs_at(ne);

        // Remove oldn from the neig list of its j-th neig
        const auto ngsOK = ngs.erase(s);

        GRAFFINE_ENSURE(ngsOK, "EndSlot {", s.w, " ", s.e, "} not found among",
                      "ngs of the slot {", ne.w, " ", ne.e, " connected to it");

        // Remove the j-th neig from the oldn's list of neigs
        const auto neOK = sr.erase(ne);

        GRAFFINE_ENSURE(neOK, "EndSlot {", ne.w, " ", ne.e, "} not found among",
                     "ngs of the slot {", s.w, " ", s.e, " connected to it");
    }
}


/// slots connected to 'nov' will be checked, and if there is 'old',
/// it will be replaced by 'nov'
template<typename CC>
void Graph<CC>::
replace_slot_in_neigs(
    const EndSlot& old,
    const EndSlot& nov)
{
    const auto& nov_ngs = ngs_at(nov);

    for (auto& ne : nov_ngs()) {

        auto& ne_ngs = ngs_at(ne);

        const auto ok = ne_ngs.replace(old, nov);

        GRAFFINE_ENSURE(ok, "EndSlot {", old.w, " ", old.e, "} not found among",
                    "ngs of the slot {", ne.w, " ", ne.e, " connected to it");
    }
}


template<typename CC>
constexpr
void Graph<CC>::
connect(const EndSlot& s1,
        const EndSlot& s2)
{
    cn[s1.w].ngs[s1.e].insert(s2);
    cn[s2.w].ngs[s2.e].insert(s1);
}


template<typename CC>
constexpr
auto Graph<CC>::
ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}


template<typename CC>
constexpr
auto Graph<CC>::
ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

template<typename CC>
constexpr
auto Graph<CC>::
ng_inds_at(const EndSlot& s) const noexcept -> EgIdGs
{
    const auto& ngs = ngs_at(s);

    EgIdGs inds(ngs.num());

    for (std::size_t i {}; i<ngs.num(); ++i)
        inds[i] = slot2ind(ngs[i]);

    return inds;
}

template<typename CC>
constexpr
bool Graph<CC>::
is_same_cycle(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept
{
    return cn[s1.w].is_cycle() && s1 == s2.opp();
}


template<typename CC>
auto Graph<CC>::
inds_to_chain_link(
    const EgIdG ind1,
    const EgIdG ind2
) const noexcept -> std::array<EndSlot, 2>
{
    const auto w = egl[ind1].w;

    for (const auto e : End::Ids) {
        if (cn[w].end2ind(e) == ind1) {
            auto s1 = EndSlot{w, e};
            for (const auto& s2 : cn[w].ngs[e]())
                if (cn[s2.w].end2ind(s2.e) == ind2)
                    for (const auto& n2 : cn[s2.w].ngs[s2.e]())
                        if (n2 == s1)
                            return {s1, s2};

        }
    }
    return {{}};
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_NEIGHBOURS_H
