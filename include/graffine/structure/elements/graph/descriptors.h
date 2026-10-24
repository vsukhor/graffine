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
 * \file descriptors.h
 * \brief Updates of the graph-wide edge and vertex descriptors and adjacency lists.
 * \details Part of the implementation of graph.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_DESCRIPTORS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_DESCRIPTORS_H

#include "graffine/structure/elements/graph.h"

namespace graffine::structure::detail {

template<typename CC>
auto Graph<CC>::
get_ved(const VIdG ind) const -> const VertexDescr&
{
    refresh_vertex_descriptors();

    GRAFFINE_ASSERT(ind < ved.size(), "ind exceeds the number of vertices");

    return ved[ind()];
}

template<typename CC>
auto Graph<CC>::
get_ved(const VIdG ind) -> VertexDescr&
{
    refresh_vertex_descriptors();

    GRAFFINE_ASSERT(ind < ved.size(), "ind exceeds the number of vertices");

    return ved[ind()];
}

template<typename CC>
void Graph<CC>::
update_adjacency_edges(const EgIdG ind) noexcept
{
    const auto c = cn[egl[ind].w].c;
    ct[c].adjacency_list_edges();
}


template<typename CC>
void Graph<CC>::
update_adjacency() noexcept
{
    for (auto& c : ct)
        c.adjacency_list_edges();
}


template<typename CC>
template<bool withVertices>
void Graph<CC>::
update(const std::initializer_list<ChIdG> ww)
{
    update_edge_descriptors(ww);
    if constexpr (useAgl)
        update_adjacency();
    if constexpr (withVertices)
        update_vertex_descriptors();
}


template<typename CC>
template<bool withVertices>
void Graph<CC>::
update()
{
    update_edge_descriptors();
    if constexpr (useAgl)
        update_adjacency();
    if constexpr (withVertices)
        update_vertex_descriptors();
}


template<typename CC>
void Graph<CC>::
update_edge_descriptors()
{
    egl.set(*this);
}


template<typename CC>
void Graph<CC>::
update_edge_descriptors(const std::initializer_list<ChIdG> ww)
{
    egl.set(*this, ww);

#ifdef DEBUG
    // An incremental update must agree with a full one.
    EdgeDescriptors full;
    full.set(*this);
    GRAFFINE_ASSERT(full.size() == egl.size(), "incremental egl: size differs");
    for (std::size_t i {}; i < full.size(); ++i)
        GRAFFINE_ASSERT(full[i].w == egl[i].w && full[i].a == egl[i].a &&
                        full[i].h == egl[i].h,
                        "incremental egl: stale descriptor of edge ", i,
                        ": chain ", egl[i].w, " vs ", full[i].w);
#endif
}


template<typename CC>
void Graph<CC>::
update_vertex_descriptors() noexcept
{
    vedStale = true;
}


template<typename CC>
void Graph<CC>::
refresh_vertex_descriptors() const
{
    if (vedStale) {
        ved.set(*this);
        vedStale = false;
    }
}


template<typename CC>
template<typename F>
auto Graph<CC>::
adjacency_list_edges(F(*f)(const Edge&)) const noexcept -> vec2<F>
{
    vec2<F> x(numEdges);

    for (const auto& m : cn) {
        for (std::size_t a {}; a < m.length(); ++a) {

            const auto ind = m.g[a].ind;

            if (m.is_tail(a)) {
                for (const auto& s : m.ngs[End::A]())
                    // Connection backwards: only other chains might be found:
                    x[ind].push_back(f(edge(slot2ind(s))));

                if (m.length() == 1)
                    // Connection forwards: to other chain:
                    for (const auto& s : m.ngs[End::B]())
                        x[ind].push_back(f(edge(slot2ind(s))));

                else
                    // Connection forwards: to the same chain:
                    x[ind].push_back(f(m.g[a+1]));
            }
            else if (m.is_head(a)) {
                // but not  a1 == 0  =>  m.length() > 1
                // Connection backwards: to the same chain:
                x[ind].push_back(f(m.g[a-1]));
                // Connection forwards: to other chains:
                for (const auto& s : m.ngs[End::B]())
                    x[ind].push_back(f(edge(slot2ind(s))));
            }
            else {
                // Edge in the chain interior: a > 0 && a < m.length() - 1:
                // Connection backwards: to the same chain:
                x[ind].push_back(f(m.g[a-1]));
                // Connection forwards: to the same chain:
                x[ind].push_back(f(m.g[a+1]));
            }
        }
    }

    return x;
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_DESCRIPTORS_H
