/* =============================================================================

This file is part of graph-mutator, a lightweight graph transformation library.

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
 * \brief Printing and output of a chain.
 * \details Part of the implementation of chain.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_PRINT_H
#define GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_PRINT_H

#include "graffine/structure/elements/chain.h"

namespace graffine::structure::detail {

template<typename E>
template<bool endLine, typename... T>
auto Chain<E>::
print_connectivity(T&&... tag) const -> std::size_t
{
    std::string s;
    if constexpr (sizeof...(T))
        s = Msgr::oss(tag...).str();

    jot<false>(colorcodes::MAGENTA, s, " ", colorcodes::BOLDRED,
               idw, colorcodes::RESET);

    ngs[endA].print();
    jot<false>("**");
    ngs[endB].print();

    if constexpr (endLine)
        jot("");

    return s.length();
}


template<typename E>
template<typename... T>
void Chain<E>::
print(T&&... tag) const
{
    const auto tag_length = print_connectivity<false>(tag...);

    jot<false>(colorcodes::BOLDCYAN, c, colorcodes::RESET, " [", idc, "]");

    jot<false>(" len ", colorcodes::GREEN, length(), colorcodes::RESET);

    for (std::size_t i {}; i<length(); ++i) {
        if (i && i % 10 == 0)
            jot<false>(' ');
        jot<false>(g[i].points_forwards() ? '>' : '<');
    }

    jot<false>('(');
    for (const auto& eg : g)
        jot<false>(eg.ind);
    jot(')');

    if constexpr (printEdges) {
        const std::string spaces(tag_length, ' ');
        for (const auto& eg : g)
            eg.print(spaces);
    }
    if constexpr (printVertices)
        for (EgIdA aa {}; const auto v : vertices()) {
            const auto a = aa() / 2;  // integer division
            jot<false>("[",
                       colorcodes::YELLOW,
                       a, " ",
                       (aa() % 2) ? End::str(endB) : End::str(endA),
                       colorcodes::RESET,
                       " ind ", colorcodes::GREEN, g[a].ind, colorcodes::RESET,
                       " indc ", colorcodes::MAGENTA, g[a].indc, colorcodes::RESET,
                       "]: ");
            v ? v->print("") : jot("unconnected");
            ++aa;
        }
}


template<typename E>
void Chain<E>::
write(std::ofstream& ofs) const
{
    const auto len = length();
    ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
    ofs.write(reinterpret_cast<const char*>(&idw), sizeof(ChIdG));
    ofs.write(reinterpret_cast<const char*>(&idc), sizeof(ChIdC));
    ofs.write(reinterpret_cast<const char*>(&c), sizeof(CmpId));

    for (const auto& ng : ngs)
        ng.write(ofs);

    for (const auto& a : g)
        a.write(ofs);
}


template<typename E>
template<typename... T>
void Chain<E>::
ensure(
    const bool cond,
    T&&... message
) const
{
    if (!cond) [[unlikely]]
        utils::raise<invariant_error>(nullptr, std::source_location::current(),
                                      message...);
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_PRINT_H
