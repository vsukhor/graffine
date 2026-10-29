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
 * \file connectivity.h
 * \brief Connectivity of the ends of a chain.
 * \details Part of the implementation of chain.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_CONNECTIVITY_H
#define GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_CONNECTIVITY_H

#include "graffine/structure/elements/chain.h"

namespace graffine::structure::detail {

template<typename E>
constexpr
auto Chain<E>::
get_single_connected_end() const -> EndId
{
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endA;
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        "Error in Chain::get_single_connected_end(): ", "the connected end is not single!");
}

template<typename E>
constexpr
auto Chain<E>::
get_single_leaf_end() const -> EndId
{
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        "Error in Chain::get_single_leaf_end(): ", "the free end is not single!");
}


template<typename E>
constexpr
auto Chain<E>::
get_single_3way_end() const -> EndId
{
    if (ngs[endA].num() == 2 && ngs[endB].num() != 2) return endA;
    if (ngs[endA].num() != 2 && ngs[endB].num() == 2) return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        "Error in Chain::get_single_3way_end(): ", "the 3-way end is not single!");
}


template<typename E>
constexpr
auto Chain<E>::
get_single_4way_end() const -> EndId
{
    if (ngs[endA].num() == 3 && ngs[endB].num() != 3) return endA;
    if (ngs[endA].num() != 3 && ngs[endB].num() == 3) return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        "Error in Chain::get_single_4way_end(): ", "the 4-way end is not single!");
}


template<typename E>
constexpr
bool Chain<E>::
has_single_leaf_vertex() const noexcept
{
    const auto a = is_connected_at(endA);
    const auto b = is_connected_at(endB);

    if ((!a &&  b) ||
        ( a && !b)) return true;

    return false;
}

template<typename E>
constexpr
auto Chain<E>::
the_only_leaf_end() const noexcept -> EndId
{
    GRAFFINE_ASSERT(has_single_leaf_vertex(),
            "chain ", idw, " does not have single leaf vertex");

    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    return endUndef;
}


template<typename E>
constexpr
auto Chain<E>::
has_a_leaf_end() const noexcept -> std::pair<bool, EndId>
{
    // return the end index if true

    if (!is_connected_at(endA))
        return {true, endA};

    if (!is_connected_at(endB))
        return {true, endB};

    return {false, endUndef};
}


template<typename E>
constexpr
bool Chain<E>::
is_connected_at(const EndId e) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    return ngs[e].num() > 0;
}

template<typename E>
constexpr
bool Chain<E>::
is_connected() const noexcept
{
    return ngs[endA].num() > 0 || ngs[endB].num() > 0;
}

template<typename E>
constexpr
bool Chain<E>::
is_unconnected_linear() const noexcept
{
    return !ngs[endA].num() || !ngs[endB].num();
}


template<typename E>
constexpr
bool Chain<E>::
is_unconnected_cycle() const noexcept
{
//    const auto vA = vertex_at_end(endA);
//    const auto vB = vertex_at_end(endB);
//    const auto A = vA->deg() == 2 && vA == vB;

//    const auto B = ngs[endA].num() == 1 &&
//           ngs[endB].num() == 1 &&
//           ngs[endA][0] ==
//           ngs[endB][0].opp();

//    GRAFFINE_ASSERT(A == B, " B ", B, " vA->deg() ", vA->deg(),  " Addr vA ", vA, " addr vB ", vB);
//    const auto vA = vertex_at_end(endA);
//    const auto vB = vertex_at_end(endB);

//    return vA->deg() == 2 && vA == vB;
    return ngs[endA].num() == 1 &&
           ngs[endB].num() == 1 &&
           *ngs[endA].begin() ==
           (*ngs[endB].begin()).opp();
}


template<typename E>
constexpr
bool Chain<E>::
is_connected_cycle() const noexcept
{
    if (ngs[endA].num() > 1) {

        if (!is_connected_at(endB))
            return false;

        for (const auto& sA : ngs[endA]())
            for (const auto& sB : ngs[endB]())
                if (sA ==sB)
                    return true;
    }

    return false;
}


template<typename E>
constexpr
bool Chain<E>::
is_cycle() const noexcept
{
    return is_unconnected_cycle() ||
           is_connected_cycle();
}


template<typename E>
constexpr
bool Chain<E>::
is_shrinkable() const noexcept
{
    if (has_single_leaf_vertex())
        return true;

    const auto l = length();

    return is_cycle() ? l > minCycleLength
                      : l > minLength;
}

template<typename E>
constexpr
bool Chain<E>::
is_shrinkable_by(const std::size_t n) const noexcept
{
    const auto l = length();

    return l >= n + (is_cycle() ? minCycleLength : minLength);
}

template<typename E>
constexpr
bool Chain<E>::
is_unconnected_cycle_boundary(const Vertex& v) const noexcept
{
    return is_unconnected_cycle() && vertex_at_end(endA)->operator==(v);
}

template<typename E>
constexpr
bool Chain<E>::
is_cycle_boundary(const Vertex& v) const noexcept
{
    return is_cycle() && vertex_at_end(endA)->operator==(v);
}

template<typename E>
bool Chain<E>::
has_such_neig(const EndId e,
              const ChIdG n) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    for (const auto& s : ngs[e]())
        if (s.w == n)
            return true;

    return false;
}


template<typename E>
constexpr
auto Chain<E>::
num_neigs(const EndId e) const noexcept -> std::size_t
{
    GRAFFINE_ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    return ngs[e].num();
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_CONNECTIVITY_H
