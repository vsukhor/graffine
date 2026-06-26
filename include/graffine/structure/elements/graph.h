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
 * \file graph.h
 * \brief Contains class template encapsulating graph structure-related features.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_H
#define GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/chains.h"
#include "graffine/structure/containers/components.h"
#include "graffine/structure/containers/edges_in_graph.h"
#include "graffine/structure/containers/vertices_in_graph.h"
#include "graffine/structure/analyzers/adjacency.h"
#include "graffine/structure/common.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/paths/over_endslots/generic.h"
#include "graffine/structure/descriptors/edge_in_graph.h"
#include "graffine/structure/descriptors/vertex_in_graph.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <filesystem>
#include <memory>
#include <ostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>


namespace graffine::structure {
namespace detail {

template<typename CC,
         typename... Props>
struct Graph
{};


/**
 * Implementation of a multicomponent Graph.
 * \details Implements major structure-related proterties important for
 * topological transformations, such as collections and classifications of
 * graph Chains, but is unaware of any physical or geometric properties.
 * Forms base for clases adding graph-specific characteristics.
 * \tparam CC Type of connected components forming the graph object.
*/
template<typename CC>
struct Graph<CC>
{
    using Self = Graph<CC>;
    using Compt = CC;  // Connected component
    using Component = CC;
    using Components = structure::containers::Components<Compt>;
    using Chain = Compt::Chain;
    using Chains = Compt::Chains;
    using Edge = Chain::Edge;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using End = Chain::End;  ///< Edge ends.
    using EndId = End::Id;
//    using Facet = Compt::Facet;
//    using Facets = Compt::Facets;
    using EdgeDescr = descriptors::EdgeInGraph;
    using EdgeDescriptors = structure::containers::EdgeDescriptorsInGraph<Self>;
    using Vertex = Edge::Vertex;
    using VertexDescr = descriptors::VertexInGraph;
    using VertexDescriptors = structure::containers::VertexDescriptorsInGraph<Self>;
    using PathsOverEndSlots = paths::over_endslots::Generic<Compt>;

    static constexpr auto Type = Elements::Graph;
    static constexpr bool isSpatial {};
    static constexpr bool isMulticomponent {true};
    static constexpr bool useAgl {};

    template<typename, bool> friend struct IntegralTests;
    template<typename> friend struct GraphFactory;
    friend struct structure::containers::EdgeDescriptorsInGraph<Self>;
    friend struct structure::containers::VertexDescriptorsInGraph<Self>;

// VARIABLES -------------------------------------------------------------------

    Adjacency<Graph> adj {*this};

protected:

    Chains     cn;  ///< The collection of chains.
    Components ct;  ///< The collection of components.

    std::size_t numEdges {};  ///< Current number of edges.

    /// Map of graph-wide edge indexes to chain indexes and in-chain positions.
    EdgeDescriptors egl;

    /// Graph-wide collection of vertex data.
    VertexDescriptors ved;

// METHODS ---------------------------------------------------------------------

public:

    constexpr Graph() = default;
    constexpr ~Graph() = default;

    constexpr Graph(Graph&& gr) = default;
    constexpr auto operator=(Graph&& gr) -> Graph& = default;

    // Non-copiable because vertices are owned by std::unique_ptr;
    // state this explicitely for clarity.
    constexpr Graph(const Graph& gr) = delete;
    constexpr auto operator=(const Graph& gr) -> Graph& = delete;

    explicit constexpr Graph(Chains&& gr);

    /**
     * Generate initial set of the graph components.
     * \details The components are single chains of equal length 'len'.
     * \param[in] num Desired number of components.
     * \param[in] len Component length.
     */
    void generate_single_chain_components(
        std::size_t num,
        std::size_t len
    );

    /// Appends an unconnected chain to the graph.
    void add_single_chain_component(std::size_t len);

    void add_component(Compt&& cmp);

    void remove_edge(EgIdG ind);

//    /**
//     * Updates the connected component indexes.
//     * \param f Initial index.
//     * \param t Final index.
//     */
//    void changechange_cmpt_cmpt(CmpId f, CmpId t) noexcept;

//    /**
//     * Updates the connected component indexes.
//     * \param c Initial index.
//     */
//    void update_indc(CmpId c) noexcept;

