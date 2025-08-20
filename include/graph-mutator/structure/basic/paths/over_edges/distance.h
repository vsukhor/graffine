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
 * @file distance.h
 * @brief Contains class template defining distances over the graph components.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_BASIC_PATHS_OVER_EDGES_DISTANCE_H
#define GRAPH_MUTATOR_STRUCTURE_BASIC_PATHS_OVER_EDGES_DISTANCE_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>

#include "../../../../definitions.h"


namespace graph_mutator::structure::basic::paths::over_edges {


template<typename Component>
struct Distance {

    using Edge = Component::Edge;
    using EdgeWeight = typename Edge::weight_t;

    static constexpr EdgeWeight zero {};
    static constexpr auto inf = Edge::maxWeight;

    /**
     * @brief Default constructor.
     * @details Initializes the distance with an undefined previous edge id
     * and infinite distance.
    */
    constexpr Distance() noexcept = default;

    /** Copy constructor. */
    constexpr Distance(const Distance& all) noexcept = default;

    /** Move constructor. */
    constexpr Distance(Distance&& all) noexcept = default;

    /**
     * @brief Copy assignment operator.
     * @return Reference to this object.
     */
    constexpr auto operator=(const Distance& all) -> Distance& = default;

    /**
     * @brief Move assignment operator.
     * @return Reference to this object.
     */
    constexpr auto operator=(Distance&& all) -> Distance& = default;

    /**
     * @brief Constructs a distance object with a previous edge id and a distance.
     * @param pr Previous edge id in the path.
     * @param d Distance to the edge.
     */
    constexpr explicit Distance(
        const EgId pr,
        const EdgeWeight d
    ) noexcept
        : prev {pr}
        , dist {d}
    {}

    /**
     * @brief Compares two distance objects.
     * @param c The other distance object to compare with.
     * @return True if this distance is less than the other, false otherwise.
     */
    constexpr bool operator<(const Distance& c) const noexcept
    {
        return prev < c.prev;
    }

    /**
     * @brief Sets the previous edge id and distance.
     * @param pr Previous edge id in the path.
     * @param d Distance to the edge measured in edge weights.
     */
    void set(const EgId pr,
             const EdgeWeight d) noexcept
    {
        prev = pr;
        dist = d;
    }

    /**
     * @brief Gets the previous edge id.
     * @return Previous edge id in the path.
     */
    constexpr auto get_prev() const noexcept -> EgId
    {
        return prev;
    }

    /**
     * @brief Gets the distance to the edge.
     * @return Distance to the edge measured in edge weights.
     */
    [[nodiscard]]  // NOLINT(readability-convert-member-functions-to-static)
    constexpr auto get_dist() const noexcept -> EdgeWeight
    {
        return dist;
    }

    /**
     * @brief Sets the distance to the edge.
     * @param d Distance to the edge measured in edge weights.
     */
    constexpr void set_dist(const EdgeWeight d) noexcept
    {
        dist = d;
    }

    constexpr bool is_finite() const noexcept
    {
        return dist < inf;
    }

    /**
     * @brief Resets the distance to the edge.
     * @details Sets the previous edge id to undefined
     * and the distance to infinity.
     */
    constexpr void reset() noexcept
    {
        prev = undefined<EgId>;
        dist = inf;
    }

    void print(const EgId target) const
    {
        log_<false>(target, "=>[", prev, " ", dist, "] ");
    }

private:

    /// Element preceding the target edge in the shortest path.
    EgId prev {undefined<EgId>};

    /// Shortest distance to the target edge.
    EdgeWeight dist {inf};
};


}  // namespace graph_mutator::structure::basic::paths::over_edges

#endif  // GRAPH_MUTATOR_STRUCTURE_BASIC_PATHS_OVER_EDGES_DISTANCE_H
