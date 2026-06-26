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
 * \file facet.h
 * \brief Implements facet.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_FACET_H
#define GRAFFINE_STRUCTURE_ELEMENTS_FACET_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/common.h"
#include "graffine/structure/elements/vertex.h"

#include "graffine/utils/arrays/all.h"

namespace graffine::structure {


template<typename V,
         typename E>
struct Facet
{};

/**
 * Represents the ends of an edge or those of a chain.
 */
template<>
struct Facet<core::Edge::Vertex,
             core::Edge<maxDegree>>
{
    using Vertex = core::Edge::Vertex;
    using Vertices = std::array<VIdH, 3>;
    using Edge = core::Edge<maxDegree>;
    using Edges = std::array<EgId, 3>;

    static constexpr auto Type = Types::Facet;

    FId ind;
    FId indc;

    const Vertices v;   ///< Vertices.
    const Edges    eg;  ///< Edges.

//    const arrays::A3<real> n;   ///< Normal.

    std::vector<FId> neigs;

    explicit Facet(
        const FId ind,
        const FId indc,
        const Edges& eg,
        const Vertices& v
//        std::vector<Point>& centroids,     // ref
//        const Point& n={}
    )
        : ind {ind}
        , indc {indc}
        , v {v}
        , eg {eg}
//        , n {n}
    {
//        centroids.push_back((*v[0] + *v[1] + *v[2])/three);
    }

    constexpr bool operator==(const Facet& other) const noexcept
    {
    //    ASSERT(eg == other.eg);
        return eg == other.eg;
    }

protected:


    static constexpr auto edges(
        const Vertices& vv
    ) noexcept -> Edges
    {
        const auto e01 = vv[0].common_edge(vv[1]);
        ASSERT(e01.is_defined(), "No common edge vv0 <-> vv1.");

        const auto e12 = vv[1].common_edge(vv[2]);
        ASSERT(e12.is_defined(), "No common edge vv1 <-> vv2.");

        const auto e20 = vv[2].common_edge(vv[0]);
        ASSERT(e20.is_defined(), "No common edge vv2 <-> vv0.");

        return {&e01, &e12, &e20};
    }

};  // struct Facet

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_FACET_H
