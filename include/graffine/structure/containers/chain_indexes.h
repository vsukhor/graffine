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
 * \file chain_indexes.h
 * \brief Contains collections of chain ids classified by end degrees.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_INDEXES_H
#define GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_INDEXES_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <map>
#include <ostream>
#include <vector>

/// Classes defining collections of graph elements and their descriptors.
namespace graffine::structure::containers {

/**
 * Chain indexes classified according to end degrees.
 * \tparam ES Type of end slot.
 */
template<typename ES>
struct ChainIndexes
{
    using End = descriptors::End;
    using EndSlot = ES;  ///< Type of end slot.
    using ChId = ES::HostId;
    using ChIds = std::vector<ChId>;
    using Slots = std::vector<EndSlot>;
    using Key = std::array<Degree, 2>;
    template<typename> friend struct Component;
    static constexpr ChIds emptyChids {};
    static constexpr Slots emptySlots {};

private:

    std::map<Degree, ChIds> symmetric;
    std::map<Key, Slots> asymmetric;

public:

    constexpr ChainIndexes() = default;
    constexpr ChainIndexes(const ChainIndexes& other) = default;
    constexpr ChainIndexes(ChainIndexes&& other) = default;
    constexpr auto operator=(const ChainIndexes& other) -> ChainIndexes& = default;
    constexpr auto operator=(ChainIndexes&& other) -> ChainIndexes& = default;
    ~ChainIndexes() = default;

    bool operator==(const ChainIndexes& other) const noexcept = default;

    constexpr auto operator()(Degree d) const noexcept -> const ChIds&;
    constexpr auto operator()(const Key& k) const noexcept -> const Slots&;

    constexpr auto cn_11() const noexcept -> const ChIds& { return (*this)(Deg1); }
    constexpr auto cn_22() const noexcept -> const ChIds& { return (*this)(Deg2); }
    constexpr auto cn_33() const noexcept -> const ChIds& { return (*this)(Deg3); }
    constexpr auto cn_44() const noexcept -> const ChIds& { return (*this)(Deg4); }
    constexpr auto cn_13() const noexcept -> const Slots& { return (*this)({Deg1, Deg3}); }
    constexpr auto cn_14() const noexcept -> const Slots& { return (*this)({Deg1, Deg4}); }
    constexpr auto cn_34() const noexcept -> const Slots& { return (*this)({Deg3, Deg4}); }

    constexpr void insert(Degree k, ChId v) noexcept;
    constexpr void insert(const Key& k, EndSlot&& v) noexcept;

    constexpr auto num(const Key& k) const noexcept -> std::size_t;

    constexpr auto num_chains() const noexcept -> std::size_t;

    constexpr void clear(Degree k) noexcept;
    constexpr void clear(const Key& k) noexcept;
    constexpr void clear() noexcept;

    void append(ChainIndexes&& other);

    template<typename Chain>
    void include(const Chain& m);

    template<typename Chain>
    void remove(const Chain& m);

    template<typename Chains>
    void populate(const Chains& cn);

    template<typename Chains>
    void populate(const Chains& cn, const ChIds& ww);

    void report(std::ostream& ofs) const;

