/* =============================================================================

This file is part of graph-mutator, a lightweight graph transformation library.

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
    static constexpr std::size_t minCycleLength {2};
    static constexpr std::size_t minLength {1};
    static constexpr auto isSpatial = false;

    Edges g;  ///< Edges forming this chain.

    /// Slots connected to this chain at each end.
    Neighbours ngs;

    ChIdG idw {ChIdG::undefined};  ///< Identifier over the whole graph.
    ChIdC idc {ChIdC::undefined};  ///< Identifier over the current component.

    /// Identifier of the current connected component the chain bleongs to.
    CmpId c {CmpId::undefined};

    /**
     * Empty chain constructor without component specs.
     * \param[in] idw Graph-wide index of of this chain.
     */
    explicit constexpr Chain(ChIdG idw);

    /**
     * A chain with specific parameters as a part of a graph.
     * \param[in] idw Graph-wide index of of this chain.
     * \param[in] idc Index of of this chain in the host component.
     * \param[in] c Index of connected component to which the chain belongs.
     */
    explicit constexpr Chain(
        ChIdG idw,
        ChIdC idc,
        CmpId c
    );

    constexpr Chain() = default;
    constexpr Chain(const Chain& other);
    constexpr Chain(Chain&& other);
    constexpr auto operator=(const Chain& other) -> Chain&;
    constexpr auto operator=(Chain&& other) -> Chain&;
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

    constexpr void connect_end_vertex(EndId e, Vertex& v) noexcept;
    constexpr void connect_bulk_vertex(EgIdA a, Vertex& v) noexcept;
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
     * \param[in] ind1 Graph_wide index of the 1st edge.
     * \param[in] ind2 Graph_wide index of the 2nd edge.
     * \note \p ind1 and \p ind2 must be neighbouring edges, both part of
     * this chain
     */
    constexpr auto vertex_at_bulk(EgIdG ind1,
                                  EgIdG ind2) const noexcept -> const Vertex*;
    constexpr auto vertex_at_bulk(const Edge& eg1,
                                  const Edge& eg2) const noexcept -> const Vertex*;
    /**
     * Vertex at slot \p s . Depending on the slot type, returns either internal
     * or boundary vertex.
     * \tparam s Slot type
     * \return Pointer to vertex at slot \p s
     */
    template<typename S>
    constexpr auto vertex_at_slot(const auto& s) const noexcept -> const Vertex*;

    /// Reports degree of the boundary vertex at chain end \p e
    constexpr auto degree_at_end(EndId e) const noexcept -> Degree;

    /// Reports max vertex degree among the vertices of this chain.
    constexpr auto max_degree() const noexcept -> Degree;

    ///@}  // Vertex operations.


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
     * Get pointer to the edge connected at \p end egEnd to the edge at
     * chain position \p a .
     * \param[in] a Intra-chain position of the edge.x
     * \param[in] egEnd Edge internal end.
     */
    constexpr auto connected_edge(
        EgIdA a,
        EndId egEnd
    ) const noexcept -> const Edge*;

    constexpr auto ind2a(const EgIdG ind) const noexcept -> EgIdA;
    constexpr auto ind2eg(const EgIdG ind) const noexcept -> const Edge*;
    constexpr auto ind2eg(const EgIdG ind)       noexcept ->       Edge*;
    constexpr auto indc2a(const EgIdC indc) const noexcept -> EgIdA;
    constexpr auto indc2eg(const EgIdC indc) const noexcept -> const Edge*;
    constexpr auto indc2eg(const EgIdC indc)       noexcept ->       Edge*;

    /**
     * Calculate chain end slot from edge position and edge internal end
     * \param[in] a Edge position within the chain.
     * \param[in] internalEgEnd Edge end in edge internal representatinon.
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

    constexpr auto a2egEnd(EgIdA a) const noexcept -> EndId;

    /**
     * Convert chain end to position index of the boundary edge.
     * \details Converts the chain end of the boundary edge to internal
     * position in the chain.
     * \param[in] e Chain end index.
     * \return Internal positionof the boundary edge at chain end \p e .
     * \return The last edge index in the current connected component.
     */
    constexpr auto end2a(EndId e) const noexcept -> EgIdA;

   /**
     * Edge position to chain end conversion.
     * \details Converts edge position within the chain to the end
     * if the edge is a the boundary edge.
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
     * is on chain boundary. For single-edge chains end cannot be determined.
     * \param[in] ind Graph-wide edge index.
     * \param[in] ege Edge internal end.
     * \return One of the following:
     * Chain end index [1, 2] if chain length > 1 and the edge is at the border;
     * 0 if chain length == 1
     * undefined if edge is not at the border.
     */
    auto ind2end(EgIdG ind,
                 EndId ege = End::undefined) const -> EndId;

    /**
     * Global edge index to chain end conversion.
     * \details Convert graph-wide edge index to chain end if the edge
     * is on chain boundary and the chain is connected at this end.
     * \param[in] ieg Graph-wide edge index.
     * \param[in] nb Index of the connected partner chain.
     * \return One of the following:
     * Chain end index [1, 2] if chain length > 1 and the edge is at the border;
     * 0 if chain length == 1
     * undefined if edge is not at the border.
     */
    auto ind2end(EgIdG ieg,
                 const EndSlot& nb) const -> EndId;

    constexpr auto indc2end(EgIdC indc,
                            EndId egEnd) const noexcept -> EndId;

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
     * Determines if chain is connected to any segments at any of its ends.
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
     * Determines if the chain has one free end.
     * \return The end index if true.
     */
    constexpr bool has_single_leaf_vertex() const noexcept;

    constexpr auto the_only_leaf_end() const noexcept -> EndId;

    /**
     * Determines if at least one of the chain ends is not connected.
     */
    constexpr auto has_a_leaf_end() const noexcept -> std::pair<bool, EndId>;

    /**
     * Checks if the chain has its both ends unconnected.
     * \details This chain is unconnected iff it is
     * not connected its boundary vertices.
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
     * \details A chain is cycled onto itself if it is its ends are connected
     * to each other.
     * \return \a true if the chain is cycled onto itself, \a false otherwise.
     */
    constexpr bool is_cycle() const noexcept;

    /**
     * Checks if an edge can be deleted from the chain.
     * \details An edge can be deleted from the chain if the chain length is
     * larger than the minimal length alowed for the given chain topology.
     * \return \a true if an edge can be deleted from the chain,
     *         \a false otherwise.
     */
    constexpr bool is_shrinkable() const noexcept;

    constexpr bool is_unconnected_cycle_boundary(const Vertex& v) const noexcept;

    constexpr bool is_cycle_boundary(const Vertex& v) const noexcept;

    constexpr bool edge_points_forwards(EgIdA a) const noexcept;

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

    /**
     * Fiind if the given edge is at the chain head.
     * \param ind Edge index in graph scope.
     * \param egEnd Inner end of the edge at \p ind
     */
    constexpr bool egEnd_is_head(EgIdG ind,
                                 EndId egEnd) const noexcept;

    /**
     * Fiind if the given edge is at the chain tail.
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
     * \name Vertex addition and removal.connect_bulk_vertex
     * @{
     */

    /**
     * Connects an edge end and a vertex.
     * \details Arranges descriptors in vertex \p v and in edge \p eg to match
     * each other. After this operation, the vertex \p v is assigned to the end
     * \p egend of edge \p eg and the latter is set as the incoming slot to \p v.
     * \param[in] v Vertex object to which the new connection is appended.
     * \param[in] eg Edge object to which the new vertex is appended.
     * \param[in] egend End of the edge accepting the the new vertex.
     */
    constexpr void connect(
        Vertex& v,  // by reference
        Edge& eg,   // by reference
        End::Id egend
    ) const noexcept;

    /**
     * Disconnects an edge end from a vertex.
     * \details Removes the descriptors in vertex \p v pointing to edge \p eg
     * end \p egend and the other vay around. After this operation, the
     * vertex \p v is not assigned to the end \p egend of edge \p eg  any more
     * and the latter is removed from the incoming slots of \p v.
     * \param[in] v Vertex object from which the connection is removed.
     * \param[in] eg Edge object from which the new vertex is desconnected.
     * \param[in] egend End of the edge losing the the connection.
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
     * Checks that \p cond is satisfied, else terminates program.
     * \details Prints out the component using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * \tparam T Types of the message arguments.
     * \param[in] cond Condition to be checked.
     * \param[in] tag Tag to the component data to be printed if \p cond is false.
     * \param[in] message Arguments bound to form the message printed at
     *                    termination if the condition does not hold.
     */
    template<typename... T>
    void ensure(
        bool cond,
        std::string_view tag,
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
Chain<E>::
Chain(Chain&& other)
    : g {std::move(other.g)}
    , ngs {std::move(other.ngs)}
    , idw {std::move(other.idw)}
    , idc {std::move(other.idc)}
    , c {std::move(other.c)}
{}


template<typename E>
constexpr
Chain<E>::
Chain(const Chain& other)
    : g {other.g}
    , ngs {other.ngs}
    , idw {other.idw}
    , idc {other.idc}
    , c {other.c}
{}


template<typename E>
constexpr
auto Chain<E>::
operator=(Chain&& other) -> Chain&
{
    g = std::move(other.g);
    ngs = std::move(other.ngs);
    idw = std::move(other.idw);
    idc = std::move(other.idc);
    c = std::move(other.c);

    return *this;
}


template<typename E>
constexpr
auto Chain<E>::
operator=(const Chain& other) -> Chain&
{
    g = other.g;
    ngs = other.ngs;
    idw = other.idw;
    idc = other.idc;
    c = other.c;

    return *this;
}


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
void Chain<E>::
connect_bulk_vertex(const EgIdA a, Vertex& v) noexcept
{
    ASSERT(a < length(), "End not valid.");

    g[a-1].connect_to_outer(End::B, v);
    g[a  ].connect_to_outer(End::A, v);
}


template<typename E>
constexpr
void Chain<E>::
connect_end_vertex(const EndId e, Vertex& v) noexcept
{
    ASSERT(End::is_valid(e), "End not valid.");

    end_edge(e).connect_to_outer(e, v);
}


template<typename E>
constexpr
void Chain<E>::
disconnect_end_vertex(const EndId e) noexcept
{
    ASSERT(End::is_valid(e), "End not valid.");

    end_edge(e).disconnect_end_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_end(const EndId e) noexcept -> Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).vertex_at_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_end(const EndId e) const noexcept -> const Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).vertex_at_outer(e);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdA a) const noexcept -> const Vertex*
{
    ASSERT(a > 0 , a, " is not a bulk position");

    return g[a].vertex_at_outer(End::A);
}

