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

/// Defines identificators for graph structural elements
namespace graffine::structure::descriptors::ids {


enum class ElementType {
    END,
    VERTEX,
    EDGE,
    CHAIN,
    COMPONENT,
    FACET,
    UNDEF
};

enum class IdScope {
    RUN,
    GRAPH,
    COMPONENT,
    CHAIN
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
    constexpr Id(const Id& q) : v{q.v} {}
    constexpr Id(const value_type v) : v{v} {}

    constexpr auto operator=(const Id& id) noexcept -> Id& { v = id.v; return *this; }
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

    constexpr auto operator--(const int) const noexcept -> Id // postfix
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

    auto as_string() const noexcept -> std::string
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

/// Edge ids.
using EgIdH = Id<ElementType::EDGE, IdScope::RUN>;

using EgIdG = Id<ElementType::EDGE, IdScope::GRAPH>;
using EgIdC = Id<ElementType::EDGE, IdScope::COMPONENT>;
using EgIdA = Id<ElementType::EDGE, IdScope::CHAIN>;

using VIdH = Id<ElementType::VERTEX, IdScope::RUN>;       ///< Vertex ids: simulation-wide.
using VIdG = Id<ElementType::VERTEX, IdScope::GRAPH>;     ///< Vertex ids: graph-wide.
using VIdC = Id<ElementType::VERTEX, IdScope::COMPONENT>; ///< Vertex ids: component-wide.

using CmpId = Id<ElementType::COMPONENT, IdScope::GRAPH>;  ///< Component ids.
using FId   = Id<ElementType::FACET, IdScope::GRAPH>;      ///< Facet ids.

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