    /**
     * Appends \p donor component to \p acceptor component.
     * \note If necessary, lhe last component in \a ct is moved to the donor
     * place and reindexed.
     * \param[in] acceptor Component to which \p donor component is appended.
     * \param[in] donor Component merged to the \p acceptor.
     * \return reference to the \p acceptor object
     */
    auto merge_components(
        Compt& acceptor,
        Compt& donor
    ) noexcept -> Compt&;

    /**
     * Appends \p donor component to \p acceptor component.
     * \note If necessary, lhe last component in \a ct is moved to the donor
     * place and reindexed.
     * \param[in] acceptor Component to which \p don component is appended.
     * \param[in] donor Component merged to the \p acc.
     * \return reference to the \p acceptor object
     */
    auto merge_components(
        CmpId acceptor,
        CmpId donor
    ) noexcept -> Compt&;

    auto split_component(
        Compt& cmp,
        const PathsOverEndSlots& pp,
        const EndSlot& s
    )-> Compt&;

    auto split_component(
        Compt& cmp,
        ChIdGs&& rm
    ) -> Compt&;

    auto split_component(
        Compt& cmp,
        Chain& m
    ) -> Compt&;

    constexpr auto compts() const noexcept -> const Components& { return ct; }
    constexpr auto compts()       noexcept ->       Components& { return ct; }
    constexpr auto compt(const CmpId c) const noexcept -> const Component& { return ct[c]; }
    constexpr auto compt(const CmpId c)       noexcept ->       Component& { return ct[c]; }
    constexpr auto compt(const ChIdG w) const noexcept -> const Component& { return ct[cn[w].c]; }
    constexpr auto compt(const ChIdG w)       noexcept ->       Component& { return ct[cn[w].c]; }
    constexpr auto compt(const EgIdG i) const noexcept -> const Component& { return ct[egl[i].c]; }
    constexpr auto compt(const EgIdG i)       noexcept ->       Component& { return ct[egl[i].c]; }
    constexpr auto compt_last() const noexcept -> const Component& { return ct.back(); }
    constexpr auto compt_last()       noexcept ->       Component& { return ct.back(); }
    constexpr auto cmpid(const ChIdG w) const noexcept -> CmpId { return cn[w].c; }
    constexpr auto cmpid(const EgIdG i) const noexcept -> CmpId { return edge(i).c; }

    constexpr auto chids() const noexcept -> ChIdGs { return cn.chids(); }
    constexpr auto chid(const EgIdG ind) const noexcept -> ChIdG;
    constexpr auto chains() const noexcept -> const Chains& { return cn; }
    constexpr auto chains()       noexcept ->       Chains& { return cn; }
    constexpr auto chain(const ChIdG& w) const noexcept -> const Chain& { return cn[w]; }
    constexpr auto chain(const ChIdG& w)       noexcept ->       Chain& { return cn[w]; }
    constexpr auto chain(const EndSlot& s) const noexcept -> const Chain&;
    constexpr auto chain(const EndSlot& s)       noexcept ->       Chain&;
    constexpr auto chain(const EgIdG ind) const noexcept -> const Chain&;
    constexpr auto chain(const EgIdG ind)       noexcept ->       Chain&;
    constexpr auto chain_last() const noexcept -> const Chain& { return cn.back(); }
    constexpr auto chain_last()       noexcept ->       Chain& { return cn.back(); }

    constexpr auto edge(EgIdG ind) const noexcept -> const Edge&;
    constexpr auto edge(EgIdG ind)       noexcept ->       Edge&;
    constexpr auto edge(const EndSlot& s) noexcept -> Edge&;
    constexpr auto edge(const BulkSlot& s) noexcept -> Edge&;
    constexpr auto edge_last() const noexcept -> const Edge& { return edge(numEdges-1); };
    constexpr auto edge_last()       noexcept ->       Edge& { return edge(numEdges-1); };