template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdA a) noexcept -> Vertex*
{
    ASSERT(a > 0, a, " is not a bulk position");

    return g[a].vertex_at_outer(End::A);
}


template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const EgIdG ind1,
               const EgIdG ind2) const noexcept -> const Vertex*
{
    const auto eg1 = ind2eg(ind1);

    ASSERT(eg1, "Edge ind ", ind1, " not found in chain ", idw);

    const auto eg2 = ind2eg(ind2);

    ASSERT(eg2, "Edge ind ", ind2, " not found in chain ", idw);

    return vertex_at_bulk(eg1, eg2);
}

template<typename E>
constexpr
auto Chain<E>::
vertex_at_bulk(const Edge& eg1,
               const Edge& eg2) const noexcept -> const Vertex*
{
    ASSERT(eg1.ind != eg2.ind, "Identical inds: ", eg1.ind);

    ASSERT(eg1->indw == eg2->indw + 1 || eg2->indw == eg1->indw + 1,
           "Edges at a1 ", eg1->indw, " a2 ", eg2->indw, " are expected ",
           "to have a common bulk vertex");

    return eg1->indw < eg2->indw
        ? eg2->vertex_at_outer(End::A)
        : eg1->vertex_at_outer(End::A);
}


template<typename E>
template<typename S>
constexpr
auto Chain<E>::
vertex_at_slot(const auto& s) const noexcept -> const Vertex*
{
    const auto [w, ea] = s.we();

    if constexpr (std::is_same_v<S, EndSlot>)
        return vertex_at_end(ea);
    else
        return vertex_at_bulk(ea);
}

