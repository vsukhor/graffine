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
 * \brief Contains wrapper class for degree-independent vertex split
 * transformation.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/transforms/vertex_split/to_11.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"
#include "graffine/transforms/vertex_split/to_22.h"

#include <memory>
#include <vector>
#include <type_traits>


namespace graffine::transforms::vertex_split {

/**
 * Wrapper functor for vertex degree-independent vertex split operations.
 * \tparam Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
struct Functor
{
    using Graph = G;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    using Res = std::array<ResT, 2>;

    /**
     * Constructs a functor for vertex split operation acting on a Graph.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit constexpr Functor(Graph& gr,
                               std::shared_ptr<PP> pp = nullptr);

    /**
     * Divides the graph chain at given vertex \p v.
     * This is a wrapper operator able to process all implemented vertex degrees.
     * \param[in] v Internal chain vertex.
     * \param[in] ss Slots to get removed from the vertex.
     */
    auto operator()(Vertex& v,
                    const std::vector<typename Vertex::EgDescr>& ss) -> Res;

protected:

    Graph& gr;

    To<Deg1, Deg1, G, PP> to_1_1;
    To<Deg1, Deg2, G, PP> to_1_2;
    To<Deg1, Deg3, G, PP> to_1_3;
    To<Deg2, Deg2, G, PP> to_2_2;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
constexpr
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : gr {gr}
    , to_1_1 {gr, pp}
    , to_1_2 {gr, pp}
    , to_1_3 {gr, pp}
    , to_2_2 {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto Functor<G, PP>::
operator()(Vertex& v,
           const std::vector<typename Vertex::EgDescr>& ss) -> Res
{
    const auto d = v.deg();

    if (d == Deg2)      return to_1_1(v);
    if (d == Deg3)      return to_1_2(v, ss[0]);
    /* if (d == Deg4)*/ return ss.size() == 1 ? to_1_3(v, ss[0])
                                              : to_2_2(v, ss);
}


}  // namespace graffine::transforms::vertex_split

#endif // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_FUNCTOR_H