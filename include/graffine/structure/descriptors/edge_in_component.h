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
 * \file edge_in_component.h
 * \brief Descriptor of an edge, as recorded by its host component.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H
#define GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"


namespace graffine::structure::descriptors {

/** Descriptor of an edge, as recorded by its host component.
 * \details A component keeps one descriptor per edge, at the edge's
 * component-wide index EgIdC, for locating the edge without going through
 * the chains. The descriptor copies the ids of the edge; check() verifies
 * that the copies are still up to date.
 * \tparam Edge Edge type.
 */
template<typename Edge>
struct EdgeInComponent
{
    ChIdG w {ChIdG::undefined};  ///< Index of the edge's host chain.
    EgIdA a {EgIdA::undefined};  ///< Position inside the host chain.
    EgIdG i {EgIdG::undefined};  ///< Graph-wide index of the edge.
    EgIdH h {EgIdH::undefined};  ///< Stable handle of the edge.

    static constexpr EdgeInComponent Undef {};  ///< Undefined descriptor.

    constexpr EdgeInComponent() = default;

    /// Constructs the descriptor of edge \p e.
    explicit constexpr EdgeInComponent(const Edge& e)
        : w {e.w}
        , a {e.indw}
        , i {e.ind}
        , h {e.id}
    {}

    /// Slot of the vertex preceding the edge along its host chain, i.e.
    /// between edges a-1 and a. A valid BulkSlot only if a > 0.
    constexpr auto bulk_slot() const noexcept -> BulkSlot
    {
        return BulkSlot{w, a};
    }

    /// Checks that the descriptor differs from \ref Undef.
    constexpr auto is_defined()
    {
        return *this != Undef;
    }

    /** Checks that this descriptor is the one of edge \p eg.
     */
    constexpr bool matches(const Edge& eg) const noexcept
    {
        return w == eg.w &&
               a == eg.indw &&
               i == eg.ind &&
               h == eg.id;
    }

    /** Checks that this descriptor is the one of edge \p eg.
     * \param eg Edge.
     * \param ic Host component, for the error message.
     * \param k Index of the descriptor, for the error message.
     * \throws invariant_error If a field disagrees with the edge.
     */
    void check(const Edge& eg,
               const CmpId ic,
               const auto k) const
    {
        GRAFFINE_ENSURE(w == eg.w,
           "egl[", k, "].w != eg.w in component ", ic, " eg.ind ", eg.ind);

        GRAFFINE_ENSURE(a == eg.indw,
           "egl[", k, "].a != eg.a in component ", ic, " eg.ind ", eg.ind);

        GRAFFINE_ENSURE(i == eg.ind,
           "egl[", k, "].i != eg.i in component ", ic, " eg.ind ", eg.ind);

        GRAFFINE_ENSURE(h == eg.id,
           "egl[", k, "].h != eg.id in component ", ic, " eg.ind ", eg.ind);
    }

    /// Prints the descriptor, preceded by its index \p j.
    void print(const EgIdC j) const
    {
        jot(j, "     ", w, "  ", a, "  ", i, "  ", h);
    }
};  // EdgeInComponent



}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H