    constexpr auto vertices(Degree d) const noexcept -> VertexDescriptors;
    constexpr auto vertex(const VertexDescr& vd) const noexcept -> const Vertex&;
    constexpr auto vertex(const VertexDescr& vd)       noexcept ->       Vertex&;
    constexpr auto vertex(VIdG ind) const noexcept -> const Vertex&;
    constexpr auto vertex(VIdG ind)       noexcept ->       Vertex&;
    constexpr auto vertex(VIdH h) const noexcept -> const Vertex*;
    constexpr auto vertex(VIdH h)       noexcept ->       Vertex*;
    constexpr auto vertex(const EndSlot& s) const noexcept -> const Vertex*;
    constexpr auto vertex(const EndSlot& s)       noexcept ->       Vertex*;
    constexpr auto vertex(const BulkSlot& s) const noexcept -> const Vertex*;
    constexpr auto vertex(const BulkSlot& s)       noexcept ->       Vertex*;

    constexpr auto get_egl() const noexcept -> const EdgeDescriptors& { return egl; }
    constexpr auto get_egl()       noexcept ->       EdgeDescriptors& { return egl; }
    constexpr auto get_egl(EgIdG ind) const noexcept -> const EdgeDescr&;
    constexpr auto get_egl(EgIdG ind)       noexcept ->       EdgeDescr&;

    constexpr auto get_ved() const noexcept -> const VertexDescriptors& { return ved; }
    constexpr auto get_ved()       noexcept ->       VertexDescriptors& { return ved; }
    constexpr auto get_ved(VIdG ind) const noexcept -> const VertexDescr&;
    constexpr auto get_ved(VIdG ind)       noexcept ->       VertexDescr&;
    constexpr auto get_ved(VIdH h) const noexcept -> const VertexDescr& { return ved[h]; }
    constexpr auto get_ved(VIdH h)       noexcept ->       VertexDescr& { return ved[h]; }

    constexpr void rotate_chain(Chain& m, std::size_t n);

    /**
     * \note Does not update edge connectivity.
     *       Does not update chain connectivity and 'ngs'.
     *       Does not remove source chain if it is emptied.
    */
    constexpr void move_edges(
        Chain& fr,
        std::size_t posFrBegin,
        std::size_t posFrEnd,  // position AFTER the last moved element
        Chain& to,
        std::size_t posTo  // position of the first inserted element
    );

    constexpr auto slot2a(const EndSlot& s) const noexcept -> EgIdA;

    constexpr auto slot2ind(const EndSlot& s) const noexcept -> EgIdG;
    constexpr auto ind2bslot(EgIdG ind) const noexcept -> BulkSlot;

    constexpr auto edge_end(const EndSlot& s) const noexcept -> EndId;

    /// Updates internal data for component c.
//    void update_adjacency_cmpt(CmpId c) noexcept;

    /// Updates internal data related to edge 'ind'.
    void update_adjacency_edges(EgIdG ind) noexcept;

    /// Updates internal data.
    void update_adjacency() noexcept;

    /**
     * Updates internal vectors.
     * \details Calls update_edge_descriptors(), update_vertex_descriptors()
     * and update_adjacency().
     */
    template<bool withVertices = true>
    void update() noexcept;

    /**
     * Initializes or updates vector of graph-wide edge descriptors.
     */
    void update_edge_descriptors() noexcept;

    /**
     * Initializes or updates vector of graph-wide vertex descriptors.
     */
    void update_vertex_descriptors() noexcept;

    /**
     * Initializes or updates adjacency list.
     * \tparam F Type of element of the generated list.
     * \param[in] f pointer to the function used for the lelment generation.
     */
    template<typename F>
    auto adjacency_list_edges(F(*f)(const Edge&)) const noexcept -> vec2<F>;

    auto adjacency_list_chains() const noexcept -> vec2<ChIdG>;

//    auto make_adjacency_list_old() const noexcept -> Base::adjLT;
//    auto make_adjacency_list_old(std::size_t c) noexcept -> vec2<size_t>;

    constexpr auto max_degree() const noexcept -> Degree;

    /// Number of vertices for the specific vertex degree \p d.
    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;