template<typename E>
constexpr
auto Chain<E>::
degree_at_end(EndId e) const noexcept -> Degree
{
    return ngs[e].num() + 1;
}

template<typename E>
constexpr
auto Chain<E>::
max_degree() const noexcept -> Degree
{
    return std::max(std::max(degree_at_end(End::A),
                             degree_at_end(End::B)),
                    (length() > 1) ? Deg2 : Deg0);
}


template<typename E>
bool Chain<E>::
has_edge(const EgIdG ind) const noexcept
{
    for (const auto& eg : g)
        if (eg.ind == ind)
            return true;

    return false;
}


template<typename E>
constexpr
auto Chain<E>::
end_edge(const EndId e) const noexcept -> const Edge&
{
    ASSERT(End::is_valid(e), "End not valid.");

    return e == endA ? g.front()
                     : g.back();
}


template<typename E>
constexpr
auto Chain<E>::
end_edge(const EndId e) noexcept -> Edge&
{
    ASSERT(End::is_valid(e), "End not valid.");

    return e == endA ? g.front()
                     : g.back();
}


// Convert chain edge index and edge end in chain internal to edge end
template<typename E>
constexpr
auto Chain<E>::
edge_internal_end(const EgIdA a, const EndId e) const noexcept -> EndId
{
    ASSERT(End::is_valid(e), "End not valid.");

    return g[a].inner_endId(e);
}


