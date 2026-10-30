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
 * \file graph_factory.h
 * \brief Implements assembling of graph structure from collection
 * of independent vertex and edge indexes.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_MODIFIERS_GRAPH_FACTORY_H
#define GRAFFINE_STRUCTURE_MODIFIERS_GRAPH_FACTORY_H

#include "graffine/definitions.h"
#include "graffine/structure/analyzers/connectivity.h"
#include "graffine/utils/errors.h"

#include <algorithm>
#include <list>
#include <memory>
#include <source_location>
#include <vector>

/// Handlers of large-scale structural modification of the graph
namespace graffine::structure::modifiers {


/**
 * Assembles a graph from its vertices and edges.
 * \details The input edges must be connected to the input vertices
 * (Edge::connect_to_outer()). The factory groups the edges into chains,
 * which run through the vertices of degree 2, joins the chains at the other
 * vertices, and forms the components from the chains connected to each
 * other, assigning all indexes.
 * \tparam G Graph type.
 */
template<typename G>
struct GraphFactory
{
    using Graph = G;
    using Edge = Graph::Edge;
    using Vertex = Graph::Vertex;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using EndSlot = Chain::EndSlot;
    using EdgeInVertex = Vertex::EgDescr;

    /**
     * Creates the graph assembled from \p globalVertices and
     * \p globalEdges .
     * \param[in] globalVertices All vertices of the graph.
     * \param[in] globalEdges All edges, connected to the vertices.
     * \return The assembled graph.
     */
    template<bool beLoud = false>
    auto create(
        std::vector<std::unique_ptr<Vertex>>&& globalVertices,
        std::vector<Edge>&& globalEdges
    ) -> Graph;