    /// Total number of vertices.
    constexpr auto num_vertices() const noexcept -> std::size_t;

    /// Current number of components.
    constexpr auto num_compts() const noexcept -> std::size_t { return ct.size(); };
    constexpr auto num_components() const noexcept -> std::size_t { return num_compts(); };

    /// Current number of chains.
    constexpr auto num_chains() const noexcept -> std::size_t { return cn.num(); };

    /// Current number of edges.
    constexpr auto num_edges() const noexcept -> std::size_t { return numEdges; };
    constexpr auto set_num_edges() noexcept { numEdges = cn.num_edges(); }
    constexpr auto set_num_edges(const std::size_t n) noexcept { numEdges = n; }

    constexpr auto ind_last_cmpt() const noexcept -> CmpId;
    constexpr auto ind_last_chain() const noexcept -> ChIdG;

    constexpr void connect(const EndSlot& s1,
                           const EndSlot& s2) noexcept;

    constexpr auto ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&;
    constexpr auto ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&;

    constexpr auto ng_inds_at(const EndSlot& s) const noexcept -> EgIdGs;

    constexpr auto edge_vertices(EgIdG ind) const noexcept -> const Edge::Vertices&;
    constexpr auto edge_vids(EgIdG ind) const noexcept -> Edge::VIDs;

    auto inds_to_chain_link(
        EgIdG ind1,
        EgIdG ind2
    ) const noexcept -> std::array<EndSlot, 2>;

    /**
     * Updates graph chain indexes.
     * \details The indexes are updated so that the chain indexed as source
     * will acquire the identity of the target:
     * - connections of the source become connections of the target (end to end).
     * - the source edges are moved to the target.
     * - target acquires component index of the source.
     * Source now devoid of edges retains its neigs which are now identical to
     * those of the target.
     * \param[in] f Source chain index.
     * \param[in] t Target chain index.
     */
    template<bool updateChis=true>
    void rename_chain(ChIdG f, ChIdG t);

    /**
     * Copies connection partners to a new chain.
     * \details Copies assigning the connected slots of \p f to \p t and
     * updates neigs of \p f to become neigs of \p t . Previous neigs of \p t
     * are deleted; \p f neigs are not cleared.
     * \param[in] f Source slot.
     * \param[in] t Target slot.
     */
    void copy_neigs(
        const EndSlot& f,
        const EndSlot& t
    );

    /**
     * Removes chain end slot \p s from list of \a neigs in slots connected to it.
     * \param[in] s Chain end slot to be removed.
     */
    void remove_slot_from_neigs(const EndSlot& s);

    /**
     * Replaces \p old with \p nov in slots connected to \p nov .
     * \details Slots connected to \p nov will be checked, and if there is
     * \p old, it will be replaced by \p nov .
     * \note Important: \p old retains list of slots formerly connected to it.
     */
    void replace_slot_in_neigs(
        const EndSlot& old,
        const EndSlot& nov
    );

    constexpr bool is_same_cycle(
        const EndSlot& s1,
        const EndSlot& s2
    ) const noexcept;

    /**
     * Checks if the graph is equilibrated.
     * \return True if equilibrated, false otherwise.
     */
    bool is_equilibrated() const noexcept;

    /**
     * Prints the graph chains using prefix \p tag.
     * \param[in] tag Human-readable description common to all records.
     */
    void print_chains(const std::string& tag) const;

    /**
     * Prints the graph components using prefix \p tag.
     * \param[in] tag Human-readable description common to all records.
     */
    void print_components(std::string_view tag="") const;

    void print_component_chains(std::string_view tag="") const;
    void print_adjacency_lists(std::string_view tag="") const;
    void print_vertices(std::string_view tag=" ") const;
    void print_vertices_deg(
        Degree d,
        std::string_view tag=" "
    ) const;

