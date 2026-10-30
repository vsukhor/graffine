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
 * \tparam Ch Type of the Chain forming the graph.
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
    using Triangles = structure::Triangles;
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
    Vertices vv; ///< Container holding vertices of this connected component.

    /// Chain indexes classified by degrees of their boundary vertices.
    Chis chis;

    Adjacency<Component> adj;

    modifiers::JunctionShifter<Component> jShifter;

    /// Chain triangles of the component, as found by the last call to
    /// detect_chain_triangles().
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
    constexpr Component(Component&& other) noexcept;
    constexpr auto operator=(const Component& other) -> Component& = delete;
    constexpr auto operator=(Component&& other) noexcept -> Component&;
    ~Component() = default;

    constexpr bool operator==(const Component& other);

    constexpr void set_ind(CmpId i) noexcept;
    constexpr CmpId get_id() const noexcept { return ind; };

    void set_chains() noexcept;
    /**
     * Assigns chain \p m to this component.
     * \details Sets the component index and the component-wide chain index
     * \p idc of \p m, and numbers its edges consecutively from \p indc .
     * \param[in] m Chain to be assigned.
     * \param[in] idc Component-wide index given to \p m .
     * \param[inout] indc Component-wide index given to the first edge of
     * \p m ; on return, the one following its last edge.
     */
    void set_chain(
        Chain& m,
        ChIdC idc,
        EgIdC& indc  // ref
    ) noexcept;

    /**
     * Renumbers the chains of this component and their edges.
     * \details As set_chain() for each chain of \a ww in turn: the chains are
     * numbered from \p idc and the edges from \p indc . Used to append the
     * chains to another component's numbering.
     */
    void reind_chains(ChIdC idc,
                      EgIdC indc) noexcept;

    template <bool updateChis = true>
    void rename_chain(ChIdG f, ChIdG t);

    /**
     * Change component index in internal records of all edges
     * keeping the chain index unaltered.
     * \details Change connected graph component-related indexes of the
     * chain edges, keeping the chain index unaltered.
     */
    void set_edges() noexcept;
    void set_edges(
        Chain& m,
        EgIdC& indc // ref
    ) noexcept;
    void set_vertices() noexcept;

    constexpr void set_egl() { egl.set(*this); };
    constexpr auto get_egl() const noexcept -> const EdgeDescriptors & { return egl; };
    constexpr auto get_egl() noexcept -> EdgeDescriptors & { return egl; };
    constexpr auto get_egl(const EgIdC i) const noexcept -> const EdgeDescr&  { return egl[i]; };
    constexpr auto get_egl(const EgIdC i) noexcept -> EdgeDescr&  { return egl[i]; };
    constexpr void append(EdgeDescriptors &&other) noexcept { egl.append(std::move(other)); };

    void clear_egl() noexcept { egl.clear(); };
    void update_edge_descriptors() noexcept;

    auto set_chis() -> Component&;
    /// Moves the chain classification out of this component, leaving
    /// \a chis in a moved-from state.
    auto get_chis() noexcept -> Chis&&;
    void clear_chis() noexcept { chis.clear(); }

    void clear();

    //    constexpr void detect_facets() const noexcept;
    /**
     * Finds the chain triangles of the component and stores them in \a trs.
     * \details A chain triangle is formed by three distinct chains, none a
     * cycle, joined end to end into a loop through three distinct junction
     * vertices. Each triangle is stored once, with its chains and vertices
     * in ascending order of their ids.
     * \note \a trs is not kept up to date by the transformations: call this
     * function again after changing the component.
     */
    void detect_chain_triangles();

    constexpr auto egId_last() const noexcept -> EgIdC;
    constexpr auto num_edges() const noexcept -> std::size_t;
    constexpr auto num_chains() const noexcept -> std::size_t;
    //    constexpr auto num_facets() const noexcept -> std::size_t;
    /// Number of chain triangles in \a trs, see detect_chain_triangles().
    constexpr auto num_triangles() const noexcept -> std::size_t;

    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;
    constexpr auto num_vertices() const noexcept -> std::size_t;

    /// Whether the component holds an unconnected linear chain, i.e. a chain
    /// classified by end degrees {1, 1}.
    constexpr bool is_single_unconnected_chain() const;
    /// Whether the component holds an unconnected cycle, i.e. a chain
    /// classified by end degrees {2, 2}.
    constexpr bool is_unconnected_cycle() const;

    /// Whether the component is a complete graph: an edge links every pair
    /// of its vertices.
    constexpr bool is_complete() const noexcept;

    constexpr auto max_degree() const noexcept -> Degree;

    constexpr auto create_chain() -> Chain&;
    constexpr auto create_chain(
        std::size_t len,
        EgIdG& indIni // ref
    ) -> Chain&;

    /**
     * Removes an edge having component ind \p indc .
     * \details Removes an edge at chain coordinate a = egl[indc].a while
     * shifting original g[a+1], g[a+2], ... backwards. These then become
     * g[a], g[a+1], ... .
     * The two vertices of the removed edge are merged into one, and the
     * last edge of the component takes over its component-wide index.
     * \note Does not update global edge indices.
     * \note The host chain must have more than one edge (asserted).
     * \tparam preserveTopology Whether to assert that the merged vertex has
     * an implemented degree.
     * \param[in] indc Component-wide index of the edge to be removed.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not at the chain head, \a nullptr otherwise.
     */
    template <bool preserveTopology = true>
    auto remove_edge(const EgIdC indc) -> Edge*;

    /**
     * Removes an edge at position \p a .
     * \details Removes an edge g[a] while shifting original
     * g[a+1], g[a+2], ... backwards. These then become g[a], g[a+1], ... .
     * The two vertices of the removed edge are merged into one, and the
     * last edge of the component takes over its component-wide index.
     * \note Does not update global edge indices.
     * \note The host chain must have more than one edge (asserted).
     * \tparam preserveTopology Whether to assert that the merged vertex has
     * an implemented degree.
     * \param[in] w Graph-wide id of the chain to get processed.
     * \param[in] a Position of the removed edge in the original chain.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not at the chain head, \a nullptr otherwise.
     */
    template <bool preserveTopology = true>
    auto remove_edge(ChIdG w, EgIdA a) -> Edge*;

    /**
     * Removes an edge at position \p a .
     * \details Removes an edge g[a] while shifting original
     * g[a+1], g[a+2], ... backwards. These then become g[a], g[a+1], ... .
     * The two vertices of the removed edge are merged into one, and the
     * last edge of the component takes over its component-wide index.
     * \note Does not update global edge indices.
     * \note The host chain must have more than one edge (asserted).
     * \tparam preserveTopology Whether to assert that the merged vertex has
     * an implemented degree.
     * \param[in] m Chain to get processed.
     * \param[in] a Position of the removed edge in the original chain.
     * \return Pointer to the edge g[a] after the shift if the removed edge
     * was not at the chain head, \a nullptr otherwise.
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
     * \return Reference to the newly inserted edge.
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
     * \return Reference to the newly inserted edge.
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

    /**
     * Rotates the edges of chain \p m by \p n positions, as Chain::rotate(),
     * and updates their positions in the component-wide edge descriptors.
     * \note Meaningful for cycles, whose ends are at the same vertex.
     */
    constexpr void rotate_chain(Chain& m, const std::size_t n);

    constexpr bool contains_chain(ChIdG w) const noexcept;
    constexpr bool contains_edge(EgIdG ei) const noexcept;
    constexpr bool contains_vertex(VIdH vi) const noexcept;
    constexpr bool contains(const Chain& m) const noexcept;
    constexpr bool contains(const Edge& eg) const noexcept;
    constexpr bool contains(const Vertex& v) const noexcept;
    constexpr auto find(const Vertex& v) const noexcept -> Vertex* ;
    constexpr auto find_vertex(const VIdH vi) const noexcept -> Vertex* ;

    /**
     * \name Appending to this component.
     * \details The chains appended are numbered after those already in the
     * component, and their edges after its edges. The vertices are moved only
     * when appending a whole component.
     * @{
     */

    /// Appends the chains, edges and vertices of component \p other, leaving
    /// it empty. \p other must be a different component.
    void append(Component&& other);
    /// \overload
    void append(Component& other);
    /// Appends the chains \p vv, without their vertices.
    void append(const ChIdGs& vv);
    /// Appends the chains \p mm, without their vertices.
    void append(Chains& mm);
    /// Appends chain \p m, without its vertices: it must not be part of this
    /// component.
    void append(Chain& m);
    /// As append(Edge&), for each edge of \p g .
    void append(Edges &g);
    /// Assigns edge \p eg to this component, with the next component-wide
    /// index, and rebuilds the edge descriptors.
    void append(Edge& eg);

    ///@}  // Appending to this component.

    /**
     * \name Removal from the records of this component.
     * \details The elements are removed from the component-wide records only:
     * the chains and edges stay in the graph, and the vertices in the
     * component. Elements following the removed ones in these records are
     * renumbered: e.g. the last edge takes over the component-wide index of
     * a removed edge.
     * @{
     */

    /// As remove(const Edge&), for each edge of \p g .
    void remove(const Edges &g);
    /// Removes edge \p eg from the component-wide edge descriptors.
    void remove(const Edge& eg);
    /// Removes chain \p m and its edges from the records of this component.
    void remove(const Chain& m);
    /// Removes chains \p mm and their edges from the records of this
    /// component.
    void remove(const Chains& mm);
    /// Removes chains \p vv and their edges from the records of this
    /// component.
    void remove(const ChIdGs& vv);
    ///@}  // Removal from the records of this component.

    /// Deletes vertex \p vp, which must have no edges connected.
    void remove_disconnected(Vertex* vp);
    /// Disconnects vertex \p vi from its edges and deletes it, leaving the
    /// edge ends free. The chain connectivity (Chain::ngs) is not updated.
    void remove_vertex(const VIdH vi);
    /// Disconnects outer end \p e of edge \p eg and deletes the vertex it
    /// was connected to, which must have no other edges.
    constexpr void remove_vertex_at_edge_outer(Edge& eg, EndId e) noexcept;

    /**
     * Deletes the boundary vertex of the unconnected cycle forming this
     * component.
     * \details Disconnects the edges at both chain ends from the vertex, then
     * deletes it, so that the chain ends become free. The chain connectivity
     * (Chain::ngs) is not updated.
     * \note The component must consist of a single unconnected cycle.
     * \return The edges at chain ends A and B, in this order.
     */
    constexpr auto rem_unconnected_cycle_boundary_vertex() noexcept
        -> std::array<Edge*, 2>;
    /**
     * Deletes internal vertex \p a of chain \p w .
     * \details Disconnects edges g[a-1] and g[a] from the vertex they share
     * (BulkSlot {w, a}), then deletes it, leaving the two edge ends free.
     * The chain itself is not split.
     * \note Requires 0 < \p a < length().
     * \return Edges g[a-1] and g[a], in this order.
     */
    constexpr auto rem_chain_internal_vertex(ChIdG w, EgIdA a) noexcept
        -> std::array<Edge*, 2>;

    /**
     * Removes \p s from list of \a neigs in slots connected to it.
     */
    void remove_slot_from_neigs(const EndSlot& s);

    /**
     * Replaces \p old with \p nov in the neighbour lists of the slots
     * connected to \p nov .
     * \details To be called once \p nov holds the neighbours of \p old,
     * e.g. after copy_neigs(): each of these must list \p old, which is
     * replaced by \p nov .
     * \note Important: \p old retains list of slots formerly connected to it.
     * \throws invariant_error If a slot connected to \p nov does not list
     * \p old.
     */
    void replace_slot_in_neigs(
        const EndSlot& old,
        const EndSlot& nov);

    /**
     * Reconnect edges from vertex \p f to vertex \p t .
     * \details Disconnect edges connected to vertex \p f and
     * connect them to vertex \p t
     */
    void reconnect_edges(Vertex* f, Vertex* t);

    /**
     * \name Moving chains to component \p c .
     * \details The chains are removed from the records of this component and
     * appended to those of \p c .
     * @{
     */

    /// Moves chains \p vv, with their vertices, as move_to(Component&, Chain&)
    /// does, in descending order of the chain indexes.
    void move_to(Component& c, ChIdGs&& vv);
    /// Moves chains \p mm, with their vertices.
    void move_to(Component& c, Chains& mm);
    /// Moves chain \p m, with its vertices.
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

    /// The edges of the component, in the order of their component-wide index.
    constexpr auto edges() const noexcept -> std::vector<const Edge*>;
    constexpr auto edge(EgIdC indc) const noexcept -> const Edge&;
    constexpr auto edge(EgIdC indc)       noexcept ->       Edge&;
    constexpr auto edge(const EndSlot& s) const noexcept -> const Edge&;
    constexpr auto edge(const EndSlot& s)       noexcept ->       Edge&;
    constexpr auto edge(const BulkSlot& s) const noexcept -> const Edge&;
    constexpr auto edge(const BulkSlot& s)       noexcept ->       Edge&;
    constexpr auto edge(EgIdG i) const noexcept -> const Edge&;

    /// The chains of the component, in the order of \a ww.
    constexpr auto chains() const noexcept -> std::vector<const Chain*>;
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
     * functor structure::modifiers::JunctionShifter.
     */
    void shift_chain_junction(const EndSlot& f,
                              const EndSlot& t);

    /**
     * Checks that \p cond is satisfied.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] message Arguments forming the message of the exception
     *                    thrown if the condition does not hold.
     * \throws invariant_error If \p cond is false.
     */
    template <typename... T>
    void ensure(
        bool cond,
        T &&...message) const;

    /**
     * \name Internal tests.
     * \return \a nullptr or \a false iff all is ok
     * @{
     */

    /// Ensure that each edge descriptor of each vertex refers to an edge of
    /// the component connected to the vertex at the recorded inner end.
    /// \return The first vertex failing the check, or \a nullptr.
    template<bool beLoud = true>
    auto check_connectivity_vertex_edge() const -> Vertex* ;

    /// Ensure that the vertices record this component, and pass
    /// check_connectivity_vertex_edge().
    /// \return \a false iff all is ok.
    template<bool beLoud = true>
    bool check_vertex_data() const;

    /// Ensure correctness of chain indices component-wise.
    /// \return \a false iff all is ok.
    template<bool beLoud = true>
    bool check_chain_idc() const;

    /// Ensure correctness of edge indices component-wise.
    /// \return \a false iff all is ok.
    template<bool beLoud = true>
    bool check_edge_indc() const;

    /// Ensure correctness of edge descriptors.
    /// \return \a false iff all is ok.
    template<bool beLoud = true>
    bool check_egl() const;

    /// Ensure correctness of chain classification.
    /// \return \a false iff all is ok.
    template<bool beLoud = true>
    bool check_chis() const;

    /**
     * Ensure that no cycle chain is shorter than Chain::minCycleLength.
     * \note Unlike the other checks, reports a failure in all builds.
     * \return \a false iff all is ok.
     */
    template<bool beLoud = true>
    bool check_chain_lengths() const;

    /// Collection of tests of this component internal structures and indices.
    /// \return \a false iff all is ok.
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

    void print_egl() const;

    template <bool with_top = true>
    void print_ww() const;

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
Component(Component&& other) noexcept
    : ind{other.ind}
    , ww{std::move(other.ww)}
    , vv{std::move(other.vv)}
    , chis{std::move(other.chis)}
    , adj{std::move(other.adj), *this}  // must refer to this, not to 'other'
    , trs{std::move(other.trs)}
    , cn{other.cn}
    , egl{std::move(other.egl)}
{}