// Convert chain boundary id to edge internal end id
template<typename E>
constexpr
auto Chain<E>::
edge_internal_end_at_end(const EndId e) const noexcept -> EndId
{
    ASSERT(End::is_valid(e), "End not valid.");

    return end_edge(e).inner_endId(e);
}


template<typename E>
constexpr
bool Chain<E>::
egEnd_is_head(const EgIdG ind,
              const EndId egEnd) const noexcept
{
    ASSERT(End::is_valid(egEnd), "Edge end not valid.");

    return is_headind(ind) &&
        (( head().points_forwards() && egEnd == End::B) ||
         (!head().points_forwards() && egEnd == End::A));
}


template<typename E>
constexpr
bool Chain<E>::
egEnd_is_tail(const EgIdG ind,
              const EndId egEnd) const noexcept
{
    ASSERT(End::is_valid(egEnd), "Edge end not valid.");

    return is_tailind(ind) &&
        (( tail().points_forwards() && egEnd == End::A) ||
         (!tail().points_forwards() && egEnd == End::B));
}


template<typename E>
constexpr
auto Chain<E>::
connected_edge(
    const EgIdA a,
    const EndId ege
) const noexcept -> const Edge*
{
    ASSERT(!egEnd_is_tail(g[a].ind, ege),
           "Edge end ", End::str(ege), " at ", a, ", w ", idw, " is chain tail");
    ASSERT(!egEnd_is_head(g[a].ind, ege),
           "Edge end ", End::str(ege), " at ", a, ", w ", idw, " is chain head");


    const auto pf = g[a].points_forwards();

    if (a < length() - 1) {
        if (( pf && ege == End::B) ||
            (!pf && ege == End::A))
            return &g[a+1];
    }

    if (a > 0) {
        if (( pf && ege == End::A) ||
            (!pf && ege == End::B))
            return &g[a-1];
    }

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
ind2a(const EgIdG ind) const noexcept -> EgIdA
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return a;

    return EgIdA::undefined;
}


template<typename E>
constexpr
auto Chain<E>::
ind2eg(const EgIdG ind) const noexcept -> const Edge*
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
ind2eg(const EgIdG ind) noexcept -> Edge*
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].ind == ind)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
indc2a(const EgIdC indc) const noexcept -> EgIdA
{
    for (std::size_t a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return a;

    return EgIdA::undefined;
}

template<typename E>
constexpr
auto Chain<E>::
indc2eg(const EgIdC indc) const noexcept -> const Edge*
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return &(g[a]);

    return nullptr;
}

