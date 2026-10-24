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
 * \brief Contains genetic templates for degree-specific vertex pulling.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_COMMON_H
#define GRAFFINE_TRANSFORMS_PULLING_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_creation/common.h"

#include <type_traits>

#include <algorithm>  // find
#include <array>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::Pulling, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    static constexpr auto Type = TransformType::Pulling;
};


/// Enables pulling of a graph edge holding it at specific end.
namespace pulling {

template<typename C>
struct Path;

/**
 * Processor of non-topological properties usable by the pullings.
 * \details Either void, for no processing, or a type providing the hooks
 * called with the path: on_start once before pulling along it, and on_end
 * after each of the steps of the pull.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              Path<typename G::Compt>& path)
    {
        pp.pulling.on_start(path);
        pp.pulling.on_end(path);
    };


template<typename G>
using Trait = transforms::Trait<TransformType::Pulling, G>;

inline constexpr std::array pullableDegrees {Deg1, Deg2, Deg3};

template <Degree D>
constexpr auto is_pullable_degree =
    std::find(pullableDegrees.begin(), pullableDegrees.end(), D) !=
    pullableDegrees.end();

template <Degree D>
concept pullable_degree =
    std::find(pullableDegrees.begin(), pullableDegrees.end(), D) !=
    pullableDegrees.end();

template<Orientation Dir>
constexpr char signchar = Dir == Orientation::Backwards ? '-' : '+';

inline constexpr bool omitUnconnectedCycles = true;

/**
 * Generic template for specifying vertex degree-specific pulling classes.
 * \tparam D Degree of the vertex from which the pull operation is initiated.
 * \tparam Dir Direction of the pull operation.
 * \tparam G Graph to which the transformation is applied.
 */
template<Degree D,
         Orientation Dir,
         typename G,
         typename PP = void>
    requires pullable_degree<D> &&
             pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct On {};


}  // namespace pulling
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_PULLING_COMMON_H
