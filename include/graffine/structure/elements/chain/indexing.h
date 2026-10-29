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
 * \file indexing.h
 * \brief Conversions between edge indexes, positions, ends and slots of a chain.
 * \details Part of the implementation of chain.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_INDEXING_H
#define GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_INDEXING_H

#include "graffine/structure/elements/chain.h"

namespace graffine::structure::detail {

template<typename E>
bool Chain<E>::
has_edge(const EgIdG ind) const noexcept
{
    for (const auto& eg : g)
        if (eg.ind == ind)
            return true;

    return false;
}


template<typename E>
constexpr
auto Chain<E>::
end_edge(const EndId e) const noexcept -> const Edge&
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return e == endA ? g.front()
                     : g.back();
}


template<typename E>
constexpr
auto Chain<E>::
end_edge(const EndId e) noexcept -> Edge&
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return e == endA ? g.front()
                     : g.back();
}


// Convert chain edge index and edge end in chain internal to edge end
template<typename E>
constexpr
auto Chain<E>::
edge_internal_end(const EgIdA a, const EndId e) const noexcept -> EndId
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return g[a].inner_endId(e);
}


// Convert chain boundary id to edge internal end id
template<typename E>
constexpr
auto Chain<E>::
edge_internal_end_at_end(const EndId e) const noexcept -> EndId
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).inner_endId(e);
}


template<typename E>
constexpr
bool Chain<E>::
egEnd_is_head(const EgIdG ind,
              const EndId egEnd) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(egEnd), "Edge end not valid.");

    return is_headind(ind) &&
        (( head().points_forwards() && egEnd == End::B) ||
         (!head().points_forwards() && egEnd == End::A));
}


template<typename E>
constexpr
bool Chain<E>::
egEnd_is_tail(const EgIdG ind,
              const EndId egEnd) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(egEnd), "Edge end not valid.");

    return is_tailind(ind) &&
        (( tail().points_forwards() && egEnd == End::A) ||
         (!tail().points_forwards() && egEnd == End::B));
}


template<typename E>
constexpr
auto Chain<E>::
connected_edge(
    const EgIdA a,
    const EndId ege
) const noexcept -> const Edge*
{
    GRAFFINE_ASSERT(!egEnd_is_tail(g[a].ind, ege),
           "Edge end ", End::str(ege), " at ", a, ", w ", idw, " is chain tail");
    GRAFFINE_ASSERT(!egEnd_is_head(g[a].ind, ege),
           "Edge end ", End::str(ege), " at ", a, ", w ", idw, " is chain head");


    const auto pf = g[a].points_forwards();

    if (a < length() - 1) {
        if (( pf && ege == End::B) ||
            (!pf && ege == End::A))
            return &g[a+1];
    }

    if (a > 0) {
        if (( pf && ege == End::A) ||
            (!pf && ege == End::B))
            return &g[a-1];
    }

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
ind2a(const EgIdG ind) const noexcept -> EgIdA
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return a;

    return EgIdA::undefined;
}


template<typename E>
constexpr
auto Chain<E>::
ind2eg(const EgIdG ind) const noexcept -> const Edge*
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
ind2eg(const EgIdG ind) noexcept -> Edge*
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
indc2a(const EgIdC indc) const noexcept -> EgIdA
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return a;

    return EgIdA::undefined;
}

template<typename E>
constexpr
auto Chain<E>::
indc2eg(const EgIdC indc) const noexcept -> const Edge*
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return &(g[a]);

    return nullptr;
}

template<typename E>
constexpr
auto Chain<E>::
indc2eg(const EgIdC indc) noexcept -> Edge*
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
neig_a_of_egEnd(
    const EgIdA a,    // edge position inside the chain
    const EndId ee    // edge end
) const noexcept -> EgIdA
{
    const auto shift = g[a].points_forwards() ? (ee == endB ? 1 : -1)
                                              : (ee == endB ? -1 : 1);
    const auto b = static_cast<EgIdA>(a + shift);

    return b < length() ? b : EgIdA::undefined;
}


// Finds edge end of the edge \p b to which the edge at
// \p a is connected via a vertex internal to the chain.
// Should only be applied to the chain internal vertices.
template<typename E>
constexpr
auto Chain<E>::
internal_egEnd(
    const EgIdA a,   // chain position of the 1st edge
    const EgIdA b    // chain position of the 2nd edge
) const noexcept -> EndId
{
    GRAFFINE_ASSERT(a.is_defined() && b.is_defined(),
           "a = ", a, ", b = ", b, " are not chain-wide edge indexes");
    GRAFFINE_ASSERT(a == b + 1 || b == a + 1,
           "a = ", a, ", b = ", b, " do not share a vertex");

    return g[b].points_forwards() ? (a > b ? endB : endA)
                                  : (a > b ? endA : endB);
}


