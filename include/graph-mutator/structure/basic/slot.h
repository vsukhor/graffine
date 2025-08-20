/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file slot.h
 * @brief Defines structure handling connectivity between anf inside chains.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_SLOT_H
#define GRAPH_MUTATOR_STRUCTURE_SLOT_H

#include <array>
#include <iostream>
#include <string>

#include "../definitions.h"
#include "ends.h"


namespace graph_mutator::structure {

/**
 * @brief Template for a slot representing a connection point in a chain.
 * @tparam HostId Type of the host identifier (e.g., ChId).
 * @tparam Location Type of the location identifier (e.g., Ends or Bulks).
 * @details The slot is used to represent a connection point in a chain, which can
 * be either an end or a bulk. The slot contains the host identifier and the
 * location identifier. It provides methods for comparison, equality checks,
 * and string representation. The slot can be used to represent both ends of a
 * chain and bulk connections within a chain.
 */
template<typename HostId,
         SlotCoord Location>
struct _Slot {

    using LocId = Location::Id;

    static constexpr auto isEnd = std::is_same_v<Location, Ends>;

    constexpr _Slot() = default;
    constexpr _Slot(const _Slot& s) = default;
    constexpr _Slot(_Slot&& s) = default;
    constexpr ~_Slot() = default;
    constexpr _Slot& operator=(const _Slot& s) = default;
    constexpr _Slot& operator=(_Slot&&) = default;

    explicit constexpr _Slot(
        const HostId w,
        const LocId e
    )
        : w {w}
        , e {e}
    {
        if constexpr (isEnd)
            ASSERT(e < Location::num || e == Location::Undefined,
                   "wrong end index e: ", ea_str());
    }

    constexpr auto operator==(const _Slot& s) const noexcept -> bool
    {
        return w == s.w &&
               e == s.e;
    }

    constexpr auto operator<(const _Slot& s) const noexcept -> bool
    {
        return (w < s.w ||
               (w == s.w && e < s.e));
    }

    constexpr auto is_defined() const noexcept -> bool
    {
        return *this != _Slot{};
    }

    constexpr auto we() const noexcept
    {
        return std::array {w, e};
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
           ? _Slot{w, Ends::opp(e)}
           : _Slot{};
    }

    constexpr void write(std::ofstream& ofs) const
    {
        ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));
        ofs.write(reinterpret_cast<const char*>(&e), sizeof(e));
    }

    void print() const
    {
       log_<false>('{', str_short(), '}');
    }

    auto str_long() const -> std::string
    {
        return "w "s + std::to_string(w) +
               (isEnd ? " e " : " a ") + ea_str();
    }

    auto str_short() const -> std::string
    {
        return std::to_string(w) + " " + ea_str();
    }

    HostId w {undefined<HostId>};
    LocId  e {undefined<LocId>};
};


/// A pair {Edge::indc, Edge::end}
using IndcEgEnd = _Slot<EgId, Ends>;


/// A pair {Edge::ind, Chain::end}
using IndChEnd = _Slot<EgId, Ends>;


/// A pair {Edge::ind, Edge::end}
using IndEgEnd = _Slot<EgId, Ends>;


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_SLOT_H
