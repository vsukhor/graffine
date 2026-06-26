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
 * \brief Contains class templates for degree-specific vertex merger.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/transforms/common.h"

#include <array>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::VertexMerger, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    using Res = std::array<ResT, 1>;

    static constexpr auto Type = TransformType::VertexMerger;

    static constexpr const char* shortName {"v_m_"};
    static constexpr const char* fullName  {"vertex_merger"};
};

namespace vertex_merger {

template<typename G>
using Trait = transforms::Trait<TransformType::VertexMerger, G>;

/**
 * Generic template for vertex degree-specific merger executing classes.
 * \tparam D1 Degree of the 1st input vertex.
 * \tparam D2 Degree of the 2nd input vertex.
 * \tparam G Graph to which the transformation is applied.
 */
template<Degree D1,
         Degree D2,
         typename G,
         typename PP = void> requires areCompatibleDegrees<D1, D2>
struct From {};

/**
 *  Template for vertex degree-specific merger with a vertex havingDegree 1.
 * \tparam D Degree of the 2nd input vertex.
 * \tparam G Graph to which the transformation is applied.
 */
template<Degree D,
         typename G,
         typename PP = void>
struct From1 {};

template<Degree D,
         typename G,
         typename PP = void>
struct FromN {};

inline constexpr auto UnconnectedCycleEnd = structure::descriptors::End::A;


}  // namespace vertex_merger
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H
