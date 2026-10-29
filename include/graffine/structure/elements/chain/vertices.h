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
 * \brief Vertices of a chain.
 * \details Part of the implementation of chain.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_VERTICES_H
#define GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_VERTICES_H

#include "graffine/structure/elements/chain.h"

namespace graffine::structure::detail {

template<typename E>
constexpr
void Chain<E>::
connect_bulk_vertex(const EgIdA a, Vertex& v) noexcept
{
    GRAFFINE_ASSERT(a < length(), "End not valid.");

    g[a-1].connect_to_outer(End::B, v);
    g[a  ].connect_to_outer(End::A, v);
}


template<typename E>
constexpr
void Chain<E>::
connect_end_vertex(const EndId e, Vertex& v)
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    end_edge(e).connect_to_outer(e, v);
}


template<typename E>
constexpr
void Chain<E>::
disconnect_end_vertex(const EndId e) noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    end_edge(e).disconnect_end_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_end(const EndId e) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).vertex_at_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_end(const EndId e) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).vertex_at_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdA a) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(a > 0 , a, " is not a bulk position");

    return g[a].vertex_at_outer(End::A);
}

template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdA a) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(a > 0, a, " is not a bulk position");

    return g[a].vertex_at_outer(End::A);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdG ind1,
               const EgIdG ind2) const noexcept -> const Vertex*
{
    const auto eg1 = ind2eg(ind1);

    GRAFFINE_ASSERT(eg1, "Edge ind ", ind1, " not found in chain ", idw);

    const auto eg2 = ind2eg(ind2);

    GRAFFINE_ASSERT(eg2, "Edge ind ", ind2, " not found in chain ", idw);

    return vertex_at_bulk(eg1, eg2);
}

template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const Edge& eg1,
               const Edge& eg2) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(eg1.ind != eg2.ind, "Identical inds: ", eg1.ind);

    GRAFFINE_ASSERT(eg1->indw == eg2->indw + 1 || eg2->indw == eg1->indw + 1,
           "Edges at a1 ", eg1->indw, " a2 ", eg2->indw, " are expected ",
           "to have a common bulk vertex");

    return eg1->indw < eg2->indw
        ? eg2->vertex_at_outer(End::A)
        : eg1->vertex_at_outer(End::A);
}


template<typename E>
template<typename S>
constexpr
auto Chain<E>::
vertex_at_slot(const auto& s) const noexcept -> const Vertex*
{
    const auto [w, ea] = s.we();

    if constexpr (std::is_same_v<S, EndSlot>)
        return vertex_at_end(ea);
    else
        return vertex_at_bulk(ea);
}

template<typename E>
constexpr
auto Chain<E>::
degree_at_end(EndId e) const noexcept -> Degree
{
    return ngs[e].num() + 1;
}

template<typename E>
constexpr
auto Chain<E>::
max_degree() const noexcept -> Degree
{
    return std::max(std::max(degree_at_end(End::A),
                             degree_at_end(End::B)),
                    (length() > 1) ? Deg2 : Deg0);
}


template<typename E>
constexpr
auto Chain<E>::
num_vertices(const Degree d) const noexcept -> std::size_t
{
    GRAFFINE_ASSERT(is_implemented_degree(d), "Degree ", d, " is not implemented");

    if (d == Deg1) {  // count vertices of degree 1

        if ( is_connected_at(endA) &&  is_connected_at(endB)) return 0;
        if (!is_connected_at(endA) && !is_connected_at(endB)) return 2;
        /* else */                                            return 1;
    }

    if (d == Deg2)  // count internal chain vertices

        return is_unconnected_cycle() ? length() : length() - 1;

    // Count vertices of degree > 2

    const std::size_t N {containers::num_neigs(d)};
    const auto nA = ngs[endA].num();
    const auto nB = ngs[endB].num();

    if (nA == N && nB == N) return 2;
    if (nA == N || nB == N) return 1;
    if (nA != N && nB != N) return 0;

    return utils::undefined<size_t>;
}

template<typename E>
constexpr
auto Chain<E>::
vertices() const noexcept -> std::vector<const Vertex*>
{
    std::vector<const Vertex*> w;
    for (const auto& eg : g)
        for (const auto e : End::Ids)
            w.push_back(eg.vertex_at_outer(e));

    return w;
}


#if 0  // Disabled, as their declarations in chain.h.
template<typename E>
constexpr
void Chain<E>::
connect(Vertex& v,
        Edge& eg,
        const EndId egend) const noexcept
{
    v.insert(std::move(EdgeSlot{eg.ind, egend}));
    eg.connect_to_inner(egend, v);
}

template<typename E>
constexpr
void Chain<E>::
disconnect(Vertex& v,
           Edge& eg,
           const EndId egend) const noexcept
{
    v.erase_edge_descriptor(EdgeSlot{eg.ind, egend});
    eg.disconnect_end_inner(egend);
}
#endif

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_VERTICES_H
