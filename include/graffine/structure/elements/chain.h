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
 * \file chain.h
 * \brief The chain: a linear run of edges with no branches.
 * \details Only graphs containing vertices of max degree 4 are implemented.
 * A related implementation for max degree 3 is available at
 * https://github.com/vsukhor/mitoSim/blob/master/include/segment.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_H
#define GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_H

#include "graffine/definitions.h"
#include "graffine/structure/common.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/containers/edges.h"
#include "graffine/structure/containers/neigs.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include <algorithm>  // rotate
#include <array>
#include <string>
#include <vector>


namespace graffine::structure {
namespace detail {

/** Generic chain template; see the specialization Chain<E> for the
 * implemented chain.
 */
template<typename E,
         typename... Props>
struct Chain
{};


/** Chain template for Edge classes having max vertex degree equal to 4.
 * \details A chain is a sequence of edges linked linearly, without
 * branches: all its internal vertices have degree 2. Its ends may be
 * free (degree 1), or joined to the ends of other chains at junctions
 * (degree 3 or 4). A chain not connected to other chains, or a maximal
 * set of chains connected to each other, forms a graph component.
 * Chains make graffine efficient for graphs dominated by long unbranched
 * runs: a position along a chain is a plain index, and most
 * transformations touch only a few chains.
 *
 * Chain coordinates:
 * - the edges are stored in \a g, ordered from chain end A to chain end B:
 *   edge \a g[a] has position \a a (EgIdA), the \a tail g[0] is at end A
 *   and the \a head g[length()-1] at end B. The outer ends of the edges
 *   (see Edge) follow this orientation: outer end A faces chain end A;
 * - consecutive edges g[a-1] and g[a] share an internal (bulk) vertex,
 *   addressed by BulkSlot {idw, a} with 0 < a < length();
 * - each chain end \a e has a boundary vertex, addressed by EndSlot
 *   {idw, e}. \a ngs[e] lists the other chain end slots joined at it, so
 *   that its degree is \a ngs[e].num() + 1.
 *
 * A chain is a \a cycle if both its ends are at the same vertex: each end
 * then lists the opposite end of the same chain. An unconnected cycle is
 * joined to nothing else, so that both its ends have degree 2.
 * \tparam E Edge class having max vertex degree 4.
 */
template<typename E>
struct Chain<E>
{
    using Edge = E;
    using Edges = containers::Edges<Edge>;
    using EdgeEnds = E::End;  ///< Edge ends.
    using End = descriptors::End;
    using Bulk = descriptors::Bulk;
    using EndId = End::Id;
    using EndSlot = descriptors::EndSlot;
    using BulkSlot = descriptors::BulkSlot;
    using Neigs = containers::Neigs<EndSlot>;  ///< Slots connected to the chain ends.
    using Vertex = Edge::Vertex;
    using Self = Chain<Edge>;
    using Neighbours = std::array<Neigs, End::num>;  ///< Neighbours at ends A and B.

    static constexpr auto Type = Elements::Chain;
    // A chain is a linear sequence of edges, so it has 2 ends.
    static constexpr auto endA = End::A;            ///< Tail end.
    static constexpr auto endB = End::B;            ///< Head end.
    static constexpr auto endUndef = End::undefined;  ///< Undefined end.
    /// Minimal number of edges of a cycle.
    static constexpr std::size_t minCycleLength {2};
    /// Minimal number of edges of a chain.
    static constexpr std::size_t minLength {1};
    /// True iff the chain lives in a geometric space.
    static constexpr auto isSpatial = false;

    Edges g;  ///< Edges forming this chain, from end A to end B.

    /// End slots of the other chains joined at each end.
    Neighbours ngs;

    ChIdG idw {ChIdG::undefined};  ///< Position in the graph (ChIdG).
    ChIdC idc {ChIdC::undefined};  ///< Position in the current component (ChIdC).

    /// Identifier of the current connected component the chain belongs to.
    CmpId c {CmpId::undefined};

    /** Empty chain constructor without component specs.
     * \param[in] idw Graph-wide index of this chain.
     */
    explicit constexpr Chain(ChIdG idw);

    /** A chain with specific parameters as a part of a graph.
     * \param[in] idw Graph-wide index of this chain.
     * \param[in] idc Index of this chain in the host component.
     * \param[in] c Index of connected component to which the chain belongs.
     */
    explicit constexpr Chain(
        ChIdG idw,
        ChIdC idc,
        CmpId c
    );

