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
 * \brief Graph-wide table of vertex descriptors.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_VERTEX_DESCRIPTORS_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_CONTAINERS_VERTEX_DESCRIPTORS_IN_GRAPH_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/vertex_in_graph.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


namespace graffine::structure::containers {

/** Graph-wide table of vertex descriptors, indexed by VIdG.
 * \details Vertices are owned by the components. The graph assembles this
 * table on demand: it lists every vertex with its incident edges and
 * neighbours by graph-wide indexes (see descriptors::VertexInGraph), for
 * the graph-wide adjacency analyses and paths. The graph rebuilds the table
 * on its first access after a change of the graph.
 * \tparam G Graph type.
 */
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

    /// Descriptor at position \p i.
    constexpr auto operator[](const VIdG i)       noexcept ->       VertexDescr& { return m_data[i()]; }
    /// Descriptor at position \p i.
    constexpr auto operator[](const VIdG i) const noexcept -> const VertexDescr& { return m_data[i()]; }
    /// Descriptor of the vertex with handle \p h, which must be present.
    constexpr auto operator[](const VIdH h) const noexcept -> const VertexDescr&;
    /// Descriptor at position \p i.
    constexpr auto operator[](const std::size_t i)       noexcept ->       VertexDescr& { return m_data[i]; }
    /// Descriptor at position \p i.
    constexpr auto operator[](const std::size_t i) const noexcept -> const VertexDescr& { return m_data[i]; }

    /// Rebuilds the table from the vertices of all the components of
    /// graph \p gr, in component order.
    constexpr void set(const Graph& gr);
    /// Removes all the descriptors.
    constexpr void clear() { m_data.clear(); index.clear(); }

    /** Position of the descriptor of vertex \p vid, or
     * \ref utils::undefined if absent.
     * \details Constant time through an index by handle; falls back to a
     * linear scan if the index is out of date.
     */
    constexpr auto find(const VIdH vid) const noexcept -> std::size_t;
    /// Appends a copy of \p vd, setting its position \a ind.
    constexpr auto push_back(const VertexDescr& vd);

    /// Checks (in DEBUG builds) that the component ids and positions held
    /// are within the bounds of graph \p gr.
    void check(const Graph& gr) const;
    /// Prints the descriptors, preceded by the title \p s if not empty.
    void print(std::string_view s="") const noexcept;

private:

    using Base::m_data;

    /// Positions in m_data, indexed by the vertex id.
    /// \details The vertex ids are issued by a counter over the run (see
    /// Vertex::create_id()), so they are dense: a table indexed by them is
    /// faster than a hash map, and takes 4 bytes per vertex created in the
    /// run. Entries of vertices no longer in the graph are not cleared:
    /// find() confirms a hit by the id of the descriptor found.
    std::vector<std::uint32_t> index;

    /** Sets the index entry of the vertex \p id to the position \p pos.
     */
    constexpr void set_index(std::uint32_t id, std::size_t pos);
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

            GRAFFINE_ASSERT(cmp.ind == v->get_c());

            set_index(static_cast<std::uint32_t>(v->get_id()()), m_data.size());
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
            uint i {};
            for (const auto& edv : v->edges()) {  // edge descriptors in vertex

                const auto& edge = gr.edge(egl[edv.w].i);
                const auto oend = descriptors::End::opp(edv.e);
                m_data[vi].links[i++] = VertexDescr::Link{
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
    set_index(v.idh, m_data.size());
    m_data.push_back(std::move(v));
}

template<typename G>
constexpr
void VertexDescriptorsInGraph<G>::
set_index(const std::uint32_t id,
          const std::size_t pos)
{
    if (id >= index.size())
        index.resize(std::max<std::size_t>(id + 1, 2 * index.size()),
                     utils::undefined<std::uint32_t>);
    index[id] = static_cast<std::uint32_t>(pos);
}

template<typename G>
constexpr
auto VertexDescriptorsInGraph<G>::
operator[](const VIdH h) const noexcept -> const VertexDescr&
{
    const auto i = find(h);

    GRAFFINE_ASSERT(utils::is_defined(i));

    return m_data[i];
}

template<typename G>
constexpr
auto VertexDescriptorsInGraph<G>::
find(const VIdH vid) const noexcept -> std::size_t
{
    const auto h = static_cast<std::uint32_t>(vid());

    // The index is kept in sync by set(), push_back() and clear(); the hit is
    // confirmed, as entries of vertices removed are not cleared, and in case
    // the data was changed through the Sequence interface.
    if (h < index.size()) {
        const std::size_t i = index[h];
        if (i < size() && m_data[i].idh == h)
            return i;
    }

    for (size_t i {}; i < size(); ++i)
        if (m_data[i].idh == h)
            return i;

    return undefined<size_t>;
}

template<typename G>
void VertexDescriptorsInGraph<G>::
check(const Graph& gr) const
{
    for (size_t i {}; i < size(); ++i) {

        GRAFFINE_ASSERT_CALLING(m_data[i].ci < gr.ct.size(), m_data[i].print(100000),
                       "ved c overflow vs ct size ", gr.ct.size());
        GRAFFINE_ASSERT_CALLING(m_data[i].ind < gr.num_vertices(), m_data[i].print(100000),
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