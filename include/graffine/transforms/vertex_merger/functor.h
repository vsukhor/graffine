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
 * \brief Contains implementation of a wrapper class for vertex merger
 * transformation.
 * \details Class encapsulating merger of vertices of all implemented degrees.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"

#include <memory>
#include <type_traits>

/**
 * Contains operations handling fusion of two vertices in a graph.
 */
namespace graffine::transforms::vertex_merger {

/**
 * Wrapper functor for vertex degree-independent vertex merger.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
    requires vertex_merger::Processor<PP, G>
struct Functor
    : public Applicable<Functor<G, PP>>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Vertex = Graph::Vertex;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit Functor(Graph& gr,
                     std::shared_ptr<PP> pp = nullptr);

    /**
     * Merges two vertices into one.
     * \details Unchecked: see Applicable.
     * \param[in] v1 The 1st parent vertex.
     * \param[in] v2 The 2nd parent vertex.
     */
    auto apply(Vertex& v1,
               Vertex& v2) -> Res;

    /// Checks that vertices \p v1 and \p v2 can be merged.
    /// \details Rejects pairs of degrees other than (1, 1), (1, 2), (1, 3)
    /// and (2, 2), in either order, then checks the pair as the
    /// degree-specific functor does.
    auto applicability(const Vertex& v1,
                       const Vertex& v2) const -> Applicability;

    From<Deg1, Deg1, G, PP> from11;
    From<Deg1, Deg2, G, PP> from12;
    From<Deg1, Deg3, G, PP> from13;
    From<Deg2, Deg2, G, PP> from22;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
    requires vertex_merger::Processor<PP, G>
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : from11 {gr, pp}
    , from12 {gr, pp}
    , from13 {gr, pp}
    , from22 {gr, pp}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
    requires vertex_merger::Processor<PP, G>
auto Functor<G, PP>::
applicability(
    const Vertex& v1,
    const Vertex& v2
) const -> Applicability
{
    const auto d1 = v1.deg();
    const auto d2 = v2.deg();

    if (d1 == Deg1 && d2 == Deg1)
        return from11.applicability(v1, v2);

    if ((d1 == Deg1 && d2 == Deg2) ||
        (d1 == Deg2 && d2 == Deg1))
        return from12.applicability(v1, v2);

    if ((d1 == Deg1 && d2 == Deg3) ||
        (d1 == Deg3 && d2 == Deg1))
        return from13.applicability(v1, v2);

    if (d1 == Deg2 && d2 == Deg2)
        return from22.applicability(v1, v2);

    return reject("merger is not implemented for these vertex degrees");
}


template<typename G,
         typename PP>
    requires vertex_merger::Processor<PP, G>
auto Functor<G, PP>::
apply(
    Vertex& v1,
    Vertex& v2
) -> Res
{
    const auto d1 = v1.deg();
    const auto d2 = v2.deg();

    if (d1 == Deg1 && d2 == Deg1)
        return from11.apply(v1, v2);

    if ((d1 == Deg1 && d2 == Deg2) ||
        (d1 == Deg2 && d2 == Deg1))
        return from12.apply(v1, v2);

    if ((d1 == Deg1 && d2 == Deg3) ||
        (d1 == Deg3 && d2 == Deg1))
        return from13.apply(v1, v2);

    /* (d1 == Deg2 && d2 == Deg2) */
    return from22.apply(v1, v2);
}

}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FUNCTOR_H