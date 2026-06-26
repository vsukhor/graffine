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
 * \file neigs.h
 * \brief Defines Container holding slots.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H
#define GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <array>
#include <iostream>
#include <set>
#include <vector>

namespace graffine::structure::containers {


/**
 * Container struct for holding and managing slots connected to the current one.
 * \details This Container holds all the slots that are directly connected to
 * the current slot in the chain. It provides methods to add, remove, and access
 * the connected slots.
 */
template<typename S>
struct Neigs
{
    static constexpr auto maxNum = maxDegree - 1;

    template<typename T> using ContainerTempl = std::set<T>;
    using Container = ContainerTempl<S>;

    using size_type = Container::size_type;
    using value_type = Container::value_type;
    using node_type = Container::node_type;
    using key_type = Container::key_type;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;
    using reference = Container::reference;
    using const_reference = Container::const_reference;

    using Slot = S;
    using Slots = Container;
    using Ws = std::vector<typename Slot::HostId>;
    using Es = std::vector<typename Slot::LocId>;
    using End = descriptors::End;
    static_assert(std::is_same_v<typename Slot::LocId, End::Id>);

    constexpr Neigs() = default;
    constexpr Neigs(const Neigs&) = default;
    constexpr Neigs(Neigs&&) = default;
    constexpr Neigs& operator=(const Neigs&) = default;
    constexpr Neigs& operator=(Neigs&&) = default;
    constexpr ~Neigs() = default;

    explicit constexpr Neigs(const Slot& s)
        : ss {s}
    {}

    explicit constexpr Neigs(Slot&& s)
        : ss {std::move(s)}
    {}

    explicit constexpr Neigs(Slots&& ss)
        : ss {std::move(ss)}
    {}

    constexpr Neigs(std::initializer_list<Slot> init) noexcept
        : ss {init}
    {}

    // ss is std::set, so the two sides must have the same number of elements
    // and each element must compare == with the element at the same position.
    bool operator==(const Neigs& other) const noexcept = default;

    constexpr auto begin() noexcept -> iterator
    {
        return ss.begin();
    }
    constexpr auto begin() const noexcept -> const_iterator
    {
        return ss.cbegin();
    }

    constexpr auto end() noexcept -> iterator
    {
        return ss.end();
    }
    constexpr auto end() const noexcept -> const_iterator
    {
        return ss.cend();
    }

    constexpr bool contains(const Slot& u) const
    {
        return ss.contains(u);
    }

    bool erase(const Slot& u)
    {
        ASSERT(contains(u), "Attempted erase of absent slot ", u.str_short());

        for (auto i = ss.cbegin(); i != ss.cend(); ++i)
            if (*i == u) {
                ss.erase(i);
                ASSERT(!contains(u), "Failed erase of ", u.str_short());
                return true;
            }

        return false;
    }

    bool replace(const Slot& f, const Slot& t)
    {
        ASSERT(contains(f), "Attempted replace of absent slot ",
               f.str_short());

        auto node = ss.extract(f);
        if (node) {
        node.value() = t;
            ss.insert(std::move(node));
            return true;
        }

        return false;
    }

    constexpr void clear() noexcept
    {
        ss.clear();
    }

    constexpr auto extract(const key_type& k) -> node_type
    {
        return ss.extract(k);
    }
    constexpr auto extract(const_iterator pos) -> node_type
    {
        return ss.extract(pos);
    }

    constexpr void insert(const Slot& s)
    {
        ASSERT_CALLING(!contains(s), s.print(),
                       "inserting the slot already present");
        ss.insert(s);
    }

    constexpr void insert(Slot&& s)
    {
        ASSERT_CALLING(!contains(s), s.print(),
                       "inserting the slot already present");

        ss.insert(std::move(s));
    }

    constexpr void insert(node_type&& s)
    {
        ss.insert(std::move(s));
    }

    constexpr auto operator()() const noexcept -> const Container&
    {
        return ss;
    }

    constexpr auto num() const noexcept -> std::size_t
    {
        return size();
    }
    constexpr auto size() const noexcept -> std::size_t
    {
        return ss.size();
    }

    constexpr auto as_vector() const noexcept -> std::vector<value_type>
    {
        std::vector<value_type> v;
        v.reserve(num());

        for (auto s : ss)
            v.push_back(s);

        return v;
    }

    template<size_t N>
    constexpr auto as_array() const noexcept -> std::array<value_type, N>
    {
        ASSERT(N <= num(), "Number of neogbours < N");

        std::array<value_type, N> v;

        for (std::size_t i {}; const auto s: ss)
            v[i++] = s;

        return v;
    }

    constexpr auto ww() const noexcept -> Ws
    {
        Ws res {};
        for (const auto& s : ss)
            res.push_back(s.w);

        return res;
    }

    constexpr auto ee() const noexcept -> Es
    {
        Es res {};
        for (const auto& s : ss)
            res.push_back(s.e);

        return res;
    }

    auto other_than(const Container& uu) const -> Container
    {
        Slots res = ss;

        for (const auto& u : uu)
            for (auto i = res.cbegin(); i != res.cend(); ++i)
                if (*i == u) {
                    res.erase(i);
                    break;
                }

        return res;
    }

    auto incoming() const -> ContainerTempl<const_reference>
    {
        ContainerTempl<const_reference> res;

        for (const auto& s : ss)
            if (s.e == End::B)
                res.insert(s);

        return res;
    }

    auto outgiong() const -> ContainerTempl<const_reference>
    {
        ContainerTempl<const_reference> res;

        for (const auto& s : ss)
            if (s.e == End::A)
                res.insert(s);

        return res;
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

    Container ss;
};


template<Degree D>
inline constexpr auto numNeigs = D - 1;

constexpr auto num_neigs(const Degree D) noexcept -> std::size_t
{
    return D - 1;
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H
