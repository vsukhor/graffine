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
 * @file neigs.h
 * @brief Defines container holding connected slots.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_NEIGS_H
#define GRAPH_MUTATOR_STRUCTURE_NEIGS_H

#include <array>
#include <iostream>
#include <vector>

#include "../definitions.h"
#include "ends.h"
#include "slot.h"
#include "vertices/vertex.h"


namespace graph_mutator::structure {


/**
 * @brief Container holding slots connected to the current one.
 * @details This container holds all the slots that are directly connected to
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
    using Es = container<Ends::Id>;

    constexpr Neigs() {};
    Neigs(const Neigs&) = default;
    Neigs(Neigs&&) = default;
    Neigs& operator=(const Neigs&) = default;
    Neigs& operator=(Neigs&&) = default;
    ~Neigs() = default;

    explicit constexpr Neigs(const Slot& s)
        : ss_ {s}
    {}

    explicit constexpr Neigs(Slot&& s)
        : ss_ {std::move(s),
               std::move(Slot{}),
               std::move(Slot{})}
    {}

    explicit constexpr Neigs(const Slot& s1, const Slot& s2)
        : ss_ {s1, s2}
    {}

    explicit constexpr Neigs(Slot&& s1, Slot&& s2)
        : ss_ {std::move(s1),
               std::move(s2)}
    {}

    explicit constexpr Neigs(const Slot& s1, const Slot& s2, const Slot& s3)
        : ss_ {s1, s2, s3}
    {}

    explicit constexpr Neigs(Slot&& s1, Slot&& s2, Slot&& s3)
        : ss_ {std::move(s1),
               std::move(s2),
               std::move(s3)}
    {}

    constexpr auto operator==(const Neigs& other) const noexcept -> bool
    {
        if (num() != other.num())
            return false;

        if (num() > 1) {
            auto ss = ss_;
            std::sort(ss.begin(), ss.end());
            auto oss = other.ss_;
            std::sort(oss.begin(), oss.end());
            return ss == oss;
        }
        else
            return ss_ == other.ss_;

    }

    constexpr auto operator[](const szt i) const noexcept -> const Slot&
    {
        auto b = ss_.begin();
        if (i) std::advance(b, i);

        return *b;
    }

    constexpr auto has(const Slot& u) const -> bool
    {
        for (const auto& s : ss_)
            if (s == u)
                return true;

        return false;
    }

    auto remove(const Slot& u) -> bool
    {
        ASSERT(has(u), "Attempted remove() of an absent slot ",
               u.w, " ",u.ea_str());

        for (auto i = ss_.cbegin(); i != ss_.cend(); ++i)
            if (*i == u) {
                ss_.erase(i);
                ASSERT(!has(u), "Failed remove() of ", u.w, " ", u.ea_str());
                return true;
            }

        return false;
    }

    auto replace(const Slot& f, const Slot& t) -> bool
    {
        ASSERT(has(f), "Attempted replace() of an absent slot ",
               f.w, " ",f.ea_str());

        for (auto i = ss_.begin(); i != ss_.end(); ++i)
            if (*i == f) {
                *i = t;
                ASSERT(!has(f), "Failed replace() of ", f.w, " ", f.ea_str());
                ASSERT(has(t), "Failed replace() of ", t.w, " ", t.ea_str());
                return true;
            }

        return false;
    }

    constexpr void clear() noexcept
    {
        ss_.clear();
    }

    constexpr void insert(const Slot& s)
    {
        ss_.push_back(s);
    }

    constexpr void insert(Slot&& s)
    {
        ss_.push_back(std::move(s));
    }

    constexpr auto front() const noexcept -> const Slot&
    {
        return ss_.front();
    }

    constexpr auto back() const noexcept -> const Slot&
    {
        return ss_.back();
    }

    constexpr auto operator()() const noexcept -> const Slots&
    {
        return ss_;
    }

    constexpr auto num() const noexcept -> szt
    {
        return ss_.size();
    }

    constexpr auto ws() const noexcept -> Ws
    {
        Ws res {};
        for (const auto& s : ss_)
            res.push_back(s.w);

        return res;
    }

    constexpr auto es() const noexcept -> Es
    {
        Ws res {};
        for (const auto& s : ss_)
            res.push_back(s.e);

        return res;
    }

    auto other_than(const Slots& uu) const -> Slots
    {
        Slots res = ss_;

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
        for (auto i1 = ss_.cbegin(); i1 != ss_.cend(); std::advance(i1, 1))
            for (auto i2 = std::next(i1); i2 != ss_.cend(); ++i2)
                if (*i1 == *i2)
                    return true;

        return false;
    }

    template<bool ENDL=false>
    void print() const
    {
        for (const auto& s : ss_)
            s.print();
        log_<ENDL>("");
    }

    void write(std::ofstream& ofs) const
    {
        const auto n = num();
        ofs.write(reinterpret_cast<const char*>(&n), sizeof(n));
        for (const auto& s : ss_)
            s.write(ofs);
    }

private:

    Slots ss_;

};

}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_NEIGS_H