    /**
     * Outputs the graph components to a text-formatted stream.
     * \param[in] ofs Output stream.
     */
    void report(std::ostream& ofs) const;

};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename CC>
constexpr
Graph<CC>::
Graph(Chains&& cn)
    : cn {std::move(cn)}
{}

template<typename CC>
void Graph<CC>::
generate_single_chain_components(
    const std::size_t num,
    const std::size_t len
)
{
    const auto n = num_chains();  // number of chains already present

    while (num_compts() < num + n)
        add_single_chain_component(len);
    if constexpr (verboseF)
        jot("Generated ", colorcodes::GREEN, num, colorcodes::RESET,
            " identical single-chain components, to ", numEdges, " edges total");
}


template<typename CC>
void Graph<CC>::
add_single_chain_component(const std::size_t len)
{
    ct.emplace_back(num_compts(), cn);  // empty
    EgIdG indIni {numEdges};
    ct.back().create_chain(len, indIni);
    numEdges += len;

    update();
}


template<typename CC>
void Graph<CC>::
add_component(Compt&& cmp)
{
    numEdges += cmp.num_edges();
    ct.push_back(std::move(cmp));

    update();
}

template<typename CC>
auto Graph<CC>::
split_component(
    Compt& cmp,
    const PathsOverEndSlots& pp,
    const EndSlot& s
) -> Compt&
{
    auto [accessible, blocked] =
        pp.classify_chains_by_connectivity(s);

    if constexpr (verboseF)
        cmp.template print_classification<false>(accessible, blocked);

    return split_component(cmp, std::move(blocked));
}

template<typename CC>
auto Graph<CC>::
split_component(Compt& cmp, ChIdGs&& rm) -> Compt&
{
    const auto indOld = cmp.ind;  // reallocation of 'ct' may invalidate ref
    auto& newcmp = ct.emplace_back(num_compts(), cn);  // empty component

    ct[indOld].move_to(newcmp, std::move(rm));  // move the chids to it

    return ct.back();
}

template<typename CC>
auto Graph<CC>::
split_component(Compt& cmp, Chain& m) -> Compt&
{
    const auto c = cmp.ind;  // reallocation of 'ct' may invalidate ref

    auto& newcmp = ct.emplace_back(num_compts(), cn);  // empty component

    ct[c].move_to(newcmp, m);  // move the chain m to it

    return ct.back();
}


template<typename CC>
auto Graph<CC>::
merge_components(
    const CmpId acceptor,
    const CmpId donor
) noexcept -> Compt&
{
    return merge_components(ct[acceptor], ct[donor]);
}


template<typename CC>
auto Graph<CC>::
merge_components(
    Compt& acceptor,
    Compt& donor
) noexcept -> Compt&
{
    const auto indDon = donor.ind;
    const auto indAcc = acceptor.ind;

    ASSERT(indDon != indAcc, "Merging component ", indAcc, " into itself.");

    if constexpr (verboseF)
        jot("Merging cmp into ", indAcc , " <-- ", indDon);

    const auto acceptor_is_last = (indAcc == ind_last_cmpt());

    donor.set_ind(indAcc);
    donor.reind_chains(acceptor.num_chains(), acceptor.num_edges());
    acceptor.vv.append(donor.vertices().get());
    acceptor.ww.insert(acceptor.ww.end(), donor.ww.begin(), donor.ww.end());
    donor.ww.clear();
    acceptor.set_chis();
    donor.clear_chis();
    acceptor.set_egl();
    donor.clear_egl();

    if (indDon != ind_last_cmpt()) {
        ct.back().set_ind(indDon);
        donor = std::move(ct.back());
    }
    ct.pop_back();

    auto& res = acceptor_is_last ? donor : acceptor;

    if constexpr (verboseF)
        acceptor_is_last ? jot("result of the component merge is donor")
                         : jot("result of the component merge is acceptor");

    if constexpr (verboseF)
        jot("Component merge produces ind ", res.ind, '\n',
            "Total number of components: ", num_compts());

    return res;
}

template<typename CC>
void Graph<CC>::
remove_edge(EgIdG ind)
{
    const auto& eg = edge(ind);
    [[maybe_unused]] const auto w = eg.w;
    [[maybe_unused]] const auto a = eg.indw;

    ct[eg.c].remove_edge(eg.indc);

    const auto indLast = numEdges - 1;
    if (ind < indLast) {
        const auto wLast = egl[indLast].w;
        const auto aLast =
            (wLast == w && egl[indLast].a > a) ? egl[indLast].a - 1
                                               : egl[indLast].a;
        auto& egLast = cn[wLast].g[aLast];
        egLast.ind = ind;
        ct[egLast.c].get_egl(egLast.indc).i = ind;
    }
    numEdges--;
    update();
}


template<typename CC>
constexpr
auto Graph<CC>::
ind_last_cmpt() const noexcept -> CmpId
{
    return ct.back().ind;
}


template<typename CC>
constexpr
auto Graph<CC>::
ind_last_chain() const noexcept -> ChIdG
{
    return cn.ind_last();
}


template<typename CC>
constexpr
void Graph<CC>::
rotate_chain(Chain& m, const std::size_t n)
{
    ct[m.c].rotate_chain(m, n);

    for (const auto& eg : m.g)
        egl[eg.ind].a = eg.indw;
}


template<typename CC>
constexpr
void Graph<CC>::
move_edges(
    Chain& fr,
    const std::size_t posFrBegin,
    const std::size_t posFrEnd,  // position after the last moved element
    Chain& to,
    const std::size_t posTo  // position of the first inserted element
)
{
    // Does not update chain connectivity and 'ngs'.
    // Does not remove source chain if it is emptied.

    ASSERT(fr.c == to.c,
            "Source chain componenet is different from that of the target");

    ct[fr.c].move_edges(fr, posFrBegin, posFrEnd, to, posTo);

    for (std::size_t i = posTo; i < to.length(); ++i) {
        const auto& tg = to.g[i];
        egl[tg.ind].set(to.idw, tg.indw);
    }
}


template<typename CC>
constexpr
auto Graph<CC>::
get_egl(const EgIdG ind) const noexcept -> const EdgeDescr&
{
    ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind];
}

template<typename CC>
constexpr
auto Graph<CC>::
get_egl(const EgIdG ind) noexcept -> EdgeDescr&
{
    ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind];
}

