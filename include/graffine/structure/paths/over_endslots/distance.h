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
 * \brief Distance record of the shortest paths over the chain ends of a
 * component.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H
#define GRAFFINE_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H

#include "graffine/definitions.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>


namespace graffine::structure::paths::over_endslots {


/** Distance record of a chain end in a shortest path search over chain ends.
 * \details Holds the shortest distance found to the target chain end and
 * the chain end preceding it on that path. The same type serves as the
 * entry of the search queue, where \a prev holds the queued slot itself.
 * \tparam Component Type of the graph component.
 */
template<typename Component>
struct Distance
{
    using Edge = Component::Edge;
    using EndSlot = Component::EndSlot;  ///< Type alias for end slot.
    using EdgeWeight = typename Edge::weight_t;  ///< Type of the distances.

    static constexpr EdgeWeight zero {};          ///< Zero distance.
    static constexpr auto inf = Edge::maxWeight;  ///< Distance of an unreached slot.
    static constexpr auto undefined_element = EndSlot {};  ///< No slot.

    /// Element preceding the target in the shortest path.
    EndSlot prev {undefined_element};

    /// Shortest distance to the target.
    EdgeWeight dist {inf};

    /// Orders the records by slot \a prev first, then by distance.
    constexpr bool operator<(const Distance& c) const noexcept
    {
        return prev < c.prev ||
              (prev == c.prev && dist < c.dist);
    }

    /// Sets the preceding slot and the distance.
    void set(const EndSlot& pr,
             const EdgeWeight d) noexcept
    {
        prev = pr;
        dist = d;
    }

    /// Checks if the slot was reached by the search.
    bool is_finite() const noexcept
    {
        return dist < inf;
    }

    /// Resets the record to an unreached slot.
    void reset() noexcept
    {
        prev = undefined_element;
        dist = inf;
    }

    /// Prints the record of slot \p target.
    void print(const EndSlot& target) const
    {
        jot("{", target.str_short(), "}=>",
            "[{", prev.str_short(), "} ", dist, "]");
    }
};


}  // namespace graffine::structure::paths::over_endslots

#endif  // GRAFFINE_STRUCTURE_PATHS_OVER_ENDSLOTS_DISTANCE_H