    // Defaulted: the moves are then noexcept, so that std containers of
    // chains move them rather than copying.
    constexpr Chain() = default;
    constexpr Chain(const Chain& other) = default;
    constexpr Chain(Chain&& other) noexcept = default;
    constexpr auto operator=(const Chain& other) -> Chain& = default;
    constexpr auto operator=(Chain&& other) noexcept -> Chain& = default;
    ~Chain() = default;

    /// Compares the edges, the neighbours and the ids.
    constexpr bool operator==(const Chain& other) const noexcept;

    /** Reverses the order of the edges, so that ends A and B swap.
     * \details Each edge flips its orientation, keeping its vertices, and
     * gets its new position. The neighbours \a ngs and the slots referring
     * to this chain elsewhere are not updated: that is up to the caller.
     */
    void reverse_g();

    /** Rotates the edges to the left by \p n positions (std::rotate), and
     * renumbers their positions.
     * \details Meaningful for a cycle, whose ends then move to the vertex
     * formerly at position \p n.
     */
    constexpr void rotate(const std::size_t n);

    /** Sets the component-related ids of the chain, but not those of its
     * edges.
     * \param[in] newc Index of the new connected component.
     * \param[in] newidc New index of this chain inside the component.
     */
    void set_cmpt(CmpId newc,
                  ChIdC newidc) noexcept;

    /**
     * \name Vertex operations
     * @{
     */

    /** Counts the vertices of this chain having a given degree.
     * \param[in] d Vertex degree.
     * \return The number of vertices having degree \p d.
     */
    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;

    /** Vertices at both ends of every edge, in chain order.
     * \details Two entries per edge: each internal vertex appears twice,
     * and the entry of an unconnected end is nullptr.
     * \return Pointers to vertices forming this chain.
     */
    constexpr auto vertices() const noexcept -> std::vector<const Vertex*>;

    /** Connects vertex \p v at chain end \p e , i.e. to the outer end \p e
     * of the edge at that end.
     */
    constexpr void connect_end_vertex(EndId e, Vertex& v);
    /** Connects vertex \p v at internal position \p a , i.e. between edges
     * g[a-1] and g[a]. Requires 0 < \p a < length().
     */
    constexpr void connect_bulk_vertex(EgIdA a, Vertex& v) noexcept;
    /** Disconnects the vertex at chain end \p e from the edge at that end.
     * The vertex is not deleted.
     */
    constexpr void disconnect_end_vertex(EndId e) noexcept;

    /**
     * \return Boundary vertex at chain end \p e
     */
    constexpr auto vertex_at_end(EndId e) noexcept -> Vertex*;
    /**
     * \overload
     */
    constexpr auto vertex_at_end(EndId e) const noexcept -> const Vertex*;

    /**
     * \return Internal vertex at position \p a, between edges g[a-1] and
     * g[a]; requires 0 < \p a < length().
     */
    constexpr auto vertex_at_bulk(EgIdA a) noexcept -> Vertex*;
    /**
     * \overload
     */
    constexpr auto vertex_at_bulk(EgIdA a) const noexcept -> const Vertex*;

    /**
     * \return Internal vertex linking edges \p ind1 and \p ind2
     * \param[in] ind1 Graph-wide index of the 1st edge.
     * \param[in] ind2 Graph-wide index of the 2nd edge.
     * \note \p ind1 and \p ind2 must be neighbouring edges, both part of
     * this chain
     */
    constexpr auto vertex_at_bulk(EgIdG ind1,
                                  EgIdG ind2) const noexcept -> const Vertex*;
    /**
     * \overload
     */
    constexpr auto vertex_at_bulk(const Edge& eg1,
                                  const Edge& eg2) const noexcept -> const Vertex*;
    /** Vertex at slot \p s of this chain: a boundary vertex for an
     * EndSlot, an internal one for a BulkSlot.
     * \tparam S Slot type: EndSlot or BulkSlot.
     * \return Pointer to vertex at slot \p s
     */
    template<typename S>
    constexpr auto vertex_at_slot(const auto& s) const noexcept -> const Vertex*;

    /** Degree of the boundary vertex at chain end \p e, as given by the
     * number of neighbours: ngs[e].num() + 1.
     */
    constexpr auto degree_at_end(EndId e) const noexcept -> Degree;

