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
 * \file functor.h
 * \brief Contains classes performing deletion of specific graph edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_DELETION_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_EDGE_DELETION_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_deletion/in_multi_edge_chain.h"
#include "graffine/transforms/edge_deletion/in_single_edge_chain.h"

#include <memory>

/**
 * Contains operations handling edge deletion in a graph.
 * These are both functors for deleting edges based on chain lengths and vertex
 * degrees and their generic wrappers.
 */
namespace graffine::transforms::edge_deletion {


/**
 * Wrapper functor for edge deletion able to handle all possible configurations.
 * \tparam Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
struct Functor
{
    using Trait = edge_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    /**
     * Constructs the Functor object based on a Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    constexpr explicit Functor(Graph& gr,
                               std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the deletion using edge index.
     * \param[in] ind Graph-wide index of the deleted edge.
     */
    auto operator()(EgIdG ind) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    InSingleEdgeChain<G, PP> withChain;
    InMultiEdgeChain<G, PP>  withoutChain;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
constexpr
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : gr {gr}
    , withChain {gr, pp}
    , withoutChain {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto Functor<G, PP>::
operator()(const EgIdG ind) noexcept -> Res
{
    const auto& m = gr.chain(ind);

    ASSERT_CALLING(m.is_shrinkable(),
                   m.print("NOT SHRINCABLE"),
                   "cannot delete edge ", ind, ": chain is not shrinkable");

    return m.length() == 1 &&
           m.has_single_leaf_vertex()

        ? withChain(ind)
        : withoutChain(ind);
}


}  // namespace graffine::transforms::edge_deletion

#endif  // GRAFFINE_TRANSFORMS_EDGE_DELETION_FUNCTOR_H