template<typename E>
constexpr
auto Chain<E>::
indc2eg(const EgIdC indc) noexcept -> Edge*
{
    for (EgIdA a {}; a < length(); ++a)
        if (g[a].indc == indc)
            return &(g[a]);

    return nullptr;
}


template<typename E>
constexpr
auto Chain<E>::
neig_a_of_egEnd(
    const EgIdA a,    // edge position inside the chain
    const EndId ee    // edge end
) const noexcept -> EgIdA
{
    const auto shift = g[a].points_forwards() ? (ee == endB ? 1 : -1)
                                              : (ee == endB ? -1 : 1);
    const auto b = static_cast<EgIdA>(a + shift);

    return b < length() ? b : EgIdA::undefined;
}


// Finds edge end of the edge \p b to which the edge at
// \p a is connected via a vertex internal to the chain.
// Should only be applied to the chain internal vertices.
template<typename E>
constexpr
auto Chain<E>::
internal_egEnd(
    const EgIdA a,   // chain position of the 1st edge
    const EgIdA b    // chain position of the 2nd edge
) const noexcept -> EndId
{
    ASSERT(a.is_defined() && b.is_defined(),
           "a = ", a, ", b = ", b, " are not chain-wide edge indexes");
    ASSERT(a == b + 1 || b == a + 1,
           "a = ", a, ", b = ", b, " do not share a vertex");

    return g[b].points_forwards() ? (a > b ? endB : endA)
                                  : (a > b ? endA : endB);
}


// Edge end of an edge a
template<typename E>
constexpr
auto Chain<E>::
a2egEnd(const EgIdA a) const noexcept -> EndId
{
    return g[a].outer_endId(endA);
}


/*
template<typename E>
constexpr
auto Chain<E>::
end(const EgId indc,
    const EndId egEnd) const noexcept -> EndId
{
    ASSERT(g.front().indc == indc || g.back().indc == indc,
           "edge having component index ", indc, " is not an at chain end");

    const auto a = g[0].indc == indc
                 ? 0
                 : length() - 1;
    const auto isForward = g[a].points_forwards();

    return a == 0
        ? (egEnd == End::A ? (isForward ? End::A : End::B)
                            : (isForward ? End::B : End::A))
        : (isForward ? End::B : End::A);
}
*/


template<typename E>
constexpr
auto Chain<E>::
end2a(const EndId e) const noexcept -> EgIdA
{
    return e == endA ? 0 : length() - 1;
}


template<typename E>
constexpr
auto Chain<E>::
a2end(const EgIdA a) const noexcept -> EndId
{
    ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());
    ASSERT(a == 0 || a == length() - 1, "a = ", a, " is not an end edge.");

    return length() == 1
        ? endUndef            // end is undefined for single-edge chains
        : a == 0 ? endA
                 : a == length() - 1 ? endB
                                     : endUndef;
}


template<typename E>
constexpr
auto Chain<E>::
egEnd_to_bulkslot(const EndId egE,
                  const EgIdA a) const noexcept -> BulkSlot
{
    ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());

    const auto frw = g[a].points_forwards();

    return egE == endA ? (frw ? BulkSlot{idw, a}
                              : BulkSlot{idw, a + 1})
                       : (frw ? BulkSlot{idw, a + 1}
                              : BulkSlot{idw, a});
}


template<typename E>
constexpr
auto Chain<E>::
end2ind(const EndId e) const noexcept -> EgIdG
{
    return end_edge(e).ind;
}


