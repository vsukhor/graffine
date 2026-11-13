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
 * \file slot.h
 * \brief Slots: addresses of vertices by their position on a chain or an edge.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_SLOT_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_SLOT_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/bulk.h"
#include "graffine/structure/descriptors/end.h"

#include <array>
#include <iostream>
#include <string>
#include <type_traits>  // is_same_v
#include <tuple>

namespace graffine::structure::descriptors {

/// Type of the location of a slot on its host: End or Bulk.
template<typename T>
concept SlotCoord = std::is_same_v<T, End> ||
                    std::is_same_v<T, Bulk>;


/** Slot: the address of a vertex by its location on a host chain or edge.
 * \details Vertices are not addressed by ids of their own; instead a slot
 * names the host element \a w and the location \a e of the vertex on it:
 * - with Location End (EndSlot), \a e is an end of the host, A or B;
 * - with Location Bulk (BulkSlot), \a e is the position \a a of the vertex
 *   inside the host chain: the vertex between edges g[a-1] and g[a], with
 *   0 < a < length() (see Chain).
 *
 * A default-constructed slot is undefined. Slots are ordered by host, then
 * by location.
 * \tparam HId Type of the host identifier (e.g., ChIdG or EgIdC).
 * \tparam Location Type of the location identifier (End or Bulk).
 */
template<typename HId,
         SlotCoord Location>
struct _Slot
{
    using HostId = HId;           ///< Type of the host identifier.
    using LocId = Location::Id;   ///< Type of the location identifier.

    /// True for a slot at an end of the host, false for one inside it.
    static constexpr auto isEnd = std::is_same_v<Location, End>;

    constexpr _Slot() = default;
    constexpr _Slot(const _Slot& s) = default;
    constexpr _Slot(_Slot&& s) = default;
    constexpr ~_Slot() = default;
    constexpr _Slot& operator=(const _Slot& s) = default;
    constexpr _Slot& operator=(_Slot&&) = default;

    /** Constructor.
     * \param w Host identifier.
     * \param e Location on the host.
     */
    constexpr _Slot(
        const HostId w,
        const LocId e
    )
        : w {w}
        , e {e}
    {
        if constexpr (isEnd) {
            GRAFFINE_ASSERT(e < Location::num || e == Location::undefined,
                   "wrong end index e: ", ea_str());
        }
    }

    constexpr bool operator==(const _Slot& s) const noexcept
    {
        return w == s.w &&
               e == s.e;
    }

    constexpr bool operator<(const _Slot& s) const noexcept
    {
        return w < s.w ||
               (w == s.w && e < s.e);
    }

    /// Checks that the slot differs from a default-constructed one.
    constexpr bool is_defined() const noexcept
    {
        return *this != _Slot{};
    }

    /// The host and the location, for structured binding:
    /// <tt>auto [w, e] = s.we();</tt>
    constexpr auto we() const noexcept -> std::tuple<HostId, LocId>
    {
        return std::tie(w, e);
    }

    /** Location \a e; for a BulkSlot, the position in the host chain.
     */
    constexpr auto a() const noexcept -> LocId { return e; }

    /// The location as a string.
    constexpr auto ea_str() const -> std::string
    {
        return Location::str(e);
    }

    /**
     * \brief Static counterpart of opp().
     * \return \p s.opp()
     */
    static constexpr auto opp(const _Slot& s) noexcept -> _Slot
    {
        return s.opp();
    }

    /** Slot at the opposite end of the same host; undefined for a BulkSlot.
     */
    constexpr auto opp() const noexcept -> _Slot
    {
        return isEnd
           ? _Slot{w, End::opp(e)}
           : _Slot{};
    }

    /// Prints the slot in braces, without a line break.
    void print(const char* color=colorcodes::YELLOW) const
    {
       jot<false>('{', color, str_short(), colorcodes::RESET, '}');
    }

    /// The slot as a string with labelled fields, e.g. "w 3 e B".
    auto str_long() const -> std::string
    {
        return "w "s + std::to_string(w) +
               (isEnd ? " e " : " a ") + ea_str();
    }

    /// The slot as a string of the bare fields, e.g. "3 B".
    auto str_short() const -> std::string
    {
        return std::to_string(w()) + " " + ea_str();
    }

    /// Static counterpart of str_short().
    static auto str_short(_Slot s) -> std::string
    {
        return s.str_short();
    }

    HostId w {HostId::undefined};  ///< Host chain or edge.
    LocId  e {LocId::undefined};   ///< Location on the host.
};


using EdgeSlotC = _Slot<EgIdC, End>;  ///< End of an edge, by component-wide index.
using EdgeSlotW = _Slot<EgIdG, End>;  ///< End of an edge, by graph-wide index.
/// Chain end: the vertex at end A or B of a chain, which is a leaf, a
/// junction, or the joining point of a cycle.
using EndSlot = _Slot<ChIdG, End>;
/// Chain internal vertex, of degree 2.
using BulkSlot = _Slot<ChIdG, Bulk>;

/// Slot type addressing a vertex of degree \p D in the degree-specific
/// transforms: BulkSlot for degree 2, EndSlot otherwise.
template<Degree D>
using Slot = std::conditional_t<D == Deg2, BulkSlot, EndSlot>;

/// A chain slot type: EndSlot or BulkSlot.
template<typename T>
concept slot = std::is_same_v<T, EndSlot> ||
               std::is_same_v<T, BulkSlot>;

/// True for a chain slot type: EndSlot or BulkSlot.
template<typename T>
constexpr auto isSlot = std::is_same_v<T, EndSlot> ||
                 std::is_same_v<T, BulkSlot>;


/// A chain slot type, or a chain id standing for the whole chain.
template<typename T>
concept generic_slot = std::is_same_v<T, EndSlot> ||
                       std::is_same_v<T, BulkSlot> ||
                       std::is_same_v<T, ChIdG>;


}  // namespace graffine::structure::descriptors


// Hashes of the chain slots, for use as keys of unordered containers.
namespace std {

    using graffine_BulkSlot =
        graffine::structure::descriptors::_Slot<graffine::structure::descriptors::ids::ChIdG,
                                                graffine::structure::descriptors::Bulk>;

    template<> struct hash<graffine_BulkSlot>
    {
        std::size_t operator()(const graffine_BulkSlot& s) const noexcept
        {
            return hash<graffine_BulkSlot::HostId::value_type>()(s.w()) * 31 +
                   hash<graffine_BulkSlot::LocId::value_type>()(s.e());
        }
    };
    using graffine_EndSlot =
        graffine::structure::descriptors::_Slot<graffine::structure::descriptors::ids::ChIdG,
                                                graffine::structure::descriptors::End>;

    template<> struct hash<graffine_EndSlot>
    {
        std::size_t operator()(const graffine_EndSlot& s) const noexcept
        {
            return hash<graffine_EndSlot::HostId::value_type>()(s.w()) * 31 +
                   hash<graffine_EndSlot::LocId>()(s.e);
        }
    };

}  // namespace std

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_SLOT_H
