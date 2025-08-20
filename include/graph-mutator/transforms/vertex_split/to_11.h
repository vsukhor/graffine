/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file to_11.h
 * @brief Contains class template enabling splits of specific graph vertices.
 * @details This Functor template specialization operates on vertices of
 * degree 2 to produce two degree 1 vertices.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_11_H
#define GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_11_H

#include <algorithm>
#include <array>
#include <iostream>

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "../vertex_merger/core.h"
#include "common.h"
#include "log.h"


namespace graph_mutator::vertex_split {

/**
 * @brief Template specialization for a functor generating 2 degree 1 vertices.
 * @details Adds vertex type-specific split capability and updates
 * the graph for it. This implementation operates on degree 2 vertices
 * at specific position to produce two degree 1 vertices: V2 -> 2V1.
 * @tparam G Graph to which the split operation is applied.
 */
template<typename G>
struct To<Deg1, Deg1, G> {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);
    static_assert(are_compatible_degrees<Deg1, Deg1>);

    static constexpr auto J1 = Deg1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Deg1;  ///< Degree of the 2nd output vertex.
    static constexpr auto I1 = J1 + J2;  ///< Degree of the input vertex.
    static constexpr auto I2 = undefined<Degree>;  ///< No 2nd input vertex.
    static constexpr auto I = I1;                  ///< Input vertex degree.

    static constexpr auto dd = string_ops::str2<J1, J2>;
    static constexpr auto shortName = string_ops::concat<shortNameStem, dd, 2>;
    static constexpr auto fullName  = string_ops::concat<fullNameStem, dd, 2>;

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using ResT = CmpId;
    using Res = std::array<ResT, 2>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit To(Graph& gr);

    /**
     * @brief Divides the graph chain at a vertex of degree 2.
     * @param w Global chain index.
     * @param a The vertex position inside the chain.
     */
    auto operator()(const BulkSlot& s) -> Res;

protected:

    /**
     * @brief Divides a disconnected cycle chain at its boundary vertex.
     * @param w Global chain index.
     */
    auto disconnected_cycle_d0(ChId w) -> Res;

    /**
     * @brief Divides a disconnected cycle chain at its bulk vertex.
     * @param w Global chain index.
     * @param a The vertex position inside the chain. Is counted from 1 and
     * is the last edge to remain in the original chain.
     */
    auto disconnected_cycle_d2(const BulkSlot& s) -> Res;

    /**
     * @brief Divides a linear chain at its bulk vertex.
     * @param w Global chain index.
     * @param a The vertex position inside the chain. Is counted from 1 and
     * is the last edge to remain in the original chain.
     */
    auto linear(const BulkSlot& s) -> Res;

private:

    vertex_merger::Core<Graph> merge;

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains&   cn;     ///< Reference to the graph edge chains.

