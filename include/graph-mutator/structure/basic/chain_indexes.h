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
 * @file chain_indexes.h
 * @brief Contains collections of chain ids classified by end degrees.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_CHAIN_INDEXES_H
#define GRAPH_MUTATOR_STRUCTURE_CHAIN_INDEXES_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <ostream>
#include <string>
#include <vector>

#include "../definitions.h"
#include "chain.h"


namespace graph_mutator::structure {


/**
 * @brief Chain indexes collected according to end degrees.
 * @tparam isSingleChain If true, the component consists of a single chain.
 * @tparam ES Type of end slot.
 */
template<bool isSingleChain,
         typename ES>
struct ChainIndexes
{
    using EndSlot = ES;  ///< Type of end slot.
    using SingleChainId =
        std::conditional_t<isSingleChain, ChId, ChIds>;

    /// Index of disconnected linear chain if it is this component or undefined.
    SingleChainId cn11;

    /// Index of disconnected cycled chain if it is this component or undefined.
    SingleChainId cn22;

    /// Indexes of chains spanned between vertices of degree 3.
    ChIds cn33;

    /// Indexes of chains spanned between vertices of degree 4.
    ChIds cn44;

    /// End slots of chains spanned between vertices of degrees 1 and 3.
    std::vector<EndSlot> cn13;

    /// End slots of chains spanned between vertices of degrees 1 and 4.
    std::vector<EndSlot> cn14;

    /// End slots of chains spanned between vertices of degrees 3 and 4.
    std::vector<EndSlot> cn34;

    constexpr ChainIndexes();
    constexpr explicit ChainIndexes(const ChainIndexes& other) = default;
    constexpr explicit ChainIndexes(ChainIndexes&& other);
    constexpr auto operator=(const ChainIndexes& other) -> ChainIndexes& = default;
    constexpr auto operator=(ChainIndexes&& other) -> ChainIndexes&;
    ~ChainIndexes() = default;

