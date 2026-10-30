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
 * \file id.h
 * \brief Contains template struct implementing Id of graph elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H

#include "graffine/utils/misc.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <ostream>
#include <string>
#include <type_traits>

/**
 * Defines identifiers for graph structural elements.
 * \details An identifier is typed by the kind of element it identifies and
 * by its scope, i.e. the range over which it is unique. The alias names
 * encode both: \c EgId, \c ChId, \c VId for edges, chains and vertices,
 * with suffix
 * - \c H (IdScope::RUN): unique over the program run, and stable: it is
 *   assigned once, when the element is created;
 * - \c G (IdScope::GRAPH): position in a container of the graph;
 * - \c C (IdScope::COMPONENT): position in a container of the component;
 * - \c A (IdScope::CHAIN): position of an edge along its chain.
 *
 * The G, C and A ids are positions, so they change when the graph does:
 * - removal of an element moves the last element of the same container
 *   into the vacated position, renumbering it: e.g. after Graph::remove_edge,
 *   the formerly last edge of the graph takes over the removed EgIdG, and
 *   the last edge of the component the removed EgIdC; chains and components
 *   are renumbered alike;
 * - merging and splitting of components renumbers the chains and edges
 *   they contain (ChIdC, EgIdC) and may change CmpId;
 * - insertion or removal of an edge in a chain shifts the EgIdA of the
 *   edges following it.
 *
 * Only the H ids therefore remain valid over transformations. The data
 * members holding the ids are named as follows:
 * - Edge: \a id (EgIdH), \a ind (EgIdG), \a indc (EgIdC), \a indw (EgIdA),
 *   \a w (ChIdG of the host chain), \a c (CmpId of the host component);
 * - Chain: \a idw (ChIdG), \a idc (ChIdC), \a c (CmpId);
 * - Component: \a ind (CmpId), \a ww (ChIdGs of its chains);
 * - Vertex: \a id (VIdH), \a c (CmpId);
 * - slots and edge descriptors: \a w (chain), \a a (EgIdA), \a e (end).
 */
namespace graffine::structure::descriptors::ids {

/// Kind of the element identified.
enum class ElementType {
    END,
    VERTEX,
    EDGE,
    CHAIN,
    COMPONENT,
    FACET,
    UNDEF
};

/// Range over which an identifier is unique; see the namespace description.
enum class IdScope {
    RUN,        ///< Program run: stable handle.
    GRAPH,      ///< Position in a graph container.
    COMPONENT,  ///< Position in a component container.
    CHAIN       ///< Position along a chain.
};

/// Struct template for element ids.
template<ElementType T,
         IdScope S>
struct Id
{
    using value_type = std::size_t;

    static constexpr ElementType Element = T;
    static constexpr IdScope Scope = S;
    static constexpr Id undefined {utils::undefined<value_type>};

    constexpr Id() : v {} {}
    // Defaulted to keep Id trivially copyable and nothrow movable, so that
    // elements holding ids are moved rather than copied by std containers.
    constexpr Id(const Id&) = default;
    constexpr Id(const value_type v) : v{v} {}

    constexpr auto operator=(const Id&) -> Id& = default;
    constexpr auto operator()() noexcept -> value_type& { return v; }
    constexpr auto operator()() const noexcept -> value_type { return v; }
    constexpr bool operator==(const Id& id) const noexcept { return v == id.v; }
    constexpr bool operator==(const value_type vv) const noexcept { return v == vv; }
    constexpr bool operator<(const Id& id) const noexcept { return v < id.v; }
    constexpr bool operator>(const Id& id) const noexcept { return v > id.v; }
    constexpr bool operator>(const value_type vv) const noexcept { return v > vv; }
    constexpr bool operator<(const value_type vv) const noexcept { return v < vv; }
    constexpr bool operator<=(const Id& id) const noexcept { return v <= id.v; }
    constexpr bool operator<=(const value_type vv) const noexcept { return v <= vv; }
    constexpr bool operator>=(const Id& id) const noexcept { return v >= id.v; }
    constexpr bool operator>=(const value_type vv) const noexcept { return v >= vv; }

