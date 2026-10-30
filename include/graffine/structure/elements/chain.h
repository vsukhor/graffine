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
 * \brief Contains Chain class template and its specialization.
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

template<typename E,
         typename... Props>
struct Chain
{};


/**
 * Chain template for Edge classes having max vertex degree equal to 4.
 * \details Chain is a sequence of edges linked linearly (without branches).
 * Chain ends may form branching sites, if connected to other chains.
 * A chain not connected to other chains or a complete collection of chains
 * connected to each other form a graph component (aka 'component').
 * The class handles the tasks and properties specific to a single
 * chain and its relation to other graph components.
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
    using Neighbours = std::array<Neigs, End::num>;

    static constexpr auto Type = Elements::Chain;
    // A chain is a linear sequence of edges, so it has 2 ends.
    static constexpr auto endA = End::A;
    static constexpr auto endB = End::B;
    static constexpr auto endUndef = End::undefined;
    /// Minimal number of edges of a cycle.
    static constexpr std::size_t minCycleLength {2};
    /// Minimal number of edges of a chain.
    static constexpr std::size_t minLength {1};
    static constexpr auto isSpatial = false;

    Edges g;  ///< Edges forming this chain.

    /// Slots connected to this chain at each end.
    Neighbours ngs;

    ChIdG idw {ChIdG::undefined};  ///< Identifier over the whole graph.
    ChIdC idc {ChIdC::undefined};  ///< Identifier over the current component.

    /// Identifier of the current connected component the chain belongs to.
    CmpId c {CmpId::undefined};

    /**
     * Empty chain constructor without component specs.
     * \param[in] idw Graph-wide index of this chain.
     */
    explicit constexpr Chain(ChIdG idw);

    /**
     * A chain with specific parameters as a part of a graph.
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

    constexpr bool operator==(const Chain& other) const noexcept;

    /// Reflects the chain edges.
    void reverse_g();

    /// Rotates the sequence of chain edges using std::rotate.
    constexpr void rotate(const std::size_t n);

    /**
     * Set data related to the connected component hosting this chain, without
     * setting the edges.
     * \details Change connected graph component-related indexes of the
     * chain, without setting them for the edges.
     * \param[in] newc Index of the new connected component.
     * \param[in] newidc New index of this chain inside the component.
     */
    void set_cmpt(CmpId newc,
                  ChIdC newidc) noexcept;

    /**
     * \name Vertex operations
     * @{
     */

    /**
     * Counts vertices of a given degree.
     * \param[in] d Vertex degree.
     * \return The number of vertices having degree D.
     */
    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;

    /**
     * Collection of all vertices of this chain.
     * Does not discriminate between internal and boundary vertices.
     * \return Pointers to vertices forming this chain.
     */
    constexpr auto vertices() const noexcept -> std::vector<const Vertex*>;

    /// Connects vertex \p v at chain end \p e , i.e. to the outer end \p e
    /// of the edge at that end.
    constexpr void connect_end_vertex(EndId e, Vertex& v);
    /// Connects vertex \p v at internal position \p a , i.e. between edges
    /// g[a-1] and g[a]. Requires 0 < \p a < length().
    constexpr void connect_bulk_vertex(EgIdA a, Vertex& v) noexcept;
    /// Disconnects the vertex at chain end \p e from the edge at that end.
    /// The vertex is not deleted.
    constexpr void disconnect_end_vertex(EndId e) noexcept;

    /// \return Boundary vertex at chain end \p e
    constexpr auto vertex_at_end(EndId e) noexcept -> Vertex*;
    /// \overload
    constexpr auto vertex_at_end(EndId e) const noexcept -> const Vertex*;

    /// \return Internal vertex at edge position \p a
    constexpr auto vertex_at_bulk(EgIdA a) noexcept -> Vertex*;
    /// \overload
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
    /// \overload
    constexpr auto vertex_at_bulk(const Edge& eg1,
                                  const Edge& eg2) const noexcept -> const Vertex*;
    /**
     * Vertex at slot \p s . Depending on the slot type, returns either internal
     * or boundary vertex.
     * \tparam S Slot type
     * \return Pointer to vertex at slot \p s
     */
    template<typename S>
    constexpr auto vertex_at_slot(const auto& s) const noexcept -> const Vertex*;

    /// Reports degree of the boundary vertex at chain end \p e
    constexpr auto degree_at_end(EndId e) const noexcept -> Degree;

    /// Reports max vertex degree among the vertices of this chain.
    constexpr auto max_degree() const noexcept -> Degree;

    ///@}  // Vertex operations.


    /// Whether edge \p ind is part of this chain.
    bool has_edge(EgIdG ind) const noexcept;

    /**
     * Get the edge at the specified end.
     * \param[in] e Index of the chain end.
     * \return Reference to the edge at the specified end.
     */
    constexpr auto end_edge(EndId e) const noexcept -> const Edge&;
    constexpr auto end_edge(EndId e) noexcept -> Edge&;

    /**
     * Convert chain edge index and edge end in chain to internal edge end
     * \param[in] a Edge index inside chain;
     * \param[in] e Edge end in chain coordinates.
     * \return Internal edge end.
     */
    constexpr auto edge_internal_end(
        EgIdA a,
        EndId e
    ) const noexcept -> EndId;

    /**
     * Get the edge internal end at the specified chain boundary.
     * \param[in] e Chain end index.
     * \return Edge internal end at the specified chain boundary.
     */
    constexpr auto edge_internal_end_at_end(EndId e) const noexcept -> EndId;

    /**
     * Get pointer to the edge connected at edge end \p egEnd to the edge at
     * chain position \p a .
     * \param[in] a Intra-chain position of the edge.
     * \param[in] egEnd Edge internal end.
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

    /**
     * Calculate chain end slot from edge position and edge internal end
     * \param[in] a Edge position within the chain.
     * \param[in] internalEgEnd Edge end in edge internal representation.
     * \return End slot of this chain if input is valid, otherwise Slot::undef;
    */
    constexpr auto end_slot(
        EgIdA a,
        EndId internalEgEnd
    ) const noexcept -> EndSlot;

    /**
     * Position of the edge neigh to egEnd \p ee of edge at \p a.
     * \note Should only be applied to the chain internal vertices.
     * \return Position in chain of the edge neigh to egEnd \p ee of edge at
     * \p a if \p ee is at internal chain vertex; undefined otherwise.
     */
    constexpr auto neig_a_of_egEnd(
        EgIdA a,     ///< Edge position inside the chain.
        EndId ee     ///< Edge end.
    ) const noexcept -> EgIdA;

    /**
     * Edge end of an internal vertex.
     * \details Finds edge end of the edge \p b to which the edge at
     * \p a is connected via a vertex internal to the chain.
     * \note Should only be applied to the chain internal vertices.
     * Then, the searched edge end is unique.
     */
    constexpr auto internal_egEnd(
        EgIdA a,
        EgIdA b
    ) const noexcept -> EndId;

    /// Inner end of edge g[\p a] facing chain end A.
    constexpr auto a2egEnd(EgIdA a) const noexcept -> EndId;

    /**
     * Convert chain end to position index of the boundary edge.
     * \details Converts the chain end of the boundary edge to internal
     * position in the chain.
     * \param[in] e Chain end index.
     * \return Internal position of the boundary edge at chain end \p e .
     */
    constexpr auto end2a(EndId e) const noexcept -> EgIdA;

   /**
     * Edge position to chain end conversion.
     * \details Converts edge position within the chain to the end
     * if the edge is the boundary edge.
     * \return The chain end, or End::undefined for single-edge chains.
     * \param[in] a Edge position within the chain.
     */
    constexpr auto a2end(EgIdA a) const noexcept -> EndId;

    /**
     * Convert edge end to interior slot.
     * \details Maps the edge end to the corresponding interior slot.
     * \param[in] egE Edge end.
     * \param[in] a Edge position in a chain.
     * \return Corresponding interior slot.
     * \note The edge end must be in the internal of the chain.
     */
    constexpr auto egEnd_to_bulkslot(EndId egE,
                                     EgIdA a) const noexcept -> BulkSlot;

    /**
     * Convert chain end to graph-wide edge index.
     * \details Converts chain end to the boundary edge graph-wide index.
     * \param[in] e Chain end index.
     */
    constexpr auto end2ind(EndId e) const noexcept -> EgIdG;

    /**
     * Convert graph-wide edge index to chain end.
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

    /**
     * Global edge index to chain end conversion.
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

    /**
     * Chain end at the edge having component-wide index \p indc .
     * \details As ind2end(EgIdG, EndId), for a component-wide index: for a
     * single-edge chain, the end is the one at inner end \p egEnd of the edge.
     * \throws precondition_error If the edge is not at a chain end.
     */
    constexpr auto indc2end(EgIdC indc,
                            EndId egEnd) const -> EndId;

    /**
     * Determines if the chain is connected at its end 'e' to chain 'n'.
     * \param[in] e End index at the current chain.
     * \param[in] n Chain index of the eventual neighbor.
     */
    bool has_such_neig(EndId e, ChIdG n) const noexcept;

    /**
     * Return index of the chain end opposite to the end \p e .
     * \param[in] e Chain end index.
     * \return Opposite end index.
     */
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    /**
     * Determines if end \p e has any connected chains.
     * \return \a true iff the chain has neighbours at its  end \p e
     */
    constexpr bool is_connected_at(EndId e) const noexcept;

    /**
     * Determines if chain is connected to any chains at any of its ends.
     * \return \a true iff the chain is connected at any of its ends.
     */
    constexpr bool is_connected() const noexcept;

    /**
     * Index of the connected end for a chain connected at one end.
     */
    constexpr auto get_single_connected_end() const -> EndId;

    /**
     * Index of the unconnected end for a chain connected at one end.
     */
    constexpr auto get_single_leaf_end() const -> EndId;

    /**
     * Index of the end connected via a 3-way junction.
     * \note Is valid for chains having single such connection only.
     */
    constexpr auto get_single_3way_end() const -> EndId;

    /**
     * Index of the end connected via a 4-way junction.
     * \note Is valid for chains having single such connection only.
     */
    constexpr auto get_single_4way_end() const -> EndId;

    /**
     * Determines if the chain has exactly one free end.
     * \return \a true iff one end is connected and the other is not.
     */
    constexpr bool has_single_leaf_vertex() const noexcept;

    /**
     * Index of the free end of a chain having exactly one free end.
     * \note Valid only if has_single_leaf_vertex() is true.
     */
    constexpr auto the_only_leaf_end() const noexcept -> EndId;

    /**
     * Determines if at least one of the chain ends is not connected.
     * \return \a true and the free end (End::A if both are free), or
     *         \a false and End::undefined.
     */
    constexpr auto has_a_leaf_end() const noexcept -> std::pair<bool, EndId>;

    /**
     * Checks if the chain has its both ends unconnected.
     * \details This chain is unconnected iff it is
     * not connected at its boundary vertices.
     * \return \a true if the chain is unconnected at its boundary vertices,
     *         \a false otherwise.
     */
    constexpr bool is_unconnected_linear() const noexcept;


    /**
     * Checks if the chain is unconnected and cycled onto itself.
     * \details A chain is unconnected and cycled onto itself if it is
     * not connected to chains other than itself, while its ends are connected
     * to each other.
     * \return \a true if the chain is unconnected and cycled onto itself,
     *         \a false otherwise.
     */
    constexpr bool is_unconnected_cycle() const noexcept;

    /**
     * Checks if the chain is connected and cycled onto itself.
     * \details A chain is connected and cycled onto itself if it is
     * connected to chains other than itself, while its ends are also connected
     * to each other.
     * \return \a true if the chain is connected and cycled onto itself,
     *         \a false otherwise.
     */
    constexpr bool is_connected_cycle() const noexcept;

    /**
     * Checks if the chain is cycled onto itself.
     * \details A chain is cycled onto itself if its ends are connected
     * to each other.
     * \return \a true if the chain is cycled onto itself, \a false otherwise.
     */
    constexpr bool is_cycle() const noexcept;

    /**
     * Checks if an edge can be deleted from the chain.
     * \details An edge can be deleted from the chain if the chain length is
     * larger than the minimal length allowed for the given chain topology,
     * or if the chain has a single leaf end: then, removing its last edge
     * removes the chain.
     * \return \a true if an edge can be deleted from the chain,
     *         \a false otherwise.
     */
    constexpr bool is_shrinkable() const noexcept;

    /**
     * Checks if \p n edges can be removed from the chain, which is preserved.
     * \details Unlike is_shrinkable(), does not let a chain having a leaf
     * vertex disappear: the chain should keep at least minLength edges, or
     * minCycleLength edges if it is a cycle.
     */
    constexpr bool is_shrinkable_by(std::size_t n) const noexcept;

    /// Whether the chain is an unconnected cycle and \p v its boundary vertex.
    constexpr bool is_unconnected_cycle_boundary(const Vertex& v) const noexcept;

    /// Whether the chain is a cycle and \p v its boundary vertex, at which
    /// both chain ends are.
    constexpr bool is_cycle_boundary(const Vertex& v) const noexcept;

    /// Whether edge g[\p a] points forwards (see Edge).
    constexpr bool edge_points_forwards(EgIdA a) const noexcept;

    /// Number of chain end slots joined at end \p e .
    constexpr auto num_neigs(const EndId e) const noexcept -> std::size_t;

    /**
     * Reports the chain length measured in edges.
     * \details The chain length is equal to the number of edges in the chain.
     * \return Chain length measured in edges.
     */
    constexpr auto length() const noexcept -> std::size_t;

    /**
     * Reports the total weight of the chain edges.
     * \details The chain weight is equal to the sum of the weights of all edges
     * in the chain.
     * \return Chain weight measured in units of Edge::weight_t.
     */
    constexpr auto weight() const noexcept -> typename Edge::weight_t;

    /**
     * Head edge of the chain.
     * \details The head edge is the last edge in the chain.
     * \return Const reference to the head edge.
     */
    constexpr auto head() const noexcept -> const Edge& { return g.back(); }

    /**
     * Tail edge of the chain.
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
    constexpr auto headind() const noexcept -> EgIdG { return head().ind; }
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

    /**
     * Find if the given edge is at the chain head.
     * \param ind Edge index in graph scope.
     * \param egEnd Inner end of the edge at \p ind
     */
    constexpr bool egEnd_is_head(EgIdG ind,
                                 EndId egEnd) const noexcept;

    /**
     * Find if the given edge is at the chain tail.
     * \param ind Edge index in graph scope.
     * \param egEnd Inner end of the edge at \p ind
     */
    constexpr bool egEnd_is_tail(EgIdG ind,
                                 EndId egEnd) const noexcept;

    constexpr void set_w(ChIdG w) noexcept;
    constexpr void set_g_w(ChIdG w) noexcept;
    constexpr void set_g_w() noexcept;
