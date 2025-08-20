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
 * @file chain_collection.h
 * @brief Contains implementations for managing collections of chains in the graph.
 * @details Adds chain-specific functionality to the VectorContainer class.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_CHAIN_COLLECTION_H
#define GRAPH_MUTATOR_STRUCTURE_CHAIN_COLLECTION_H


#include <string>

#include "../definitions.h"
#include "../vector_container.h"
#include "chain.h"
#include "ends.h"
#include "slot.h"


namespace graph_mutator::structure {

/**
 * @brief Implements class for managing collections of chains in the graph.
 * @details Derives from VectorContainer class adding to it chain-specific
 * functionality.
 * @tparam Ch Container element type: A chain of edges.
 */
template<typename Ch>
struct ChainContainer
    : public VectorContainer<Ch> {

    using Base = VectorContainer<Ch>;
    using size_type = Base::size_type;
    using EndSlot = Ch::EndSlot;

    constexpr auto num() const noexcept -> size_type;

    constexpr auto ind_last() const noexcept -> size_type;

    constexpr auto ngs_of(const EndSlot s) const noexcept;

    void print(const std::string& tag) const;

    /**
     * @brief Checks that \p cond is satisfied, else terminates program.
     * @details Prints out the collection using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * @tparam T Types of the message arguments.
     * @param cond Condition to be checked.
     * @param tag Tag to the collection data to be printed if \p cond is false.
     */
    template<typename... T>
    void ensure(
        bool cond,
        const std::string& tag,
        T&&... message
    ) const;

private:

    using Base::data;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Ch>
constexpr
auto ChainContainer<Ch>::
num() const noexcept -> size_type
{
    return this->size();
}


template<typename Ch>
constexpr
auto ChainContainer<Ch>::
ind_last() const noexcept -> size_type
{
    const auto n = num();

    return n ? num() - 1
             : undefined<size_type>;
}


template<typename Ch>
constexpr
auto ChainContainer<Ch>::
ngs_of(const EndSlot s) const noexcept
{
    return data[s.w].ngs[s.e];
}


template<typename Ch>
void ChainContainer<Ch>::
print(const std::string& tag) const
{
    for (const auto& m: data)
        m.print(tag);
}


template<typename Ch>
template<typename... T>
void ChainContainer<Ch>::
ensure(
    bool cond,
    const std::string& tag,
    T&&... message
) const
{
    if (!cond) {
        print(tag);
        ABORT(message...);
    }
}


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_CHAIN_COLLECTION_H
