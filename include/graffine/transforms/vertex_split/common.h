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
 * \file common.h
 * \brief Contains classes controling vertex split transformation.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H

#include "graffine/definitions.h"
#include "graffine/transforms/common.h"

#include <array>


namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::VertexSplit, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    using Res = std::array<ResT, 2>;

    static constexpr auto Type = TransformType::VertexSplit;

    static constexpr const char* fullName  {"vertex_split_"};
    static constexpr const char* shortName {"v_s_"};
};



/// Enables split of a graph vertex into two daughter ones.
namespace vertex_split {

template<typename G>
using Trait = transforms::Trait<TransformType::VertexSplit, G>;

/**
 * Template for functors performing vertex split.
 * \details This class template is specialized for specific vertex degrees
 * and provides the functionality to split a vertex into two vertices of
 * specified degrees.
 * \tparam D1 Degree of the 1st resulting vertex.
 * \tparam D2 Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 */
template<Degree D1,
         Degree D2,
         typename G,
         typename PP = void>
struct To
{};

/**
 * Template for functors performing vertex split which produce a degree 1 vertex.
 * \details This class template is specialized for specific vertex degrees
 * and provides the functionality to split a vertex into two vertices of
 * specified degrees.
 * \tparam D Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 */
template<Degree D,
         typename G,
         typename PP = void>
struct To1
{};

/**
 * Template for functors performing vertex split which produce a degree 2 vertex.
 * \details This class template is specialized for specific vertex degrees
 * and provides the functionality to split a vertex into two vertices of
 * specified degrees.
 * \tparam D Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 */
template<Degree D,
         typename G,
         typename PP = void>
struct To2
{};


}  // namespace vertex_split
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H
