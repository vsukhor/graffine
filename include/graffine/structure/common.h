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
 * \brief Kinds of the structural elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_COMMON_H
#define GRAFFINE_STRUCTURE_COMMON_H

#include "graffine/definitions.h"

/// Graph elements, their containers and the structure-related
/// functionality: analyzers, paths, I/O and modifiers.
namespace graffine::structure {

/// Kind of a structural element; each element type names its own kind in
/// a static member \c Type.
enum class Elements {

    None = 0,   ///< No element.
    Vertex,     ///< Vertex.
    Edge,       ///< Edge.
    Chain,      ///< Chain: a linear run of edges with no branches.
    Triangle,   ///< Chain triangle: three chains forming a loop.
    Component,  ///< Connected component.
    Graph,      ///< The whole graph.
    Any         ///< Any kind of element.
};

}  // namespace graffine::structure

#endif  //  GRAFFINE_STRUCTURE_COMMON_H