    constexpr auto operator++() noexcept -> Id&  // prefix
    {
        ++v;
        return *this;
    }

    constexpr auto operator++(const int) noexcept -> Id // postfix
    {
        auto result = *this;
        operator++();
        return result;
    }

    constexpr auto operator--() noexcept -> Id&  // prefix
    {
        --v;
        return *this;
    }

    constexpr auto operator--(const int) noexcept -> Id // postfix
    {
        auto result = *this;
        operator--();
        return result;
    }

    // compound assignment (does not need to be a member,
    // but often is, to modify the private members)
    constexpr auto operator+=(const Id& rhs) noexcept -> Id&
    {
        v += rhs.v;
        return *this; // return the result by reference
    }

    // compound assignment (does not need to be a member,
    // but often is, to modify the private members)
    constexpr auto operator-=(const Id& rhs) noexcept -> Id&
    {
        v -= rhs.v;
        return *this; // return the result by reference
    }

    // passing lhs by value helps optimize chained a+b+c
    // otherwise, both parameters may be const references
    friend constexpr auto operator+(Id lhs,
                                    const Id& rhs) noexcept
    {
        lhs += rhs; // reuse compound assignment
        return lhs; // return the result by value (uses move constructor)
    }
    friend constexpr auto operator-(Id lhs,
                                    const Id& rhs) noexcept
    {
        lhs -= rhs; // reuse compound assignment
        return lhs; // return the result by value (uses move constructor)
    }

    friend auto operator<<(std::ostream& stream,
                           const Id& id) -> std::ostream&
    {
        stream << id.v;
        return stream;
    }

    auto as_string() const -> std::string
    {
        return std::to_string(v);
    }

    constexpr bool is_defined() const noexcept
    {
        return v != utils::undefined<value_type>;
    }

private:

    value_type v {};
};


/// Chain ids relative to the hosting graph.
using ChIdG = Id<ElementType::CHAIN, IdScope::GRAPH>;

/// Chain ids relative to the hosting component.
using ChIdC = Id<ElementType::CHAIN, IdScope::COMPONENT>;

/// Edge ids unique over the program run.
using EgIdH = Id<ElementType::EDGE, IdScope::RUN>;
/// Edge ids relative to the hosting graph.
using EgIdG = Id<ElementType::EDGE, IdScope::GRAPH>;
/// Edge ids relative to the hosting component.
using EgIdC = Id<ElementType::EDGE, IdScope::COMPONENT>;
/// Edge positions in the hosting chain, from 0 at chain end A.
using EgIdA = Id<ElementType::EDGE, IdScope::CHAIN>;

using VIdH = Id<ElementType::VERTEX, IdScope::RUN>;       ///< Vertex ids: unique over the program run.
/// Vertex ids: positions in the graph-wide vertex descriptors, which are
/// rebuilt on their first access after a change of the graph.
using VIdG = Id<ElementType::VERTEX, IdScope::GRAPH>;
using VIdC = Id<ElementType::VERTEX, IdScope::COMPONENT>; ///< Vertex ids: component-wide.

using CmpId = Id<ElementType::COMPONENT, IdScope::GRAPH>;  ///< Component ids.
using FId   = Id<ElementType::FACET, IdScope::GRAPH>;      ///< Facet ids.

static_assert(std::is_trivially_copyable_v<EgIdG> &&
              std::is_nothrow_move_constructible_v<EgIdG>,
              "Id must stay trivially copyable");

using ChIdGs = std::vector<ChIdG>;  ///< Container for graph chain ids.
using ChIdCs = std::vector<ChIdC>;  ///< Container for component chain ids.
using EgIdGs = std::vector<EgIdG>;  ///< Container for graph edge ids.

template<typename IdT, std::size_t N>
using Ids = std::array<IdT, N>;


template<typename IdT, std::size_t N>
auto id_sequence(const IdT initVal=IdT{}) noexcept -> Ids<IdT, N>
{
    Ids<IdT, N> res;
    std::iota(res.begin(), res.end(), initVal);

    return res;
}


}  // namespace graffine::structure::descriptors::ids


#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H
