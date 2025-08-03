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

#ifndef GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H
#define GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>

#include "../../../definitions.h"


namespace graph_mutator::structure::paths::over_endslots {


template<typename Component>
struct Distance {

    using Edge = Component::Edge;
    using EndSlot = Component::EndSlot;  ///< Type alias for end slot.
    using EdgeWeight = typename Edge::weight_t;

    static constexpr EdgeWeight zero {};
    static constexpr auto inf = Edge::maxWeight;

    static constexpr auto undefined_element = EndSlot {};

    /// Element preceding the target in the shortest path.
    EndSlot prev {undefined_element};

    /// Shortest distance to the target.
    EdgeWeight dist {inf};

    constexpr bool operator<(const Distance& c) const noexcept
    {
        return prev < c.prev ||
              (prev == c.prev && dist < c.dist);
    }

    void set(const EndSlot& pr,
             const EdgeWeight d) noexcept
    {
        prev = pr;
        dist = d;
    }

    bool is_finite() const noexcept
    {
        return dist < inf;
    }

    void reset() noexcept
    {
        prev = undefined_element;
        dist = inf;
    }

    void print(const EndSlot& target) const
    {
        log_("{", target.str_short(), "}=>",
             "[{", prev.str_short(), "} ", dist, "]");
    }
};


}  // namespace graph_mutator::structure::paths::over_endslots

#endif  // GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H
