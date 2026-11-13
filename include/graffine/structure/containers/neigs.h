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
 * \brief Ordered set of the slots meeting at a vertex.
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


/** Ordered set of slots connected at a common vertex.
 * \details Holds, for a chain end (Chain::ngs), the end slots of the other
 * chains joined at its boundary vertex, and, for a vertex
 * (Vertex::edges()), the descriptors of its incident edge ends. The set
 * provides functions to add, remove, replace and list the slots.
 * \note \a maxNum is the maximal size for a chain end: a vertex may hold up
 * to maxDegree descriptors.
 * \tparam S Slot type; its location must be an end (End::Id).
 */
template<typename S>
struct Neigs
{
    /// Maximal number of neighbours of a chain end: the other ends at a
    /// vertex of the maximal degree.
    static constexpr auto maxNum = maxDegree - 1;

    template<typename T> using ContainerTempl = std::set<T>;
    using Container = ContainerTempl<S>;  ///< Underlying ordered set.

    using size_type = Container::size_type;
    using value_type = Container::value_type;
    using node_type = Container::node_type;
    using key_type = Container::key_type;
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;
    using reference = Container::reference;
    using const_reference = Container::const_reference;

    using Slot = S;          ///< Slot type.
    using Slots = Container;  ///< Set of slots.
    using Ws = std::vector<typename Slot::HostId>;  ///< List of slot hosts.
    using Es = std::vector<typename Slot::LocId>;   ///< List of slot ends.
    using End = descriptors::End;
    static_assert(std::is_same_v<typename Slot::LocId, End::Id>);

    constexpr Neigs() = default;
    constexpr Neigs(const Neigs&) = default;
    constexpr Neigs(Neigs&&) = default;
    constexpr Neigs& operator=(const Neigs&) = default;
    constexpr Neigs& operator=(Neigs&&) = default;
    constexpr ~Neigs() = default;

    /// Constructs a set holding the single slot \p s.
    explicit constexpr Neigs(const Slot& s)
        : ss {s}
    {}

    /// Constructs a set holding the single slot \p s.
    explicit constexpr Neigs(Slot&& s)
        : ss {std::move(s)}
    {}

    /// Constructs a set taking over the slots \p ss.
    explicit constexpr Neigs(Slots&& ss)
        : ss {std::move(ss)}
    {}

    /// Constructs a set holding the slots listed.
    constexpr Neigs(std::initializer_list<Slot> init)
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

    /// Checks if slot \p u is in the set.
    constexpr bool contains(const Slot& u) const
    {
        return ss.contains(u);
    }

    /**
     * \brief Removes slot \p u.
     * \return Whether \p u was present: the callers check it.
     */
    bool erase(const Slot& u)
    {
        return ss.erase(u) > 0;
    }

    /**
     * \brief Replaces slot \p f with slot \p t, without reallocating.
     * \return Whether \p f was present: the callers check it.
     */
    bool replace(const Slot& f, const Slot& t)
    {
        auto node = ss.extract(f);
        if (node) {
        node.value() = t;
            ss.insert(std::move(node));
            return true;
        }

        return false;
    }

    /// Removes all the slots.
    constexpr void clear() noexcept
    {
        ss.clear();
    }

    /// Unlinks slot \p k from the set and returns its node (empty if
    /// absent), see std::set::extract.
    constexpr auto extract(const key_type& k) -> node_type
    {
        return ss.extract(k);
    }
    /// Unlinks the slot at \p pos from the set and returns its node.
    constexpr auto extract(const_iterator pos) -> node_type
    {
        return ss.extract(pos);
    }

    /// Inserts slot \p s, which must not be present already.
    constexpr void insert(const Slot& s)
    {
        GRAFFINE_ASSERT_CALLING(!contains(s), s.print(),
                       "inserting the slot already present");
        ss.insert(s);
    }

    /// Inserts slot \p s, which must not be present already.
    constexpr void insert(Slot&& s)
    {
        GRAFFINE_ASSERT_CALLING(!contains(s), s.print(),
                       "inserting the slot already present");

        ss.insert(std::move(s));
    }

    /// Inserts a node obtained from extract().
    constexpr void insert(node_type&& s)
    {
        ss.insert(std::move(s));
    }

    /// The underlying set.
    constexpr auto operator()() const noexcept -> const Container&
    {
        return ss;
    }

    /// Number of slots; the same as size().
    constexpr auto num() const noexcept -> std::size_t
    {
        return size();
    }
    /// Number of slots.
    constexpr auto size() const noexcept -> std::size_t
    {
        return ss.size();
    }

    /// Copy of the slots, in ascending order, as a vector.
    constexpr auto as_vector() const -> std::vector<value_type>
    {
        std::vector<value_type> v;
        v.reserve(num());

        for (auto s : ss)
            v.push_back(s);

        return v;
    }

    /// Copy of the slots, in ascending order, as an array; \p N must
    /// equal num().
    template<size_t N>
    constexpr auto as_array() const noexcept -> std::array<value_type, N>
    {
        GRAFFINE_ASSERT(N <= num(), "Number of neighbours < N");

        std::array<value_type, N> v;

        for (std::size_t i {}; const auto s: ss)
            v[i++] = s;

        return v;
    }

    /// Hosts (chains or edges) of the slots, in slot order.
    constexpr auto ww() const -> Ws
    {
        Ws res {};
        for (const auto& s : ss)
            res.push_back(s.w);

        return res;
    }

    /// Ends of the slots, in slot order.
    constexpr auto ee() const -> Es
    {
        Es res {};
        for (const auto& s : ss)
            res.push_back(s.e);

        return res;
    }

    /// Copy of the set without the slots of \p uu.
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

    /// The slots at end B of their host, e.g. the edges pointing into a
    /// vertex by their own orientation.
    auto incoming() const -> Slots
    {
        Slots res;

        for (const auto& s : ss)
            if (s.e == End::B)
                res.insert(s);

        return res;
    }

    /// The slots at end A of their host, e.g. the edges pointing out of a
    /// vertex by their own orientation.
    auto outgoing() const -> Slots
    {
        Slots res;

        for (const auto& s : ss)
            if (s.e == End::A)
                res.insert(s);

        return res;
    }

    /// Prints the slots, optionally followed by a line break.
    template<bool ENDL=false>
    void print() const
    {
        for (const auto& s : ss)
            s.print();
        jot<ENDL>("");
    }

private:

    Container ss;  ///< The slots.
};


/// Number of neighbours of a chain end at a vertex of degree \p D: the
/// other ends meeting there.
template<Degree D>
inline constexpr auto numNeigs = D - 1;

/// Number of neighbours of a chain end at a vertex of degree \p D: the
/// other ends meeting there.
constexpr auto num_neigs(const Degree D) noexcept -> std::size_t
{
    return D - 1;
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_NEIGS_H
