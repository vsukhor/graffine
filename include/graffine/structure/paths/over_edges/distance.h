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
 * \file distance.h
 * \brief Distance record of the shortest paths over the edges of a component.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_DISTANCE_H
#define GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_DISTANCE_H

#include "graffine/definitions.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>


namespace graffine::structure::paths::over_edges {


/** Distance record of an edge in a shortest path search over edges.
 * \details Holds the shortest distance found to the target edge and the
 * edge preceding it on that path. The same type serves as the entry of the
 * search queue, where \a prev holds the queued edge itself.
 * \tparam Component Type of the graph component.
 */
template<typename Component>
struct Distance
{
    using Edge = Component::Edge;
    using EdgeWeight = typename Edge::weight_t;  ///< Type of the distances.

    static constexpr EdgeWeight zero {};          ///< Zero distance.
    static constexpr auto inf = Edge::maxWeight;  ///< Distance of an unreached edge.

    /** Default constructor.
     * \details Initializes the distance with an undefined previous edge id
     * and infinite distance.
     */
    constexpr Distance() noexcept = default;

    /** Copy constructor.
     */
    constexpr Distance(const Distance& all) noexcept = default;

    /** Move constructor.
     */
    constexpr Distance(Distance&& all) noexcept = default;

    /** Copy assignment operator.
     * \return Reference to this object.
     */
    constexpr auto operator=(const Distance& all) -> Distance& = default;

    /** Move assignment operator.
     * \return Reference to this object.
     */
    constexpr auto operator=(Distance&& all) -> Distance& = default;

    /** Constructs a distance object with a previous edge id and a distance.
     * \param[in] pr Previous edge id in the path.
     * \param[in] d Distance to the edge.
     */
    constexpr explicit Distance(
        const EgIdC pr,
        const EdgeWeight d
    ) noexcept
        : prev {pr}
        , dist {d}
    {}

    /** Orders the records by distance, then by edge \a prev, so that the
     * search queue yields the nearest edge first.
     * \param[in] c The other distance object to compare with.
     */
    constexpr bool operator<(const Distance& c) const noexcept
    {
        return dist < c.dist ||
               (dist == c.dist && prev < c.prev);
    }

    /** Sets the previous edge id and distance.
     * \param[in] pr Previous edge id in the path.
     * \param[in] d Distance to the edge measured in edge weights.
     */
    void set(const EgIdC pr,
             const EdgeWeight d) noexcept
    {
        prev = pr;
        dist = d;
    }

    /** Gets the previous edge id.
     * \return Previous edge id in the path.
     */
    constexpr auto get_prev() const noexcept -> EgIdC
    {
        return prev;
    }

    /** Gets the distance to the edge.
     * \return Distance to the edge measured in edge weights.
     */
    [[nodiscard]]  // NOLINT(readability-convert-member-functions-to-static)
    constexpr auto get_dist() const noexcept -> EdgeWeight
    {
        return dist;
    }

    /** Sets the distance to the edge.
     * \param[in] d Distance to the edge measured in edge weights.
     */
    constexpr void set_dist(const EdgeWeight d) noexcept
    {
        dist = d;
    }

    /** Checks if the edge was reached by the search.
     */
    constexpr bool is_finite() const noexcept
    {
        return dist < inf;
    }

    /** Resets the distance to the edge.
     * \details Sets the previous edge id to undefined
     * and sets the distance to infinity.
     */
    constexpr void reset() noexcept
    {
        prev = EgIdC::undefined;
        dist = inf;
    }

    /** Prints the record of edge \p target, without a line break.
     */
    void print(const EgIdC target) const
    {
        jot<false>(target, "=>[", prev, " ", dist, "] ");
    }

private:

    /// Element preceding the target edge in the shortest path.
    EgIdC prev {EgIdC::undefined};

    /// Shortest distance to the target edge.
    EdgeWeight dist {inf};
};


}  // namespace graffine::structure::paths::over_edges

#endif  // GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_DISTANCE_H
