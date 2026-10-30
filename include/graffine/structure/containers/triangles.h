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
 * \file triangles.h
 * \brief Implements container holding chain triangles.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H
#define GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H

#include "graffine/definitions.h"

#include <array>
#include <cstddef>
#include <vector>


namespace graffine::structure {


/**
 * A chain triangle: three chains joined end to end into a loop.
 * \details The chains meet pairwise at three distinct junction vertices.
 */
struct Triangle
{
    static constexpr std::size_t num {3};

    using ChIds = std::array<ChIdG, num>;
    using VIds = std::array<VIdH, num>;

    ChIds ww;  ///< Graph-wide ids of the chains, in ascending order.
    VIds vv;   ///< Ids of the junction vertices, in ascending order.

    bool operator==(const Triangle&) const = default;
};


/// Collection of chain triangles.
struct Triangles
{
    std::vector<Triangle> tris;  ///< The triangles.

    /// Number of triangles.
    constexpr auto num() const noexcept -> std::size_t
    {
        return tris.size();
    }

    /// Removes all triangles.
    constexpr void clear() noexcept
    {
        tris.clear();
    }
};

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H
