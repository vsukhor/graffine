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
 * \file edge_in_vertex.h
 * \brief Contains implementation of the edge parameters saved as a part of
 * vertex structure when the edge is connected to the vertex.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_VERTEX_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_VERTEX_H

#include "graffine/definitions.h"
#include "graffine/structure/common.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include <string>

namespace graffine::structure::descriptors {


/**
 * Descriptor of an edge known to vertex connected to it.
 */
template<IdScope Sc>
struct EdgeInVertex
    : public _Slot<Id<ids::ElementType::EDGE, Sc>, End>
{
    static constexpr auto Scope = Sc;
    using Id_t = Id<ids::ElementType::EDGE, Scope>;
    using End = descriptors::End;
    using Base = _Slot<Id_t, End>;
    using HostId = Base::HostId;
    using LocId = Base::LocId;

    static constexpr auto hostElements = std::array {Elements::Vertex};

//    using Base::w;  <-- misleading notation, so hide it behind indc()
    using Base::e;
    const EgIdH idh;

    explicit constexpr EdgeInVertex(
        const EgIdH idh,
        const Id_t id,
        const End::Id e
    )
        : Base {id, e}
        , idh {idh}
    {}

    constexpr ~EdgeInVertex() = default;

    constexpr bool operator==(const EdgeInVertex& p) const noexcept = default;

    constexpr bool operator<(const EdgeInVertex& p) const noexcept
    {
        return idh < p.idh ||
               (idh == p.idh && e < p.e);
    }

    constexpr Id_t indc() const noexcept
    {
        static_assert(Scope == IdScope::COMPONENT);
        return this->w;
    }

    constexpr bool is_in() const noexcept { return e == End::B; }
    constexpr bool is_out() const noexcept{ return !is_in(); }

    auto str_short() const -> std::string
    {
        return idh.as_string() + " " +
               indc().as_string()  + " " +
               End::str(e);
    }

    void print(const char* color=colorcodes::YELLOW) const
    {
        jot<false>('{', color, str_short(), colorcodes::RESET, '}');
    }

    void print_edgeSlot(const char* color=colorcodes::YELLOW) const
    {
        jot<false>('{', color, indc().as_string(), " ",
                               End::str(e), colorcodes::RESET, '}');
    }
};

}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_EDGE_IN_VERTEX_H
