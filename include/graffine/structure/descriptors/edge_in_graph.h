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
 * \brief Descriptor of an edge, as recorded by the graph.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"

namespace graffine::structure::descriptors {

/** Descriptor of an edge, as recorded by the graph.
 * \details The graph keeps one descriptor per edge, at the edge's
 * graph-wide index EgIdG, for locating the edge in its chain without going
 * through the components.
 */
struct EdgeInGraph
{
    ChIdG w {ChIdG::undefined};  ///< Index of the edge's host chain.
    EgIdA a {EgIdA::undefined};  ///< Position inside the host chain.
    EgIdH h {EgIdH::undefined};  ///< Stable handle of the edge.

    constexpr EdgeInGraph() = default;

    /** Constructor.
     * \param w Host chain.
     * \param a Position inside the host chain.
     * \param h Stable handle of the edge.
     */
    explicit constexpr EdgeInGraph(const ChIdG w,
                                   const EgIdA a,
                                   const EgIdH h)
        : w {w}
        , a {a}
        , h {h}
    {}

    /// Sets all the fields.
    constexpr void set(const ChIdG ww,
                       const EgIdA aa,
                       const EgIdH hh) noexcept
    {
        w = ww;
        a = aa;
        h = hh;
    }

    /// Sets the location of the edge, keeping its handle.
    constexpr void set(const ChIdG ww,
                       const EgIdA aa) noexcept
    {
        w = ww;
        a = aa;
    }

    /// Slot of the vertex preceding the edge along its host chain, i.e.
    /// between edges a-1 and a. A valid BulkSlot only if a > 0.
    constexpr auto bulk_slot() const noexcept -> _Slot<ChIdG, descriptors::Bulk>
    {
        return _Slot<ChIdG, descriptors::Bulk>{w, a};
    }

    /** Checks that the descriptor holds the given edge data.
     * \param ww Host chain of the edge.
     * \param aa Position of the edge in the chain.
     * \param hh Stable handle of the edge.
     * \param ind Graph-wide index of the edge, for the error message.
     * \param k Index of the descriptor, for the error message.
     * \throws invariant_error If a field disagrees.
     */
    void check(const ChIdG ww,
               const EgIdA aa,
               const EgIdH hh,
               const EgIdG ind,
               const EgIdG k) const
    {
        GRAFFINE_ENSURE(w == ww, "egl[", k, "].w != eg.w: eg.ind ", ind);
        GRAFFINE_ENSURE(a == aa, "egl[", k, "].a != eg.a: eg.ind ", ind);
        GRAFFINE_ENSURE(h == hh, "egl[", k, "].h != eg.id: eg.ind ", ind);
    }

    /// Prints the descriptor, preceded by its index \p j.
    void print(const EgIdG j) const noexcept
    {
        jot(j, "     ", w, "  ", a, "  ", h);
    }
};

}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_GRAPH_H