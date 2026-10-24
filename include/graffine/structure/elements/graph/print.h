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
 * \file print.h
 * \brief Printing of a graph.
 * \details Part of the implementation of graph.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_PRINT_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_PRINT_H

#include "graffine/structure/elements/graph.h"

namespace graffine::structure::detail {

template<typename CC>
void Graph<CC>::
print_chains(const std::string& tag) const
{
    cn.print(tag);
}


template<typename CC>
void Graph<CC>::
print_components(const std::string_view tag) const
{
    if (ct.size()) {

        jot("Graph contains ", ct.size(), " connected component",
            ct.size() > 1 ? "s" : "", " (",
            cn.num(), " chains, ",
            numEdges, " edges, ",
            num_vertices(), " vertices):");

        for (const auto& c : ct)
            c.print(tag);
    }
    else
        jot("No components: the graph is empty!");
}


template<typename CC>
void Graph<CC>::
print_component_chains(const std::string_view tag) const
{
    jot("");
    for (const auto& c : ct) {
        c.print_chains(tag);
        jot("");
    }

    if (!ct.size())
        jot("No components: the graph is empty!");
}


template<typename CC>
void Graph<CC>::
print_adjacency_lists(const std::string_view tag) const
{
    for (const auto& c : ct)
        c.print_adjacency_list_edges(tag);
}


template<typename CC>
void Graph<CC>::
print_vertices(const std::string_view tag) const
{
    jot("");
    for (const auto& c : ct) {
        c.print_vertices(tag);
        jot("");
    }

    if (!ct.size())
        jot("No components: the graph is empty!");
}

template<typename CC>
void Graph<CC>::
print_vertices_deg(
    const Degree d,
    const std::string_view tag
) const
{
    jot("");
    for (const auto& c : ct) {
        c.print_vertices_deg(d, tag);
        jot("");
    }

    if (!ct.size())
        jot("No components: the graph is empty!");
}

template<typename CC>
void Graph<CC>::
report(std::ostream& ofs) const
{
    ofs << " v ";
    for (Degree d {}; d < maxDegree; ++d)
        (d < maxDegree - 1) ? (ofs << num_vertices(d) << " ")
                            : (ofs << num_vertices(d));
    ofs << " egn " << numEdges
        << " cnn " << num_chains()
        << " ctn " << num_compts();
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_PRINT_H
