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
 * @file generic.h
 * @brief Specification of the template for paths over chain end slots.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_GENERIC_H
#define GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_GENERIC_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>

#include "../../../definitions.h"
#include "../../vertices/collections.h"
#include "../../vertices/vertex.h"
#include "distance.h"


namespace graph_mutator::structure::paths::over_endslots {

template<typename Component>
struct Generic {

    using Chain = Component::Chain;
    using Edge = Chain::Edge;
    using EndSlot = typename Chain::EndSlot;  ///< Type alias for end slot.
    using Ends = Edge::Ends;  ///< Type alias for edge ends.

    /// Type alias for path over consecutively connected vertexes.
    using Path = std::deque<EndSlot>;

    using Dist = Distance<Component>;
    using Distances = std::vector<Dist>;

    const Component& cmp;   ///< The graph component.

    const szt numSlots;  // number of end slots in the cluster: 2 * cmp.num_chains()

    explicit constexpr Generic(const Component& cmp) noexcept;

    template<bool withChain1>
    auto are_connected(const EndSlot& s1,
                       const EndSlot& s2) -> bool;

//    template<bool withSourceChain>
//    void mark_reachable_from_source(const EndSlot& source);

    /**
     * \brief Computes paths connecting a vertex in the graph
     * \details Computes paths starting at vertex \p source to other vertexes
     * in the connected component of the graph;
     * \note utilizes the component adjacency list of chains.
     * \note Implements Dijkstra's algorithm
     * \ref https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm.
     * \param[in] s Path element from which the paths are computed (source).
     */
    template<bool withSourceChain>
    void compute_from_source(const EndSlot& s);

    /**
     * \brief The shortest path between two graph vertex slots.
     * \details Calculates the shortest path between graph vertex slots \p s1
     * and \p s2 using \ref compute method.
     * \param[in] s1 The first slot of the path.
     * \param[in] s2 The last slot of the path.
     * \return The shortest path between slots \p s1 and \p s2 .
     */
    template<bool computeFromSource,
             bool withSourceChain>
    auto find_shortest_path(
        const EndSlot& s1,
        const EndSlot& s2
    ) -> Path;

    template<bool withSourceChain>
    auto classify_chains_by_connectivity(const EndSlot& s) const
        -> std::array<ChIds, 2>;

    auto path_chains_numedges(const Path& path) const noexcept
        -> std::tuple<std::set<ChId>, szt>;

    template<typename... Args>
    void print_distances(Args... args) const noexcept;

    template<typename... Args>
    void print_path(const Path& path, Args... args) const noexcept;

private:

    using Q = std::set<Dist>;
    Q q;

    Distances distances;

    void reset();

    /**
     * \brief Computes element indexes of the \a min_distance and \a previous.
     */
    constexpr auto element_ind(const EndSlot& s) const noexcept -> szt;

    constexpr auto element(szt i) const noexcept -> EndSlot;

