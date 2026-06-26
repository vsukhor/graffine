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
 * \brief Defines structure handling connectivity between and inside chains.
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

template<typename T>
concept SlotCoord = std::is_same_v<T, End> ||
                    std::is_same_v<T, Bulk>;


/**
 * Template for a slot representing a connection point in an edge or chain.
 * \details The slot is used to represent a connection point in a chain, which can
 * be either one of the ends or interior. The slot contains the host identifier
 *  and the location identifier. It provides methods for comparison, equality
 * checks, and string representation. The slot can be used to represent both
 * ends of a chain and internal connections within a chain.
 * \tparam HostId Type of the host identifier (e.g., ChId or EgId).
 * \tparam Location Type of the location identifier (e.g., End or Bulk).
 */
template<typename HId,
         SlotCoord Location>
struct _Slot
{
    using HostId = HId;
    using LocId = Location::Id;

    static constexpr auto isEnd = std::is_same_v<Location, End>;

    constexpr _Slot() = default;
    constexpr _Slot(const _Slot& s) = default;
    constexpr _Slot(_Slot&& s) = default;
    constexpr ~_Slot() = default;
    constexpr _Slot& operator=(const _Slot& s) = default;
    constexpr _Slot& operator=(_Slot&&) = default;

    constexpr _Slot(
        const HostId w,
        const LocId e
    )
        : w {w}
        , e {e}
    {
        if constexpr (isEnd) {
            ASSERT(e < Location::num || e == Location::undefined,
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

    constexpr bool is_defined() const noexcept
    {
        return *this != _Slot{};
    }

    constexpr auto we() const noexcept -> std::tuple<HostId, LocId>
    {
        return std::tie(w, e);
    }

    constexpr auto a() const noexcept -> LocId { return e; }

    constexpr auto ea_str() const noexcept -> std::string
    {
        return Location::str(e);
    }

    static constexpr auto opp(const _Slot& s) noexcept -> _Slot
    {
        return s.opp();
    }

    constexpr auto opp() const noexcept -> _Slot
    {
        return isEnd
           ? _Slot{w, End::opp(e)}
           : _Slot{};
    }

    constexpr auto next() const -> _Slot
    {
        if constexpr (std::is_same_v<Location, Bulk>)
            return _Slot {w, e + 1};
        else
            return _Slot {w, e.opp()};
    }

    constexpr void write(std::ofstream& ofs) const
    {
        ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));
        ofs.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    void print(const char* color=colorcodes::YELLOW) const
    {
       jot<false>('{', color, str_short(), colorcodes::RESET, '}');
    }

    auto str_long() const -> std::string
    {
        return "w "s + std::to_string(w) +
               (isEnd ? " e " : " a ") + ea_str();
    }

    auto str_short() const -> std::string
    {
        return std::to_string(w()) + " " + ea_str();
    }

    static auto str_short(_Slot s) -> std::string
    {
        return s.str_short();
    }

    HostId w {HostId::undefined};
    LocId  e {LocId::undefined};
};


using EdgeSlotC = _Slot<EgIdC, End>;
using EdgeSlotW = _Slot<EgIdG, End>;
using EndSlot = _Slot<ChIdG, End>;
using BulkSlot = _Slot<ChIdG, Bulk>;

template<Degree D>
using Slot = std::conditional_t<D == Deg2, BulkSlot, EndSlot>;

template<typename T>
concept slot = std::is_same_v<T, EndSlot> ||
               std::is_same_v<T, BulkSlot>;

template<typename T>
constexpr auto isSlot = std::is_same_v<T, EndSlot> ||
                 std::is_same_v<T, BulkSlot>;


template<typename T>
concept generic_slot = std::is_same_v<T, EndSlot> ||
                       std::is_same_v<T, BulkSlot> ||
                       std::is_same_v<T, ChIdG>;

constexpr auto to_BulkSlot(const _Slot<ChIdG, End>& s) noexcept -> _Slot<ChIdG, Bulk>
{
    return _Slot<ChIdG, Bulk> {s.w, s.e};
};


}  // namespace graffine::structure::descriptors


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