    /** Maximal degree among the vertices of this chain.
     */
    constexpr auto max_degree() const noexcept -> Degree;

    ///@}  // Vertex operations.


    /** Whether edge \p ind is part of this chain.
     */
    bool has_edge(EgIdG ind) const noexcept;

    /** The edge at chain end \p e: the tail for A, the head for B.
     * \param[in] e Index of the chain end.
     * \return Reference to the edge at the specified end.
     */
    constexpr auto end_edge(EndId e) const noexcept -> const Edge&;
    /** \overload */
    constexpr auto end_edge(EndId e) noexcept -> Edge&;

    /** Converts an outer end of edge g[\p a] to its inner end.
     * \param[in] a Edge position inside the chain.
     * \param[in] e Edge end in chain (outer) coordinates.
     * \return Inner edge end.
     */
    constexpr auto edge_internal_end(
        EgIdA a,
        EndId e
    ) const noexcept -> EndId;

    /** Inner end, of the edge at chain end \p e, that lies at that end.
     * \param[in] e Chain end index.
     * \return Inner edge end at the specified chain boundary.
     */
    constexpr auto edge_internal_end_at_end(EndId e) const noexcept -> EndId;

    /** The edge of this chain joined to inner end \p egEnd of the edge at
     * position \p a, or nullptr.
     * \note \p egEnd must not lie at a chain end.
     * \param[in] a Intra-chain position of the edge.
     * \param[in] egEnd Inner end of the edge.
     */
    constexpr auto connected_edge(
        EgIdA a,
        EndId egEnd
    ) const noexcept -> const Edge*;

    /**
     * \name Edge lookup by graph-wide or component-wide edge index.
     * \details Linear in the chain length. Return the position, or the edge,
     * of the edge having the index; undefined, or \a nullptr, if no edge of
     * this chain has it.
     * @{
     */
    constexpr auto ind2a(const EgIdG ind) const noexcept -> EgIdA;
    constexpr auto ind2eg(const EgIdG ind) const noexcept -> const Edge*;
    constexpr auto ind2eg(const EgIdG ind)       noexcept ->       Edge*;
    constexpr auto indc2a(const EgIdC indc) const noexcept -> EgIdA;
    constexpr auto indc2eg(const EgIdC indc) const noexcept -> const Edge*;
    constexpr auto indc2eg(const EgIdC indc)       noexcept ->       Edge*;

    ///@}  // Edge lookup.

    /** Chain end slot at inner end \p internalEgEnd of the edge at
     * position \p a, which must be the head or the tail.
     * \param[in] a Edge position within the chain.
     * \param[in] internalEgEnd Inner edge end.
     * \return End slot of this chain if the input is valid, otherwise an
     * undefined slot.
     */
    constexpr auto end_slot(
        EgIdA a,
        EndId internalEgEnd
    ) const noexcept -> EndSlot;

    /** Position of the edge joined to inner end \p ee of the edge at
     * position \p a.
     * \note Should only be applied to the chain internal vertices.
     * \return The position of that edge if \p ee is at an internal vertex
     * of the chain; undefined otherwise.
     */
    constexpr auto neig_a_of_egEnd(
        EgIdA a,     ///< Edge position inside the chain.
        EndId ee     ///< Edge end.
    ) const noexcept -> EgIdA;

    /** Inner end of the edge at position \p b by which it is joined to
     * the adjacent edge at position \p a.
     * \note \p a and \p b must be consecutive positions, so that the
     * edges share an internal vertex; then the end sought is unique.
     */
    constexpr auto internal_egEnd(
        EgIdA a,
        EgIdA b
    ) const noexcept -> EndId;

    /** Inner end of edge g[\p a] facing chain end A.
     */
    constexpr auto a2egEnd(EgIdA a) const noexcept -> EndId;

    /** Position of the edge at chain end \p e: 0 for A, length()-1 for B.
     * \param[in] e Chain end index.
     */
    constexpr auto end2a(EndId e) const noexcept -> EgIdA;

   /** Chain end at which the edge at position \p a lies, which must be the
    * head or the tail.
    * \return The chain end, or End::undefined for single-edge chains,
    * whose edge lies at both.
    * \param[in] a Edge position within the chain.
    */
    constexpr auto a2end(EgIdA a) const noexcept -> EndId;

