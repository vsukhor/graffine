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
 * \brief Contains generic declarations for classes deleting graph edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_COMMON_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_split/common.h"

#include <type_traits>

#include <array>
#include <utility>


namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::EdgeDeletion, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    static constexpr auto Type = TransformType::EdgeDeletion;
};

/**
 * Enables edge deletion operations on a graph.
 * \details These are functors for deleting edges based on chain lengths and
 * vertex degrees.
 */
namespace edge_deletion {

/**
 * Processor of non-topological properties usable by the edge deletions.
 * \details Either void, for no processing, or a type providing the hooks
 * called by the edge deletions: on_start<isChainDeleted> with the edge to
 * delete, and on_end<isChainDeleted> with a vertex at its former position.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              typename G::Edge& eg,
              typename G::Vertex& v)
    {
        pp.edgeDeletion.template on_start<true>(eg);
        pp.edgeDeletion.template on_start<false>(eg);
        pp.edgeDeletion.template on_end<true>(v);
        pp.edgeDeletion.template on_end<false>(v);
    };


template<typename G>
using Trait = transforms::Trait<TransformType::EdgeDeletion, G>;

}  // namespace edge_deletion
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_EDGE_DELETION_COMMON_H