#if 0
    /**
     * \name Vertex addition and removal.
     * @{
     */

    /**
     * Connects an edge end and a vertex.
     * \details Arranges descriptors in vertex \p v and in edge \p eg to match
     * each other. After this operation, the vertex \p v is assigned to the end
     * \p egend of edge \p eg and the latter is set as the incoming slot to \p v.
     * \param[in] v Vertex object to which the new connection is appended.
     * \param[in] eg Edge object to which the new vertex is appended.
     * \param[in] egend End of the edge accepting the new vertex.
     */
    constexpr void connect(
        Vertex& v,  // by reference
        Edge& eg,   // by reference
        End::Id egend
    ) const noexcept;

    /**
     * Disconnects an edge end from a vertex.
     * \details Removes the descriptors in vertex \p v pointing to edge \p eg
     * end \p egend and the other way around. After this operation, the
     * vertex \p v is not assigned to the end \p egend of edge \p eg  any more
     * and the latter is removed from the incoming slots of \p v.
     * \param[in] v Vertex object from which the connection is removed.
     * \param[in] eg Edge object from which the vertex is disconnected.
     * \param[in] egend End of the edge losing the connection.
     */
    constexpr void disconnect(
        Vertex& v,  // by reference
        Edge& eg,   // by reference
        End::Id egend
    ) const noexcept;

    ///@}  // Vertex addition and removal.
#endif
    /**
     * \name Formatted printing.
     * @{
     */

    /**
     * Prints neighbor data on both ends.
     * \param[in] tag Human-readable description common to all records.
     */
    template<bool endLine,
             typename... T>
    auto print_connectivity(T&&... tag) const -> std::size_t;

    /**
     * Prints chain parameters.
     * \param[in] tag Human-readable description prepended to the printed data.
     */
    template<typename... T>
    void print(T&&... tag) const;

    ///@}  // Formatted printing.

    /**
     * Writes the chain to a binary file.
     * \param[in] ofs File stream for writing.
     */
    void write(std::ofstream& ofs) const;

    /**
     * Checks that \p cond is satisfied.
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

template<typename E> constexpr Chain<E> nullChain {ChIdG::undefined};

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
