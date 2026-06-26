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
 * \file vertex_in_graph.h
 * \brief Contains structure holding vertex parameters in graph-wide coordinates.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_IN_GRAPH_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

namespace graffine::structure::descriptors {

/// Descriptor of a vertex.
struct VertexInGraph
{
    uint ci {utils::undefined<uint>};   ///< Id of the host component.
    uint ind {utils::undefined<uint>};  ///< Index in the container.
    uint idh {utils::undefined<uint>};  ///< Vertex hash.
    uint d {utils::undefined<uint>};    ///< Vertex degree

    struct Link
    {
        // EgIdG of an incndent edge (either in or out).
        uint egidW {utils::undefined<uint>};

        /// Index in 'VerticesInGraph' of the neighbour vertex linked by 'egidW'
        uint vidW {utils::undefined<uint>};
    };

    Link links[numDegrees];


    constexpr VertexInGraph() = default;
    explicit constexpr VertexInGraph(const CmpId ci,
                                     const uint ind,
                                     const VIdH idh,
                                     const Degree d)
        : ci {static_cast<uint>(ci())}
        , ind {ind}
        , idh {static_cast<uint>(idh())}
        , d {static_cast<uint>(d)}
    {}

    constexpr void set(const CmpId cc,
                       const uint ii,
                       const VIdH hh,
                       const Degree dd) noexcept
    {
        ci = static_cast<uint>(cc());
        ind = ii;
        idh = static_cast<uint>(hh());
        d = static_cast<uint>(dd);
    }

    constexpr bool has_same_data_as(const VertexInGraph& other) const noexcept
    {
        if (ci != other.ci ||
            ind != other.ind ||
            idh != other.idh ||
            d != other.d) return false;

        return true;
    }

    constexpr bool operator==(const VertexInGraph& other) const noexcept
    {
        if (!has_same_data_as(other)) return false;

        for (uint k {}; k < d; ++k)
            if (links[k].egidW != other.links[k].egidW ||
                links[k].vidW  != other.links[k].vidW)

                return false;

        return true;
    }

    void print(const std::size_t j) const noexcept
    {
        jot<false>(j, " c ", ci, " ind ", ind, " h ", idh, " d ", d,
                   " [EgIdG,VIdG] ");
        for (uint k {}; k < d; ++k)
            jot<false>("{", links[k].egidW, " ", links[k].vidW, "} ");
        jot("");
    }
};

}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_IN_GRAPH_H