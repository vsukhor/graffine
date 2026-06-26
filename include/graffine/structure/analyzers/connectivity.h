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

    auto find_connected_chains(const EndSlot& source) noexcept -> ChIdGs;
    auto find_connected_chains(ChIdG seed) noexcept -> ChIdGs;


    /**
     * Depth-first search of the component edges.
     * \note Performs necessary initializations of the auxiliary variables
     * before lanching the search.
     * \param[in] source Initial slot.
     * \param[in] target Target slot.
     * \param[in] n size of the chain set to parce
     * \return True if there is a connection between \p source and \p target.
     */
    bool dfs(
        const EndSlot& source,
        const EndSlot& target,
        std::size_t n);


protected:

    const Chains& cn;

    ChIdGs visited;  ///< Auxiliary: IDs of chains visited during a search.

    void reset_search(std::size_t s) noexcept;

    /**
     * Depth-first search over the component edges.
     * \note Assumes that initialization of the auxiliary variables is
     * already done.
     * \param[in] source Search starting slot.
     * \param[in] target Search target slot.
     * \param[in] overComponent true if enumerates over single component
     * \return True iff there is a connection between \p source and \p target.
     */
   bool dfs_(
        const EndSlot& source,
        const EndSlot& target,
        bool overComponent);
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
find_connected_chains(const EndSlot& source) noexcept -> ChIdGs
{
    reset_search(cn.size());

    dfs_(source, EndSlot{}, false);

    ChIdGs vv {};

    std::copy_if(visited.begin(), visited.end(),
                 std::back_inserter(vv),
                 [&](ChIdG v) { return v.is_defined(); });

    if (std::find(vv.begin(), vv.end(), source.w) == vv.end())
        vv.push_back(source.w);

   return vv;
}


template<typename Ch>
auto Connectivity<Ch>::
find_connected_chains(const ChIdG seed) noexcept -> ChIdGs
{
    auto r1 = find_connected_chains(EndSlot{seed, End::A});
    auto r2 = find_connected_chains(EndSlot{seed, End::B});

    std::sort(r1.begin(), r1.end());
    std::sort(r2.begin(), r2.end());

    ChIdGs res {};

    auto append = [&res](auto&& r)
    {
        for (auto& rr : r)
            if (std::find(res.begin(), res.end(), rr) == res.end())
                res.push_back(rr);
    };

    append(r1);
    append(r2);

    return res;
}


template<typename Ch>
bool Connectivity<Ch>::
dfs(const EndSlot& source,
    const EndSlot& target,
    const std::size_t n)
{
    // Nothing is visited in the beginning of the search.
    reset_search(n);

    // This is true iff there is a connection to s2 via a path outgoing from s1.
    return dfs_(source,
                target,
                n == cn.size() ? false : true);
}


template<typename Ch>
bool Connectivity<Ch>::
dfs_(const EndSlot& source,
     const EndSlot& target,
     const bool overComponent)
{
    auto j = [&](const ChIdG& w)
    {
        return overComponent ? cn[w].idc()
                             : w();
    };

    if (source == target) {

        // Target found: mark current source as visited and finish:
        visited[j(source.w)] = source.w;
        return true;
    }

    // Else process neigs of the source slot:
    for (const auto& s : cn[source.w].ngs[source.e]()) {

        if (!cn[s.w].is_connected_at(End::opp(s.e))) {
            // If neig chain is not connected on its opposite end:

            if (s.w == target.w) {
                if (s.e == target.e) {
                    // If its connected slot is a target, finish
                    return true;
                }
            }
            else {
                // if neig chain is not a target chain:
                auto& v = visited[j(s.w)];  // ref
                if (!v.is_defined()) {
                    // If it was not yet visited, mark it
                    // and proceed starting from its opposite end
                    v = s.w;
                    if (dfs_(s.opp(), target, overComponent))
                        return true;
                }
            }
        }
        else if (cn[s.w].is_connected_cycle()) {

            // If neig chain is connected cycle, it has no neigs on its
            // opposite end which are not on the path to parce, so mark it
            // If it is a target chain, finish, else let the for loop proceed:
            visited[j(s.w)] = s.w;
            if (s.w == target.w)
                return true;
        }
    }

    return false;
}


template<typename Ch>
void Connectivity<Ch>::
reset_search(const std::size_t n) noexcept
{
    visited.resize(n);
    std::fill(visited.begin(),
              visited.begin() + static_cast<long>(n),
              ChIdG::undefined);
}


}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ANALYZERS_CONNECTIVITY_H