template<typename E>
auto Chain<E>::
ind2end(const EgIdG ind,                  // graph-wide edge index
        const EndId ege) const -> EndId   // edge internal end
{
    if (length() == 1) {
        if (ind == g[0].ind)
            return g[0].outer_endId(ege);

        print("ABORTED");
        abort(ind, " is not an end edge ind of chain ", idw);

        return endUndef;  // unreachable
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    print("ABORTED");
    abort(ind, " is not an end edge ind of chain ", idw);

    return endUndef;  // unreachable
}


template<typename E>
auto Chain<E>::
ind2end(const EgIdG ind,                    // graph-wide edge index
        const EndSlot& nb) const -> EndId   // EndSlot connected to the end
{
    if (length() == 1) {
        if (ind == g.front().ind && ngs[endA].contains(nb))
            return endA;
        else if (ind == g.back().ind && ngs[endB].contains(nb))
            return endB;

        print("ABORTED");
        abort(ind, " is not an end edge ind of chain ", idw);

        return endUndef;  // unreachable
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    print("ABORTED");
    abort(ind, " is not an end edge ind of chain ", idw);

    return endUndef;  // unreachable
}


template<typename E>
constexpr
auto Chain<E>::
indc2end(const EgIdC indc,
         const EndId ege) const noexcept -> EndId
{
    if (length() == 1) {
        if (indc == g[0].indc)
            return g[0].outer_endId(ege);

        print("ABORTED");
        abort(indc, " is not an end edge indc of chain ", idw);

        return endUndef;  // unreachable
    }

    if (indc == g.front().indc)
        return endA;

    if (indc == g.back().indc)
        return endB;

    print("ABORTED");
    abort(indc, " is not an end edge indc of chain ", idw);

    return endUndef;  // unreachable
}


template<typename E>
constexpr
auto Chain<E>::
get_single_connected_end() const -> EndId
{
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endA;
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endB;

    print("ABORTED");
    abort("Error in Chain::get_single_connected_end(): ",
          "the connected end is not single!");

    return endUndef;  // unreachable
}

template<typename E>
constexpr
auto Chain<E>::
get_single_leaf_end() const -> EndId
{
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    print("ABORTED");
    abort("Error in Chain::get_single_leaf_end(): ",
          "the free end is not single!");

    return endUndef;  // unreachable
}


template<typename E>
constexpr
auto Chain<E>::
get_single_3way_end() const -> EndId
{
    if (ngs[endA].num() == 2 && ngs[endB].num() != 2) return endA;
    if (ngs[endA].num() != 2 && ngs[endB].num() == 2) return endB;

    print("ABORTED");
    abort("Error in Chain::get_single_3way_end(): ",
          "the 3-way end is not single!");

    return endUndef;  // unreachable
}


template<typename E>
constexpr
auto Chain<E>::
get_single_4way_end() const -> EndId
{
    if (ngs[endA].num() == 3 && ngs[endB].num() != 3) return endA;
    if (ngs[endA].num() != 3 && ngs[endB].num() == 3) return endB;

    print("ABORTED");
    abort("Error in Chain::get_single_4way_end(): ",
          "the 4-way end is not single!");

    return endUndef;  // unreachable
}


template<typename E>
constexpr
bool Chain<E>::
has_single_leaf_vertex() const noexcept
{
    const auto a = is_connected_at(endA);
    const auto b = is_connected_at(endB);

    if ((!a &&  b) ||
        ( a && !b)) return true;

    return false;
}

template<typename E>
constexpr
auto Chain<E>::
the_only_leaf_end() const noexcept -> EndId
{
    ASSERT(has_single_leaf_vertex(),
            "chain ", idw, " does not have single leaf vertex");

    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    return endUndef;
}


template<typename E>
constexpr
auto Chain<E>::
has_a_leaf_end() const noexcept -> std::pair<bool, EndId>
{
    // return the end index if true

    if (!is_connected_at(endA))
        return {true, endA};

    if (!is_connected_at(endB))
        return {true, endB};

    return {false, endUndef};
}


template<typename E>
constexpr
auto Chain<E>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    ASSERT(End::is_valid(e), "End ", e, " is not valid");

    return End::opp(e);
}


template<typename E>
constexpr
bool Chain<E>::
edge_points_forwards(const EgIdA a) const noexcept
{
    ASSERT(a < length(),
           "a ", a, " is not valid for length: ", length(), " of chain ", idw);

    return g[a].points_forwards();
}


template<typename E>
constexpr
auto Chain<E>::
end_slot(const EgIdA a,
         const EndId internalEgEnd) const noexcept -> EndSlot
{

    ASSERT(a < length(),
           "a ", a, " is not valid for length: ", length(), " of chain ", idw);

    const auto is_valid = is_head(a) || is_tail(a);

    ASSERT(is_valid, "Edge pos ", a, " is not at chain end");

    return is_valid ? EndSlot{idw, g[a].outer_endId(internalEgEnd)}
                    : EndSlot{};

}


template<typename E>
constexpr
bool Chain<E>::
is_connected_at(const EndId e) const noexcept
{
    ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    return ngs[e].num() > 0;
}

template<typename E>
constexpr
bool Chain<E>::
is_connected() const noexcept
{
    return ngs[endA].num() > 0 || ngs[endB].num() > 0;
}

template<typename E>
constexpr
bool Chain<E>::
is_unconnected_linear() const noexcept
{
    return !ngs[endA].num() || !ngs[endB].num();
}


template<typename E>
constexpr
bool Chain<E>::
is_unconnected_cycle() const noexcept
{
//    const auto vA = vertex_at_end(endA);
//    const auto vB = vertex_at_end(endB);
//    const auto A = vA->deg() == 2 && vA == vB;

//    const auto B = ngs[endA].num() == 1 &&
//           ngs[endB].num() == 1 &&
//           ngs[endA][0] ==
//           ngs[endB][0].opp();

//    ASSERT(A == B, " B ", B, " vA->deg() ", vA->deg(),  " Addr vA ", vA, " addr vB ", vB);
//    const auto vA = vertex_at_end(endA);
//    const auto vB = vertex_at_end(endB);

//    return vA->deg() == 2 && vA == vB;
    return ngs[endA].num() == 1 &&
           ngs[endB].num() == 1 &&
           *ngs[endA].begin() ==
           (*ngs[endB].begin()).opp();
}


template<typename E>
constexpr
bool Chain<E>::
is_connected_cycle() const noexcept
{
    if (ngs[endA].num() > 1) {

        if (!is_connected_at(endB))
            return false;

        for (const auto& sA : ngs[endA]())
            for (const auto& sB : ngs[endB]())
                if (sA ==sB)
                    return true;
    }

    return false;
}


template<typename E>
constexpr
bool Chain<E>::
is_cycle() const noexcept
{
    return is_unconnected_cycle() ||
           is_connected_cycle();
}


template<typename E>
constexpr
bool Chain<E>::
is_shrinkable() const noexcept
{
    if (has_single_leaf_vertex())
        return true;

    const auto l = length();

    return is_cycle() ? l > minCycleLength
                      : l > minLength;
}

template<typename E>
constexpr
bool Chain<E>::
is_unconnected_cycle_boundary(const Vertex& v) const noexcept
{
    return is_unconnected_cycle() && vertex_at_end(endA)->operator==(v);
}

template<typename E>
constexpr
bool Chain<E>::
is_cycle_boundary(const Vertex& v) const noexcept
{
    return is_cycle() && vertex_at_end(endA)->operator==(v);
}

template<typename E>
bool Chain<E>::
has_such_neig(const EndId e,
              const ChIdG n) const noexcept
{
    ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    for (const auto& s : ngs[e]())
        if (s.w == n)
            return true;

    return false;
}


template<typename E>
constexpr
auto Chain<E>::
num_neigs(const EndId e) const noexcept -> std::size_t
{
    ASSERT(End::is_valid(e), "End ", e, " is not valid, chain ", idw);

    return ngs[e].num();
}


template<typename E>
constexpr
auto Chain<E>::
num_vertices(const Degree d) const noexcept -> std::size_t
{
    ASSERT(is_implemented_degree(d), "Degree ", d, " is not implemented");

    if (d == Deg1) {  // count vertices of degree 1

        if ( is_connected_at(endA) &&  is_connected_at(endB)) return 0;
        if (!is_connected_at(endA) && !is_connected_at(endB)) return 2;
        /* else */                                            return 1;
    }

    if (d == Deg2)  // count internal chain vertices

        return is_unconnected_cycle() ? length() : length() - 1;

    // Count vertices of degree > 2

    const std::size_t N {containers::num_neigs(d)};
    const auto nA = ngs[endA].num();
    const auto nB = ngs[endB].num();

    if (nA == N && nB == N) return 2;
    if (nA == N || nB == N) return 1;
    if (nA != N && nB != N) return 0;

    return utils::undefined<size_t>;
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
auto Chain<E>::
vertices() const noexcept -> std::vector<const Vertex*>
{
    std::vector<const Vertex*> w;
    for (const auto& eg : g)
        for (const auto e : End::Ids)
            w.push_back(eg.vertex_at_outer(e));

    return w;
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

#if 0
template<typename E>
constexpr
void Chain<E>::
connect(Vertex& v,
        Edge& eg,
        const EndId egend) const noexcept
{
    v.insert(std::move(EdgeSlot{eg.ind, egend}));
    eg.connect_to_inner(egend, v);
}

template<typename E>
constexpr
void Chain<E>::
disconnect(Vertex& v,
           Edge& eg,
           const EndId egend) const noexcept
{
    v.erase_edge_descriptor(EdgeSlot{eg.ind, egend});
    eg.disconnect_end_inner(egend);
}
#endif

template<typename E>
template<bool endLine, typename... T>
auto Chain<E>::
print_connectivity(T&&... tag) const -> std::size_t
{
    std::string s;
    if constexpr (sizeof...(T))
        s = Msgr::oss(tag...).str();

    jot<false>(colorcodes::MAGENTA, s, " ", colorcodes::BOLDRED,
               idw, colorcodes::RESET);

    ngs[endA].print();
    jot<false>("**");
    ngs[endB].print();

    if constexpr (endLine)
        jot("");

    return s.length();
}


template<typename E>
template<typename... T>
void Chain<E>::
print(T&&... tag) const
{
    const auto tag_length = print_connectivity<false>(tag...);

    jot<false>(colorcodes::BOLDCYAN, c, colorcodes::RESET, " [", idc, "]");

    jot<false>(" len ", colorcodes::GREEN, length(), colorcodes::RESET);

    for (std::size_t i {}; i<length(); ++i) {
        if (i && i % 10 == 0)
            jot<false>(' ');
        jot<false>(g[i].points_forwards() ? '>' : '<');
    }

    jot<false>('(');
    for (const auto& eg : g)
        jot<false>(eg.ind);
    jot(')');

    if constexpr (print_edges) {
        const std::string spaces(tag_length, ' ');
        for (const auto& eg : g)
            eg.print(spaces);
    }
    if constexpr (print_vertices)
        for (EgIdA aa {}; const auto v : vertices()) {
            const auto a = aa() / 2;  // integer division
            jot<false>("[",
                       colorcodes::YELLOW,
                       a, " ",
                       (aa() % 2) ? End::str(endB) : End::str(endA),
                       colorcodes::RESET,
                       " ind ", colorcodes::GREEN, g[a].ind, colorcodes::RESET,
                       " indc ", colorcodes::MAGENTA, g[a].indc, colorcodes::RESET,
                       "]: ");
            v ? v->print("") : jot("unconnected");
            ++aa;
        }
}


template<typename E>
void Chain<E>::
write(std::ofstream& ofs) const
{
    const auto len = length();
    ofs.write(reinterpret_cast<const char*>(&len), sizeof(len));
    ofs.write(reinterpret_cast<const char*>(&idw), sizeof(ChIdG));
    ofs.write(reinterpret_cast<const char*>(&idc), sizeof(ChIdC));
    ofs.write(reinterpret_cast<const char*>(&c), sizeof(CmpId));

    for (const auto& ng : ngs)
        ng.write(ofs);

    for (const auto& a : g)
        a.write(ofs);
}


template<typename E>
template<typename... T>
void Chain<E>::
ensure(
    const bool cond,
    const std::string_view tag,
    T&&... message
) const
{
    if (!cond) {
        print(tag);
        ABORT(cond, message...);
    }
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

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_CHAIN_H
