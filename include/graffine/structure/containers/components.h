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
 * \file components.h
 * \brief Contains class implementating collections of components in the graph.
 * \details Adds components-specific functionality to the Sequence container.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_COMPONENTS_H
#define GRAFFINE_STRUCTURE_CONTAINERS_COMPONENTS_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/descriptors/edge_in_graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"


#include <string>

namespace graffine::structure::containers {

/**
 * Implements class for managing collections of chains in the graph.
 * \details Derives from Sequence container class adding to it chain-specific
 * functionality.
 * \tparam Cmp Container element type: Disconnected component.
 */
template<typename Cmp>
struct Components
    : public Sequence<std::vector<Cmp>>
{
    using Base = Sequence<std::vector<Cmp>>;
    using size_type = Base::size_type;
    using value_type = Base::value_type;

    /// Actual number of chains in the container.
    constexpr auto num() const noexcept -> size_type;
//    constexpr auto num_edges() const noexcept -> std::size_t;
    constexpr auto ind_last() const noexcept -> size_type;

//    constexpr auto edge(const EdgeDescr& egd) const noexcept -> const Edge&;
//    constexpr auto edge(const EdgeDescr& egd) noexcept -> Edge&;

    constexpr auto operator[](const CmpId w) -> value_type& { return m_data[w()]; }
    constexpr auto operator[](const CmpId w) const -> const value_type& { return m_data[w()]; }

    void print(std::string_view tag) const;

    /**
     * Checks that \p cond is satisfied, else terminates program.
     * \details Prints out the component using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] tag Tag to the component data to be printed if \p cond is false.
     * \param[in] message Arguments bound to form the message printed at
     *                    termination if the condition does not hold.
     */
    template<typename... T>
    void ensure(
        bool cond,
        const std::string_view tag,
        T&&... message
    ) const;

private:

    using Base::m_data;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Cmp>
constexpr
auto Components<Cmp>::
num() const noexcept -> size_type
{
    return this->size();
}


template<typename Cmp>
constexpr
auto Components<Cmp>::
ind_last() const noexcept -> size_type
{
    const auto n = num();

    return n ? num() - 1
             : utils::undefined<size_type>;
}


template<typename Cmp>
void Components<Cmp>::
print(const std::string_view tag) const
{
    for (const auto& cmp : m_data)
        cmp.print(tag);
}


template<typename Cmp>
template<typename... T>
void Components<Cmp>::
ensure(
    bool cond,
    const std::string_view tag,
    T&&... message
) const
{
    if (!cond) {
        print(tag);
        ABORT(message...);
    }
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_COMPONENTS_H
