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
 * \brief Container of the components of a graph.
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

/** Container of the components of a graph.
 * \details Extends Sequence with indexing by component id (CmpId).
 * \tparam Cmp Container element type: a connected component.
 */
template<typename Cmp>
struct Components
    : public Sequence<std::vector<Cmp>>
{
    using Base = Sequence<std::vector<Cmp>>;
    using size_type = Base::size_type;
    using value_type = Base::value_type;

    /** Number of components in the container.
     */
    constexpr auto num() const noexcept -> size_type;
    /** Index of the last component, or \ref utils::undefined if the
     * container is empty.
     */
    constexpr auto ind_last() const noexcept -> size_type;

    /// The component with id \p w.
    constexpr auto operator[](const CmpId w) -> value_type& { return m_data[w()]; }
    /// The component with id \p w.
    constexpr auto operator[](const CmpId w) const -> const value_type& { return m_data[w()]; }

    /** Prints all the components.
     * \param tag Label printed with each component.
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
    T&&... message
) const
{
    if (!cond) [[unlikely]]
        utils::raise<invariant_error>(nullptr, std::source_location::current(),
                                      message...);
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_COMPONENTS_H