    /** Slot of the internal vertex at inner end \p egE of the edge at
     * position \p a.
     * \param[in] egE Inner edge end.
     * \param[in] a Edge position in the chain.
     * \return Corresponding internal slot.
     * \note The edge end must not lie at a chain end.
     */
    constexpr auto egEnd_to_bulkslot(EndId egE,
                                     EgIdA a) const noexcept -> BulkSlot;

    /** Graph-wide index of the edge at chain end \p e.
     * \param[in] e Chain end index.
     */
    constexpr auto end2ind(EndId e) const noexcept -> EgIdG;

    /** Convert graph-wide edge index to chain end.
     * \details Convert graph-wide edge index to chain end if the edge
     * is on chain boundary. For single-edge chains, the end is determined
     * by the edge internal end \p ege .
     * \param[in] ind Graph-wide edge index.
     * \param[in] ege Edge internal end; used only for single-edge chains.
     * \return End::A if the edge is the chain tail, End::B if it is the head;
     * for single-edge chains, the chain end at the edge end \p ege .
     * \throws precondition_error If the edge is not at a chain end.
     */
    auto ind2end(EgIdG ind,
                 EndId ege = End::undefined) const -> EndId;

    /** Global edge index to chain end conversion.
     * \details Convert graph-wide edge index to chain end if the edge
     * is on chain boundary. For single-edge chains, the end is the one
     * connected to \p nb .
     * \param[in] ieg Graph-wide edge index.
     * \param[in] nb Slot connected to the searched end; used only for
     * single-edge chains.
     * \return End::A if the edge is the chain tail, End::B if it is the head;
     * for single-edge chains, the chain end connected to \p nb .
     * \throws precondition_error If the edge is not at a chain end, or, for
     * single-edge chains, if no end is connected to \p nb .
     */
    auto ind2end(EgIdG ieg,
                 const EndSlot& nb) const -> EndId;

    /** Chain end at the edge having component-wide index \p indc .
     * \details As ind2end(EgIdG, EndId), for a component-wide index: for a
     * single-edge chain, the end is the one at inner end \p egEnd of the edge.
     * \throws precondition_error If the edge is not at a chain end.
     */
    constexpr auto indc2end(EgIdC indc,
                            EndId egEnd) const -> EndId;

    /** Determines if the chain is joined at its end \p e to chain \p n.
     * \param[in] e End of this chain.
     * \param[in] n Id of the presumed neighbour chain.
     */
    bool has_such_neig(EndId e, ChIdG n) const noexcept;

    /** The chain end opposite to end \p e.
     * \param[in] e Chain end index.
     * \return Opposite end index.
     */
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    /** Determines if end \p e is joined to any chain end.
     * \return \a true iff the chain has neighbours at its end \p e.
     */
    constexpr bool is_connected_at(EndId e) const noexcept;

    /** Determines if the chain is joined to any chain end at either end.
     * \return \a true iff the chain is connected at any of its ends.
     */
    constexpr bool is_connected() const noexcept;

    /** The connected end of a chain connected at exactly one end.
     * \throws precondition_error If the chain is connected at both or
     * neither end.
     */
    constexpr auto get_single_connected_end() const -> EndId;

    /** The free end of a chain connected at exactly one end.
     * \throws precondition_error If the chain is connected at both or
     * neither end.
     */
    constexpr auto get_single_leaf_end() const -> EndId;

    /** The end lying at a degree 3 junction.
     * \throws precondition_error If both or neither end lie at one.
     */
    constexpr auto get_single_3way_end() const -> EndId;

    /** The end lying at a degree 4 junction.
     * \throws precondition_error If both or neither end lie at one.
     */
    constexpr auto get_single_4way_end() const -> EndId;

    /** Determines if the chain has exactly one free end.
     * \return \a true iff one end is connected and the other is not.
     */
    constexpr bool has_single_leaf_vertex() const noexcept;

    /** Index of the free end of a chain having exactly one free end.
     * \note Valid only if has_single_leaf_vertex() is true.
     */
    constexpr auto the_only_leaf_end() const noexcept -> EndId;

    /** Determines if at least one of the chain ends is free.
     * \return \a true and the free end (End::A if both are free), or
     *         \a false and End::undefined.
     */
    constexpr auto has_a_leaf_end() const noexcept -> std::pair<bool, EndId>;

