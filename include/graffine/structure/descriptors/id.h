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
 * \brief Typed identifiers of the graph elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H

#include "graffine/utils/misc.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <ostream>
#include <string>
#include <type_traits>

/** Defines identifiers for graph structural elements.
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
    END,        ///< Edge or chain end.
    VERTEX,     ///< Vertex.
    EDGE,       ///< Edge.
    CHAIN,      ///< Chain: a linear run of edges with no branches.
    COMPONENT,  ///< Connected component.
    TRIANGLE,   ///< Chain triangle: three chains forming a loop, see elements/triangle.h.
    UNDEF       ///< Not specified.
};

/// Range over which an identifier is unique; see the namespace description.
enum class IdScope {
    RUN,        ///< Program run: stable handle.
    GRAPH,      ///< Position in a graph container.
    COMPONENT,  ///< Position in a component container.
    CHAIN       ///< Position along a chain.
};

/** Identifier of a graph element: an index typed by element kind and scope.
 * \details Ids of different kinds or scopes are distinct types, so they
 * cannot be mixed up by mistake. An id converts implicitly from its numeric
 * value; the call operator returns that value, e.g. for indexing a
 * container: <tt>edges[ind()]</tt>. Arithmetic and comparison operators
 * act on the value.
 * \tparam T Kind of the element identified.
 * \tparam S Range over which the id is unique.
 */
template<ElementType T,
         IdScope S>
struct Id
{
    using value_type = std::size_t;  ///< Type of the numeric value.

    static constexpr ElementType Element = T;  ///< Kind of the element identified.
    static constexpr IdScope Scope = S;        ///< Range of uniqueness.
    /// Sentinel value of an id not referring to any element.
    static constexpr Id undefined {utils::undefined<value_type>};

    /// Default constructor: the id has value 0 (not \ref undefined).
    constexpr Id() : v {} {}
    // Defaulted to keep Id trivially copyable and nothrow movable, so that
    // elements holding ids are moved rather than copied by std containers.
    constexpr Id(const Id&) = default;
    /// Implicit conversion from the numeric value \p v.
    constexpr Id(const value_type v) : v{v} {}

    constexpr auto operator=(const Id&) -> Id& = default;
    /// Reference to the numeric value.
    constexpr auto operator()() noexcept -> value_type& { return v; }
    /// The numeric value.
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

    /// Adds the value of \p rhs to this id.
    constexpr auto operator+=(const Id& rhs) noexcept -> Id&
    {
        v += rhs.v;
        return *this;
    }

    /// Subtracts the value of \p rhs from this id.
    constexpr auto operator-=(const Id& rhs) noexcept -> Id&
    {
        v -= rhs.v;
        return *this;
    }

    /// Sum of the values of two ids. Taking \p lhs by value helps
    /// optimize chained sums a+b+c.
    friend constexpr auto operator+(Id lhs,
                                    const Id& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }
    /// Difference of the values of two ids.
    friend constexpr auto operator-(Id lhs,
                                    const Id& rhs) noexcept
    {
        lhs -= rhs;
        return lhs;
    }

    /// Writes the numeric value to \p stream.
    friend auto operator<<(std::ostream& stream,
                           const Id& id) -> std::ostream&
    {
        stream << id.v;
        return stream;
    }

    /// The numeric value as a string.
    auto as_string() const -> std::string
    {
        return std::to_string(v);
    }

    /// Checks that the id is not \ref undefined.
    constexpr bool is_defined() const noexcept
    {
        return v != utils::undefined<value_type>;
    }

private:

    value_type v {};  ///< The numeric value.
};


/// Chain ids: positions in the graph's chain container.
using ChIdG = Id<ElementType::CHAIN, IdScope::GRAPH>;

/// Chain ids: positions in the host component's chain container.
using ChIdC = Id<ElementType::CHAIN, IdScope::COMPONENT>;

/// Edge ids unique over the program run.
using EgIdH = Id<ElementType::EDGE, IdScope::RUN>;
/// Edge ids: positions in the graph's edge container.
using EgIdG = Id<ElementType::EDGE, IdScope::GRAPH>;
/// Edge ids: positions in the host component's edge container.
using EgIdC = Id<ElementType::EDGE, IdScope::COMPONENT>;
/// Edge positions in the hosting chain, from 0 at chain end A.
using EgIdA = Id<ElementType::EDGE, IdScope::CHAIN>;

using VIdH = Id<ElementType::VERTEX, IdScope::RUN>;       ///< Vertex ids: unique over the program run.
/// Vertex ids: positions in the graph-wide vertex descriptors, which are
/// rebuilt on their first access after a change of the graph.
using VIdG = Id<ElementType::VERTEX, IdScope::GRAPH>;
using VIdC = Id<ElementType::VERTEX, IdScope::COMPONENT>; ///< Vertex ids: component-wide.

/// Component ids: positions in the graph's component container.
using CmpId = Id<ElementType::COMPONENT, IdScope::GRAPH>;
using TrId  = Id<ElementType::TRIANGLE, IdScope::GRAPH>;   ///< Triangle ids.

static_assert(std::is_trivially_copyable_v<EgIdG> &&
              std::is_nothrow_move_constructible_v<EgIdG>,
              "Id must stay trivially copyable");

using ChIdGs = std::vector<ChIdG>;  ///< Container for graph chain ids.
using ChIdCs = std::vector<ChIdC>;  ///< Container for component chain ids.
using EgIdGs = std::vector<EgIdG>;  ///< Container for graph edge ids.

/// Fixed-size array of \p N ids of type \p IdT.
template<typename IdT, std::size_t N>
using Ids = std::array<IdT, N>;


/// Returns the \p N consecutive ids starting at \p initVal.
template<typename IdT, std::size_t N>
auto id_sequence(const IdT initVal=IdT{}) noexcept -> Ids<IdT, N>
{
    Ids<IdT, N> res;
    std::iota(res.begin(), res.end(), initVal);

    return res;
}


}  // namespace graffine::structure::descriptors::ids


#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_ID_H