// Edge end of an edge a
template<typename E>
constexpr
auto Chain<E>::
a2egEnd(const EgIdA a) const noexcept -> EndId
{
    return g[a].outer_endId(endA);
}


/*
template<typename E>
constexpr
auto Chain<E>::
end(const EgId indc,
    const EndId egEnd) const noexcept -> EndId
{
    GRAFFINE_ASSERT(g.front().indc == indc || g.back().indc == indc,
           "edge having component index ", indc, " is not an at chain end");

    const auto a = g[0].indc == indc
                 ? 0
                 : length() - 1;
    const auto isForward = g[a].points_forwards();

    return a == 0
        ? (egEnd == End::A ? (isForward ? End::A : End::B)
                            : (isForward ? End::B : End::A))
        : (isForward ? End::B : End::A);
}
*/


template<typename E>
constexpr
auto Chain<E>::
end2a(const EndId e) const noexcept -> EgIdA
{
    return e == endA ? 0 : length() - 1;
}


template<typename E>
constexpr
auto Chain<E>::
a2end(const EgIdA a) const noexcept -> EndId
{
    GRAFFINE_ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());
    GRAFFINE_ASSERT(a == 0 || a == length() - 1, "a = ", a, " is not an end edge.");

    return length() == 1
        ? endUndef            // end is undefined for single-edge chains
        : a == 0 ? endA
                 : a == length() - 1 ? endB
                                     : endUndef;
}


template<typename E>
constexpr
auto Chain<E>::
egEnd_to_bulkslot(const EndId egE,
                  const EgIdA a) const noexcept -> BulkSlot
{
    GRAFFINE_ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());

    const auto frw = g[a].points_forwards();

    return egE == endA ? (frw ? BulkSlot{idw, a}
                              : BulkSlot{idw, a + 1})
                       : (frw ? BulkSlot{idw, a + 1}
                              : BulkSlot{idw, a});
}


template<typename E>
constexpr
auto Chain<E>::
end2ind(const EndId e) const noexcept -> EgIdG
{
    return end_edge(e).ind;
}


template<typename E>
auto Chain<E>::
ind2end(const EgIdG ind,                  // graph-wide edge index
        const EndId ege) const -> EndId   // edge internal end
{
    if (length() == 1) {
        if (ind == g[0].ind)
            return g[0].outer_endId(ege);

        utils::raise<precondition_error>(
            nullptr, std::source_location::current(),
            ind, " is not an end edge ind of chain ", idw);
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        ind, " is not an end edge ind of chain ", idw);
}


template<typename E>
auto Chain<E>::
ind2end(const EgIdG ind,                    // graph-wide edge index
        const EndSlot& nb) const -> EndId   // EndSlot connected to the end
{
    if (length() == 1) {
        if (ind == g.front().ind && ngs[endA].contains(nb))
            return endA;
        else if (ind == g.back().ind && ngs[endB].contains(nb))
            return endB;

        utils::raise<precondition_error>(
            nullptr, std::source_location::current(),
            ind, " is not an end edge ind of chain ", idw);
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        ind, " is not an end edge ind of chain ", idw);
}


template<typename E>
constexpr
auto Chain<E>::
indc2end(const EgIdC indc,
         const EndId ege) const -> EndId
{
    if (length() == 1) {
        if (indc == g[0].indc)
            return g[0].outer_endId(ege);

        utils::raise<precondition_error>(
            nullptr, std::source_location::current(),
            indc, " is not an end edge indc of chain ", idw);
    }

    if (indc == g.front().indc)
        return endA;

    if (indc == g.back().indc)
        return endB;

    utils::raise<precondition_error>(
        nullptr, std::source_location::current(),
        indc, " is not an end edge indc of chain ", idw);
}


template<typename E>
constexpr
auto Chain<E>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    GRAFFINE_ASSERT(End::is_valid(e), "End ", e, " is not valid");

    return End::opp(e);
}


template<typename E>
constexpr
bool Chain<E>::
edge_points_forwards(const EgIdA a) const noexcept
{
    GRAFFINE_ASSERT(a < length(),
           "a ", a, " is not valid for length: ", length(), " of chain ", idw);

    return g[a].points_forwards();
}


template<typename E>
constexpr
auto Chain<E>::
end_slot(const EgIdA a,
         const EndId internalEgEnd) const noexcept -> EndSlot
{

    GRAFFINE_ASSERT(a < length(),
           "a ", a, " is not valid for length: ", length(), " of chain ", idw);

    const auto is_valid = is_head(a) || is_tail(a);

    GRAFFINE_ASSERT(is_valid, "Edge pos ", a, " is not at chain end");

    return is_valid ? EndSlot{idw, g[a].outer_endId(internalEgEnd)}
                    : EndSlot{};

}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_INDEXING_H
