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
 * \file component.h
 * \brief Contains class template defining connected components of a graph.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_H

#include "graffine/definitions.h"
#include "graffine/structure/containers/chain_indexes.h"
#include "graffine/structure/containers/chains.h"
#include "graffine/structure/containers/edges_in_component.h"
#include "graffine/structure/containers/triangles.h"
#include "graffine/structure/containers/vertices.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/common.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/analyzers/adjacency.h"
#include "graffine/structure/modifiers/junction_shifter.h"
#include "graffine/structure/analyzers/connectivity.h"
// #include "graffine/structure/elements/facet.h"
#include "graffine/structure/descriptors/edge_in_component.h"
#include "graffine/structure/descriptors/edge_in_vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <algorithm> // remove, ranges::sort
#include <array>
#include <deque>
#include <numeric> // iota
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace graffine::structure {
namespace detail {

template <typename Ch,
          typename... Props>
struct Component
{};

/**
 * Connected component of a graph.
 * \tparam Chain Type of the Chain forming the graph.
 */
template <typename Ch>
struct Component<Ch>
{
    using Chain = Ch;
    using Chains = structure::containers::Chains<Chain>;
    using Edge = Chain::Edge;
    using Edges = Chain::Edges;
    using End = descriptors::End;
    using EndId = End::Id;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using EdgeDescr = descriptors::EdgeInComponent<Edge>;
    using EdgeDescriptors = containers::EdgeDescriptorsInComponent<Component>;
    using Vertex = Edge::Vertex;
    using Vertices = containers::Vertices<Vertex, Component>;
    using EdgeInVertex = Vertex::EgDescr;
    using Trinangles = structure::Triangles;
    using Chis = containers::ChainIndexes<EndSlot>;

    static constexpr auto Type = Elements::Component;
    static constexpr bool isMulticomponent{};
    static constexpr bool isSpatial{};

    friend struct containers::Vertices<Vertex, Component>;
    friend struct Adjacency<Component>;

public: // Variables

    /// Identifier of this component: its index in the hosting graph container
    CmpId ind{CmpId::undefined};

    ChIdGs ww;   ///< Graph-wide ids of the chains hosted by this component.
    Vertices vv; ///< Containter holdingertices of this connected component.

    /// Chain indexes classified by degrees of their boundary vertices.
    Chis chis;

    Adjacency<Component> adj;

    modifiers::JunctionShifter<Component> jShifter;

    Triangles trs;

protected:  // Variables

    Chains& cn; ///< Reference to the parent graph container holding chains.

    EdgeDescriptors egl; ///< Edge descriptors.

public: // Functions

    explicit constexpr Component(
        CmpId ind,
        Chains& cn);

    Component() = delete;
    constexpr Component(const Component& other) = delete;
    constexpr Component(Component&& other);
    constexpr auto operator=(const Component& other) -> Component& = delete;
    constexpr auto operator=(Component&& other) -> Component&;
    ~Component() = default;

    constexpr bool operator==(const Component& other);

    constexpr void set_ind(CmpId i) noexcept;
    constexpr CmpId get_id() const noexcept { return ind; };

    void set_chains() noexcept;
    void set_chain(
        Chain& m,
        ChIdC idc,
        EgIdC& indc  // ref
    ) noexcept;

    void reind_chains(ChIdC idc,
                      EgIdC indc) noexcept;

    template <bool updateChis = true>
    void rename_chain(ChIdG f, ChIdG t) noexcept;

    /**
     * Change component index in internal records of all edges
     * keeping the chain index unaltered.
     * \details Change connected graph component-related indexes of the
     * chain edges, keeping the chain index unoltered.
     */
    void set_edges() noexcept;
    void set_edges(
        Chain& m,
        EgIdC& indc // ref
    ) noexcept;
    void set_vertices() noexcept;

    constexpr void set_egl() noexcept { egl.set(*this); };
    constexpr auto get_egl() const noexcept -> const EdgeDescriptors & { return egl; };
    constexpr auto get_egl() noexcept -> EdgeDescriptors && { return std::move(egl); };
    constexpr auto get_egl(const EgIdC i) const noexcept -> const EdgeDescr&  { return egl[i]; };
    constexpr auto get_egl(const EgIdC i) noexcept -> EdgeDescr&  { return egl[i]; };
    constexpr void append(EdgeDescriptors &&other) noexcept { egl.append(std::move(other)); };

    void clear_egl() noexcept { egl.clear(); };
    void update_edge_descriptors() noexcept;

    auto set_chis() noexcept -> Component&;
    auto get_chis() noexcept -> Chis&&;
    void clear_chis() noexcept { chis.clear(); }

    void clear();

    //    constexpr void detect_facets() const noexcept;
    constexpr void detect_triangles() const noexcept;

    constexpr auto egId_last() const noexcept -> EgIdC;
    constexpr auto num_edges() const noexcept -> std::size_t;
    constexpr auto num_chains() const noexcept -> std::size_t;
    //    constexpr auto num_facets() const noexcept -> std::size_t;
    constexpr auto num_triangles() const noexcept -> std::size_t;

    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;
    constexpr auto num_vertices() const noexcept -> std::size_t;

    constexpr bool is_single_unconnected_chain() const noexcept;
    constexpr bool is_unconnected_cycle() const noexcept;

    constexpr bool is_complete() const noexcept;

    constexpr auto max_degree() const noexcept -> Degree;

    constexpr auto create_chain() noexcept -> Chain&;
    constexpr auto create_chain(
        std::size_t len,
        EgIdG& indIni // ref
    ) noexcept -> Chain&;

    /**
     * Removes an edge having component ind \p indc .
     * \details Removes an edge at chain coordinate a = egl[indc].a while
     * shifting original g[a+1], g[a+2], ... backwards. These then become
     * g[a], g[a+1], ... .
     * \note Does not update global edge indices.
     * \note Does not remove the host chain even if the removed edge was the
     * last one.
     * \param[in] indc Component_wide index of the edge to be removed.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not the at the chain head, \a nullptr otherwise.
     */
    template <bool preserveTopology = true>
    auto remove_edge(const EgIdC indc) -> Edge*;

    /**
     * Removes an edge at position \p a .
     * \details Removes an edge g[a] while shifting original
     * g[a+1], g[a+2], ... backwards. These then become g[a], g[a+1], ... .
     * \note Does not update global edge indices.
     * \note Does not remove the host chain even if the removed edge was the
     * last one.
     * \param[in] w Graph-wide id of the chain to get processed.
     * \param[in] a Position of the removed edge in the original chain.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not the at the chain head, \a nullptr otherwise.
     */
    template <bool preserveTopology = true>
    auto remove_edge(ChIdG w, EgIdA a) -> Edge*;

    /**
     * Removes an edge at position \p a .
     * \details Removes an edge g[a] while shifting original
     * g[a+1], g[a+2], ... backwards. These then become g[a], g[a+1], ... .
     * \note Does not update global edge indices.
     * \note Does not remove the host chain even if the removed edge was the
     * last one.
     * \param[in] m Chain to get processed.
     * \param[in] a Position of the removed edge in the original chain.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not the at the chain head, \a nullptr otherwise.
     */
    template <bool preserveTopology = true>
    auto remove_edge(Chain& m, EgIdA a) -> Edge*;

    /**
     * Inserts an edge at g[a].
     * \note Original edges g[a], g[a+1], ... are shifted forwards,
     * becoming g[a+1], g[a+2], ....
     * Insertion is not limited to the chain internal:
     *   if a == 0, prepends the new edge at end A,
     *   if a == length(), appends the new edge at end B.
     * \param[in] e Edge object to be inserted.
     * \param[in] w Graph-wide id of the chain to get processed.
     * \param[in] a Position the inserted edge will have in the chain.
     * \return Pointer to the newly inserted edge.
     */
    auto insert_edge_into_chain(Edge&& e, ChIdG w, EgIdA a) -> Edge&;
    auto insert_edge_into_chain(Edge&& e, Chain& m, EgIdA a) -> Edge&;

    /**
     * Inserts an edge at g[a].
     * \note Original edges g[a], g[a+1], ... are shifted forwards,
     * becoming g[a+1], g[a+2], ....
     * \note insertion is limited to the chain internal: 0 < a < m.length()
     * \param[in] e Edge object to be inserted.
     * \param[in] w Graph-wide id of the chain to get processed.
     * \param[in] a Position the inserted edge will have in the chain.
     * \return Pointer to the newly inserted edge.
     */
    auto insert_edge_inside_chain(Edge&& e, ChIdG w, EgIdA a) -> Edge&;
    auto insert_edge_inside_chain(Edge&& e, Chain& m, EgIdA a) -> Edge&;

    auto insert_first_edge_into_chain(Edge&& eg, ChIdG w) -> Edge&;
    auto insert_first_edge_into_chain(Edge&& eg, Chain& m) -> Edge&;

    /**
     * Appends an edge at the chain head, i.e. at the back of g.
     * \param[in] e Edge object to be appended.
     * \param[in] w Graph-wide id of the chain to get processed.
     * \return Reference to the appended edge.
     */
    auto append_edge_to_chain(Edge&& e, ChIdG w) -> Edge&;
    auto append_edge_to_chain(Edge&& e, Chain& m) -> Edge&;

    auto prepend_edge_to_chain(Edge&& e, ChIdG w) -> Edge&;
    auto prepend_edge_to_chain(Edge&& e, Chain& m) -> Edge&;

    constexpr void rotate_chain(Chain& m, const std::size_t n);

    constexpr bool contains_chain(ChIdG w) const noexcept;
    constexpr bool contains_edge(EgIdG ei) const noexcept;
    constexpr bool contains_vertex(VIdH vi) const noexcept;
    constexpr bool contains(const Chain& m) const noexcept;
    constexpr bool contains(const Edge& eg) const noexcept;
    constexpr bool contains(const Vertex& v) const noexcept;
    constexpr auto find(const Vertex& v) const noexcept -> Vertex* ;
    constexpr auto find_vertex(const VIdH vi) const noexcept -> Vertex* ;

    void append(Component&& other);
    void append(Component& other);
    void append(const ChIdGs& vv);
    void append(Chains& mm);
    void append(Chain& m);
    void append(Edges &g);
    void append(Edge& eg);

    void remove(const Edges &g);
    void remove(const Edge& eg);
    void remove(const Chain& m);
    void remove(const Chains& mm);
    void remove(const ChIdGs& vv);
    void remove_disconnected(Vertex* vp);
    void remove_vertex(const VIdH vi);
    constexpr void remove_vertex_at_edge_outer(Edge& eg, EndId e) noexcept;

    constexpr auto rem_unconnected_cycle_boindary_vetrex() noexcept
        -> std::array<Edge*, 2>;
    constexpr auto rem_chain_internal_vetrex(ChIdG w, EgIdA a) noexcept
        -> std::array<Edge*, 2>;

    /**
     * Removes \p s from list of \a neigs in slots connected to it.
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
        const EndSlot& nov);

    /**
     * Reconnect edges from vA to vB.
     * \details Disconnect edges connected to vertex \p f and
     * connect them to vertex \p t
     */
    void reconnect_edges(Vertex* f, Vertex* t);

    void move_to(Component& c, ChIdGs&& vv);
    void move_to(Component& c, Chains& mm);
    void move_to(Component& c, Chain& m);

    /**
     * \note Does not update edge connectivity.
     *       Does not update chain connectivity and 'ngs'.
     *       Does not remove source chain if it is emptied.
     */
    constexpr void move_edges(
        Chain& fr,
        const std::size_t posFrBegin,
        const std::size_t posFrEnd, // position after the last moved element
        Chain& to,
        const std::size_t posTo // position of the first inserted element
    );

    constexpr auto vertices() const noexcept -> const Vertices&;
    constexpr auto vertices()       noexcept ->       Vertices&;
    constexpr auto vertex(VIdH vi) const noexcept -> const Vertex&;
    constexpr auto vertex(VIdH vi)       noexcept ->       Vertex&;

    constexpr auto common_vertex(const EgIdC e1,
                                 const EgIdC e2) noexcept -> Vertex*;
    constexpr auto edge_vertices(EgIdC indc) const noexcept -> const Edge::Vertices&;
    constexpr auto edge_vids(EgIdC indc) const noexcept -> Edge::VIDs;

    constexpr auto edges() const noexcept -> std::vector<const Edge&>;
    constexpr auto edge(EgIdC indc) const noexcept -> const Edge&;
    constexpr auto edge(EgIdC indc)       noexcept ->       Edge&;
    constexpr auto edge(const EndSlot& s) const noexcept -> const Edge&;
    constexpr auto edge(const EndSlot& s)       noexcept ->       Edge&;
    constexpr auto edge(const BulkSlot& s) const noexcept -> const Edge&;
    constexpr auto edge(const BulkSlot& s)       noexcept ->       Edge&;
    constexpr auto edge(EgIdG i) const noexcept -> const Edge&;

    constexpr auto chains() const noexcept -> std::vector<const Chain&>;
    constexpr auto chain(ChIdC w) const noexcept -> const Chain&;
    constexpr auto chain(ChIdC w)       noexcept ->       Chain&;
    constexpr auto chain(ChIdG w) const noexcept -> const Chain&;
    constexpr auto chain(ChIdG w)       noexcept ->       Chain&;
    constexpr auto chid(ChIdC idc) const noexcept -> ChIdG;

    constexpr auto chids() const noexcept -> const ChIdGs& { return ww; }
    constexpr auto chids()       noexcept ->       ChIdGs& { return ww; }

    constexpr auto bulk_slot(const EdgeInVertex& ed) const noexcept -> BulkSlot;
    constexpr auto end_slot(const EdgeInVertex& ed) const noexcept -> EndSlot;

    constexpr auto ind2indc(EgIdG ind) const noexcept -> EgIdC;

    constexpr auto ngs_of(const Vertex& v) const noexcept -> std::vector<Vertex*>;
    constexpr auto ngs_of(const Edge& eg, EndId eInner) const noexcept -> std::vector<Edge*>;
    constexpr auto ngs_of(const Chain& m, EndId e) const noexcept -> std::vector<Chain*>;

    constexpr auto ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&;
    constexpr auto ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&;

    constexpr bool ww_is_sorted() const noexcept;

    /**
     * Shift an edge from f.e end of f.w to t.e end of t.w
     * \note Provides functionality similar to the one implemented by
     * functor core::modifiers::JunctionShifter.
     */
    void shift_chain_junction(const EndSlot& f,
                              const EndSlot& t);

    /**
     * Checks that \p cond is satisfied; if it is not, terminates the program.
     * \details Prints out the component using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] tag Prepend the data to be printed with it if \p cond is false.
     * \param[in] message Arguments bound to form the message printed at
     *                    termination if the condition does not hold.
     */
    template <typename... T>
    void ensure(
        bool cond,
        std::string_view tag,
        T &&...message) const;

    /**
     * \name Internal tests.
     * \return \a nullptr or \a false iff all is ok
     * @{
     */

    template<bool beLoud = true>
    auto check_connectivity_vetrex_edge() const -> Vertex* ;

    template<bool beLoud = true>
    bool check_vertex_data() const;

    /// Ensure corectness of chain indices component-wise
    template<bool beLoud = true>
    bool check_chain_idc() const;

    /// Ensure corectness of edge indices component-wise
    template<bool beLoud = true>
    bool check_edge_indc() const;

    /// Ensure corectness of edge desctiptors.
    template<bool beLoud = true>
    bool check_egl() const;

    /// Ensure corectness of chain classification.
    template<bool beLoud = true>
    bool check_chis() const;

    /// Collection of test of this component internal structures and indices.
    template<bool beLoud = true>
    bool check(std::string_view s = "") const;

    ///@}  // Internal tests.

    /**
     * \name Printing.
     * @{
     */

    template <bool isCycle>
    void print_classification(const ChIdGs& accessible,
                              const ChIdGs& blocked) const;

    template <typename... Args>
    void print(Args... args) const;

    template <typename... Args>
    void print_chains(Args... args) const;

    template <typename... Args>
    void print_vertices(Args&&... args) const;

    void print_vertices_deg(const Degree d, std::string_view s = " ") const;

    void print_egl() const noexcept;

    template <bool with_top = true>
    void print_ww() const noexcept;

    ///@}  // Printing.
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template <typename Ch>
constexpr Component<Ch>::
Component(
    const CmpId ind,
    Chains& cn
)
    : ind{ind}
    , vv{}
    , adj{*this}
    , cn{cn}
{}

template <typename Ch>
constexpr Component<Ch>::
Component(Component&& other)
    : ind{other.ind}
    , ww{std::move(other.ww)}
    , vv{std::move(other.vv)}
    , chis{std::move(other.chis)}
    , adj{std::move(other.adj)}
    , cn{other.cn}
    , egl{std::move(other.egl)}
{}

template <typename Ch>
constexpr auto Component<Ch>::
operator=(Component&& other) -> Component&
{
    ind = other.ind;
    ww = std::move(other.ww);
    vv = std::move(other.vv);
    chis = std::move(other.chis);
    cn = other.cn;
    egl = std::move(other.egl);

    return *this;
}

template <typename Ch>
constexpr bool Component<Ch>::
operator==(const Component& other)
{
    if (num_chains() != other.num_chains())
        return false;

    if (num_chains() > 1) {
        std::sort(ww.begin(), ww.end());
        std::sort(other.ww.begin(), other.ww.end());
    }

    return ww == other.ww;
}

template <typename Ch>
constexpr void Component<Ch>::
set_ind(const CmpId i) noexcept
{
    ind = i;

    for (const auto& w : ww) {
        cn[w].c = i;
        for (auto& g : cn[w].g)
            g.c = i;
    }
    vv.set_c(i);
}

template <typename Ch>
void Component<Ch>::
clear()
{
    egl.clear();
    ww.clear();
    vv.clear();
    chis.clear();
}

template <typename Ch>
constexpr auto Component<Ch>::
egId_last() const noexcept -> EgIdC
{
    return num_edges() - 1;
}

template <typename Ch>
constexpr auto Component<Ch>::
num_edges() const noexcept -> std::size_t
{
    return egl.size();
}

template <typename Ch>
constexpr auto Component<Ch>::
num_chains() const noexcept -> std::size_t
{
    return ww.size();
}

template <typename Ch>
constexpr auto Component<Ch>::
num_triangles() const noexcept -> std::size_t
{
    return trs.num();
}

template <typename Ch>
constexpr auto Component<Ch>::
num_vertices(const Degree d) const noexcept -> std::size_t
{
    return vv.num(d);
}

template <typename Ch>
constexpr auto Component<Ch>::
num_vertices() const noexcept -> std::size_t
{
    return vv.num();
}

/*
template<typename Ch>
constexpr
auto Component<Ch>::
num_facets() const noexcept -> FId
{
    return static_cast<FId>(facets.size());
}
*/

template <typename Ch>
constexpr bool Component<Ch>::
is_complete() const noexcept
{
    const auto n = num_vertices();

    return num_edges() == n * (n - 1) / 2;
}

template <typename Ch>
constexpr bool Component<Ch>::
is_single_unconnected_chain() const noexcept
{
    return chis.num({1, 1});
}

template <typename Ch>
constexpr bool Component<Ch>::
is_unconnected_cycle() const noexcept
{
    return chis.num({2, 2});
}

// Reconnect edges from vA to vB
template <typename Ch>
void Component<Ch>::
reconnect_edges(Vertex* f, Vertex* t)
{
    auto& ss = f->edges();
    while (ss.size()) {

        auto s = ss.begin();
        auto& eg = edge(s->w);
        eg.disconnect_end_inner(s->e);
        eg.connect_to_inner(s->e, *t);
    }
}

// Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(const ChIdG w, // id of the chain modified by the edge removal
            const EgIdA a) -> Edge*
{
    return remove_edge(cn[w], a);
}

// Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(const EgIdC indc) -> Edge*
{
    const auto& gl = egl[indc];

    return remove_edge(cn[gl.w], gl.a);
}

// Removes edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(Chain& m, // chain modified by the edge removal
            const EgIdA a) -> Edge*
{
    auto& g = m.g;
    auto& h = g[a]; // edge to be removed

    ASSERT_CALLING(a < m.length(),
                   m.print("FAIL"),
                   "Erasing an edge at ", a, " is beyond chain length.");
    ASSERT(m.length(), "Erasing an edge in empty chain.");

    // If the chain is to get empty after removal, it needs to be intentional.
     ASSERT( //! preserveTopology ||
        m.length() > 1,
        "Erasing an edge in single-edge chain.");

    auto vA = h.vertex_at_outer(End::A);
    auto vB = h.vertex_at_outer(End::B); // to be preserved

    ASSERT(!preserveTopology ||
           is_implemented_degree(vA->deg() + vB->deg() - Deg2),
           "Erasing edge produces vertex above max implemented degre.");

    h.disconnect_end_outer(End::A);
    h.disconnect_end_outer(End::B);

    // reconnect edges from vA to vB
    if (m.is_head(a)) {
                                 // vB is at chain end B
        reconnect_edges(vA, vB); // vA to be removed, vB to be preserved
        vv.remove_disconnected(vA);
    }
    else {

        reconnect_edges(vB, vA); // vB to be removed, vA to be preserved
        vv.remove_disconnected(vB);
    }

    if (h.indc < egId_last()) {

        auto& last = edge(egId_last());
        vv.reind_edge(last, h.indc);
        last.indc = h.indc;
    }

    g.erase(g.begin() + a());

    for (EgIdA i {a}; i < m.length(); ++i)
        g[i].indw = i;

    set_egl();

    return a < m.length() ? &g[a] : nullptr;
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is not limited to the chain internal:
//   if a == 0, prepends the new edge at end A,
//   if a == length(), appends the new edge at end B.
template <typename Ch>
auto Component<Ch>::
insert_edge_into_chain(
    Edge&& eg,
    const ChIdG w,
    const EgIdA a) -> Edge&
{
    return insert_edge_into_chain(std::move(eg), cn[w], a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is not limited to the chain internal:
//   if a == 0, prepends the new edge at end A,
//   if a == length(), appends the new edge at end B.
template <typename Ch>
auto Component<Ch>::
insert_edge_into_chain(
    Edge&& eg,
    Chain& m,
    const EgIdA a) -> Edge&
{
    ASSERT_CALLING(!eg.is_connected(), eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    ASSERT_CALLING(!contains(eg), eg.print("FAIL"),
                   "Appending edge which is already included");
    ASSERT_CALLING(a <= m.length(), m.print("FAIL"),
                   "Inserting edge to pos ", a, " is beyond chain length");
    ASSERT(m.length() || (!m.ngs[End::A].num() && !m.ngs[End::B].num()),
           "Edge cannot be insetred into an empty connected chain");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    if (a == m.length())
        return append_edge_to_chain(std::move(eg), m);

    if (a == 0)
        return prepend_edge_to_chain(std::move(eg), m);

    // else: insert inbetween other edges of chain 'm'
    return insert_edge_inside_chain(std::move(eg), m, a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is limited to the chain internal: 0 < a < m.length()
template <typename Ch>
auto Component<Ch>::
insert_edge_inside_chain(
    Edge&& eg,
    const ChIdG w,
    const EgIdA a) -> Edge&
{
    return insert_edge_inside_chain(std::move(eg), cn[w], a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is limited to the chain internal: 0 < a < m.length()
template <typename Ch>
auto Component<Ch>::
insert_edge_inside_chain(
    Edge&& eg,
    Chain& m,
    const EgIdA a) -> Edge&
{
    ASSERT_CALLING(!eg.is_connected(), eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    ASSERT_CALLING(!contains(eg), eg.print("FAIL"),
                   "Appending edge which is already included");
    ASSERT_CALLING(a > 0, m.print("FAIL"),
                   "Inserting edge at chain tail");
    ASSERT_CALLING(a < m.length(), m.print("FAIL"),
                   "Inserting edge to pos ", a, " is beyond chain length");
    ASSERT(m.length(), "Inserting edge inside an empty chain");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    eg.w = m.idw;
    eg.set_cmp(ind, num_edges());
    auto& g = m.g;
    auto& p = *g.insert(g.begin() + a(), std::move(eg));
    for (EgIdA i{a}; i < m.length(); ++i)
        g[i].indw = i;
    set_egl();

    auto& egA = g[a - 1];

    // 'eg' will be connected at its end B to the vertex to which egA is
    // originally connected at its end B.
    //  A new BULK vertex will be created to join end B egA to end A of 'eg'.

    auto gvB = egA.vertex_at_outer(End::B);
    ASSERT(gvB, "end B of ", egA.indw, " is not connected.");
    egA.disconnect_end_outer(End::B);
    p.connect_to_outer(End::B, *gvB);

    auto& v = vv.create_vertex(ind);
    egA.connect_to_outer(End::B, v);
    p.connect_to_outer(End::A, v);

    return p;
}

// Insert first edge into an empty chain. The chain must be disconnected.
template <typename Ch>
auto Component<Ch>::
insert_first_edge_into_chain(
    Edge&& eg,
    const ChIdG w) -> Edge&
{
    return insert_first_edge_into_chain(std::move(eg), cn[w]);
}

// Insert first edge into an empty chain. The chain must be disconnected.
template <typename Ch>
auto Component<Ch>::
insert_first_edge_into_chain(Edge&& eg,
                             Chain& m) -> Edge&
{
    ASSERT_CALLING(!eg.is_connected(),
                   eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    ASSERT_CALLING(!contains(eg),
                   eg.print("FAIL"),
                   "Appending edge which is already included");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    ASSERT_CALLING(!m.length(),
                   m.print("FAIL"),
                   "Inserting 1st edge into chain which is not empty.");
    ASSERT_CALLING(!m.is_connected(),
                   m.print("FAIL"),
                   "Chain is connected.");

    eg.w = m.idw;
    eg.indw = 0;
    eg.set_cmp(ind, num_edges());
    egl.append(eg);

    m.g.push_back(std::move(eg));
    auto& p = m.g.back();

    auto& v1 = vv.create_vertex(ind);
    auto& v2 = vv.create_vertex(ind);

    p.connect_to_outer(End::A, v1);
    p.connect_to_outer(End::B, v2);

    return p;
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
append_edge_to_chain(Edge&& eg,
                     const ChIdG w) -> Edge&
{
    return append_edge_to_chain(std::move(eg), cn[w]);
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
append_edge_to_chain(Edge&& eg,
                     Chain& m) -> Edge&
{

    ASSERT_CALLING(!eg.is_connected(),
                   eg.print("FAIL"),
                   "Passing a connected edge for appending.");
    ASSERT_CALLING(!contains(eg),
                   eg.print("FAIL"),
                   "Appending edge which is already included");
    warn_if(eg.indc.is_defined(), "indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    eg.w = m.idw;
    eg.indw = m.length();
    eg.set_cmp(ind, num_edges());
    egl.append(eg);

    m.g.push_back(std::move(eg));
    auto& p = m.g.back();

    auto& egA = m.g[p.indw - 1]; // edge preceding 'p' in chain

    // 'eg' will be connected at its end B to the vertex to which egA is
    // originally connected at its end B.
    // A new BULK vertex will be created to join end B egA to end A of 'eg'.

    auto gvB = egA.vertex_at_outer(End::B);
    ASSERT(gvB, "end B of ", egA.indw, " is not connected.");
    egA.disconnect_end_outer(End::B);
    p.connect_to_outer(End::B, *gvB);

    auto& v = vv.create_vertex(ind);
    egA.connect_to_outer(End::B, v);
    p.connect_to_outer(End::A, v);

    return p;
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
prepend_edge_to_chain(Edge&& eg,
                      const ChIdG w) -> Edge&
{
    return prepend_edge_to_chain(std::move(eg), cn[w]);
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
prepend_edge_to_chain(Edge&& eg,
                      Chain& m) -> Edge&
{
    ASSERT(!eg.is_connected(), "Passing a connected edge for appending.");
    warn_if(eg.indc.is_defined(), "indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    eg.w = m.idw;
    eg.indw = 0;
    eg.set_cmp(ind, num_edges());

    auto& p = *m.g.insert(m.g.begin(), std::move(eg));
    for (EgIdA i{1}; i < m.length(); ++i)
        m.g[i].indw = i;
    set_egl();

    auto& egB = m.g[1]; // edge following 'p' in chain

    // 'eg' will be connected at its end A to the vertex to which egA is
    // originally connected at its end A.
    // A new BULK vertex will be created to join end A of egB to end B of 'eg'.

    auto gvA = egB.vertex_at_outer(End::A);
    ASSERT(gvA, "end A of ", egB.indw, " is not connected.");
    egB.disconnect_end_outer(End::A);
    p.connect_to_outer(End::A, *gvA);

    auto& v = vv.create_vertex(ind);
    egB.connect_to_outer(End::A, v);
    p.connect_to_outer(End::B, v);

    return p;
}

// Create an empty chain and insert it into the component.
// Result:  an unconnected chain as a part of the component..
template <typename Ch>
constexpr auto Component<Ch>::
create_chain() noexcept -> Chain&
{
    cn.emplace_back(cn.num(), num_chains(), ind);
    auto& m = cn.back();

    ASSERT(!contains(m), "Appending chain which is already included");
    ww.push_back(m.idw);

    chis.include(m);

    return m;
}

// Create a chain of specific length and insert it into an empty component.
// Result: a chain as part of a single-chain component.
template <typename Ch>
constexpr auto Component<Ch>::
create_chain(
    const std::size_t len,
    EgIdG& indIni // ref
) noexcept -> Chain&
{
    ASSERT(!num_chains(), "Component is not empty");

    auto& m = create_chain();

    for (std::size_t i{}; i < len; ++i)
        append_edge_to_chain(Edge{indIni++}, m.idw);

    return m;
}

template <typename Ch>
constexpr void Component<Ch>::
rotate_chain(Chain& m, const std::size_t n)
{
    m.rotate(n);
    for (const auto& eg : m.g)
        egl[eg.indc].a = eg.indw;
}

template <typename Ch>
constexpr void Component<Ch>::
move_edges(
    Chain& fr,
    const std::size_t posFrBegin,
    const std::size_t posFrEnd, // position after the last moved element
    Chain& to,
    const std::size_t posTo  // position of the first inserted element
)
{
    // Does not update edge connectivity.
    // Does not update chain connectivity and 'ngs'.
    // Does not remove source chain if it is emptied.

    ASSERT(fr.c == to.c,
           "Source chain componenet is different from that of the target");

    to.g.insert(to.g.begin() + posTo,
                std::make_move_iterator(fr.g.begin() + posFrBegin),
                std::make_move_iterator(fr.g.begin() + posFrEnd));
    fr.g.erase(fr.g.begin() + posFrBegin,
               fr.g.begin() + posFrEnd);

    for (std::size_t i{posFrBegin}; i < fr.length(); ++i) {

        fr.g[i].indw = i;
        egl[fr.g[i].indc].a = i;
    }
    for (EgIdA i{posTo}; auto& eg : to.g) {

        eg.indw = i;
        eg.w = to.idw;
        egl[eg.indc].a = i;
        egl[eg.indc].w = to.idw;
        ++i;
    }
}

template <typename Ch>
constexpr void Component<Ch>::
detect_triangles() const noexcept
{
    using Tiple = std::array<ChIdG, 3>;
    std::set<Tiple> eee{};

    for (const auto w : chis.cn_33()) {

        const auto& m = cn[w];
        for (const auto& nAm : m.ngs[End::A]()) {

            const auto& mA = cn[nAm.w];
            for (const auto& nBm : m.ngs[End::B]()) {

                const auto& mB = cn[nBm.w];
                for (const auto& nnA : mA.ngs[End::opp(nAm.e)]())
                    if (mB.ngs[End::opp(nBm.e)].contains(nnA))
                        eee.insert({w, nAm.w, nBm.w});
            }
        }
    }
}

template <typename Ch>
constexpr auto Component<Ch>::
edge_vertices(const EgIdC indc) const noexcept -> const Edge::Vertices&
{
    return edge(indc).vertices();
}

template <typename Ch>
constexpr auto Component<Ch>::
edge_vids(const EgIdC indc) const noexcept -> Edge::VIDs
{
    return edge(indc).vids();
}

template <typename Ch>
void Component<Ch>::
set_edges() noexcept
{
    EgIdC indc{};
    for (const auto& w : ww)
        for (auto& eg : cn[w].g) {

            vv.reind_edge(eg, indc);
            eg.set_cmp(ind, indc++);
        }
}

template <typename Ch>
void Component<Ch>::
set_edges(
    Chain& m,
    EgIdC& indc // ref
) noexcept
{
    for (auto& eg : m.g) {

        vv.reind_edge(eg, indc);
        eg.set_cmp(ind, indc++);
    }
}

template <typename Ch>
void Component<Ch>::
set_chain(
    Chain& m,
    const ChIdC idc,
    EgIdC& indc // ref
) noexcept
{
    m.set_cmpt(ind, idc);
    set_edges(m, indc);
}

template <typename Ch>
void Component<Ch>::
set_chains() noexcept
{
    EgIdC indc{};
    ChIdC idc{};
    for (const auto& w : ww)
        set_chain(cn[w], idc++, indc);
}

template <typename Ch>
void Component<Ch>::
reind_chains(ChIdC idc,
             EgIdC indc) noexcept
{
    for (const auto& w : ww)
        set_chain(cn[w], idc++, indc);
}

template <typename Ch>
template <bool updateChis>
void Component<Ch>::
rename_chain(const ChIdG f,
             const ChIdG t) noexcept
{
    if (std::find(ww.begin(), ww.end(), f) != ww.end())
        std::replace(ww.begin(), ww.end(), f, t);

    std::for_each(egl.begin(), egl.end(),
                  [&](EdgeDescr& g)
                  { if (g.w == f) g.w = t; });

    if constexpr (updateChis)
        chis.populate(cn, ww);
}

template <typename Ch>
constexpr bool Component<Ch>::
contains_chain(const ChIdG w) const noexcept
{
    return std::find(ww.begin(), ww.end(), w) != ww.end();
}

template <typename Ch>
constexpr bool Component<Ch>::
contains(const Chain& m) const noexcept
{
    return contains_chain(m.idw);
}

template <typename Ch>
constexpr bool Component<Ch>::
contains(const Vertex& v) const noexcept
{
    return vv.find(v) != nullptr;
}

template <typename Ch>
constexpr bool Component<Ch>::
contains_vertex(const VIdH vi) const noexcept
{
    return vv.find(vi) != nullptr;
}

template <typename Ch>
constexpr bool Component<Ch>::
contains_edge(const EgIdG ei) const noexcept
{
    return std::find_if(
               egl.begin(),
               egl.end(),
               [&](const EdgeDescr& i)
               { return i.i == ei; }) != egl.end();
}

template <typename Ch>
constexpr auto Component<Ch>::
find_vertex(const VIdH vi) const noexcept -> Vertex*
{
    return vv.find(vi);
}

template <typename Ch>
constexpr auto Component<Ch>::
find(const Vertex& v) const noexcept -> Vertex*
{
    return vv.find(v);
}

template <typename Ch>
constexpr bool Component<Ch>::
contains(const Edge& eg) const noexcept
{
    return contains_edge(eg.ind);
}

template <typename Ch>
constexpr auto Component<Ch>::
vertices() const noexcept -> const Vertices&
{
    return vv;
}

template <typename Ch>
constexpr auto Component<Ch>::
vertices() noexcept -> Vertices&
{
    return vv;
}

template <typename Ch>
constexpr auto Component<Ch>::
vertex(const VIdH vi) const noexcept -> const Vertex&
{
    const auto vp = vv.find(vi);

    ASSERT(vp, "Accessing non-existing vertex idh ", vi, " in component ", ind);

    return *vp;
}

template <typename Ch>
constexpr auto Component<Ch>::
vertex(const VIdH vi) noexcept -> Vertex&
{
    const auto vp = vv.find(vi);

    ASSERT(vp, "Accessing non-existing vertex idh ", vi, " in component ", ind);

    return *vp;
}

template <typename Ch>
constexpr auto Component<Ch>::
common_vertex(const EgIdC e1,
              const EgIdC e2) noexcept -> Vertex*
{
    ASSERT(e1 < num_edges(),
           "Accessing non-existing edge idc ", e1, " in component ", ind);
    ASSERT(e2 < num_edges(),
           "Accessing non-existing edge idc ", e2, " in component ", ind);
    ASSERT(e1 != e2, "Equal edge ids");

    const auto& v1 = edge_vertices(e1);
    const auto& v2 = edge_vertices(e2);

    // for distinct edges at most a single common vertex is possible
    if (v1[0]->operator==(*v2[0]) || v1[0]->operator==(*v2[1])) return v1[0];
    if (v1[1]->operator==(*v2[0]) || v1[1]->operator==(*v2[1])) return v1[1];

    return nullptr;
}

template <typename Ch>
constexpr auto Component<Ch>::
max_degree() const noexcept -> Degree
{
    auto d = Deg0;
    for (const auto& w : ww) {

        const auto md = cn[w].max_degree();
        if (md > d)
            d = md;
    }
    return d;
}

template <typename Ch>
constexpr auto Component<Ch>::
rem_unconnected_cycle_boindary_vetrex() noexcept
    -> std::array<Edge*, 2>
{
    const auto w = ww[0];

    ASSERT(num_chains() == 1,
           "component ", ind, " is supposed to hold un unconnected cycle");
    ASSERT(cn[w].is_unconnected_cycle(),
           "chain ", w, " is supposed to be un unconnected cycle");

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    auto& eg1 = cn[w].end_edge(eA);
    auto& eg2 = cn[w].end_edge(eB);

    auto v = eg1.vertex_at_outer(eA);

    eg1.disconnect_end_outer(eA);
    eg2.disconnect_end_outer(eB);

    vv.remove_disconnected(v);

    return {&eg1, &eg2};
}

template <typename Ch>
constexpr auto Component<Ch>::
rem_chain_internal_vetrex(const ChIdG w,
                          const EgIdA a) noexcept -> std::array<Edge*, 2>
{
    ASSERT(contains_chain(w), "chain ", w, " is not part of copmonent ", ind);

    auto& m = cn[w];

    ASSERT(m.length() > 1, "Chain does not contain bulk vertices.");
    ASSERT(a > 0, "Not a bulk vertex at a: ", a, " in chain ", w);
    ASSERT(a < m.length(),
           "indw ", a, " not valid for chain ", w, " of length ", m.length());

    auto& eg1 = m.g[a - 1];
    auto& eg2 = m.g[a];

    auto v = eg1.vertex_at_outer(End::B);

    eg1.disconnect_end_outer(End::B);
    eg2.disconnect_end_outer(End::A);

    vv.remove_disconnected(v);

    return {&eg1, &eg2};
}

template <typename Ch>
void Component<Ch>::
append(Component&& other)
{
    EgIdC indc{num_edges()};
    ChIdC idc{num_chains()};

    ASSERT(other.ind != ind, "Appending identical component");

    for (const auto w : other.ww)
        set_chain(cn[w], idc++, indc);

    vv.append(other);

    std::move(other.egl.begin(), other.egl.end(), std::back_inserter(egl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}

template <typename Ch>
void Component<Ch>::
append(Component& other)
{
    EgIdC indc{num_edges()};
    ChIdC idc{num_chains()};

    ASSERT(other.ind != ind, "appending identical component");

    for (const auto w : other.ww)
        set_chain(cn[w], idc++, indc);

    vv.append(other);

    std::move(other.egl.begin(), other.egl.end(), std::back_inserter(egl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}

template <typename Ch>
void Component<Ch>::
append(const ChIdGs& vv)
{
    for (const auto& v : vv)
        append(cn[v]);
}

template <typename Ch>
void Component<Ch>::
append(Chains& mm)
{
    for (auto& m : mm)
        append(m);
}

template <typename Ch>
void Component<Ch>::
append(Chain& m)
{
    if constexpr (verboseF)
        jot("appending chain ", m.idw);

    EgIdC indc{num_edges()};
    const ChIdC idc{num_chains()};

    ASSERT(!contains(m), "appending chain which is already included");

    set_chain(m, idc, indc);

    for (const auto& g : m.g)
        egl.append(g);

    ww.push_back(m.idw);
    chis.include(m);
}

template <typename Ch>
void Component<Ch>::
append(Edges &g)
{
    for (auto& eg : g)
        append(eg);
}

template <typename Ch>
void Component<Ch>::
append(Edge& eg)
{
    ASSERT(!contains(eg), "appending edge which is already included");

    eg.set_cmp(ind, num_edges());
    set_egl();
}

template <typename Ch>
void Component<Ch>::
remove(const Edges &g)
{
    for (const auto& eg : g)
        remove(eg);
}

// Component-related data inside \p eg are not to be altered, hence const ref.
template <typename Ch>
void Component<Ch>::
remove(const Edge& eg)
{
    if constexpr (verboseF)
        jot("-removing edge ", eg.ind, " of chain ", eg.w, " cmp ", ind, ":");

    ASSERT(contains(eg),
           "attempt to remove a missing edge ", eg.ind, " from cmpt ", ind);
    ASSERT(cn[eg.w].length(), "removing an edge from empty chain ", eg.w);

    egl.remove(eg, cn, vv);
}

template <typename Ch>
void Component<Ch>::
remove(const Chain& m)
{
    if constexpr (verboseF)
        jot("removing chain ", m.idw, " from component ", ind);

    ASSERT(contains(m),
           "attempt to remove a missing chain ", m.idw, " from cmpt ", ind);

    Edges g = m.g; // copy

    // sort in descending order of indc
    std::ranges::sort(g, [](const Edge& a,
                            const Edge& b) { return a.indc > b.indc; });

    for (const auto& eg : g)
        remove(eg);

    if (const auto last = num_chains() - 1;
        m.idc < last)

    for (const auto& w : ww)
            if (cn[w].idc == last)
                cn[w].idc = m.idc;

    std::erase(ww, m.idw);
    set_edges();
    chis.populate(cn, ww);
    set_egl();
}

template <typename Ch>
void Component<Ch>::
remove(const Chains& mm)
{
    Edges g;
    for (const auto m : mm) // copy
        g.append_range(std::move(m.g));

    // sort in descending order of indc
    std::ranges::sort(g, [](Edge& a, Edge& b)
                      { return a.indc > b.indc; });

    ASSERT(std::adjacent_find(g.begin(), g.end()) == g.end(),
           "edges having identical indc found");

    for (const auto& eg : g)
        remove(eg);

    for (const auto& m : mm) {

        if (const auto last = num_chains() - 1;
            m.idc < last)

            for (const auto w : ww)
                if (cn[w].idc == last)
                    cn[w].idc = m.idc;

        std::erase(ww, m.idw);
        m.is_unconnected_cycle()
            ? chis.populate(cn, ww)
            : chis.remove(m);
    }
}

template <typename Ch>
void Component<Ch>::
remove(const ChIdGs& vv)
{
    for (const auto v : vv)
        remove(cn[v]);
}

template <typename Ch>
void Component<Ch>::
remove_disconnected(Vertex* vp)
{
    vv.remove_disconnected(vp); // vp is set to nullptr inside
}

template <typename Ch>
constexpr void Component<Ch>::
remove_vertex_at_edge_outer(
    Edge& eg,
    const EndId e) noexcept
{
    vv.remove_connected_to_outer(eg, e);
}

template <typename Ch>
void Component<Ch>::
    remove_vertex(const VIdH vi)
{
    vv.remove(vi, *this);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, Chain& m)
{
    vv.move_to(c, m);
    remove(m);
    c.append(m);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, Chains& mm)
{
    remove(mm);
    c.append(mm);
}

template <typename Ch>
void Component<Ch>::
move_to(Component& c, ChIdGs&& uu)
{
    std::sort(uu.rbegin(), uu.rend());
    for (const auto& u : uu) {

        if constexpr (verboseF)
            jot("Moving ", u);

        move_to(c, cn[u]);
    }
}

template <typename Ch>
auto Component<Ch>::
set_chis() noexcept -> Component&
{
    chis.populate(cn, ww);

    return *this;
}

template <typename Ch>
auto Component<Ch>::
get_chis() noexcept -> Chis&&
{
    return std::move(chis);
}

template <typename Ch>
void Component<Ch>::
set_vertices() noexcept
{
    vv.populate(*this);
}

template <typename Ch>
void Component<Ch>::
update_edge_descriptors() noexcept
{
    egl.clear();
    ww.clear();

    for (const auto& m : cn)
        if (m.c == ind) {

            ww.push_back(m.idw);
            [[maybe_unused]] EgIdC i{};
            for (const auto& g : m.g) {

                ASSERT(i++ == g.indc,
                       "i =", i, " != g.indc = ", g.indc, " in chain ", m.idw);

                egl.emplace_back(g.w, g.indw, g.ind);
            }
        }
}

template <typename Ch>
constexpr bool Component<Ch>::
ww_is_sorted() const noexcept
{
    const auto n = num_chains();

    if (n > 1) {

        std::vector<ChIdC> h(n);
        std::iota(h.begin(), h.end(), ChIdC{});

        return std::all_of(h.begin(), h.end(),
                           [&](ChIdC i)
                           { return cn[ww[i()]].idc == i; });
    }
    else
        return true;
}

template <typename Ch>
constexpr auto Component<Ch>::
edges() const noexcept -> std::vector<const Edge&>
{
    std::vector<const Edge&> g;
    for (const auto& h : egl)
        g.push_back(cn[h.w].g[h.a]);

    return g;
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EgIdC indc) const noexcept -> const Edge&
{
    ASSERT(indc < num_edges(),
           "Incorrect indc ", indc,
           " for component ", ind, " having ", num_edges(), " edges");

    const auto& h = egl[indc];

    return cn[h.w].g[h.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EgIdC indc) noexcept -> Edge&
{
    ASSERT(indc < num_edges(),
           "Incorrect indc ", indc,
           " for component ", ind, " having ", num_edges(), " edges");

    const auto& h = egl[indc];

    return cn[h.w].g[h.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EndSlot& s) const noexcept -> const Edge&
{
    return cn[s.w].end_edge(s.e);
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EndSlot& s) noexcept -> Edge&
{
    return cn[s.w].end_edge(s.e);
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const BulkSlot& s) const noexcept -> const Edge&
{
    return cn[s.w].g[s.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const BulkSlot& s) noexcept -> Edge&
{
    return cn[s.w].g[s.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EgIdG i) const noexcept -> const Edge&
{
    EdgeDescr h;
    for (const auto& g : egl)
        if (g.i == i) {
            h = g;
            break;
        }

    ASSERT(h.is_defined(),
           "Error: Edge ind", i, " is not part of component ", ind);

    return &cn[h.w].g[h.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
chains() const noexcept -> std::vector<const Chain&>
{
    std::vector<const Chain&> mm;
    for (const auto w : ww)
        mm.push_back(cn[w]);

    return mm;
}

template <typename Ch>
constexpr auto Component<Ch>::
chain(const ChIdC ic) const noexcept -> const Chain&
{
    for (const auto w : ww)
        if (cn[w].indc == ic)
            return cn[w];

    return nullChain<Edge>;
}

template <typename Ch>
constexpr auto Component<Ch>::
chain(const ChIdC ic) noexcept -> Chain&
{
    for (const auto w : ww)
if (cn[w].indc == ic)
            return cn[w];

    return nullChain<Edge>;
}

template <typename Ch>
constexpr auto Component<Ch>::
chain(const ChIdG w) const noexcept -> const Chain&
{
    ASSERT(std::find(ww.begin(), ww.end(), w) != ww.end(),
           "Chain ", w, " is not part of component ", ind);

    return cn[w];
}

template <typename Ch>
constexpr auto Component<Ch>::
chain(const ChIdG w) noexcept -> Chain&
{
    ASSERT(std::find(ww.begin(), ww.end(), w) != ww.end(),
           "Chain ", w, " is not part of component ", ind);

    return cn[w];
}

template <typename Ch>
constexpr auto Component<Ch>::
chid(const ChIdC idc) const noexcept -> ChIdG
{
    for (const auto& w : ww)
        if (cn[w].idc == idc)
            return w;

    return ChIdG::undefined;
}

template <typename Ch>
constexpr
auto Component<Ch>::
bulk_slot(const EdgeInVertex& ed) const noexcept -> BulkSlot
{
    return egl[ed.indc()].bulk_slot().next();
}

template <typename Ch>
constexpr
auto Component<Ch>::
end_slot(const EdgeInVertex& ed) const noexcept -> EndSlot
{
    return EndSlot{egl[ed.indc()].w,
                   edge(ed.indc()).outer_endId(ed.e)};
}

template <typename Ch>
constexpr auto Component<Ch>::
ind2indc(EgIdG ind) const noexcept -> EgIdC
{
    for (EgIdC i{}; i < egl.size(); ++i)
        if (egl[i].i == ind)
            return i;

    return EgIdC::undefined;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Edge& eg,
      const EndId eInner) const noexcept -> std::vector<Edge*>
{
    ASSERT_CALLING(edges().contains(eg), eg.print("ERROR"),
                   "Vertex is not part of component ", ind);

    std::vector<Edge*> gg;
    const auto v = eg.vertex_at_inner(eInner);
    auto vedEg = typename Vertex::EgDescr{eg.id, eg.indc, eInner};
    for (const auto& vedNg : v->edges_connected_to(vedEg)) {

        const auto& ed = egl[vedNg.indc];
            gg.push_back(&cn[ed.w].g[ed.a]);
    }

    return gg;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Vertex& v) const noexcept -> std::vector<Vertex*>
{
    ASSERT_CALLING(vv.contains(v), v.print("ERROR"),
                   "Vertex is not part of component ", ind);

    std::vector<Vertex*> vv;
    for (const auto& edv : v.edges())
        vv.push_back(edge(egl[edv.w]).vertex_at_inner(edv.e));

    return vv;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_of(const Chain& m, const EndId e) const noexcept -> std::vector<Chain*>
{
    ASSERT_CALLING(std::find(ww.begin(), ww.end(), m.idw) != ww.end(),
                   m.print("ERROR"), "Vertex is not part of component ", ind);

    std::vector<Chain*> mm;
    for (const auto& n : m.ngs[e])
        mm.push_back(&cn[n.w]);

    return mm;
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

template <typename Ch>
constexpr auto Component<Ch>::
ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}

/// slots connected to 'nov' will be checked, and if there is 'old',
/// it will be replaced by 'nov'
template <typename Ch>
void Component<Ch>::
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

template <typename Ch>
void Component<Ch>::
remove_slot_from_neigs(const EndSlot& s)
{
    const auto sc = ngs_at(s); // copy
    auto& sr = ngs_at(s);      // ref

    for (auto& ne : sc()) {

        auto& ngs = ngs_at(ne);

        // Remove oldn from the neig list of its j-th neig
        const auto ngsOK = ngs.remove(s);

        ENSURE(ngsOK, "EndSlot {", s.w, " ", s.e, "} not found among",
               "ngs of the slot {", ne.w, " ", ne.e, " connected to it");

        // Remove the j-th neig from the oldn's list of neigs
        const auto neOK = sr.remove(ne);

        ENSURE(neOK, "EndSlot {", ne.w, " ", ne.e, "} not found among",
               "ngs of the slot {", s.w, " ", s.e, " connected to it");
    }
}

// Shift an edge from f.e end of f.w to t.e end of t.w
template<typename Ch>
void Component<Ch>::
shift_chain_junction(const EndSlot& f,
                     const EndSlot& t)
{
    auto& g0 = cn[f.w].g;
    auto& g1 = cn[t.w].g;

    ASSERT(cn[f.w].c == cn[t.w].c, "Slots belong to differrent components");

    if (f.e == End::B) {

        if (t.e == End::A) g1.insert(g1.begin(), std::move(g0.back()));
        else                g1.push_back(std::move(g0.back()));
        g0.pop_back();
    }
    else {  // ef == 1

        if (t.e == End::A) g1.insert(g1.begin(), std::move(g0.front()));
        else                g1.push_back(std::move(g0.front()));
        g0.erase(g0.begin());
    }

    cn[f.w].set_g_w();
    cn[t.w].set_g_w();
    set_egl();
}

template <typename Ch>
template <typename... T>
void Component<Ch>::
ensure(
    bool cond,
    const std::string_view tag,
    T&&... message) const
{
    if (!cond) {

        print(tag);

        ABORT(cond, message...);
    }
}

// -----------------------------------------------------------------------------
// INTERNAL TESTS
// -----------------------------------------------------------------------------

template <typename Ch>
template<bool beLoud>
auto Component<Ch>::
check_connectivity_vetrex_edge() const -> Vertex*
{
    if constexpr (beLoud)
        jot<false>("connectivity_vetrex_edge ... ");

    for (const auto& v : vv.get())
        for (const auto& s : v->edges()) {

            ASSERT(s.w < num_edges(),
                   "EgDescr.w", s.w, " >= num_edges", num_edges());
            ASSERT(End::is_valid(s.e), "End is not valid: ", s.e);

            if (edge(s.w).vertex_at_inner(s.e) != v.get()) {

                jot<false>("Connection is not valid for vertex ");
                v->print("");
                jot<false>("and edge at inner end ", End::str(s.e),
                           "           ");
                edge(s.w).print("");
                edge(s.w).vertex_at_inner(s.e)->print("found");

                return v.get();
            }
        }
    return nullptr;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_chain_idc() const
{
    // Check continuity of idc values.

    if constexpr (beLoud)
        jot<false>("chain_idc ... ");

    std::vector<ChIdC> idcs(num_chains());
    std::iota(idcs.begin(), idcs.end(), 0);

    for (const auto& i : idcs) {

        [[maybe_unused]] bool found{};
        for (const auto& w : ww)
            if (cn[w].idc == i) {

                found = true;
                break;
            }

        ASSERT(found, "idc ", i, " not found in component ", ind);
    }

    // Check max idc.

    ChIdC maxval{};
    for (const auto& w : ww)
        if (cn[w].idc > maxval)
            maxval = cn[w].idc;

    ASSERT(!num_chains() || maxval < num_chains(),
           "Max idc ", maxval, " >= num_chains ", num_chains());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_edge_indc() const
{
    // Check continuity of indc values.

    if constexpr (beLoud)
        jot<false>("edge_indc ... ");

    std::vector<ChIdC> idcs(num_edges());
    std::iota(idcs.begin(), idcs.end(), 0);

    for (const auto& i : idcs) {

        bool found{};
        for (const auto& w : ww)
            for (const auto& eg : cn[w].g)
                if (eg.indc() == i()) {

                    found = true;
                    break;
                }
        ENSURE(found, "indc ", i, " not found in component ", ind);
    }

    // Check max indc.

    ChIdC maxval{};
    for (const auto& w : ww)
        for (const auto& eg : cn[w].g)
            if (eg.indc() > maxval())
                maxval = cn[w].idc;

    ENSURE(!num_edges() || maxval < num_edges(),
           "Max indc ", maxval, " >= num_edges ", num_edges());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_egl() const
{
    if constexpr (beLoud)
        jot<false>("egl ... ");

    ASSERT(egl.size() == num_edges(),
           "egl size is incorrect for component ", ind);

    for (EgIdC k{}; k < egl.size(); ++k)
        egl[k].check(edge(k), ind, k);

    for (const auto& w : ww)
        for (const auto& eg : cn[w].g) {

            ASSERT(egl.size() > eg.indc(),
                   "egl size", egl.size(), " is <= for component ", eg.indc);

            egl[eg.indc].check(eg, ind, eg.indc);
        }

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_chis() const
{
    if constexpr (beLoud)
        jot<false>("chis ... ");

    abort_if(chis.num_chains() != num_chains(), "Incorrect number of chains: ",
             chis.num_chains(), " != ", num_chains());

    if (!num_chains())
        return false;

    warn_if(chis.num({1, 1}) > 1,
            "Found multiple unconnected linear chains in cmp ", ind);

    if (chis.num({1, 1})) {
        [[maybe_unused]] const auto u = chis.cn_11()[0];
        // allow several unconnected chains to handle irregular components
        ASSERT(cn[u].is_unconnected_linear(),
               "Error 11 for w ", u);
    }

    warn_if(chis.num({2, 2}) > 1,
            "Found multiple unconnected cycle chains in cmp ", ind);

    if (chis.num({2, 2})) {
        [[maybe_unused]] const auto u = chis.cn_22()[0];
        // allow several unconnected chains to handle irregular components
        ASSERT(cn[u].is_unconnected_cycle(),
               "Error 22 for w ", u);
    }
    for ([[maybe_unused]] const auto& u : chis.cn_33())
        ASSERT(cn[u].ngs[End::A].num() == 2 && cn[u].ngs[End::B].num() == 2,
               "Error 33 for w ", u);

    for ([[maybe_unused]] const auto& u : chis.cn_44())
        ASSERT(cn[u].ngs[End::A].num() == 3 && cn[u].ngs[End::B].num() == 3,
               "Error 44 for w ", u);

    for ([[maybe_unused]] const auto& u : chis.cn_13())
        ASSERT(!cn[u.w].ngs[u.e].num() && cn[u.w].ngs[End::opp(u.e)].num() == 2,
               "Error 13 for w ", u.str_short());

    for ([[maybe_unused]] const auto& u : chis.cn_14())
        ASSERT(!cn[u.w].ngs[u.e].num() && cn[u.w].ngs[End::opp(u.e)].num() == 3,
               "Error 14 for w ", u.str_short());

    for ([[maybe_unused]] const auto& u : chis.cn_34())
        ASSERT(cn[u.w].ngs[u.e].num() == 2 &&
                   cn[u.w].ngs[End::opp(u.e)].num() == 3,
               "Error 34 for w ", u.str_short());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_vertex_data() const
{
    if constexpr (beLoud)
        jot<false>("vertex_c ... ");

    ASSERT(!vv.check_c(ind));
    ASSERT(!check_connectivity_vetrex_edge<beLoud>());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check(const std::string_view s) const
{
    if constexpr (beLoud)
        jot<false>("Checking component ", ind, " ", s, ": ");

    if (int err = check_chain_idc<beLoud>(); err)
        return err;
    if (int err = check_edge_indc<beLoud>(); err)
        return err;
    if (int err = check_egl<beLoud>(); err)
        return err;
    if (int err = check_chis<beLoud>(); err)
        return err;
    if (int err = check_vertex_data<beLoud>(); err)
        return err;

    if constexpr (beLoud)
        jot("Success!");

    return false;
}

// -----------------------------------------------------------------------------
// PRINTING
// -----------------------------------------------------------------------------

template <typename Ch>
template <bool isCycle>
void Component<Ch>::
print_classification(const ChIdGs& accessible,
                     const ChIdGs& blocked) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_chains(), " chains");

    ChIdGs all(num_chains(), ChIdG::undefined);
    for (const auto& a : accessible) {

    const std::size_t i = std::distance(ww.begin(),
                                            std::find(ww.begin(), ww.end(), a));
         ASSERT(i < ww.size(), "element of 'accessible' is not in component");
        all[i] = a;
    }
    constexpr auto colorAccessible = colorcodes::GREEN;
    constexpr auto colorBlocked = colorcodes::RED;
    jot<false>(colorcodes::CYAN, "all: ", colorcodes::RESET);
    for (std::size_t j{}; j < all.size(); ++j)
        jot<false>(all[j].is_defined() ? colorAccessible
                                       : colorBlocked,
                   ww[j],
                   colorcodes::RESET,
                   j == all.size() - 1 ? "" : ", ");
    jot("");

    for (int i{}; auto& v : {accessible, blocked}) {

        jot<false>(colorcodes::CYAN, i == 0 ? "accessible: " : "blocked: ",
                   colorcodes::RESET);
        for (std::size_t j{}; auto w : v)
            jot<false>(i == 0 ? colorAccessible : colorBlocked, w,
                       ++j == v.size() ? "" : ", ");
        jot(colorcodes::RESET);
        ++i;
    }
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print(Args... args) const
{
    print_chains(args...);
    print_vertices("");
    print_ww<false>();
    print_egl();
    chis.print();
    jot("");
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print_chains(Args... args) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_edges(), " edges in ", num_chains(), " chains");

    for (const auto& j : ww)
        cn[j].print(args...);
}

template <typename Ch>
template <typename... Args>
void Component<Ch>::
print_vertices(Args&&... args) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_vertices(), " vertices");

    vv.print(args...);
}

template <typename Ch>
void Component<Ch>::
print_vertices_deg(
    const Degree d,
    const std::string_view s) const
{
    jot(colorcodes::YELLOW, "Component ",
        colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
        num_vertices(), " vertices of degree ", d);

    vv.print_degree(d, s);
}

template <typename Ch>
void Component<Ch>::
print_egl() const noexcept
{
    jot("egl ", ind, ": w a ind id");
    for (std::size_t i {}; i < egl.size(); ++i)
        egl[i].print(i);
}

template <typename Ch>
template <bool with_top>
void Component<Ch>::
print_ww() const noexcept
{
    if constexpr (with_top)
        jot<false>("cmpt ", ind, " ");
    jot<false>("ww: ");
    for (const auto& w : ww)
        jot<false>(w, " ");

    jot("");
}

    }  // namespace detail

template <typename Ch>
using Component = detail::Component<Ch>;

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_H
