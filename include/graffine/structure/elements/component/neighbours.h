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
 * \brief Queries and updates of the connections between chains of a component.
 * \details Part of the implementation of component.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_NEIGHBOURS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_NEIGHBOURS_H

#include "graffine/structure/elements/component.h"

namespace graffine::structure::detail {

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Edge& eg,
      const EndId eInner) const noexcept -> std::vector<Edge*>
{
    GRAFFINE_ASSERT_CALLING(edges().contains(eg), eg.print("ERROR"),
                   "Vertex is not part of component ", ind);

    std::vector<Edge*> gg;
    const auto v = eg.vertex_at_inner(eInner);
    auto vedEg = typename Vertex::EgDescr{eg.id, eg.indc, eInner};
    for (const auto& vedNg : v->edges_connected_to(vedEg)) {

        const auto& ed = egl[vedNg.indc];
            gg.push_back(&cn[ed.w].g[ed.a]);
    }

    return gg;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Vertex& v) const noexcept -> std::vector<Vertex*>
{
    GRAFFINE_ASSERT_CALLING(vv.contains(v), v.print("ERROR"),
                   "Vertex is not part of component ", ind);

    std::vector<Vertex*> vv;
    for (const auto& edv : v.edges())
        vv.push_back(edge(egl[edv.w]).vertex_at_inner(edv.e));

    return vv;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Chain& m, const EndId e) const noexcept -> std::vector<Chain*>
{
    GRAFFINE_ASSERT_CALLING(std::find(ww.begin(), ww.end(), m.idw) != ww.end(),
                   m.print("ERROR"), "Vertex is not part of component ", ind);

    std::vector<Chain*> mm;
    for (const auto& n : m.ngs[e])
        mm.push_back(&cn[n.w]);

    return mm;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

/// slots connected to 'nov' will be checked, and if there is 'old',
/// it will be replaced by 'nov'
template <typename Ch>
void Component<Ch>::
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

template <typename Ch>
void Component<Ch>::
remove_slot_from_neigs(const EndSlot& s)
{
    const auto sc = ngs_at(s); // copy
    auto& sr = ngs_at(s);      // ref

    for (auto& ne : sc()) {

        auto& ngs = ngs_at(ne);

        // Remove oldn from the neig list of its j-th neig
        const auto ngsOK = ngs.remove(s);

        GRAFFINE_ENSURE(ngsOK, "EndSlot {", s.w, " ", s.e, "} not found among",
               "ngs of the slot {", ne.w, " ", ne.e, " connected to it");

        // Remove the j-th neig from the oldn's list of neigs
        const auto neOK = sr.remove(ne);

        GRAFFINE_ENSURE(neOK, "EndSlot {", ne.w, " ", ne.e, "} not found among",
               "ngs of the slot {", s.w, " ", s.e, " connected to it");
    }
}

// Shift an edge from f.e end of f.w to t.e end of t.w
template<typename Ch>
void Component<Ch>::
shift_chain_junction(const EndSlot& f,
                     const EndSlot& t)
{
    auto& g0 = cn[f.w].g;
    auto& g1 = cn[t.w].g;

    GRAFFINE_ASSERT(cn[f.w].c == cn[t.w].c, "Slots belong to differrent components");

    if (f.e == End::B) {

        if (t.e == End::A) g1.insert(g1.begin(), std::move(g0.back()));
        else                g1.push_back(std::move(g0.back()));
        g0.pop_back();
    }
    else {  // ef == 1

        if (t.e == End::A) g1.insert(g1.begin(), std::move(g0.front()));
        else                g1.push_back(std::move(g0.front()));
        g0.erase(g0.begin());
    }

    cn[f.w].set_g_w();
    cn[t.w].set_g_w();
    set_egl();
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_NEIGHBOURS_H