    /** Checks if the chain is linear and unconnected, i.e. a component of
     * its own with two free ends.
     */
    constexpr bool is_unconnected_linear() const noexcept;


    /** Checks if the chain is an unconnected cycle: its two ends are
     * joined to each other and to nothing else, so that the chain forms a
     * component of its own.
     */
    constexpr bool is_unconnected_cycle() const noexcept;

    /** Checks if the chain is a connected cycle: its two ends meet at a
     * junction, which also joins other chains.
     */
    constexpr bool is_connected_cycle() const noexcept;

    /** Checks if the chain is a cycle, i.e. both its ends are at the same
     * vertex, whether or not other chains are joined there.
     */
    constexpr bool is_cycle() const noexcept;

    /** Checks if an edge can be deleted from the chain.
     * \details An edge can be deleted from the chain if the chain length is
     * larger than the minimal length allowed for the given chain topology,
     * or if the chain has a single leaf end: then, removing its last edge
     * removes the chain.
     * \return \a true if an edge can be deleted from the chain,
     *         \a false otherwise.
     */
    constexpr bool is_shrinkable() const noexcept;

    /** Checks if \p n edges can be removed from the chain, which is preserved.
     * \details Unlike is_shrinkable(), does not let a chain having a leaf
     * vertex disappear: the chain should keep at least minLength edges, or
     * minCycleLength edges if it is a cycle.
     */
    constexpr bool is_shrinkable_by(std::size_t n) const noexcept;

    /** Whether the chain is an unconnected cycle and \p v its boundary vertex.
     */
    constexpr bool is_unconnected_cycle_boundary(const Vertex& v) const noexcept;

    /** Whether the chain is a cycle and \p v its boundary vertex, at which
     * both chain ends are.
     */
    constexpr bool is_cycle_boundary(const Vertex& v) const noexcept;

    /** Whether edge g[\p a] points forwards (see Edge).
     */
    constexpr bool edge_points_forwards(EgIdA a) const noexcept;

    /** Number of chain end slots joined at end \p e .
     */
    constexpr auto num_neigs(const EndId e) const noexcept -> std::size_t;

    /** Chain length: the number of its edges.
     */
    constexpr auto length() const noexcept -> std::size_t;

    /** Total weight of the chain: the sum of the weights of its edges.
     */
    constexpr auto weight() const noexcept -> typename Edge::weight_t;

    /** Head edge of the chain.
     * \details The head edge is the last edge in the chain.
     * \return Const reference to the head edge.
     */
    constexpr auto head() const noexcept -> const Edge& { return g.back(); }

    /** Tail edge of the chain.
     * \details The tail edge is the first edge in the chain.
     * \return Const reference to the tail edge.
     */
    constexpr auto tail() const noexcept -> const Edge& { return g.front(); }

    /**
     * \name Head and tail tests.
     * \details The tail is the edge at chain end A, position 0, and the head
     * the edge at chain end B, position length() - 1. In a single-edge chain,
     * the edge is both.
     * @{
     */
    /// Graph-wide index of the head edge.
    constexpr auto headind() const noexcept -> EgIdG { return head().ind; }
    /// Graph-wide index of the tail edge.
    constexpr auto tailind() const noexcept -> EgIdG { return tail().ind; }

    static constexpr bool is_tail(const EgIdA a) noexcept { return a == 0; }
    constexpr bool is_head(const EgIdA a) const noexcept { return a == length() - 1; }

    constexpr bool is_head(const Edge& eg) const noexcept { return eg.ind == head().ind; }
    constexpr bool is_tail(const Edge& eg) const noexcept { return eg.ind == tail().ind; }

    constexpr bool is_headind(const EgIdG ind) const noexcept { return ind == head().ind; }
    constexpr bool is_tailind(const EgIdG ind) const noexcept { return ind == tail().ind; }

    constexpr bool is_end(const EgIdA a) const noexcept { return is_head(a) || is_tail(a); }

    constexpr bool ind_is_end(const EgIdG ind) const noexcept { return head().ind == ind || tail().ind == ind; }

    ///@}  // Head and tail tests.

    /** Checks if inner end \p egEnd of edge \p ind lies at chain end B.
     * \param ind Edge index in graph scope.
     * \param egEnd Inner end of the edge at \p ind
     */
    constexpr bool egEnd_is_head(EgIdG ind,
                                 EndId egEnd) const noexcept;

