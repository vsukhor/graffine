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
 * \file junction_shifter.h
 * \brief Contains implementation of the mechainsm enabling smooth shifting of
 * internal chain vertices over a junction node connecting the chains.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_MODIFIERS_JUNCTION_SHIFTER_H
#define GRAFFINE_STRUCTURE_MODIFIERS_JUNCTION_SHIFTER_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/neigs.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <vector>


namespace graffine::structure::modifiers {

/**
 * Contains implementation of the mechainsm enabling smooth shifting of
 * internal chain vertices over a junction node connecting the chains.
 */
template<typename H>
struct JunctionShifter
{
    using Host = H;
    using Edge = Host::Edge;
    using EndSlot = Host::EndSlot;
    using Vertex = Host::Vertex;
    using Vertices = Host::Vertices;
    using End = descriptors::End;
    using ED = typename Vertex::EgDescr;

//    explicit constexpr JunctionShifter(Host& host)
//        : host {host}
//    {}

    ///  Shift an edge from f.e end of f.w to t.e end of t.w
    void operator()(Host& host,
                    const EndSlot& f,
                    const EndSlot& t);

    constexpr auto edge_descriptor(
        const Edge& eg,
        End::Id eOuter
    ) const noexcept -> ED;

protected:

    void shift_egS(Vertex* v,
                   Vertex* t,
                   const std::array<ED, 2>& toOmit,
                   Host& host);
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename H>
void JunctionShifter<H>::
operator()(Host& host,
           const EndSlot& f,
           const EndSlot& t)
{
// jot("Shifting over junction from ", f.str_short(), " to ", t.str_short());

    auto& mf = host.chain(f.w);
    auto& mt = host.chain(t.w);

    ASSERT(mf.c == mt.c, "Slots belong to differrent components");

    ASSERT(mf.ngs[f.e].contains(t) && mt.ngs[t.e].contains(f),
           "Slots {", f.str_short(), "} {", t.str_short(), "} are not incident");

    if (f.e == End::B) {

        auto& egf = mf.g.back();
        const auto sf = edge_descriptor(egf, f.e);

        auto v = egf.vertex_at_outer(f.e);
        auto u = egf.vertex_at_outer(End::opp(f.e));

        ASSERT(v, "Slot f ", f.str_short(), " is not connected.");
        ASSERT_CALLING(v->is_junction(), v->print("v"),
                       "Shifting a vertex outside junction");

        // reassign vertex connections
        auto& egt = mt.end_edge(t.e);
        const auto st = edge_descriptor(egt, t.e);

        ASSERT_CALLING(
            v->is_conected_to(st),
            v->print("v_at_slot_f"),
            "Vertex at slot f is not connected to slot t", t.str_short()
        );

        shift_egS(v, u, std::array<ED, 2>{{sf, st}}, host);

        // reassign edge hosting chains

        if (t.e == End::A)
            mt.g.insert(mt.g.begin(), std::move(egf));
        else {
            egf.template reverse<false>();
            mt.g.push_back(std::move(egf));
        }

        mf.g.pop_back();
    }
    else {  // f.e == End::A

        auto& egf = mf.g.front();
        const auto sf = edge_descriptor(egf, f.e);

        auto v = egf.vertex_at_outer(f.e);
        auto u = egf.vertex_at_outer(End::opp(f.e));

        ASSERT(v, "Slot f ", f.str_short(), " is not connected.");
        ASSERT_CALLING(v->is_junction(), v->print("v"),
                       "Shifting a vertex outside junction");

        // reassign vertex connections

        auto& egt = mt.end_edge(t.e);
        const auto st = edge_descriptor(egt, t.e);

        ASSERT_CALLING(
            v->is_conected_to(st),
            v->print("v_at_slot_f"),
            "Vertex at slot f is not connected to slot t", t.str_short()
        );

        shift_egS(v, u, std::array<ED, 2>{{sf, st}}, host);

        // reassign edge hosting chains

        if (t.e == End::A) {
            egf.template reverse<false>();
            mt.g.insert(mt.g.begin(), std::move(egf));
        }
        else
            mt.g.push_back(std::move(egf));


        mf.g.erase(mf.g.begin());
    }

    mf.set_g_w();
    mt.set_g_w();

    host.set_egl();
}


template<typename H>
constexpr
auto JunctionShifter<H>::
edge_descriptor(const Edge& eg,
                const End::Id eOuter) const noexcept -> ED
{
    return ED{eg.id, eg.indc, eg.inner_endId(eOuter)};
}

template<typename H>
void JunctionShifter<H>::
shift_egS(Vertex* v,
          Vertex* u,
          const std::array<ED, 2>& toOmit,
          Host& host
        )
{
    const auto nbEgSs = v->edges_connected_to(toOmit);

    for (const auto& s : nbEgSs) {
        auto& eg = host.edge(s.w);
        eg.disconnect_end_inner(s.e);
        eg.connect_to_inner(s.e, *u);
    }
}

}  // namespace graffine::structure::modifiers

#endif  // GRAFFINE_STRUCTURE_MODIFIERS_JUNCTION_SHIFTER_H