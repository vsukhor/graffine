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
 * \brief Contains generic template for transformation traits.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_COMMON_H
#define GRAFFINE_TRANSFORMS_COMMON_H

#include "graffine/definitions.h"

/// Functionality enabling transformations of the graph topology
namespace graffine::transforms {

enum TransformType
{
    ComponentCreation,
    ComponentDeletion,
    EdgeCreation,
    EdgeDeletion,
    EdgeMerger,
    EdgeSplit,
    VertexMerger,
    VertexSplit,
    Pulling
};

template<TransformType TT,
         typename Element>
struct Trait
{};

inline
auto degrees_to_string(
    const Degree d1,
    const Degree d2
) -> std::string
{
    return std::to_string(d1) + '+' + std::to_string(d2);
}

constexpr
auto make_name(
    const char* nameTrait,
    const std::string& dd = ""
) noexcept -> std::string
{
    return std::string(nameTrait) + dd;
}


}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_COMMON_H