    constexpr auto operator==(const ChainIndexes& other) const noexcept -> bool;

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


template<bool isSingleChain,
         typename ES>
constexpr
ChainIndexes<isSingleChain, ES>::
ChainIndexes()
{
    if constexpr (isSingleChain)
        cn11 = undefined<SingleChainId>;
    if constexpr(isSingleChain)
        cn22 = undefined<SingleChainId>;
}

/*
template<bool isSingleChain,
         typename ES>
constexpr
ChainIndexes<isSingleChain, ES>::
ChainIndexes(const ChainIndexes& other)
    : cn11 {other.cn11}
    , cn22 {other.cn22}
    , cn33 {other.cn33}
    , cn44 {other.cn44}
    , cn13 {other.cn13}
    , cn14 {other.cn14}
    , cn34 {other.cn34}
{}
*/

template<bool isSingleChain,
         typename ES>
constexpr
ChainIndexes<isSingleChain, ES>::
ChainIndexes(ChainIndexes&& other)
    : cn11 {std::move(other.cn11)}
    , cn22 {std::move(other.cn22)}
    , cn33 {std::move(other.cn33)}
    , cn44 {std::move(other.cn44)}
    , cn13 {std::move(other.cn13)}
    , cn14 {std::move(other.cn14)}
    , cn34 {std::move(other.cn34)}
{}

/*
template<bool isSingleChain,
         typename ES>
constexpr
auto ChainIndexes<isSingleChain, ES>::
operator=(const ChainIndexes& other) -> ChainIndexes&
{
    cn11 = other.cn11;
    cn22 = other.cn22;
    cn33 = other.cn33;
    cn44 = other.cn44;
    cn13 = other.cn13;
    cn14 = other.cn14;
    cn34 = other.cn34;

    return *this;
}
*/

template<bool isSingleChain,
         typename ES>
constexpr
auto ChainIndexes<isSingleChain, ES>::
operator=(ChainIndexes&& other) -> ChainIndexes&
{
    cn11 = std::move(other.cn11);
    cn22 = std::move(other.cn22);
    cn33 = std::move(other.cn33);
    cn44 = std::move(other.cn44);
    cn13 = std::move(other.cn13);
    cn14 = std::move(other.cn14);
    cn34 = std::move(other.cn34);

    return *this;
}


template<bool isSingleChain,
         typename ES>
constexpr
auto ChainIndexes<isSingleChain, ES>::
operator==(const ChainIndexes& other) const noexcept -> bool
{
    return cn11 == other.cn11 &&
           cn22 == other.cn22 &&
           cn33 == other.cn33 &&
           cn44 == other.cn44 &&
           cn13 == other.cn13 &&
           cn14 == other.cn14 &&
           cn34 == other.cn34;
}


template<bool isSingleChain,
         typename ES>
constexpr
void ChainIndexes<isSingleChain, ES>::
clear() noexcept
{
    if constexpr (isSingleChain)
        cn11 = undefined<SingleChainId>;
    else
        cn11.clear();

    if constexpr (isSingleChain)
        cn22 = undefined<SingleChainId>;
    else
        cn22.clear();

    cn33.clear();
    cn44.clear();
    cn13.clear();
    cn14.clear();
    cn34.clear();
}


template<bool isSingleChain,
         typename ES>
void ChainIndexes<isSingleChain, ES>::
append(ChainIndexes&& other)
{
    auto mv = [](auto& from, auto& to)
    {
        std::move(from.begin(), from.end(), std::back_inserter(to));
        from.erase(from.begin(), from.end());
    };

    if constexpr (isSingleChain) {
        ASSERT(!is_defined(cn11) || !is_defined(other.cn11), "incompatibe cn11");
        if (is_defined(other.cn11))
            cn11 = other.cn11;
    }
    else
        mv(other.cn11, cn11);

    if constexpr (isSingleChain) {
        ASSERT(!is_defined(cn22) || !is_defined(other.cn22), "incompatibe cn22");
        if (is_defined(other.cn22))
            cn22 = other.cn22;
    }
    else
        mv(other.cn22, cn22);

    mv(other.cn33, cn33);
    mv(other.cn44, cn44);
    mv(other.cn13, cn13);
    mv(other.cn14, cn14);
    mv(other.cn34, cn34);
}


template<bool isSingleChain,
         typename ES>
template<typename Chains>
void ChainIndexes<isSingleChain, ES>::
populate(const Chains& cn)
{
    clear();

    for (const auto& m: cn)
        include(m);
}


template<bool isSingleChain,
         typename ES>
template<typename Chains>
void ChainIndexes<isSingleChain, ES>::
populate(const Chains& cn,
         const ChIds& ww)
{
    clear();

    for (const auto j: ww)
        include(cn[j]);
}


template<bool isSingleChain,
         typename ES>
template<typename Chain>
void ChainIndexes<isSingleChain, ES>::
include(const Chain& m)
{
    auto assignment_impossble = [&](const ChId j)
    {
        m.print("assignment impossble ");

        abort("failed classification for chain ", j);

    };

    [[maybe_unused]] const auto nA = m.ngs[Ends::A].num();
    [[maybe_unused]] const auto nB = m.ngs[Ends::B].num();

    if (m.has_one_free_end()) {
        const auto e = m.the_only_free_end();
        const auto oe = Ends::opp(e);

        if (m.ngs[oe].num() == 2)
            cn13.push_back(EndSlot{m.idw, e});

        else if (m.ngs[oe].num() == 3)
            cn14.push_back(EndSlot{m.idw, e});

        else
            assignment_impossble(m.idw);
    }
    else if (nA == 0 && nB == 0) {
        if constexpr (isSingleChain)
            cn11 = m.idw;  // having both ends free, it is a separate chain
        else
            cn11.push_back(m.idw);
    }
    else if (m.is_disconnected_cycle()) {
        if constexpr (isSingleChain)
            cn22 = m.idw;
        else
            cn22.push_back(m.idw);  // it is a separate chain as it has 2 free ends
    }
    else if (nA == 2 && nB == 2)
        cn33.push_back(m.idw);

    else if (nA == 2 && nB == 3)
        cn34.push_back(EndSlot{m.idw, Ends::A});

    else if (nA == 3 && nB == 2)
        cn34.push_back(EndSlot{m.idw, Ends::B});

    else if (nA == 3 && nB == 3)
        cn44.push_back(m.idw);

    else
        assignment_impossble(m.idw);
}


template<bool isSingleChain,
         typename ES>
template<typename Chain>
void ChainIndexes<isSingleChain, ES>::
remove(const Chain& m)
{
    auto removal_impossble = [&](const ChId j)
    {
        m.print("removal impossble ");

        abort("failed classification for chain ", j);

    };

    [[maybe_unused]] const auto nA = m.ngs[Ends::A].num();
    [[maybe_unused]] const auto nB = m.ngs[Ends::B].num();

    if (m.has_one_free_end()) {
        const auto e = m.the_only_free_end();
        const auto oe = Ends::opp(e);

        if (m.ngs[oe].num() == 2)
            std::erase(cn13, EndSlot{m.idw, e});

        else if (m.ngs[oe].num() == 3)
            std::erase(cn14, EndSlot{m.idw, e});

        else
            removal_impossble(m.idw);
    }
    else if (nA == 0 && nB == 0) {
        if constexpr (isSingleChain)
            cn11 = undefined<ChId>;
        else
            std::erase(cn11, m.idw);
    }
    else if (m.is_disconnected_cycle()) {
        if constexpr (isSingleChain)
            cn22 = undefined<ChId>;
        else
            std::erase(cn22, m.idw);
    }
    else if (nA == 2 && nB == 2)
        std::erase(cn33, m.idw);

    else if (nA == 2 && nB == 3)
        std::erase(cn34, EndSlot{m.idw, Ends::A});

    else if (nA == 3 && nB == 2)
        std::erase(cn34, EndSlot{m.idw, Ends::B});

    else if (nA == 3 && nB == 3)
        std::erase(cn44, m.idw);

    else
        removal_impossble(m.idw);
}


template<bool isSingleChain,
         typename ES>
void ChainIndexes<isSingleChain, ES>::
report(std::ostream& ofs) const
{
    ofs <<  "s11 " << cn11.size()
        << " s13 " << cn13.size()
        << " s33 " << cn33.size()
        << " s14 " << cn14.size()
        << " s34 " << cn34.size()
        << " s22 " << cn22.size()
        << " s44 " << cn44.size();
}



template<bool isSingleChain,
         typename ES>
template<typename... Args>
void ChainIndexes<isSingleChain, ES>::
print(Args&&...args) const
{
    if constexpr (sizeof...(Args))
        log_(args...);

    log_<false>("Chains 11:  ");
    if constexpr (!isSingleChain)
        for (const auto j : cn11)
            log_<false>(j, " ");
    else
        log_<false>(cn11, " ");

    log_<false>("\nChains 22:  ");
    if constexpr (!isSingleChain)
        for (const auto j : cn22)
            log_<false>(j, " ");
    else
        log_<false>(cn22, " ");

    log_<false>("\nChains 33:  ");
    for (const auto j : cn33)
        log_<false>(j, " ");

    log_<false>("\nChains 44:  ");
    for (const auto j : cn44)
        log_<false>(j, " ");

    log_<false>("\nChains 13:  ");
    for (const auto& j : cn13)
        j.print();

    log_<false>("\nChains 14:  ");
    for (const auto& j : cn14)
        j.print();

    log_<false>("\nChains 34:  ");
    for (const auto& j : cn34)
        j.print();

    log_("");
}




}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_CHAIN_INDEXES_H
