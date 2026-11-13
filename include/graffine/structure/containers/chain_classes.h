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
 * \file chain_classes.h
 * \brief Chains classified by the degrees of their end vertices.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_CLASSES_H
#define GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_CLASSES_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <map>
#include <ostream>
#include <vector>

/// Containers of graph elements and of their descriptors.
namespace graffine::structure::containers {

/** Chains classified according to end degrees.
 * \details A chain is classified by the degrees of the vertices at its two
 * ends (Chain::degree_at_end()):
 * - a chain having degree \a d at both ends is listed by its id under key
 *   \a d, accessed by operator()(Degree);
 * - a chain having different degrees is listed by the end slot of its
 *   lower-degree end, under key {lower, higher}, accessed by
 *   operator()(const Key&).
 *
 * E.g. {1, 1} holds the unconnected linear chains, and {2, 2} the
 * unconnected cycles; num() accepts both kinds of key.
 *
 * Each component keeps its chains classified; the transformations use the
 * classes to find candidate chains quickly, e.g. all free ends (degree 1).
 * \tparam ES Type of end slot.
 */
template<typename ES>
struct ChainClasses
{
    using End = descriptors::End;
    using EndSlot = ES;  ///< Type of end slot.
    using ChId = ES::HostId;              ///< Chain id type.
    using ChIds = std::vector<ChId>;      ///< List of chain ids.
    using Slots = std::vector<EndSlot>;   ///< List of end slots.
    using Key = std::array<Degree, 2>;  ///< End degrees, in ascending order.

    template<typename> friend struct Component;
    static constexpr ChIds emptyChids {};  ///< Returned for an empty class.
    static constexpr Slots emptySlots {};  ///< Returned for an empty class.

private:

    std::map<Degree, ChIds> symmetric;  ///< Chains with equal end degrees.
    std::map<Key, Slots> asymmetric;    ///< Lower-degree ends of the others.

public:

    constexpr ChainClasses() = default;
    constexpr ChainClasses(const ChainClasses& other) = default;
    constexpr ChainClasses(ChainClasses&& other) = default;
    constexpr auto operator=(const ChainClasses& other) -> ChainClasses& = default;
    constexpr auto operator=(ChainClasses&& other) -> ChainClasses& = default;
    ~ChainClasses() = default;

    bool operator==(const ChainClasses& other) const noexcept = default;

    /// Ids of the chains having degree \p d at both ends.
    constexpr auto operator()(Degree d) const -> const ChIds&;
    /// Lower-degree end slots of the chains having end degrees \p k;
    /// \p k[0] must be lower than \p k[1].
    constexpr auto operator()(const Key& k) const -> const Slots&;

    /// Chains with both ends free: unconnected linear chains.
    constexpr auto cn_11() const -> const ChIds& { return (*this)(Deg1); }
    /// Chains with degree 2 at both ends: unconnected cycles.
    constexpr auto cn_22() const -> const ChIds& { return (*this)(Deg2); }
    /// Chains with degree 3 at both ends.
    constexpr auto cn_33() const -> const ChIds& { return (*this)(Deg3); }
    /// Chains with degree 4 at both ends.
    constexpr auto cn_44() const -> const ChIds& { return (*this)(Deg4); }
    /// Free ends of the chains with degrees 1 and 3 at their ends.
    constexpr auto cn_13() const -> const Slots& { return (*this)({Deg1, Deg3}); }
    /// Free ends of the chains with degrees 1 and 4 at their ends.
    constexpr auto cn_14() const -> const Slots& { return (*this)({Deg1, Deg4}); }
    /// Degree 3 ends of the chains with degrees 3 and 4 at their ends.
    constexpr auto cn_34() const -> const Slots& { return (*this)({Deg3, Deg4}); }

    /// Lists chain \p v as having degree \p k at both ends.
    constexpr void insert(Degree k, ChId v);
    /// Lists end slot \p v, of a chain having end degrees \p k.
    constexpr void insert(const Key& k, EndSlot&& v) noexcept;

    /** Number of chains having end degrees \p k, equal or not.
     */
    constexpr auto num(const Key& k) const -> std::size_t;

    /** Total number of chains classified.
     */
    constexpr auto num_chains() const noexcept -> std::size_t;

