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
 * \brief Contains wrapper class for degree-independent vertex pulling
 * transformation.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/transforms/pulling/on_1.h"
#include "graffine/transforms/pulling/on_2.h"
#include "graffine/transforms/pulling/on_3.h"
#include "graffine/transforms/pulling/path.h"

#include <memory>
#include <type_traits>

namespace graffine::transforms::pulling {

/**
 * Wrapper functor for vertex degree-independent vertex split operations.
 * \tparam Graph hosting the edge to be deleted.
 */
template<typename G,
         typename PP = void>
struct Functor
{
    using Trait = pulling::Trait<G>;
    using Graph = Trait::Graph;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using S = Vertex::EgDescr;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit constexpr Functor(Graph& gr,
                               std::shared_ptr<PP> pp = nullptr);

    /**
     * Pulls over edge sequence \p path.
     * This is a wrapper operator able to process all implemented degrees of
     * the driver vertices.
     * \param[in] path Path containing the string of edges to get pulled.
     * \param[in] n Number of steps to pull over.
     */
    auto operator()(Path<Compt>& path,
                    const int n) -> Res;

protected:

    On<Deg1, Orientation::Forwards, G, PP> on_1f;
    On<Deg2, Orientation::Forwards, G, PP> on_2f;
    On<Deg3, Orientation::Forwards, G, PP> on_3f;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
constexpr
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : on_1f {gr, pp}
    , on_2f {gr, pp}
    , on_3f {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto Functor<G, PP>::
operator()(Path<Compt>& path,
           const int n) -> Res
{
    const auto& v = path.driver_vertex();

    if (v.is_leaf())      return on_1f(path, n);
    if (v.is_bulk())      return on_2f(path, n);
    if (v.is_junction3()) return on_3f(path, n);
}


}  // namespace graffine::transforms::pulling

#endif // GRAFFINE_TRANSFORMS_PULLING_FUNCTOR_H