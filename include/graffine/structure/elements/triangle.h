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
 * \file triangle.h
 * \brief The chain triangle: three chains forming a loop.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_TRIANGLE_H
#define GRAFFINE_STRUCTURE_ELEMENTS_TRIANGLE_H

#include "graffine/definitions.h"

#include <array>
#include <cstddef>


namespace graffine::structure {


/** A chain triangle: three chains joined end to end into a loop.
 * \details The chains meet pairwise at three distinct junction vertices.
 * Components detect their triangles on request (see
 * Component::detect_chain_triangles()).
 */
struct Triangle
{
    static constexpr std::size_t num {3};  ///< Number of chains and of vertices.

    using ChIds = std::array<ChIdG, num>;  ///< Type of the chain ids.
    using VIds = std::array<VIdH, num>;    ///< Type of the vertex ids.

    ChIds ww;  ///< Graph-wide ids of the chains, in ascending order.
    VIds vv;   ///< Stable handles of the junction vertices, in ascending order.

    bool operator==(const Triangle&) const = default;
};

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_TRIANGLE_H