    Log<Graph> log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
To<Deg1, Deg1, G>::
To(Graph& gr)
    : merge {gr, "vm_core called from"s + shortName}
    , gr {gr}
    , cn {gr.cn}
    , log {dd, gr}
{}


template<typename G>
auto To<Deg1, Deg1, G>::
operator()(const BulkSlot& s) -> Res
{
    const auto [w, a] = s.we();

    return !a || a == cn[w].length()
        ? disconnected_cycle_d0(w)
        : cn[w].is_disconnected_cycle()
        ? disconnected_cycle_d2(s)
        : linear(s);
}


template<typename G>
auto To<1, 1, G>::
disconnected_cycle_d0(const ChId w) -> Res
{
    auto& m = cn[w];

    // Print out summary before:

    if constexpr (verboseF)
        log.before(EndSlot{w, Ends::Undefined}, "_cyc");

    ASSERT(m.is_disconnected_cycle(),
           "vs11_cyc used with non-cycle chain ", w);

    ASSERT(cn[w].length() >= Chain::minCycleLength,
           "vs11_cyc: length of chain ", w, " is below minCycleLength");

    const auto ind1 = m.g.front().ind;
    const auto ind2 = m.g.back().ind;

    for (const auto e : Ends::Ids)
        m.ngs[e].clear();

    gr.ct[m.c].chis.cn11 = w;
    gr.ct[m.c].chis.cn22 = undefined<ChId>;

    gr.update_books();
    if constexpr (Graph::useAgl) {
        gr.update_adjacency_edges(ind1);
        gr.update_adjacency_edges(ind2);
    }

    // Print out summary after:

    if constexpr (verboseF)
        log.after(w, {});

    return {m.c, m.c};
}


// divides at a vertex of degree 2 inside a disconnected cycle chain
// 'w' is a global chain index
// 'a' is the vertex position inside the chain 'w'
// 'a' is counted from 1 and is the last edge to remain in the original chain
template<typename G>
auto To<Deg1, Deg1, G>::
disconnected_cycle_d2(const BulkSlot& s) -> Res
{
    const auto [w, a] = s.we();

    // Print out summary before

    if constexpr (verboseF)
        log.before(s, "_dic");

    ASSERT(cn[w].length() > 1, "vs11_chint: chain ", w, " has less than 2 edges");
    ASSERT(a && a < cn[w].length(),
           "vs11_lin: 'a' ", a, " at the chain end: use cycle() instead");

    const auto clini = cn[w].c;
    auto& cmp = gr.ct[clini];

    const auto ind1 = cn[w].g[a-1].ind;
    const auto ind2 = cn[w].g[a].ind;

    std::move(cn[w].g.begin(),
              cn[w].g.begin() + static_cast<long>(a),
              std::back_inserter(cn[w].g));
    cn[w].g.erase(cn[w].g.begin(),
                  cn[w].g.begin() + static_cast<long>(a));

    cn[w].set_g_w();
    cmp.set_gl();

    for (const auto e : Ends::Ids)
        cn[w].ngs[e].clear();

    cmp.chis.cn11 = w;
    cmp.chis.cn22 = undefined<ChId>;

    gr.update_books();
    if constexpr (Graph::useAgl) {
        gr.update_adjacency_edges(ind1);
        gr.update_adjacency_edges(ind2);
    }

    const auto w1 = gr.glm[ind1];
    const auto w2 = gr.glm[ind2];

    ASSERT(cn[w1].idw == w && cn[w2].idw == w,
           "vs11_dc2: w ", w, " must be equal to ",
           cn[w1].idw, " and ", cn[w2].idw);
    ASSERT(cn[w1].c == clini && cn[w2].c == clini,
           "vs11_dc2: clini ", clini, " must be equal to ",
           cn[w1].c, " and ", cn[w2].c);

    // Print out summary after

    if constexpr (verboseF)
        log.after(w, {ind2}, true);

    return {clini, clini};
}


// divides at a vertex of degree 2
// 'w' is a global chain index
// 'a' is the vertex position inside the chain 'w'
// 'a' is counted from 1 and is the last edge to remain in the original chain
template<typename G>
auto To<Deg1, Deg1, G>::
linear(const BulkSlot& s) -> Res
{
    const auto [w, a] = s.we();

    // Print out summary before

    if constexpr (verboseF)
        log.before(s, "_lin");

    ASSERT(cn[w].length() > 1, "vs11_lin: chain ", w, " has less than 2 edges");
    ASSERT(a && a < cn[w].length(),
           "vs11_lin: 'a' ", a, " at the chain end: use cycle() instead");

    [[maybe_unused]] const auto clini = cn[w].c;

    const auto ind1 = cn[w].g[a-1].ind;
    const auto ind2 = cn[w].g[a].ind;

    cn.emplace_back(gr.chain_num());    // new chain produced by the split

    auto& n = cn.back();

    const auto ss = EndSlot{w, Ends::B};
    const auto isConnected = cn[w].is_connected_at(ss.e);
    bool isCycled {};
    if (isConnected) {
        typename Graph::PathsOverEndSlots pp {gr.ct[clini]};
    // this is true iff there is a connection to s2 via a path outgoing from 's'
    // knownSize is true: enumertate over the cluster
        isCycled = pp.template are_connected<false>(ss, ss.opp());
        if (!isCycled)
            gr.template split_component<false>(gr.ct[cn[w].c], pp, ss);
    }

    std::move(cn[w].g.begin() + static_cast<long>(a),
              cn[w].g.end(),
            std::back_inserter(n.g));
    cn[w].g.erase(cn[w].g.begin() + static_cast<long>(a),
                  cn[w].g.end());

    gr.copy_neigs(EndSlot{w, Ends::B},
                  EndSlot{n.idw, Ends::B});
    cn[w].ngs[Ends::B].clear();

    n.set_g_w();

    if (!isConnected) {
        gr.ct.emplace_back(n, gr.cmpt_num(), cn);
        gr.ct[cn[w].c] = typename Graph::Cmpt {gr.ct[cn[w].c].ww, cn[w].c, cn};
    }
    else {
        auto& current = gr.ct[cn[w].c];
        if (!isCycled) {
            auto& last = gr.ct.back();
            n.set_cmpt(last.ind, last.num_chains(), last.num_edges());
            last.append(n);
        }
        else
            current.append(n);
        current.set_edges();
        current.set_gl();
        current.chis.populate(cn, current.ww);
    }

    gr.update_books();
    if constexpr (Graph::useAgl) {
        gr.update_adjacency_edges(ind1);
        gr.update_adjacency_edges(ind2);
    }

    const auto w1 = gr.glm[ind1];
    const auto w2 = gr.glm[ind2];

    ASSERT(cn[w1].c == clini || cn[w2].c == clini,
        "vs11_lin: clini ", clini, " must be equal to ",
        cn[w1].c, " or ", cn[w2].c);

    // Print out summary after

    if constexpr (verboseF)
        log.after(w1, {ind2}, false);

    return {cn[w].c, n.c};
}


}  // namespace graph_mutator::vertex_split

#endif  // GRAPH_MUTATOR_VERTEX_SPLIT_FUNCTOR_11_H