    void update(
        const Q::value_type& ud,
        const EndSlot& v
    );
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Component>
constexpr
Generic<Component>::
Generic(const Component& cmp) noexcept
    : cmp {cmp}
    , numSlots {2 * cmp.num_chains()}
{}


template<typename Component>
constexpr
auto Generic<Component>::
element_ind(const EndSlot& s) const noexcept -> szt
{
    return 2 * static_cast<szt>(cmp.chain(s.w).idc) + static_cast<szt>(s.e);
}


template<typename Component>
constexpr
auto Generic<Component>::
element(const szt i) const noexcept -> EndSlot
{
    const auto w = cmp.chid(i/2); // integer division

    return is_defined(w)
        ? EndSlot {w, static_cast<Ends::Id>(i % 2)}
        : EndSlot {};
}


template<typename Component>
void Generic<Component>::
reset()
{
    distances.clear();
    distances.resize(numSlots);
    q.clear();
}


template<typename Component>
template<bool withChain1>
auto Generic<Component>::
are_connected(const EndSlot& s1,
              const EndSlot& s2) -> bool
{
    if (s1 == s2) {
        reset();
        return true;
    }

    compute_from_source<withChain1>(s1);

    return distances[element_ind(s2)].is_finite();
}


template<typename Component>
template<bool withSourceChain>
void Generic<Component>::
compute_from_source(const EndSlot& s)
{
    reset();

    distances[element_ind(s)].dist = Dist::zero;
    if constexpr (withSourceChain)
        q.emplace(s, Dist::zero);

    for (const auto& nb: cmp.chain(s.w).ngs[s.e]()) {
        distances[element_ind(nb)].set(s, Dist::zero);
        if (nb.w != s.w)
            q.emplace(nb, Dist::zero);
    }

    do {
        const auto ud = std::move(*q.begin());
        q.erase(q.begin());

        // Visit the opposite chain end of u
        update(ud, ud.prev.opp());
    } while (!q.empty());
}


template<typename Component>
void Generic<Component>::
update(
    const Q::value_type& ud,
    const EndSlot& v
)
{
    ASSERT(cmp.chain(ud.prev.w).idw == cmp.chain(v.w).idw,
           "not the same chain ends: ",
           cmp.chain(ud.prev.w).idw, " ",  cmp.chain(v.w).idw);

    const auto d = ud.dist + cmp.chain(v.w).weight();
    const auto vi = element_ind(v);

    if (d < distances[vi].dist) {
        distances[vi].set(ud.prev, d);
        q.emplace(ud.prev, d);
        for (const auto& nb: cmp.chain(v.w).ngs[v.e]()) {
            distances[element_ind(nb)].set(v, d);
            if (nb.w != v.w)
                q.emplace(nb, d);
        }
    }
}

/*
template<typename Component>
void Generic<Component>::
compute_from_source(const EndSlot& source)
{
    reset();

    distances[element_ind(source)].dist = zeroWeight;
    q.emplace(source, zeroWeight);
    for (const auto& nb: cmp.chain(source.w).ngs[source.e]()) {
        distances[element_ind(nb)].set(source, zeroWeight);
        if (nb.w != source.w)
            q.emplace(nb, zeroWeight);
    }

    do {
        const auto ud = std::move(*q.begin());
        q.erase(q.begin());

        // Visit the opposite chain end of u
        update(ud, ud.prev.opp());
    } while (!q.empty());
}
*/

template<typename Component>
template<bool computeFromSource,
         bool withSourceChain>
auto Generic<Component>::
find_shortest_path(
    const EndSlot& s1,  // starting chain end slot
    const EndSlot& s2   // final chain end slot
) -> Path
{
    if constexpr (computeFromSource)
        compute_from_source<withSourceChain>(s1);  // populate distances

    if (distances[element_ind(s2)].is_finite()) {

        // The shortest path edge sequence from s1 to s2:
        Path path {s2};
        auto u {s2};
        while(u != s1) {
            u = distances[element_ind(u)].prev;
            path.push_front(u);
        }

        return path;
    }

    return {};
}


template<typename Component>
template<bool withSourceChain>
auto Generic<Component>::
classify_chains_by_connectivity(const EndSlot& s) const -> std::array<ChIds, 2>
{
// Assumes 'distances' are set correctly.
// If not, precede by calling compute_from_source().

    ChIds accessible;
    ChIds blocked;

    for (szt i {}; i < numSlots; i +=2) {
        const ChId w = element(i).w;
        ASSERT(!withSourceChain || w != s.w ||
               distances[i].is_finite() == distances[i+1].is_finite(),
               "chain ", w, " has opposing connectivity on its ends");
        distances[i].is_finite() && w != s.w
            ? accessible.push_back(w)
            : blocked.push_back(w);
    }

    return {accessible, blocked};
}


template<typename Component>
auto Generic<Component>::
path_chains_numedges(const Path& path) const noexcept
    -> std::tuple<std::set<ChId>, szt>
{
    std::set<ChId> ww;  // set elements are unique by definition
    szt numEg {};

    for (const auto slot: path)
        ww.insert(slot.w);

    for (const auto w: ww)
        numEg += cmp.chain(w).length();

    return {ww, numEg};
}


template<typename Component>
template<typename... Args>
void Generic<Component>::
print_distances(Args... args) const noexcept
{
    log_(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, cmp.ind, colorcodes::RESET, ": Distances ",
         args...);

    for (szt i {}; i<distances.size(); ++i)
        distances[i].print(element(i));
    log_("");
}


template<typename Component>
template<typename... Args>
void Generic<Component>::
print_path(const Path& path, Args... args) const noexcept
{
    log_<false>(colorcodes::YELLOW, "Component ",
                colorcodes::BOLDCYAN, cmp.ind, colorcodes::RESET, ": ",
                args..., " path  from "),
    path.front().print();
    log_<false>(" to ");
    path.back().print();
    const auto [chs, ne] = path_chains_numedges(path);
    log_(" has length ",
         ne, (ne == 1 ? " edge" : " edges"), " over ",
         chs.size(), (chs.size() == 1 ? " chain: " : " chains: "));

    for (const auto& p: path)
        p.print();
    log_("");
}


}  // namespace graph_mutator::structure::paths::over_endslots

#endif  // GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_GENERIC_H
