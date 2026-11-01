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
 * \file vertices.h
 * \brief Access to the vertices of a graph.
 * \details Part of the implementation of graph.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_VERTICES_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_VERTICES_H

#include "graffine/structure/elements/graph.h"

namespace graffine::structure::detail {

template<typename CC>
constexpr
auto Graph<CC>::
max_degree() const noexcept -> Degree
{
    auto d = Deg0;
    for (const auto& m : cn) {
        const auto md = m.max_degree();
        if (md > d)
            d = md;
    }
    return d;
}

template<typename CC>
auto Graph<CC>::
vertices(const Degree d) const -> VertexDescriptors
{
    refresh_vertex_descriptors();

    VertexDescriptors vds;

    for (const auto& vd : ved)
        if (d == vd.d)
            vds.push_back(vd);

    return vds;
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VertexDescr& vd) const noexcept -> const Vertex&
{
    GRAFFINE_ASSERT(vd.ind < ved.size(),
           "unexpected vd.i ", vd.ind, ": exceeds ved size ", ved.size());
    GRAFFINE_ASSERT(vd.ci < ct.size(),
           "unexpected vd.c ", vd.ci, ": exceeds ct size ", ct.size());

    return ct[vd.ci].vertex(VIdH{vd.idh});
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VertexDescr& vd) noexcept -> Vertex&
{
    GRAFFINE_ASSERT(vd.ind < ved.size(),
           "unexpected vd.i ", vd.ind, ": exceeds ved size ", ved.size());
    GRAFFINE_ASSERT(vd.ci < ct.size(),
           "unexpected vd.c ", vd.ci, ": exceeds ct size ", ct.size());

    return ct[vd.ci].vertex(VIdH{vd.idh});
}

template<typename CC>
auto Graph<CC>::
vertex(const VIdG ind) const -> const Vertex&
{
    refresh_vertex_descriptors();

    GRAFFINE_ASSERT(ind < ved.size(),
           "unexpected ind ", ind, ": exceeds ved size ", ved.size());

    return vertex(ved[ind]);
}

template<typename CC>
auto Graph<CC>::
vertex(const VIdG ind) -> Vertex&
{
    refresh_vertex_descriptors();

    GRAFFINE_ASSERT(ind < ved.size(),
           "unexpected ind ", ind, ": exceeds ved size ", ved.size());

    return vertex(ved[ind]);;
}

template<typename CC>
auto Graph<CC>::
vertex(const VIdH h) const -> const Vertex*
{
    refresh_vertex_descriptors();

    const auto i = ved.find(h);  // constant time

    return utils::is_defined(i) ? &vertex(ved[i]) : nullptr;
}

template<typename CC>
auto Graph<CC>::
vertex(const VIdH h) -> Vertex*
{
    refresh_vertex_descriptors();

    const auto i = ved.find(h);  // constant time

    return utils::is_defined(i) ? &vertex(ved[i]) : nullptr;
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const EndSlot& s) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(s.w < num_chains(), "EndSlot is not valid.");

    return cn[s.w].vertex_at_end(s.e);
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const EndSlot& s) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(s.w < num_chains(), "EndSlot is not valid.");

    return cn[s.w].vertex_at_end(s.e);
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const BulkSlot& s) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(s.w < num_chains(), "BulkSlot is not valid.");

    return cn[s.w].vertex_at_bulk(s.a());
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const BulkSlot& s) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(s.w < num_chains(), "BulkSlot is not valid.");

    return cn[s.w].vertex_at_bulk(s.a());
}

template<typename CC>
constexpr
auto Graph<CC>::
num_vertices(const Degree d) const noexcept -> std::size_t
{
    auto count_vertices = [&]() noexcept
    {
        std::size_t k {};
        for (const auto& m : cn)
             k += m.num_vertices(d);

        return d > Deg2 ? k/d : k;
    };

    return count_vertices();
}


template<typename CC>
constexpr
auto Graph<CC>::
num_vertices() const noexcept -> std::size_t
{
    std::size_t n {};
    for (const auto& c : ct)
        n += c.num_vertices();

    return n;
}


template<typename CC>
void Graph<CC>::
exchange_vertices(Vertex& a, Vertex& b)
{
    if (&a == &b)
        return;

    const auto ca = a.get_c();
    const auto cb = b.get_c();
    const std::vector ea (a.edges().begin(), a.edges().end());
    const std::vector eb (b.edges().begin(), b.edges().end());

    for (const auto& s : ea)
        ct[ca].edge(s.indc()).disconnect_end_inner(s.e);
    for (const auto& s : eb)
        ct[cb].edge(s.indc()).disconnect_end_inner(s.e);
    for (const auto& s : ea)
        ct[ca].edge(s.indc()).connect_to_inner(s.e, b);
    for (const auto& s : eb)
        ct[cb].edge(s.indc()).connect_to_inner(s.e, a);

    if (ca != cb) {
        ct[ca].vertices().move_to(ct[cb], &a);
        ct[cb].vertices().move_to(ct[ca], &b);
    }

    update_vertex_descriptors();
}


template<typename CC>
constexpr
auto Graph<CC>::
edge_vertices(const EgIdG ind) const noexcept -> const Edge::Vertices&
{
    return edge(ind).vertices();
}


template<typename CC>
constexpr
auto Graph<CC>::
edge_vids(const EgIdG ind) const noexcept -> Edge::VIDs
{
    return edge(ind).vids();
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_VERTICES_H
