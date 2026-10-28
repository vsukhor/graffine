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
 * \brief Contains generic declarations for classes creating graph edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_CREATION_COMMON_H
#define GRAFFINE_TRANSFORMS_EDGE_CREATION_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/vertex_merger/common.h"

#include <type_traits>

#include <utility>


namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::EdgeCreation, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using ResT = Edge*;
    using Res = std::array<ResT, 1>;

    static constexpr const char* shortName {"ec_oc"};
    static constexpr const char* fullName {"edge_creation_old_chain"};

    static constexpr auto Type = TransformType::EdgeCreation;
};

/**
 * Enables edge creation operations on a graph.
 * \details These are functors for creating edges at specific spots of the graph.
 */
namespace edge_creation {

/**
 * Processor of non-topological properties usable by the edge creations.
 * \details Either void, for no processing, or a type providing the hooks
 * called by the edge creations. on_start receives the slot of the vertex
 * receiving the edge, or the position of the edge in an existing chain;
 * on_end<isNewChain> receives the new edge. When the edge is created in a
 * new chain, on_start is called after that chain is added to the graph, as
 * its last chain, but before it is connected.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              const typename G::EndSlot& es,
              const typename G::BulkSlot& bs,
              const ChIdG w,
              const EgIdA a,
              const bool atChainEnd,
              typename G::Edge& eg)
    {
        pp.edgeCreation.on_start(es);
        pp.edgeCreation.on_start(bs);
        pp.edgeCreation.template on_start<typename G::EndSlot>(es);
        pp.edgeCreation.template on_start<typename G::BulkSlot>(bs);
        pp.edgeCreation.on_start(w, a, atChainEnd);
        pp.edgeCreation.template on_end<true>(eg);
        pp.edgeCreation.template on_end<false>(eg);
    };


template<typename G>
using Trait = transforms::Trait<TransformType::EdgeCreation, G>;

}

} // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_EDGE_CREATION_COMMON_H