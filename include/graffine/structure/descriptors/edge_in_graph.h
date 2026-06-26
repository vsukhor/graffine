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
 * \file edge_in_graph.h
 * \brief Contains structure holding edge parameters in graph-wide coordinates.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"

namespace graffine::structure::descriptors {

/// Descriptor of an edge.
struct EdgeInGraph
{
    ChIdG w {ChIdG::undefined};  ///< Chain index.
    EgIdA a {EgIdA::undefined};  ///< Chain::idw.
    EgIdH h {EgIdH::undefined};  ///< Edge hash.

    constexpr EdgeInGraph() = default;
    explicit constexpr EdgeInGraph(const ChIdG w,
                                   const EgIdA a,
                                   const EgIdH h)
        : w {w}
        , a {a}
        , h {h}
    {}

    constexpr void set(const ChIdG ww,
                       const EgIdA aa,
                       const EgIdH hh) noexcept
    {
        w = ww;
        a = aa;
        h = hh;
    }

    constexpr void set(const ChIdG ww,
                       const EgIdA aa) noexcept
    {
        w = ww;
        a = aa;
    }

    constexpr auto bulk_slot() const noexcept -> _Slot<ChIdG, descriptors::Bulk>
    {
        return _Slot<ChIdG, descriptors::Bulk>{w, a};
    }

    void check(const ChIdG ww,
               const EgIdA aa,
               const EgIdH hh,
               const EgIdG ind,
               const EgIdG k) const
    {
        ENSURE(w == ww, "egl[", k, "].w != eg.w: eg.ind ", ind);
        ENSURE(a == aa, "egl[", k, "].a != eg.a: eg.ind ", ind);
        ENSURE(h == hh, "egl[", k, "].h != eg.id: eg.ind ", ind);
    }

    void print(const EgIdG j) const noexcept
    {
        jot(j, "     ", w, "  ", a, "  ", h);
    }
};

}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H