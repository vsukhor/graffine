/* =============================================================================

This file is part of graffine, a lightweight graph transformation library.

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
 * \file neigs.h
 * \brief Defines container holding slots.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H
#define GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H

#include "graffine/definitions.h"
#include "graffine/structure/core/elements/vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <array>
#include <iostream>
#include <vector>

namespace graffine::structure::containers {


/**
 * Container struct for holding and managing slots connected to the current one.
 * \details This container holds all the slots that are directly connected to
 * the current slot in the chain. It provides methods to add, remove, and access
 * the connected slots.
 */
template<typename S>
struct Neigs {

    static constexpr auto maxNum = maxDegree - 1;

    template<typename T>
    using container = std::vector<T>;

    using Slot = S;
    using Slots = container<Slot>;
    using Ws = container<typename Slot::LocId>;
    using Es = container<descriptors::End::Id>;

    constexpr Neigs() = default;
    constexpr Neigs(const Neigs&) = default;
    constexpr Neigs(Neigs&&) = default;
    constexpr Neigs& operator=(const Neigs&) = default;
    constexpr Neigs& operator=(Neigs&&) = default;
    ~Neigs() = default;

    explicit constexpr Neigs(const Slot& s)
        : ss {s}
    {}

    explicit constexpr Neigs(Slot&& s)
        : ss {std::move(s),
               std::move(Slot{}),
               std::move(Slot{})}
    {}

    explicit constexpr Neigs(const Slot& s1, const Slot& s2)
        : ss {s1, s2}
    {}

    explicit constexpr Neigs(Slot&& s1, Slot&& s2)
        : ss {std::move(s1),
               std::move(s2)}
    {}

    explicit constexpr Neigs(const Slot& s1, const Slot& s2, const Slot& s3)
        : ss {s1, s2, s3}
    {}

    explicit constexpr Neigs(Slot&& s1, Slot&& s2, Slot&& s3)
        : ss {std::move(s1),
               std::move(s2),
               std::move(s3)}
    {}

    explicit constexpr Neigs(Slots&& ss)
        : ss {std::move(ss)}
    {}

    constexpr auto operator==(const Neigs& other) const noexcept -> bool
    {
        if (num() != other.num())
            return false;

        if (num() > 1) {
            auto sortedThis = ss;
            std::sort(sortedThis.begin(), sortedThis.end());
            auto sortedOther = other.ss;
            std::sort(sortedOther.begin(), sortedOther.end());
            return sortedThis == sortedOther;
        }
        else
            return ss == other.ss;

    }


    constexpr auto operator[](const std::size_t i) const noexcept -> const Slot&
    {
        auto b = ss.begin();
        if (i) std::advance(b, i);

        return *b;
    }

    constexpr auto contains(const Slot& u) const -> bool
    {
        for (const auto& s : ss)
            if (s == u)
                return true;

        return false;
    }

    auto remove(const Slot& u) -> bool
    {
        ASSERT(contains(u), "Attempted remove() of an absent slot ",
               u.w, " ",u.ea_str());

        for (auto i = ss.cbegin(); i != ss.cend(); ++i)
            if (*i == u) {
                ss.erase(i);
                ASSERT(!contains(u), "Failed remove() of ", u.w, " ", u.ea_str());
                return true;
            }

        return false;
    }

    auto replace(const Slot& f, const Slot& t) -> bool
    {
        ASSERT(contains(f), "Attempted replace() of an absent slot ",
               f.w, " ",f.ea_str());

        for (auto i = ss.begin(); i != ss.end(); ++i)
            if (*i == f) {
                *i = t;
                ASSERT(!contains(f), "Failed replace() of ", f.w, " ", f.ea_str());
                ASSERT(contains(t), "Failed replace() of ", t.w, " ", t.ea_str());
                return true;
            }

        return false;
    }

    constexpr void clear() noexcept
    {
        ss.clear();
    }

    constexpr void insert(const Slot& s)
    {
        ss.push_back(s);
    }

    constexpr void insert(Slot&& s)
    {
        ss.push_back(std::move(s));
    }

    constexpr auto front() const noexcept -> const Slot&
    {
        return ss.front();
    }

    constexpr auto back() const noexcept -> const Slot&
    {
        return ss.back();
    }

    constexpr auto operator()() const noexcept -> const Slots&
    {
        return ss;
    }

    constexpr auto num() const noexcept -> std::size_t
    {
        return ss.size();
    }

    constexpr auto ww() const noexcept -> Ws
    {
        Ws res {};
        for (const auto& s : ss)
            res.push_back(s.w);

        return res;
    }

    constexpr auto es() const noexcept -> Es
    {
        Ws res {};
        for (const auto& s : ss)
            res.push_back(s.e);

        return res;
    }

    auto other_than(const Slots& uu) const -> Slots
    {
        Slots res = ss;

        for (const auto& u: uu)
            for (auto i = res.cbegin(); i != res.cend(); ++i)
                if (*i == u) {
                    res.erase(i);
                    break;
                }

        return res;
    }

    auto has_repeating_slots() const noexcept -> bool
    {
        for (auto i1 = ss.cbegin(); i1 != ss.cend(); std::advance(i1, 1))
            for (auto i2 = std::next(i1); i2 != ss.cend(); ++i2)
                if (*i1 == *i2)
                    return true;

        return false;
    }

    template<bool ENDL=false>
    void print() const
    {
        for (const auto& s : ss)
            s.print();
        jot<ENDL>("");
    }

    void write(std::ofstream& ofs) const
    {
        const auto n = num();
        ofs.write(reinterpret_cast<const char*>(&n), sizeof(n));
        for (const auto& s : ss)
            s.write(ofs);
    }

private:

    Slots ss;

};

template<Degree D>
inline constexpr auto numNeigs = D - 1;

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H
