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
 * \brief Contains implementation of the edge parameters uniquely identifying it
 * as a part of component.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H
#define GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/slot.h"


namespace graffine::structure::descriptors {

/// Descriptor of an edge.
template<typename Edge>
struct EdgeInComponent
{
    ChIdG w {ChIdG::undefined};  ///< Index of the edge hosting chain.
    EgIdA a {EgIdA::undefined};  ///< Position inside the host chain.
    EgIdG i {EgIdG::undefined};  ///< Graph-wide index of the edge.
    EgIdH h {EgIdH::undefined};  ///< Edge hash value.

    static constexpr EdgeInComponent Undef {};

    constexpr EdgeInComponent() = default;

    explicit constexpr EdgeInComponent(const Edge& e)
        : w {e.w}
        , a {e.indw}
        , i {e.ind}
        , h {e.id}
    {}

    constexpr auto bulk_slot() const noexcept -> BulkSlot
    {
        return BulkSlot{w, a};
    }

    constexpr auto is_defined()
    {
        return *this != Undef;
    }

    void check(const Edge& eg,
               const CmpId ic,
               const auto k) const
    {
        ENSURE(w == eg.w,
           "egl[", k, "].w != eg.w in component ", ic, " eg.ind ", eg.ind);

        ENSURE(a == eg.indw,
           "egl[", k, "].a != eg.a in component ", ic, " eg.ind ", eg.ind);

        ENSURE(i == eg.ind,
           "egl[", k, "].i != eg.i in component ", ic, " eg.ind ", eg.ind);

        ENSURE(h == eg.id,
           "egl[", k, "].h != eg.id in component ", ic, " eg.ind ", eg.ind);
    }

    void print(const EgIdC j) const noexcept
    {
        jot(j, "     ", w, "  ", a, "  ", i, "  ", h);
    }
};  // EdgeInComponent



}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORDS_EDGE_IN_COMPONENT_H