template<typename CC>
constexpr
auto Graph<CC>::
get_ved(const VIdG ind) const noexcept -> const VertexDescr&
{
    ASSERT(ind < ved.size(), "ind exceeds the number of vertices");

    return ved[ind()];
}

template<typename CC>
constexpr
auto Graph<CC>::
get_ved(const VIdG ind) noexcept -> VertexDescr&
{
    ASSERT(ind < ved.size(), "ind exceeds the number of vertices");

    return ved[ind()];
}

template<typename CC>
constexpr
auto Graph<CC>::
edge(const BulkSlot& s) noexcept -> Edge&
{
    return cn[s.w].g[s.a()];
}


template<typename CC>
constexpr
auto Graph<CC>::
edge(const EndSlot& s) noexcept -> Edge&
{
    return cn[s.w].end_edge(s.e);
}


template<typename CC>
constexpr
auto Graph<CC>::
edge(const EgIdG ind) noexcept -> Edge&
{
    return cn.edge(egl[ind]);
}


template<typename CC>
constexpr
auto Graph<CC>::
edge(const EgIdG ind) const noexcept -> const Edge&
{
    return cn.edge(egl[ind]);
}

template<typename CC>
constexpr
auto Graph<CC>::
chain(const EgIdG ind) noexcept -> Chain&
{
    return cn[egl[ind].w];
}

template<typename CC>
constexpr
auto Graph<CC>::
chain(const EndSlot& s) const noexcept -> const Chain&
{
    return cn[s.w];
}


template<typename CC>
constexpr
auto Graph<CC>::
chain(const EndSlot& s) noexcept -> Chain&
{
    return cn[s.w];
}


template<typename CC>
constexpr
auto Graph<CC>::
chid(const EgIdG ind) const noexcept -> ChIdG
{
    ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind].w;
}

template<typename CC>
constexpr
auto Graph<CC>::
chain(const EgIdG ind) const noexcept -> const Chain&
{
    ASSERT(ind < egl.size(), "ind exceeds the number of edges");
    return cn[egl[ind].w];
}

