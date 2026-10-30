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
 * \file chains.h
 * \brief Creation, renaming and transfers of the chains of a component.
 * \details Part of the implementation of component.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHAINS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHAINS_H

#include "graffine/structure/elements/component.h"

namespace graffine::structure::detail {

// Create an empty chain and insert it into the component.
// Result:  an unconnected chain as a part of the component..
template <typename Ch>
constexpr auto Component<Ch>::
create_chain() -> Chain&
{
    cn.emplace_back(cn.num(), num_chains(), ind);
    auto& m = cn.back();

    GRAFFINE_ASSERT(!contains(m), "Appending chain which is already included");
    ww.push_back(m.idw);

    chis.include(m);

    return m;
}

// Create a chain of specific length and insert it into an empty component.
// Result: a chain as part of a single-chain component.
template <typename Ch>
constexpr auto Component<Ch>::
create_chain(
    const std::size_t len,
    EgIdG& indIni // ref
) -> Chain&
{
    GRAFFINE_ASSERT(!num_chains(), "Component is not empty");

    auto& m = create_chain();

    for (std::size_t i{}; i < len; ++i)
        append_edge_to_chain(Edge{indIni++}, m.idw);

    return m;
}

template <typename Ch>
constexpr void Component<Ch>::
rotate_chain(Chain& m, const std::size_t n)
{
    m.rotate(n);
    for (const auto& eg : m.g)
        egl[eg.indc].a = eg.indw;
}

template <typename Ch>
void Component<Ch>::
set_chain(
    Chain& m,
    const ChIdC idc,
    EgIdC& indc // ref
) noexcept
{
    m.set_cmpt(ind, idc);
    set_edges(m, indc);
}

template <typename Ch>
void Component<Ch>::
set_chains() noexcept
{
    EgIdC indc{};
    ChIdC idc{};
    for (const auto& w : ww)
        set_chain(cn[w], idc++, indc);
}

template <typename Ch>
void Component<Ch>::
reind_chains(ChIdC idc,
             EgIdC indc) noexcept
{
    for (const auto& w : ww)
        set_chain(cn[w], idc++, indc);
}

template <typename Ch>
template <bool updateChis>
void Component<Ch>::
rename_chain(const ChIdG f,
             const ChIdG t)
{
    if (std::find(ww.begin(), ww.end(), f) != ww.end())
        std::replace(ww.begin(), ww.end(), f, t);

    std::for_each(egl.begin(), egl.end(),
                  [&](EdgeDescr& g)
                  { if (g.w == f) g.w = t; });

    if constexpr (updateChis)
        chis.populate(cn, ww);
}

template <typename Ch>
constexpr auto Component<Ch>::
rem_unconnected_cycle_boundary_vertex() noexcept
    -> std::array<Edge*, 2>
{
    const auto w = ww[0];

    GRAFFINE_ASSERT(num_chains() == 1,
           "component ", ind, " is supposed to hold an unconnected cycle");
    GRAFFINE_ASSERT(cn[w].is_unconnected_cycle(),
           "chain ", w, " is supposed to be an unconnected cycle");

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    auto& eg1 = cn[w].end_edge(eA);
    auto& eg2 = cn[w].end_edge(eB);

    auto v = eg1.vertex_at_outer(eA);

    eg1.disconnect_end_outer(eA);
    eg2.disconnect_end_outer(eB);

    vv.remove_disconnected(v);

    return {&eg1, &eg2};
}

template <typename Ch>
constexpr auto Component<Ch>::
rem_chain_internal_vertex(const ChIdG w,
                          const EgIdA a) noexcept -> std::array<Edge*, 2>
{
    GRAFFINE_ASSERT(contains_chain(w), "chain ", w, " is not part of component ", ind);

    auto& m = cn[w];

    GRAFFINE_ASSERT(m.length() > 1, "Chain does not contain bulk vertices.");
    GRAFFINE_ASSERT(a > 0, "Not a bulk vertex at a: ", a, " in chain ", w);
    GRAFFINE_ASSERT(a < m.length(),
           "indw ", a, " not valid for chain ", w, " of length ", m.length());

    auto& eg1 = m.g[a - 1];
    auto& eg2 = m.g[a];

    auto v = eg1.vertex_at_outer(End::B);

    eg1.disconnect_end_outer(End::B);
    eg2.disconnect_end_outer(End::A);

    vv.remove_disconnected(v);

    return {&eg1, &eg2};
}

template <typename Ch>
void Component<Ch>::
append(Component&& other)
{
    EgIdC indc{num_edges()};
    ChIdC idc{num_chains()};

    GRAFFINE_ASSERT(other.ind != ind, "Appending identical component");

    for (const auto w : other.ww)
        set_chain(cn[w], idc++, indc);

    vv.append(other);

    std::move(other.egl.begin(), other.egl.end(), std::back_inserter(egl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}

template <typename Ch>
void Component<Ch>::
append(Component& other)
{
    EgIdC indc{num_edges()};
    ChIdC idc{num_chains()};

    GRAFFINE_ASSERT(other.ind != ind, "appending identical component");

    for (const auto w : other.ww)
        set_chain(cn[w], idc++, indc);

    vv.append(other);

    std::move(other.egl.begin(), other.egl.end(), std::back_inserter(egl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}

template <typename Ch>
void Component<Ch>::
append(const ChIdGs& vv)
{
    for (const auto& v : vv)
        append(cn[v]);
}

template <typename Ch>
void Component<Ch>::
append(Chains& mm)
{
    for (auto& m : mm)
        append(m);
}

template <typename Ch>
void Component<Ch>::
append(Chain& m)
{
    if constexpr (verboseF)
        jot("appending chain ", m.idw);

    EgIdC indc{num_edges()};
    const ChIdC idc{num_chains()};

    GRAFFINE_ASSERT(!contains(m), "appending chain which is already included");

    set_chain(m, idc, indc);

    for (const auto& g : m.g)
        egl.append(g);

    ww.push_back(m.idw);
    chis.include(m);
}

template <typename Ch>
void Component<Ch>::
append(Edges &g)
{
    for (auto& eg : g)
        append(eg);
}

template <typename Ch>
void Component<Ch>::
append(Edge& eg)
{
    GRAFFINE_ASSERT(!contains(eg), "appending edge which is already included");

    eg.set_cmp(ind, num_edges());
    set_egl();
}

template <typename Ch>
void Component<Ch>::
remove(const Edges &g)
{
    for (const auto& eg : g)
        remove(eg);
}

// Component-related data inside \p eg are not to be altered, hence const ref.
template <typename Ch>
void Component<Ch>::
remove(const Edge& eg)
{
    if constexpr (verboseF)
        jot("-removing edge ", eg.ind, " of chain ", eg.w, " cmp ", ind, ":");

    GRAFFINE_ASSERT(contains(eg),
           "attempt to remove a missing edge ", eg.ind, " from cmpt ", ind);
    GRAFFINE_ASSERT(cn[eg.w].length(), "removing an edge from empty chain ", eg.w);

    egl.remove(eg, cn, vv);
}

template <typename Ch>
void Component<Ch>::
remove(const Chain& m)
{
    if constexpr (verboseF)
        jot("removing chain ", m.idw, " from component ", ind);

    GRAFFINE_ASSERT(contains(m),
           "attempt to remove a missing chain ", m.idw, " from cmpt ", ind);

    Edges g = m.g; // copy

    // sort in descending order of indc
    std::ranges::sort(g, [](const Edge& a,
                            const Edge& b) { return a.indc > b.indc; });

    for (const auto& eg : g)
        remove(eg);

    if (const auto last = num_chains() - 1;
        m.idc < last)

    for (const auto& w : ww)
            if (cn[w].idc == last)
                cn[w].idc = m.idc;

    std::erase(ww, m.idw);
    set_edges();
    chis.populate(cn, ww);
    set_egl();
}

template <typename Ch>
void Component<Ch>::
remove(const Chains& mm)
{
    Edges g;
    for (const auto m : mm) // copy
        g.append_range(std::move(m.g));

    // sort in descending order of indc
    std::ranges::sort(g, [](Edge& a, Edge& b)
                      { return a.indc > b.indc; });

    GRAFFINE_ASSERT(std::adjacent_find(g.begin(), g.end()) == g.end(),
           "edges having identical indc found");

    for (const auto& eg : g)
        remove(eg);

    for (const auto& m : mm) {

        if (const auto last = num_chains() - 1;
            m.idc < last)

            for (const auto w : ww)
                if (cn[w].idc == last)
                    cn[w].idc = m.idc;

        std::erase(ww, m.idw);
        m.is_unconnected_cycle()
            ? chis.populate(cn, ww)
            : chis.remove(m);
    }
}

template <typename Ch>
void Component<Ch>::
remove(const ChIdGs& vv)
{
    for (const auto v : vv)
        remove(cn[v]);
}

template <typename Ch>
void Component<Ch>::
remove_disconnected(Vertex* vp)
{
    vv.remove_disconnected(vp); // deletes *vp: the caller's pointer dangles
}

template <typename Ch>
constexpr void Component<Ch>::
remove_vertex_at_edge_outer(
    Edge& eg,
    const EndId e) noexcept
{
    vv.remove_connected_to_outer(eg, e);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, Chain& m)
{
    vv.move_to(c, m);
    remove(m);
    c.append(m);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, Chains& mm)
{
    remove(mm);
    c.append(mm);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, ChIdGs&& uu)
{
    std::sort(uu.rbegin(), uu.rend());
    for (const auto& u : uu) {

        if constexpr (verboseF)
            jot("Moving ", u);

        move_to(c, cn[u]);
    }
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHAINS_H