    /// Empties the class of chains having degree \p k at both ends.
    constexpr void clear(Degree k);
    /// Empties the class of chains having end degrees \p k.
    constexpr void clear(const Key& k) noexcept;
    /// Empties all the classes.
    constexpr void clear() noexcept;

    /// Moves the chains classified in \p other into the corresponding
    /// classes of this object, e.g. when merging components.
    void append(ChainClasses&& other);

    /// Classifies chain \p m by its current end degrees and lists it.
    template<typename Chain>
    void include(const Chain& m);

    /// Removes chain \p m from the class of its current end degrees.
    /// \note Must be called before the end degrees of \p m change.
    template<typename Chain>
    void remove(const Chain& m);

    /// Clears the classes, then classifies all the chains \p cn.
    template<typename Chains>
    void populate(const Chains& cn);

    /// Clears the classes, then classifies the chains \p ww of \p cn.
    template<typename Chains>
    void populate(const Chains& cn, const ChIds& ww);

    /// Writes the number of chains in each class to \p ofs, as
    /// " s_<deg1>_<deg2> <number>" per class.
    void report(std::ostream& ofs) const;

    /// Prints the classes, preceded by the concatenation of \p args, if any.
    template<typename... Args>
    void print(Args&&... args) const;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename ES>
constexpr
auto ChainClasses<ES>::
operator()(const Degree d) const -> const ChIds&
{
    return symmetric.contains(d)
        ? symmetric.at(d)
        : emptyChids;
}

template<typename ES>
constexpr
auto ChainClasses<ES>::
operator()(const Key& k) const -> const Slots&
{
    GRAFFINE_ASSERT(k[0] < k[1], "Incorrect order of key elements.");

    return asymmetric.contains(k)
        ? asymmetric.at(k)
        : emptySlots;
}

template<typename ES>
constexpr
void ChainClasses<ES>::
clear() noexcept
{
    symmetric.clear();
    asymmetric.clear();
}

template<typename ES>
constexpr
void ChainClasses<ES>::
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
void ChainClasses<ES>::
clear(const Degree d)
{
    if (symmetric.contains(d))
        symmetric.at(d).clear();
}

template<typename ES>
void ChainClasses<ES>::
append(ChainClasses&& other)
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
}

template<typename ES>
constexpr
void ChainClasses<ES>::
insert(const Degree k, const ChId v)
{
    symmetric[k].push_back(v);
}

template<typename ES>
constexpr
void ChainClasses<ES>::
insert(const Key& k, EndSlot&& v) noexcept
{
    asymmetric[k].push_back(std::move(v));
}

template<typename ES>
template<typename Chains>
void ChainClasses<ES>::
populate(const Chains& cn)
{
    clear();

    for (const auto& m: cn)
        include(m);
}

template<typename ES>
template<typename Chains>
void ChainClasses<ES>::
populate(const Chains& cn,
         const ChIds& ww)
{
    clear();

    for (const auto& j: ww)
        include(cn[j]);
}

template<typename ES>
template<typename Chain>
void ChainClasses<ES>::
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
void ChainClasses<ES>::
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
auto ChainClasses<ES>::
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
auto ChainClasses<ES>::
num(const Key& k) const -> std::size_t
{
    GRAFFINE_ASSERT(k[0] <= k[1], "Incorrect order of key elements.");

    return k[0] == k[1]
        ? (symmetric.contains(k[0]) ? symmetric.at(k[0]).size() : 0)
        : (asymmetric.contains(k) ? asymmetric.at(k).size() : 0);
}

template<typename ES>
void ChainClasses<ES>::
report(std::ostream& ofs) const
{
    // E.g. " s_1_1 2 s_1_3 4" for two {1, 1} chains and four {1, 3} ones.
    auto formatted = [&ofs](const Degree k0, const Degree k1, const auto& v)
    {
        ofs << " s_" << k0 << '_' << k1 << ' ' << v.size();
    };

    for (const auto& [k, v]: symmetric)
        formatted(k, k, v);

    for (const auto& [k, v]: asymmetric)
        formatted(k[0], k[1], v);
}

template<typename ES>
template<typename... Args>
void ChainClasses<ES>::
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

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_CHAIN_CLASSES_H
