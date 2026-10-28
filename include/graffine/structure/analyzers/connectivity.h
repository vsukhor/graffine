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
 * \file connectivity.h
 * \brief Contains class useful for connectivity analysis between graph elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ANALYZERS_CONNECTIVITY_H
#define GRAFFINE_STRUCTURE_ANALYZERS_CONNECTIVITY_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/chains.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/descriptors/slot.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

namespace graffine::structure {

template<typename Ch>
struct Connectivity
{
    using Chain = Ch;
    using Chains = structure::containers::Chains<Chain>;
    using End = descriptors::End;
    using EndSlot = Chain::EndSlot;


    explicit constexpr Connectivity(const Chains& cn);

    /**
     * Chains reachable from the junction at \p source.
     * \details Chain \p source.w is not entered through \p source itself,
     * but is always included in the result.
     */
    auto find_connected_chains(const EndSlot& source) -> ChIdGs;

    /// Chains of the connected set containing chain \p seed, sorted.
    auto find_connected_chains(ChIdG seed) -> ChIdGs;

    /**
     * Depth-first search over the chains reachable from the junction at
     * \p source.
     * \details Chain \p source.w is not entered through \p source itself,
     * but may be reached from its opposite end along another path.
     * \param[in] source Initial slot.
     * \param[in] target Target slot.
     * \return True if \p target is reachable from \p source.
     */
    bool dfs(
        const EndSlot& source,
        const EndSlot& target);


protected:

    const Chains& cn;

    /// Auxiliary: search epoch at which each chain was last visited.
    std::vector<std::uint32_t> mark;
    std::uint32_t epoch {};

    ChIdGs reached;           ///< Auxiliary: chains visited by a search.
    std::vector<EndSlot> todo;  ///< Auxiliary: slots awaiting expansion.

    /// Starts a new search in O(1), unless the chain set has grown.
    void reset_search();

    /// Marks chain \p w visited; returns false if it already was.
    bool visit(ChIdG w);

    /**
     * Iterative depth-first search over the chains.
     * \note Continues the current search: chains visited by preceding
     * calls since the last reset_search() are not entered again.
     * \param[in] source Search starting slot.
     * \param[in] target Search target slot; undefined to visit everything
     * reachable.
     * \return True iff \p target is reachable from \p source.
     */
    bool dfs_(
        const EndSlot& source,
        const EndSlot& target);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Ch>
constexpr
Connectivity<Ch>::
Connectivity(const Chains& cn)
    : cn {cn}
{}


template<typename Ch>
auto Connectivity<Ch>::
find_connected_chains(const EndSlot& source) -> ChIdGs
{
    reset_search();

    visit(source.w);
    dfs_(source, EndSlot{});

    return reached;
}


template<typename Ch>
auto Connectivity<Ch>::
find_connected_chains(const ChIdG seed) -> ChIdGs
{
    reset_search();

    visit(seed);
    dfs_(EndSlot{seed, End::A}, EndSlot{});
    dfs_(EndSlot{seed, End::B}, EndSlot{});

    std::sort(reached.begin(), reached.end());

    return reached;
}


template<typename Ch>
bool Connectivity<Ch>::
dfs(const EndSlot& source,
    const EndSlot& target)
{
    if (source == target)
        return true;

    reset_search();

    visit(source.w);

    return dfs_(source, target);
}


template<typename Ch>
bool Connectivity<Ch>::
dfs_(const EndSlot& source,
     const EndSlot& target)
{
    todo.clear();
    todo.push_back(source);

    while (!todo.empty()) {

        const auto s = todo.back();
        todo.pop_back();

        if (s == target)
            return true;

        // Cross the junction at s, then traverse each newly reached chain
        // to its opposite end. A chain entered at either end is complete,
        // so visiting chains rather than slots suffices.
        for (const auto& n : cn[s.w].ngs[s.e]()) {

            if (n == target)
                return true;

            if (visit(n.w))
                todo.push_back(n.opp());
        }
    }

    return false;
}


template<typename Ch>
bool Connectivity<Ch>::
visit(const ChIdG w)
{
    auto& m = mark[w()];

    if (m == epoch)
        return false;

    m = epoch;
    reached.push_back(w);

    return true;
}


template<typename Ch>
void Connectivity<Ch>::
reset_search()
{
    if (mark.size() < cn.size())
        mark.resize(cn.size(), 0);

    if (++epoch == 0) {  // wrapped around: stale marks would alias
        std::fill(mark.begin(), mark.end(), 0);
        epoch = 1;
    }

    reached.clear();
}


}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ANALYZERS_CONNECTIVITY_H
