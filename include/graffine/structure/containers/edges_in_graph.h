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
 * \file edges_in_graph.h
 * \brief Table of the edge descriptors of a graph.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_GRAPH_H
#define GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_GRAPH_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/descriptors/edge_in_graph.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>


namespace graffine::structure::containers {

/** Table of the edge descriptors of a graph, indexed by EgIdG.
 * \details The edges themselves are stored in the chains. This table maps
 * the graph-wide index of each edge to its location (chain and position)
 * and its stable handle, see descriptors::EdgeInGraph.
 * \tparam G Graph type.
 */
template<typename G>
struct EdgeDescriptorsInGraph
    : containers::Sequence<std::vector<descriptors::EdgeInGraph>>
{
    using Graph = G;
    using Component = Graph::Component;
    using Edge = Graph::Edge;
    using EdgeDescr = descriptors::EdgeInGraph;
    using Base = Sequence<std::vector<EdgeDescr>>;
    using Self = EdgeDescriptorsInGraph;
    using Vertices = Component::Vertices;
    using Chain = Component::Chain;
    using Chains = Component::Chains;

    /// Descriptor of the edge with graph-wide index \p i.
    constexpr auto operator[](const EgIdG i) -> EdgeDescr& { return m_data[i()]; }
    /// Descriptor of the edge with graph-wide index \p i.
    constexpr auto operator[](const std::size_t i) -> EdgeDescr& { return m_data[i]; }
    /// Descriptor of the edge with graph-wide index \p i.
    constexpr auto operator[](const EgIdG i) const -> const EdgeDescr& { return m_data[i()]; }
    /// Descriptor of the edge with graph-wide index \p i.
    constexpr auto operator[](const std::size_t i) const -> const EdgeDescr& { return m_data[i]; }

    /// Rebuilds the table from the edges of all the chains of graph \p gr.
    void set(const Graph& gr);

    /** Updates the descriptors of the edges of chains \p ww only.
     * \details The descriptors of the other edges must be up to date.
     * Chains not in the graph are skipped.
     */
    void set(const Graph& gr,
             std::initializer_list<ChIdG> ww);
    /// Appends the descriptor of edge \p g.
    void append(const Edge& g);
    /// Appends the descriptors of the edges of chain \p m, in chain order.
    void append(const Chain& m);
    /// Appends the descriptors of another table.
    void append(EdgeDescriptorsInGraph&& egl) noexcept;
    /// Removes all the descriptors.
    void clear() { m_data.clear(); }

private:

    using Base::m_data;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
void EdgeDescriptorsInGraph<G>::
set(const Graph& gr)
{
    m_data.resize(gr.num_edges());
    for (const auto& m : gr.cn)
        for (const auto& g : m.g)
            m_data[g.ind()].set(m.idw, g.indw, g.id);

}

template<typename G>
void EdgeDescriptorsInGraph<G>::
set(
    const Graph& gr,
    const std::initializer_list<ChIdG> ww
)
{
    m_data.resize(gr.num_edges());
    for (const auto w : ww)
        if (w < gr.num_chains())
            for (const auto& g : gr.cn[w].g)
                m_data[g.ind()].set(w, g.indw, g.id);
}

template<typename G>
void EdgeDescriptorsInGraph<G>::
append(EdgeDescriptorsInGraph&& egl) noexcept
{
    std::move(egl.begin(), egl.end(), std::back_inserter(m_data));
}

template<typename G>
void EdgeDescriptorsInGraph<G>::
append(const Chain& m)
{
    for (const auto& g : m.g)
        append(g);
}

template<typename G>
void EdgeDescriptorsInGraph<G>::
append(const Edge& g)
{
    m_data.push_back(EdgeDescr{g});
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_GRAPH_H