    template<typename... Args>
    void print(Args&&... args) const;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename ES>
constexpr
auto ChainIndexes<ES>::
operator()(const Degree d) const noexcept -> const ChIds&
{
    return symmetric.contains(d)
        ? symmetric.at(d)
        : emptyChids;
}

template<typename ES>
constexpr
auto ChainIndexes<ES>::
operator()(const Key& k) const noexcept -> const Slots&
{
    ASSERT(k[0] < k[1], "Incorreect order of key elements.");
//    ASSERT(asymmetric.contains(k), "Degree acsessed is missing.");

    return asymmetric.contains(k)
        ? asymmetric.at(k)
        : emptySlots;
}

template<typename ES>
constexpr
void ChainIndexes<ES>::
clear() noexcept
{
    symmetric.clear();
    asymmetric.clear();
}

template<typename ES>
constexpr
void ChainIndexes<ES>::
clear(const Key& k) noexcept
{
    if (k[0] == k[1]) {
        if (symmetric.contains(k[0]))
            symmetric.at(k[0]).clear();
    }
    else {
        if (asymmetric.contains(k))
            asymmetric.at(k).clear();
    }
}

template<typename ES>
constexpr
void ChainIndexes<ES>::
clear(const Degree d) noexcept
{
    if (symmetric.contains(d))
        symmetric.at(d).clear();
}

template<typename ES>
void ChainIndexes<ES>::
append(ChainIndexes&& other)
{
    auto mv = [](auto& from, auto& to)
    {
        std::move(from.begin(), from.end(), std::back_inserter(to));
        from.erase(from.begin(), from.end());
    };

    for (auto& [k, v] : other.symmetric)
        mv(v, symmetric[k]);
    for (auto& [k, v] : other.asymmetric)
        mv(v, asymmetric[k]);

//    mv(other.cn_1_1, cn_1_1);
}

template<typename ES>
constexpr
void ChainIndexes<ES>::
insert(const Degree k, const ChId v) noexcept
{
    symmetric[k].push_back(v);
}

template<typename ES>
constexpr
void ChainIndexes<ES>::
insert(const Key& k, EndSlot&& v) noexcept
{
    asymmetric[k].push_back(std::move(v));
}

template<typename ES>
template<typename Chains>
void ChainIndexes<ES>::
populate(const Chains& cn)
{
    clear();

    for (const auto& m: cn)
        include(m);
}

template<typename ES>
template<typename Chains>
void ChainIndexes<ES>::
populate(const Chains& cn,
         const ChIds& ww)
{
    clear();

    for (const auto& j: ww)
        include(cn[j]);
}

template<typename ES>
template<typename Chain>
void ChainIndexes<ES>::
include(const Chain& m)
{
    const auto degA = m.degree_at_end(End::A);
    const auto degB = m.degree_at_end(End::B);

    degA == degB
        ? symmetric[degA].push_back(m.idw)
        : degA < degB
            ? asymmetric[{degA, degB}].push_back(EndSlot{m.idw, End::A})
            : asymmetric[{degB, degA}].push_back(EndSlot{m.idw, End::B});
}

template<typename ES>
template<typename Chain>
void ChainIndexes<ES>::
remove(const Chain& m)
{
    const auto degA = m.degree_at_end(End::A);
    const auto degB = m.degree_at_end(End::B);

    degA == degB
        ? std::erase(symmetric[degA], m.idw)
        : degA < degB
            ? std::erase(asymmetric[{degA, degB}], EndSlot{m.idw, End::A})
            : std::erase(asymmetric[{degB, degA}], EndSlot{m.idw, End::B});
}

template<typename ES>
constexpr
auto ChainIndexes<ES>::
num_chains() const noexcept -> std::size_t
{
    std::size_t n {};
    for (const auto& [k, v]: symmetric)
        n += v.size();
    for (const auto& [k, v]: asymmetric)
        n += v.size();

    return n;
}

template<typename ES>
constexpr
auto ChainIndexes<ES>::
num(const Key& k) const noexcept -> std::size_t
{
    ASSERT(k[0] <= k[1], "Incorreect order of key elements.");

    return k[0] == k[1]
        ? (symmetric.contains(k[0]) ? symmetric.at(k[0]).size() : 0)
        : (asymmetric.contains(k) ? asymmetric.at(k).size() : 0);
}

template<typename ES>
void ChainIndexes<ES>::
report(std::ostream& ofs) const
{
    auto formatted = [](const auto& k, const auto& v) -> std::string_view
    {
        return {"s_", k[0], "_", k[1], " ", v.size()};
    };

    for (const auto& [k, v]: symmetric)
        ofs << formatted({k, k}, v);

    for (const auto& [k, v]: asymmetric)
        ofs << formatted(k, v);
}

template<typename ES>
template<typename... Args>
void ChainIndexes<ES>::
print(Args&&...args) const
{
    if constexpr (sizeof...(Args))
        jot(args...);

    for (const auto& [k, v]: symmetric) {

        jot<false>("Chains ", k, "_", k, ":  ");
        for (const auto& w : v)
            jot<false>(w, " ");
        jot("");
    }

    for (const auto& [k, v] : asymmetric) {

        jot<false>("Chains ", k[0], "_", k[1], ":  ");
        for (const auto& w : v)
            jot<false>("{", w.str_short(), "} ");
        jot("");
    }

    jot("");
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_INDEXES_H