    /**
     * Assembles the graph in place, in the existing empty graph \p gr.
     * \details Suits graphs referred to by other objects, which therefore
     * cannot be replaced by a newly created one.
     * \param[out] gr Empty graph to be filled.
     * \param[in] globalVertices All vertices of the graph.
     * \param[in] globalEdges All edges, connected to the vertices.
     * \throws precondition_error If \p gr is not empty.
     */
    template<bool beLoud = false>
    void create(
        Graph& gr,
        std::vector<std::unique_ptr<Vertex>>&& globalVertices,
        std::vector<Edge>&& globalEdges
    );
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename G>
template<bool beLoud>
auto GraphFactory<G>::
create(
    std::vector<std::unique_ptr<Vertex>>&& globalVertices,
    std::vector<Edge>&& globalEdges
) -> Graph
{
    Graph gr;  //  Graph to be created.
    create<beLoud>(gr, std::move(globalVertices), std::move(globalEdges));
    return gr;
}


template<typename G>
template<bool beLoud>
void GraphFactory<G>::
create(
    Graph& gr,
    std::vector<std::unique_ptr<Vertex>>&& globalVertices,
    std::vector<Edge>&& globalEdges
)
{
    using namespace descriptors;

        if (gr.num_compts() || gr.num_chains())
            utils::raise<precondition_error>(
                nullptr, std::source_location::current(),
                "Graph Factory: the graph to be filled is not empty");

        auto find_unassigned_edges = [](std::vector<Edge>& g) -> Edge*
        {
            for (auto& eg : g)
                if (!eg.w.is_defined())
                    return &eg;

            return nullptr;
        };

        auto find_unassigned_chains = [](Chains& cn) -> Chain*
        {
            for (auto& m : cn)
                if (!m.idc.is_defined())
                    return &m;

            return nullptr;
        };

        auto find_edge_by_id = [](const EgIdH& id,
                                  std::vector<Edge>& g) -> Edge*
        {
            for (auto& eg : g)
                if (eg.id == id)
                    return &eg;

            return nullptr;
        };

        ChIdG w {};
        std::vector<std::list<Edge*>> ll;
        do {  // over edges in 'globalEdges'

            auto eg = find_unassigned_edges(globalEdges);
            if (!eg) break;

            std::list<Edge*> l;
            eg->w = w;
            l.emplace_back(eg);
            for (const auto e : End::Ids) {
                // Walk from the initial edge towards chain end 'e'.
                auto current = eg;
                do {
                    const auto vA = current->vertex_at_outer(e);
                    if (vA->deg() == 2) {
                        // Vertices refer to edges by edge inner ends, while
                        // 'e' is an outer end, i.e. in chain coordinates.
                        const auto neigEgDescr = *vA->edges_connected_to(
                            current->descriptor_for_vertex_at(
                                current->inner_endId(e))
                        ).begin();
                        auto ng = find_edge_by_id(neigEgDescr.idh, globalEdges);

                        GRAFFINE_ASSERT(ng, "Edge not found by Id");

                        if (!ng->w.is_defined()) {
                            ng->w = w;
                            e == End::A ? l.emplace_front(ng)
                                        : l.emplace_back(ng);
                            current = ng;
                        }
                        else break;
                    }
                    else break;
                } while (true);
            }

            for (EgIdA a {}; const auto z : l)
                z->indw = a++;

            ll.push_back(l);
            ++w;
        } while (true);

        if (!ll.size()) {
            warn("No edges imported. Produced an empty graph.");
            return;
        }
        else {
            if constexpr (beLoud)
                jot("Graph Factory: Importing ", globalEdges.size(),
                    " edges into ", ll.size(), " chains.");
        }

        auto& cn = gr.chains();
        for (ChIdG w {}; auto& l : ll) {
            cn.emplace_back(w++);
            for (EgIdA a {}; const auto z : l) {

                z->indw = a++;
                cn.back().g.push_back(std::move(*z));
            }
        }

        globalEdges.clear();

        if constexpr (beLoud)
            jot("Graph Factory: chains are created");

        // Set connections between chains.

        for (const auto& v : globalVertices) {
            const auto d = v->deg();
            if (d > Deg1) {
                std::vector<EndSlot> ss;
                for (const auto& egds : v->edges()) {

                    for (const auto& m : cn) {
                        for (const auto e : End::Ids) {

                            const auto& eg = m.end_edge(e);
                            if (eg.id == egds.idh &&
                                eg.inner_endId(e) == egds.e)

                                ss.emplace_back(m.idw, e);
                        }
                    }
                }
                if (ss.size())
                    for (std::size_t i {}; i < d; ++i)
                        for (std::size_t j {i+1}; j < d; ++j) {

                            const auto& s1 = ss[i];
                            const auto& s2 = ss[j];
                            cn[s1.w].ngs[s1.e].insert(s2);
                            cn[s2.w].ngs[s2.e].insert(s1);
                        }
            }
        }

        if constexpr (beLoud)
            jot("Graph Factory: chain connections are set");

        // Create components using inter-chain connections:

        Connectivity<Chain> conty {cn};
        auto& ct = gr.compts();
        do {
            auto current = find_unassigned_chains(cn);
            if (!current) break;

            auto& cmp = ct.emplace_back(gr.num_compts(), cn);
            cmp.ww = conty.find_connected_chains(current->idw);

            ChIdC ic {};
            EgIdC indc {};
            for (auto& w : cmp.ww) {

                auto& m = cn[w];
                m.c = cmp.ind;
                m.idc = ic++;
                for (auto& eg : m.g) {

                    eg.c = cmp.ind;
                    eg.indc = indc++;
                    for (const auto e : End::Ids) {
                        auto v = eg.vertex_at_outer(e);
                        v->set_c(cmp.ind);
                        v->reind_edge(EdgeInVertex{eg.id,
                                                   EgIdC::undefined,
                                                   eg.inner_endId(e)},
                                      eg.indc);
                    }
                }
            }
        } while (true);

        if constexpr (beLoud)
            jot("Graph Factory: component structure is ready: ",
                gr.num_components(), " components");

        // Set vertex types.

        // Move vertices into components.

        for (auto&& v : globalVertices)
            ct[v->get_c()].vv.insert(std::move(v));

        if constexpr (beLoud)
            jot("Graph Factory: vertices are moved into components");

        // Set auxiliary component structures.

        for (auto& cmp : ct) {
            cmp.set_egl();
            cmp.set_chis();
        }

        for (const auto& cmp : ct)
            GRAFFINE_ENSURE(!cmp.template check<beLoud>(),
                            "Graph Factory: component ", cmp.ind,
                            " fails its internal checks");

        // Set auxiliary graph members:
        gr.set_num_edges();
        gr.update_edge_descriptors();
        gr.update_vertex_descriptors();
}


}  // namespace graffine::structure::modifiers

#endif  // GRAFFINE_STRUCTURE_MODIFIERS_GRAPH_FACTORY_H