    /** Checks if inner end \p egEnd of edge \p ind lies at chain end A.
     * \param ind Edge index in graph scope.
     * \param egEnd Inner end of the edge at \p ind
     */
    constexpr bool egEnd_is_tail(EgIdG ind,
                                 EndId egEnd) const noexcept;

    /// Sets the graph-wide id of the chain to \p w, and updates its edges.
    constexpr void set_w(ChIdG w) noexcept;
    /// Sets the host chain of the edges to \p w and renumbers their
    /// positions.
    constexpr void set_g_w(ChIdG w) noexcept;
    /// Sets the host chain of the edges to this chain and renumbers their
    /// positions.
    constexpr void set_g_w() noexcept;

    /**
     * \name Formatted printing.
     * @{
     */

    /** Prints the neighbours at both ends.
     * \param[in] tag Human-readable description common to all records.
     * \return Length of the printed tag, for aligning further output.
     */
    template<bool endLine,
             typename... T>
    auto print_connectivity(T&&... tag) const -> std::size_t;

    /** Prints chain parameters.
     * \param[in] tag Human-readable description prepended to the printed data.
     */
    template<typename... T>
    void print(T&&... tag) const;

    ///@}  // Formatted printing.

    /** Checks that \p cond is satisfied.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] message Arguments forming the message of the exception
     *                    thrown if the condition does not hold.
     * \throws invariant_error If \p cond is false.
     */
    template<typename... T>
    void ensure(
        bool cond,
        T&&... message
    ) const;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename E>
constexpr
Chain<E>::
Chain(const ChIdG idw)
    : idw {idw}
{}

template<typename E>
constexpr
Chain<E>::
Chain(
    const ChIdG idw,
    const ChIdC idc,
    const CmpId c
)
    : idw {idw}
    , idc {idc}
    , c {c}
{}

template<typename E>
constexpr
bool Chain<E>::
operator==(const Chain& other) const noexcept
{
    return g == other.g &&
           ngs == other.ngs &&
           idw == other.idw &&
           idc == other.idc &&
           c == other.c;
}


template<typename E>
void Chain<E>::
reverse_g()
{
    std::reverse(g.begin(), g.end());
    for (EgIdA a {}; auto& o : g) {
        o.template reverse<false>();
        o.indw = a++;
    }
}

template<typename E>
constexpr
void Chain<E>::
rotate(const std::size_t n)
{
    std::rotate(g.begin(), g.begin() + n, g.end());      // to the left

    for (EgIdA a {}; auto& o : g)
        o.indw = a++;
}

template<typename E>
void Chain<E>::
set_cmpt(
    const CmpId newc,
    const ChIdC newidc
) noexcept
{
    c = newc;
    idc = newidc;
}

template<typename E>
constexpr
auto Chain<E>::
length() const noexcept -> std::size_t
{
    return g.size();
}


template<typename E>
constexpr
auto Chain<E>::
weight() const noexcept -> typename Edge::weight_t
{
    typename Edge::weight_t res {};

    for (const auto& o : g)
        res += o.weight();

    return res;
}


template<typename E>
constexpr
void Chain<E>::
set_g_w(const ChIdG w) noexcept
{
    for (EgIdA a {}; auto& o : g) {
        o.w = w;
        o.indw = a++;
    }
}


template<typename E>
constexpr
void Chain<E>::
set_g_w() noexcept
{
    set_g_w(idw);
}


template<typename E>
constexpr
void Chain<E>::
set_w(const ChIdG w) noexcept
{
    idw = w;
    set_g_w();
}


}  // namespace detail

template<typename E> using Chain = detail::Chain<E>;

/// Empty chain with an undefined id, marking the absence of a chain.
template<typename E> constexpr Chain<E> nullChain {ChIdG::undefined};

/// Checks that \p ch differs from \ref nullChain.
template<typename T>
constexpr bool is_defined(const Chain<T>& ch) noexcept
{
    return ch != nullChain<T>;
}

}  // namespace graffine::structure

// Parts of the implementation.
#include "graffine/structure/elements/chain/vertices.h"
#include "graffine/structure/elements/chain/indexing.h"
#include "graffine/structure/elements/chain/connectivity.h"
#include "graffine/structure/elements/chain/print.h"

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_H
