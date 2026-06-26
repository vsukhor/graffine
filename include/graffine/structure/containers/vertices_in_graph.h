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
 * \file vertices_in_graph.h
 * \brief Defines collection of vertex descriptors for use in the graph object.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_VERTEX_DESCRIPTORS_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_CONTAINERS_VERTEX_DESCRIPTORS_IN_GRAPH_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/vertex_in_graph.h"

#include <string>
#include <string_view>
#include <vector>


namespace graffine::structure::containers {

template<typename G>
struct VertexDescriptorsInGraph
    : containers::Sequence<std::vector<descriptors::VertexInGraph>>
{
    using Graph = G;
    using Component = Graph::Component;
    using Vertex = Graph::Vertex;
    using VertexDescr = descriptors::VertexInGraph;
    using Base = Sequence<std::vector<VertexDescr>>;
    using Self = VertexDescriptorsInGraph;
    using Vertices = Component::Vertices;
    using Chain = Component::Chain;
    using Chains = Component::Chains;

    constexpr auto operator[](const VIdG i)       noexcept ->       VertexDescr& { return m_data[i()]; }
    constexpr auto operator[](const VIdG i) const noexcept -> const VertexDescr& { return m_data[i()]; }
    constexpr auto operator[](const VIdH h) const noexcept -> const VertexDescr&;
    constexpr auto operator[](const std::size_t i)       noexcept ->       VertexDescr& { return m_data[i]; }
    constexpr auto operator[](const std::size_t i) const noexcept -> const VertexDescr& { return m_data[i]; }

    constexpr void set(const Graph& gr);
    constexpr void clear() { m_data.clear(); }
    constexpr auto find(const VIdH vid) const noexcept -> std::size_t;
    constexpr auto push_back(const VertexDescr& vd);

    void check(const Graph& gr) const;
    void print(std::string_view s="") const noexcept;

private:

    using Base::m_data;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
void VertexDescriptorsInGraph<G>::
set(const Graph& gr)
{
    m_data.clear();
    m_data.reserve(gr.num_vertices());

    for (const auto& cmp: gr.ct)
        for (const auto& v : cmp.vertices().get()) {

            ASSERT(cmp.ind == v->get_c());

            m_data.emplace_back(static_cast<uint>(v->get_c()()),
                                static_cast<uint>(m_data.size()),
                                static_cast<uint>(v->get_id()()),
                                v->deg());
        }

    // set neigs
    uint vi {};
    for (const auto& cmp : gr.ct) {
        const auto& egl = cmp.get_egl();  // descriptors for edges of this component
        for (const auto& v : cmp.vertices().get()) {
            const auto edvs = v->edges().as_vector();
            for (uint i {}; i < v->deg(); ++i) {

                const auto& edv = edvs[i];  // edge descriptor in vertex
                const auto& edge = gr.edge(egl[edv.w].i);
                const auto oend = descriptors::End::opp(edv.e);
                m_data[vi].links[i] = VertexDescr::Link{
                    static_cast<uint>(edge.ind()),
                    static_cast<uint>(find(edge.vertex_at_inner(oend)->get_id()))
                };
            }
            ++vi;
        }
    }
}

template<typename G>
constexpr
auto VertexDescriptorsInGraph<G>::
push_back(const VertexDescr& vd)
{
    auto v = vd; // copy
    v.ind = static_cast<uint>(m_data.size());
    m_data.push_back(std::move(v));
}

template<typename G>
constexpr
auto VertexDescriptorsInGraph<G>::
operator[](const VIdH h) const noexcept -> const VertexDescr&
{
    const auto i = find(h);

    ASSERT(is_defined(i));

    return m_data[i];
}

template<typename G>
constexpr
auto VertexDescriptorsInGraph<G>::
find(const VIdH vid) const noexcept -> std::size_t
{
    for (size_t i {}; i < size(); ++i)
        if (m_data[i].idh == static_cast<uint>(vid()))
            return i;

    ASSERT(false, "should not reach here");
    return undefined<size_t>;
}

template<typename G>
void VertexDescriptorsInGraph<G>::
check(const Graph& gr) const
{
    for (size_t i {}; i < size(); ++i) {

        ASSERT_CALLING(m_data[i].ci < gr.ct.size(), m_data[i].print(100000),
                       "ved c overflow vs ct size ", gr.ct.size());
        ASSERT_CALLING(m_data[i].ind < gr.num_vertices(), m_data[i].print(100000),
                       "ved i overflow vs num_vertices ", gr.num_vertices());
    }

}

template<typename G>
void VertexDescriptorsInGraph<G>::
print(std::string_view s) const noexcept
{
    if (s.length()) jot(s);

    for (size_t i {}; i < size(); ++i)
        m_data[i].print(i);
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_VERTEX_DESCRIPTORS_IN_GRAPH_H