template <typename Ch>
constexpr auto Component<Ch>::
operator=(Component&& other) noexcept -> Component&
{
    // 'cn' is a reference, and cannot be rebound: components are only moved
    // within the graph that holds their chains.
    GRAFFINE_ASSERT(&cn == &other.cn,
                    "Moving a component between graphs is not supported");

    ind = other.ind;
    ww = std::move(other.ww);
    vv = std::move(other.vv);
    chis = std::move(other.chis);
    adj.take_lists(std::move(other.adj));
    trs = std::move(other.trs);
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
is_single_unconnected_chain() const
{
    return chis.num({1, 1});
}

template <typename Ch>
constexpr bool Component<Ch>::
is_unconnected_cycle() const
{
    return chis.num({2, 2});
}
template <typename Ch>
void Component<Ch>::
detect_chain_triangles()
{
    using TChIds = Triangle::ChIds;
    using TVIds = Triangle::VIds;

    // A chain eligible to a triangle is no cycle, and has junctions at
    // both ends.
    auto is_eligible = [](const Chain& m)
    {
        return !m.is_cycle() &&
               m.degree_at_end(End::A) >= Deg3 &&
               m.degree_at_end(End::B) >= Deg3;
    };

    trs.clear();
    std::set<TChIds> found;

    for (const auto w : ww) {

        const auto& m = cn[w];
        if (!is_eligible(m))
            continue;

        const auto vA = m.vertex_at_end(End::A)->get_id();
        const auto vB = m.vertex_at_end(End::B)->get_id();

        for (const auto& sA : m.ngs[End::A]()) {

            const auto& mA = cn[sA.w];
            if (sA.w == w || !is_eligible(mA))
                continue;
            const EndSlot fA {sA.w, End::opp(sA.e)};  // far end of mA

            for (const auto& sB : m.ngs[End::B]()) {

                const auto& mB = cn[sB.w];
                if (sB.w == w || sB.w == sA.w || !is_eligible(mB))
                    continue;
                const EndSlot fB {sB.w, End::opp(sB.e)};  // far end of mB

                // The far ends of mA and mB are joined at the third vertex.
                if (!mA.ngs[fA.e].contains(fB))
                    continue;

                const auto vC = mA.vertex_at_end(fA.e)->get_id();
                if (vC == vA || vC == vB)
                    continue;

                TChIds tw {w, sA.w, sB.w};
                std::ranges::sort(tw);
                if (!found.insert(tw).second)
                    continue;

                TVIds tv {vA, vB, vC};
                std::ranges::sort(tv);
                trs.tris.push_back(Triangle {tw, tv});
            }
        }
    }
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

    GRAFFINE_ASSERT(vp, "Accessing non-existing vertex idh ", vi, " in component ", ind);

    return *vp;
}

template <typename Ch>
constexpr auto Component<Ch>::
vertex(const VIdH vi) noexcept -> Vertex&
{
    const auto vp = vv.find(vi);

    GRAFFINE_ASSERT(vp, "Accessing non-existing vertex idh ", vi, " in component ", ind);

    return *vp;
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
void Component<Ch>::
    remove_vertex(const VIdH vi)
{
    vv.remove(vi, *this);
}

template <typename Ch>
auto Component<Ch>::
set_chis() -> Component&
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

                GRAFFINE_ASSERT(i++ == g.indc,
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
edges() const noexcept -> std::vector<const Edge*>
{
    std::vector<const Edge*> g;
    for (const auto& h : egl)
        g.push_back(&cn[h.w].g[h.a]);

    return g;
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EgIdC indc) const noexcept -> const Edge&
{
    GRAFFINE_ASSERT(indc < num_edges(),
           "Incorrect indc ", indc,
           " for component ", ind, " having ", num_edges(), " edges");

    const auto& h = egl[indc];

    return cn[h.w].g[h.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
edge(const EgIdC indc) noexcept -> Edge&
{
    GRAFFINE_ASSERT(indc < num_edges(),
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

    GRAFFINE_ASSERT(h.is_defined(),
           "Error: Edge ind", i, " is not part of component ", ind);

    return &cn[h.w].g[h.a];
}

template <typename Ch>
constexpr auto Component<Ch>::
chains() const noexcept -> std::vector<const Chain*>
{
    std::vector<const Chain*> mm;
    for (const auto w : ww)
        mm.push_back(&cn[w]);

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
    GRAFFINE_ASSERT(std::find(ww.begin(), ww.end(), w) != ww.end(),
           "Chain ", w, " is not part of component ", ind);

    return cn[w];
}

template <typename Ch>
constexpr auto Component<Ch>::
chain(const ChIdG w) noexcept -> Chain&
{
    GRAFFINE_ASSERT(std::find(ww.begin(), ww.end(), w) != ww.end(),
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
    // The vertex is at the inner end ed.e of the edge: its position in the
    // chain depends on the orientation of the edge.
    const auto& d = egl[ed.indc()];

    return cn[d.w].egEnd_to_bulkslot(ed.e, d.a);
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


    }  // namespace detail

template <typename Ch>
using Component = detail::Component<Ch>;

}  // namespace graffine::structure

// Parts of the implementation.
#include "graffine/structure/elements/component/edges.h"
#include "graffine/structure/elements/component/chains.h"
#include "graffine/structure/elements/component/neighbours.h"
#include "graffine/structure/elements/component/checks.h"
#include "graffine/structure/elements/component/print.h"

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_H