template<typename CC>
constexpr
auto Graph<CC>::
slot2a(const EndSlot& s) const noexcept -> EgIdA
{
    return cn[s.w].end2a(s.e);
}


template<typename CC>
constexpr
auto Graph<CC>::
slot2ind(const EndSlot& s) const noexcept -> EgIdG
{
    return cn[s.w].g[slot2a(s)].ind;
}


template<typename CC>
constexpr
auto Graph<CC>::
ind2bslot(const EgIdG ind) const noexcept -> BulkSlot
{
    return BulkSlot{egl[ind].w,
                    egl[ind].a};
}


template<typename CC>
constexpr
auto Graph<CC>::
edge_end(const EndSlot& s) const noexcept -> EndId
{
    return cn[s.w].edge_internal_end_at_end(s.e);
}


template<typename CC>
template<bool updateChis>
void Graph<CC>::
rename_chain(const ChIdG f, const ChIdG t)
{
    for (const auto e : End::Ids)
        copy_neigs(EndSlot{f, e}, EndSlot{t, e});

    cn[f].set_g_w(t);  // should be called before the move of g
    cn[t].g = std::move(cn[f].g);
    cn[f].g.clear();
    cn[t].c = cn[f].c;
    cn[t].idc = cn[f].idc;
    ct[cn[f].c].template rename_chain<updateChis>(f, t);
}


template<typename CC>
void Graph<CC>::
copy_neigs(
    const EndSlot& f,
    const EndSlot& t)
{
    ngs_at(t) = ngs_at(f);

    // Substitute f in f's neig's neigs for t:
    replace_slot_in_neigs(f, t);
}


template<typename CC>
void Graph<CC>::
remove_slot_from_neigs(const EndSlot& s)
{
    const auto sc = ngs_at(s);  // copy
    auto&      sr = ngs_at(s);  // ref

    for (auto& ne : sc()) {

        auto& ngs = ngs_at(ne);

        // Remove oldn from the neig list of its j-th neig
        const auto ngsOK = ngs.erase(s);

        ENSURE(ngsOK, "EndSlot {", s.w, " ", s.e, "} not found among",
                      "ngs of the slot {", ne.w, " ", ne.e, " connected to it");

        // Remove the j-th neig from the oldn's list of neigs
        const auto neOK = sr.erase(ne);

        ENSURE(neOK, "EndSlot {", ne.w, " ", ne.e, "} not found among",
                     "ngs of the slot {", s.w, " ", s.e, " connected to it");
    }
}


