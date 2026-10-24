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
 * \file print.h
 * \brief Printing of a component.
 * \details Part of the implementation of component.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_PRINT_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_PRINT_H

#include "graffine/structure/elements/component.h"

namespace graffine::structure::detail {

template <typename Ch>
template <bool isCycle>
void Component<Ch>::
print_classification(const ChIdGs& accessible,
                     const ChIdGs& blocked) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_chains(), " chains");

    ChIdGs all(num_chains(), ChIdG::undefined);
    for (const auto& a : accessible) {

    const std::size_t i = std::distance(ww.begin(),
                                            std::find(ww.begin(), ww.end(), a));
         GRAFFINE_ASSERT(i < ww.size(), "element of 'accessible' is not in component");
        all[i] = a;
    }
    constexpr auto colorAccessible = colorcodes::GREEN;
    constexpr auto colorBlocked = colorcodes::RED;
    jot<false>(colorcodes::CYAN, "all: ", colorcodes::RESET);
    for (std::size_t j{}; j < all.size(); ++j)
        jot<false>(all[j].is_defined() ? colorAccessible
                                       : colorBlocked,
                   ww[j],
                   colorcodes::RESET,
                   j == all.size() - 1 ? "" : ", ");
    jot("");

    for (int i{}; auto& v : {accessible, blocked}) {

        jot<false>(colorcodes::CYAN, i == 0 ? "accessible: " : "blocked: ",
                   colorcodes::RESET);
        for (std::size_t j{}; auto w : v)
            jot<false>(i == 0 ? colorAccessible : colorBlocked, w,
                       ++j == v.size() ? "" : ", ");
        jot(colorcodes::RESET);
        ++i;
    }
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print(Args... args) const
{
    print_chains(args...);
    print_vertices("");
    print_ww<false>();
    print_egl();
    chis.print();
    jot("");
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print_chains(Args... args) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_edges(), " edges in ", num_chains(), " chains");

    for (const auto& j : ww)
        cn[j].print(args...);
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print_vertices(Args&&... args) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_vertices(), " vertices");

    vv.print(args...);
}

template <typename Ch>
void Component<Ch>::
print_vertices_deg(
    const Degree d,
    const std::string_view s) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_vertices(), " vertices of degree ", d);

    vv.print_degree(d, s);
}

template <typename Ch>
void Component<Ch>::
print_egl() const
{
    jot("egl ", ind, ": w a ind id");
    for (std::size_t i {}; i < egl.size(); ++i)
        egl[i].print(i);
}

template <typename Ch>
template <bool with_top>
void Component<Ch>::
print_ww() const
{
    if constexpr (with_top)
        jot<false>("cmpt ", ind, " ");
    jot<false>("ww: ");
    for (const auto& w : ww)
        jot<false>(w, " ");

    jot("");
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_PRINT_H
