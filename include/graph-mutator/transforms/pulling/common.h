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
 * @file common.h
 * @brief Contains genetic templates for degree-specific vertex pulling.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_COMMON_H
#define GRAPH_MUTATOR_PULLING_COMMON_H

#include <array>

#include "../../definitions.h"
#include "../../structure/basic/graph.h"


namespace graph_mutator::pulling {

constexpr std::array<Degree, 3> pullableDegrees {1, 2, 3};

template <Degree D>
constexpr auto is_pullable_degree =
    std::find(pullableDegrees.begin(), pullableDegrees.end(), D) !=
    pullableDegrees.end();


template<Orientation Dir>
constexpr char signchar = Dir == Orientation::Backwards ? '-' : '+';


/**
 * @brief Generic template for specifying vertex degree-specific pulling classes.
 * @tparam D Degree of the vertex from which the pull operation is initiated.
 * @tparam Dir Direction of the pull operation.
 * @tparam G Graph to which the transformation is applied.
 */
template<Degree D,
         Orientation Dir,
         typename G> requires (is_pullable_degree<D>)
struct On {};


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_COMMON_H
