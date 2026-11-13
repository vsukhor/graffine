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
 * \file chains.h
 * \brief Container of the chains of a graph.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_CHAINS_H
#define GRAFFINE_STRUCTURE_CONTAINERS_CHAINS_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/descriptors/edge_in_graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <string>
#include <vector>

namespace graffine::structure::containers {

/** Container of the chains of a graph.
 * \details Extends Sequence with indexing by graph-wide chain id (ChIdG)
 * and with access to the edges held by the chains.
 * \tparam Ch Container element type: a chain of edges.
 */
template<typename Ch>
struct Chains
    : public Sequence<std::vector<Ch>>
{
    using ChId = ChIdG;
    using ChIds = ChIdGs;
    using Base = Sequence<std::vector<Ch>>;
    using size_type = Base::size_type;
    using value_type = Base::value_type;
    using EndSlot = Ch::EndSlot;
    using Edge = Ch::Edge;
    using EdgeDescr = descriptors::EdgeInGraph;

    /** Number of chains in the container.
     */
    constexpr auto num() const noexcept -> size_type;
    /** Total number of edges in all the chains.
     */
    constexpr auto num_edges() const noexcept -> std::size_t;
    /** Index of the last chain, or \ref utils::undefined if the container
     * is empty.
     */
    constexpr auto ind_last() const noexcept -> size_type;

    /** Changes the graph-wide index of the edge indexed \p from to \p to.
     * \details Searches the chains for the edge, so the cost is linear in
     * the number of edges. Does nothing if no edge is indexed \p from.
     */
    constexpr void reind_edge(EgIdG from, EgIdG to) noexcept;

    /// The edge described by \p egd.
    constexpr auto edge(const EdgeDescr& egd) const noexcept -> const Edge&;
    /// The edge described by \p egd.
    constexpr auto edge(const EdgeDescr& egd) noexcept -> Edge&;

    /// Copy of the neighbours of chain end \p s, i.e. the chain ends
    /// meeting it at its vertex.
    constexpr auto ngs_of(const EndSlot& s) const noexcept;
    /// The chain with id \p w.
    constexpr auto operator[](const ChId w) -> value_type& { return m_data[w()]; }
    /// The chain with id \p w.
    constexpr auto operator[](const ChId w) const -> const value_type& { return m_data[w()]; }

    /** Ids of all the chains: 0, 1, ..., num()-1.
     */
    constexpr auto chids() const noexcept -> const ChIds;

    /** Prints all the chains.
     * \param tag Label printed with each chain.
     */
    void print(std::string_view tag) const;

    /** Checks that \p cond is satisfied.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] message Arguments forming the message of the exception
     *                    thrown if the condition does not hold.
     * \throws invariant_error If \p cond is false.
     */
    template<typename... T>
    void ensure(
        bool cond,
        T&&... message
    ) const;

private:

    using Base::m_data;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Ch>
constexpr
auto Chains<Ch>::
num() const noexcept -> size_type
{
    return this->size();
}


template<typename Ch>
constexpr
auto Chains<Ch>::
ind_last() const noexcept -> size_type
{
    const auto n = num();

    return n ? num() - 1
             : utils::undefined<size_type>;
}

template<typename Ch>
constexpr
auto Chains<Ch>::
chids() const noexcept -> const ChIds
{
    ChIds ww(num());
    std::iota(ww.begin(), ww.end(), 0);

    return ww;
}

template<typename Ch>
constexpr
auto Chains<Ch>::
num_edges() const noexcept -> std::size_t
{
    std::size_t n {};

    for (auto& m : m_data)
        n += m.length();

    return n;
}

template<typename Ch>
constexpr
void Chains<Ch>::
reind_edge(const EgIdG from,
           const EgIdG to) noexcept
{
    for (auto& m : m_data)
        for (auto& eg : m.g)
            if (eg.ind == from) {
                eg.ind = to;
                return;
            }
}

template<typename Ch>
constexpr
auto Chains<Ch>::
edge(const EdgeDescr& egd) const noexcept -> const Edge&
{
    return m_data[egd.w()].g[egd.a()];
}

template<typename Ch>
constexpr
auto Chains<Ch>::
edge(const EdgeDescr& egd) noexcept -> Edge&
{
    return m_data[egd.w()].g[egd.a()];
}

template<typename Ch>
constexpr
auto Chains<Ch>::
ngs_of(const EndSlot& s) const noexcept
{
    return m_data[s.w].ngs[s.e];
}

template<typename Ch>
void Chains<Ch>::
print(const std::string_view tag) const
{
    for (const auto& m : m_data)
        m.print(tag);
}

template<typename Ch>
template<typename... T>
void Chains<Ch>::
ensure(
    bool cond,
    T&&... message
) const
{
    if (!cond) [[unlikely]]
        utils::raise<invariant_error>(nullptr, std::source_location::current(),
                                      message...);
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_CHAINS_H