/// slots connected to 'nov' will be checked, and if there is 'old',
/// it will be replaced by 'nov'
template<typename CC>
void Graph<CC>::
replace_slot_in_neigs(
    const EndSlot& old,
    const EndSlot& nov)
{
    const auto& nov_ngs = ngs_at(nov);

    for (auto& ne : nov_ngs()) {

        auto& ne_ngs = ngs_at(ne);

        const auto ok = ne_ngs.replace(old, nov);

        ENSURE(ok, "EndSlot {", old.w, " ", old.e, "} not found among",
                    "ngs of the slot {", ne.w, " ", ne.e, " connected to it");
    }
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
update() noexcept
{
    update_edge_descriptors();
    if constexpr (useAgl)
        update_adjacency();
    if constexpr (withVertices)
        update_vertex_descriptors();
}


template<typename CC>
void Graph<CC>::
update_edge_descriptors() noexcept
{
    egl.set(*this);
}


template<typename CC>
void Graph<CC>::
update_vertex_descriptors() noexcept
{
    ved.set(*this);
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


template<typename CC>
constexpr
void Graph<CC>::
connect(const EndSlot& s1,
        const EndSlot& s2) noexcept
{
    cn[s1.w].ngs[s1.e].insert(s2);
    cn[s2.w].ngs[s2.e].insert(s1);
}


template<typename CC>
constexpr
auto Graph<CC>::
ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}


template<typename CC>
constexpr
auto Graph<CC>::
ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

template<typename CC>
constexpr
auto Graph<CC>::
ng_inds_at(const EndSlot& s) const noexcept -> EgIdGs
{
    const auto& ngs = ngs_at(s);

    EgIdGs inds(ngs.num());

    for (std::size_t i {}; i<ngs.num(); ++i)
        inds[i] = slot2ind(ngs[i]);

    return inds;
}

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
constexpr
auto Graph<CC>::
vertices(const Degree d) const noexcept -> VertexDescriptors
{
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
    ASSERT(vd.ind < ved.size(),
           "unexpected vd.i ", vd.ind, ": exceeds ved size ", ved.size());
    ASSERT(vd.ci < ct.size(),
           "unexpected vd.c ", vd.ci, ": exceeds ct size ", ct.size());

    return ct[vd.ci].vertex(VIdH{vd.idh});
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VertexDescr& vd) noexcept -> Vertex&
{
    ASSERT(vd.ind < ved.size(),
           "unexpected vd.i ", vd.ind, ": exceeds ved size ", ved.size());
    ASSERT(vd.ci < ct.size(),
           "unexpected vd.c ", vd.ci, ": exceeds ct size ", ct.size());

    return ct[vd.ci].vertex(VIdH{vd.idh});
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VIdG ind) const noexcept -> const Vertex&
{
    ASSERT(ind < ved.size(),
           "unexpected ind ", ind, ": exceeds ved size ", ved.size());

    return vertex(ved[ind]);
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VIdG ind) noexcept -> Vertex&
{
    ASSERT(ind < ved.size(),
           "unexpected ind ", ind, ": exceeds ved size ", ved.size());

    return vertex(ved[ind]);;
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VIdH h) const noexcept -> const Vertex*
{
    for (const auto& vd : ved)
        if (h == vd.h)
            return &vertex(vd);

    return nullptr;
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const VIdH h) noexcept -> Vertex*
{
    for (const auto& vd : ved)
        if (h == vd.h)
            return &vertex(vd);

    return nullptr;
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const EndSlot& s) const noexcept -> const Vertex*
{
    ASSERT(s.w < num_chains(), "EndSlot is not valid.");

    return cn[s.w].vertex_at_end(s.e);
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const EndSlot& s) noexcept -> Vertex*
{
    ASSERT(s.w < num_chains(), "EndSlot is not valid.");

    return cn[s.w].vertex_at_end(s.e);
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const BulkSlot& s) const noexcept -> const Vertex*
{
    ASSERT(s.w < num_chains(), "BulkSlot is not valid.");

    return cn[s.w].vertex_at_bulk(s.a());
}

template<typename CC>
constexpr
auto Graph<CC>::
vertex(const BulkSlot& s) noexcept -> Vertex*
{
    ASSERT(s.w < num_chains(), "BulkSlot is not valid.");

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


template<typename CC>
constexpr
bool Graph<CC>::
is_same_cycle(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept
{
    return cn[s1.w].is_cycle() && s1 == s2.opp();
}


template<typename CC>
auto Graph<CC>::
inds_to_chain_link(
    const EgIdG ind1,
    const EgIdG ind2
) const noexcept -> std::array<EndSlot, 2>
{
    const auto w = egl[ind1].w;

    for (const auto e : End::Ids) {
        if (cn[w].end2ind(e) == ind1) {
            auto s1 = EndSlot{w, e};
            for (const auto& s2 : cn[w].ngs[e]())
                if (cn[s2.w].end2ind(s2.e) == ind2)
                    for (const auto& n2 : cn[s2.w].ngs[s2.e]())
                        if (n2 == s1)
                            return {s1, s2};

        }
    }
    return {{}};
}


template<typename CC>
bool Graph<CC>::
is_equilibrated() const noexcept
{
    return true;  // stub
}


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


}  // namespace detail

template<typename CC> using Graph = detail::Graph<CC>;


///===================================================================

/*
    template<typename Tr>
    struct PropertySetter
    {
        explicit constexpr PropertySetter(Graph& gr)
            : gr {gr}
        {}

        void start(Tr::Res& res)
        {}

        void end(Tr::Res& res)
        {}

    protected:

        Graph& gr;
    };

    PropertySetter<> on_transform {*this};
*/
}  // namespace graffine::structure::components

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_H
