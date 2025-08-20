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
 * @file chain.h
 * @brief Contains Chain class template and its specialization.
 * @details Only graphs containing vertices of max degree 4 are implemented.
 * A related implementation for max degree 3 is available at
 * https://github.com/vsukhor/mitoSim/blob/master/include/segment.h
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_CHAIN_H
#define GRAPH_MUTATOR_STRUCTURE_CHAIN_H

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "../definitions.h"
#include "edge.h"
#include "ends.h"
#include "neigs.h"
#include "slot.h"
#include "vertices/degrees.h"
#include "vertices/vertex.h"

namespace graph_mutator::structure {


/**
 * @brief Chain template for Edge classes having max vertex degree equal to 4.
 * @details Chain is a sequence of edges linked linearly (without branches).
 * Chain ends may form branching sites, if connected to other chains.
 * A chain not connected to other chains or a complete collection of chains
 * connected to each other form a disconnected graph component (aka
 * 'component'). The class handles the tasks and properties specific to a single
 * chain and its relation to other graph components.
 * @tparam E4 Edge class having max vertex degree 4.
 */
template<typename E4>
struct Chain {

    static_assert(E4::maxVertexDegree == maxDegree);
    static_assert(std::is_base_of_v<structure::EdgeBase, E4>);

    using Id = ChId;
    using Edge = E4;
    using Edges = std::vector<Edge>;
    using EdgeEnds = E4::Ends;  ///< Edge ends.
    using Ends = structure::Ends;
    using Bulks = structure::Bulks;
    using EndId = Ends::Id;
    using EndSlot = _Slot<Id, Ends>;
    using BulkSlot = _Slot<Id, Bulks>;
    using Neigs = structure::Neigs<EndSlot>;  ///< Slots connected to the chain ends.
    using thisT = Chain<Edge>;

    template<Degree D>
    using Slot = std::conditional_t<D == Deg2,
                                    BulkSlot,
                                    EndSlot>;

    // A chain is a linear sequence of edges, so it has 2 ends.
    static constexpr auto endA {Ends::A};
    static constexpr auto endB {Ends::B};
    static constexpr auto endUndef {Ends::Undefined};
    static constexpr EgId minCycleLength {2};
    static constexpr EgId minLength {1};

    Edges g;  ///< Edges forming this chain.

    /// Slots connected to this chain at each end.
    std::array<Neigs, Ends::num> ngs;

    Id idw {undefined<Id>};  ///< The chain index over whole graph.
    Id idc {undefined<Id>};  ///< The chain index over the component.

    /// Index of the disconnected component the chain bleongs to.
    CmpId c {undefined<CmpId>};

    /**
     * @brief Empty chain constructor without component specs.
     * @param idw Graph-wide index of of this chain.
     */
    explicit constexpr Chain(
        Id idw
    );

    /**
     * @brief A chain of edges without component specs.
     * @param length Number of edges in the chain.
     * @param idw Graph-wide index of of this chain.
     * @param ei Index of the last edge in this chain.
     */
    explicit constexpr Chain(
        EgId length,
        Id idw,
        EgId ei
    );

    /**
     * @brief A chain with specific parameters as a part of a graph.
     * @param length Number of edges in the chain.
     * @param c Index of disconnected component to which the chain belongs.
     * @param idw Graph-wide index of of this chain.
     * @param idc Index of of this chain in the host component.
     * @param ei Index of the last edge in this chain.
     */
    //explicit constexpr Chain(
    //    EgId length,
    //    CmpId c,
    //    Id idw,
    //    Id idc,
    //    EgId ei
    //);

    constexpr Chain() = default;
    constexpr explicit Chain(const Chain& other);
    constexpr explicit Chain(Chain&& other);
    constexpr auto operator=(const Chain& other) -> Chain&;
    constexpr auto operator=(Chain&& other) -> Chain&;
    ~Chain() = default;

    constexpr auto operator==(const Chain& other) const noexcept -> bool;

    /// Reflects the chain edges.
    void reverse_g();

    /**
     * @brief Change component index keeping the chain index unaltered.
     * @details Change disconnected graph component-related indexes of the
     * chain edges, keeping the chain index unoltered.
     * @param newc New disconnected component index.
     * @param initindc Starting edge index in the current disconnected component.
     */
    auto set_g_cmp(CmpId newc, EgId initindc) noexcept -> EgId;

    /**
     * @brief Change component index.
     * @details Change disconnected graph component-related indexes of the
     * chain edges, and the the chain itself.
     * @param newc New disconnected component index.
     * @param initindc Starting edge index in the current disconnected component.
     * @return The last edge index in the current disconnected graph component.
     */
    auto set_cmpt(CmpId newc,
                  ChId newidc,
                  EgId initindc) noexcept -> EgId;

    auto has_edge(const EgId ind) const noexcept -> bool;

    /**
     * @brief Get the edge at the specified end.
     * @param e Chain end index.
     * @return Reference to the edge at the specified end.
     */
    constexpr auto end_edge(EndId e) const noexcept -> const Edge&;

    /**
     * @brief Get the edge end at the specified chain boundary.
     * @param e Chain end index.
     * @return Edge end at the specified chain boundary.
     */
    constexpr auto edge_end(EndId e) const noexcept -> EndId;

    constexpr auto connected_edge(
        const EgId a,
        const EndId egEnd
    ) const noexcept -> Edge const*;

    /**
     * @brief Position of the edge neigh to egEnd \p ee of edge at \p a.
     * @note Should only be applied to the chain internal vertices.
     * @return Position in chain of the edge neigh to egEnd \p ee of edge at
     * \p a if \p ee is at internal chain vertex; undefined otherwise.
     */
    constexpr auto neig_a_of_egEnd(
        const EgId a,     ///< Edge position inside the chain.
        const EndId ee    ///< Edge end.
    ) const noexcept -> EgId;

    /**
     * @brief Edge end of an internal vertex.
     * @details Finds edge end of the edge \p b to which the edge at
     * \p a is connected via a vertex internal to the chain.
     * @note Should only be applied to the chain internal vertices.
     * Then, the searched edge end is unique.
     */
    constexpr auto internal_egEnd(
        const EgId a,
        const EgId b
    ) const noexcept -> EndId;

    constexpr auto a2egEnd(EgId a) const noexcept -> EndId;

    /**
     * @brief Convert chain end to position index of the boundary edge.
     * @details Converts the chain end of the boundary edge to internal
     * position in the chain.
     * @param e Chain end index.
     * @return Internal position.
     * @return The last edge index in the current disconnected component.
     */
    constexpr auto end2a(EndId e) const noexcept -> EgId;

    template<EndId E>
    constexpr auto end2a() const noexcept -> EgId;

   /**
     * @brief Edge position to chain end conversion.
     * @details Converts edge position within the chain to the end
     * if the edge is a the boundary edge.
     * @param a Edge position within the chain.
     */
    constexpr auto a2end(EgId a) const noexcept -> EndId;

    /**
     * @brief Convert edge end to bulk slot.
     * @details Maps the edge end to the corresponding bulk slot.
     * @param egE Edge end.
     * @param a Edge position in a chain.
     * @return Corresponding bulk slot.
     * @note The edge end must be in the internal of the chain.
     */
    constexpr auto egEnd_to_bulkslot(EndId egE,
                                     EgId a) const noexcept -> BulkSlot;

    /**
     * @brief Chain end to global edge index conversion.
     * @details Converts chain end to the boundary edge graph-wide index.
     * @param e Chain end index.
     */
    constexpr auto end2ind(EndId e) const noexcept -> EgId;

    /**
     * @brief Global edge index to chain end conversion.
     * @details Convert global edge index to chain end if the edge
     * is on chain boundary. For single-edge chains end cannot be determined.
     * @param ind Graph-wide edge index.
     * @param ege Edge end.
     * @return One of the following:
     * Chain end index [1, 2] if chain length > 1 and the edge is at the border;
     * 0 if chain length == 1
     * undefined if edge is not at the border.
     */
    auto ind2end(EgId ind,
                 EndId ege=Ends::Undefined) const -> EndId;

    /**
     * @brief Global edge index to chain end conversion.
     * @details Convert global edge index to chain end if the edge
     * is on chain boundary and the chain is connected at this end.
     * @param ieg Graph-wide edge index.
     * @param nb Index of the connected partner chain.
     * @param ne End with which the connected partner chain is conected.
     * @return One of the following:
     * Chain end index [1, 2] if chain length > 1 and the edge is at the border;
     * 0 if chain length == 1
     * undefined if edge is not at the border.
     */
    auto ind2end(EgId ieg, const EndSlot& nb) const -> EndId;

    constexpr auto indc2end(const EgId indc,
                            const EndId egEnd) const noexcept -> EndId;

    /**
     * @brief Determines if the chain is connected at its end 'e' to chain 'n'.
     * @param e End index at the current chain.
     * @param n Chain index of the eventual neighbor.
     */
    auto has_such_neig(EndId e, Id n) const noexcept -> bool;

    /**
     * @brief Return index of the chain end opposite to the end \p e .
     * @param e Chain end index.
     * @return Opposite end index.
     */
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    /**
     * @brief Determines if end \p e has any connected segments.
     * @return \a true if end \p e has any connected segments, \a false otherwise.
     */
    constexpr auto is_connected_at(EndId e) const noexcept -> bool;

    /**
     * @brief Index of the connected end for a chain connected at one end.
     */
    constexpr auto get_single_connected_end() const -> EndId;

    /**
     * @brief Index of the unconnected end for a chain connected at one end.
     */
    constexpr auto get_single_free_end() const -> EndId;

    /**
     * @brief Index of the end connected via a 3-way junction.
     * @note Is valid for chains having single such connection only.
     */
    constexpr auto get_single_3way_end() const -> EndId;

    /**
     * @brief Determines if the chain has one free end.
     * @return The end index if true.
     */
    constexpr auto has_one_free_end() const noexcept -> bool;

    constexpr auto the_only_free_end() const noexcept -> EndId;

    /**
     * @brief Determines if at least one of the chain ends is not connected.
     */
    constexpr auto has_a_free_end() const noexcept -> std::pair<bool, EndId>;

    /**
     * @brief Checks if the chain is unconnected and cycled onto itself.
     * @details A chain is unconnected and cycled onto itself if it is
     * not connected to chains other than itself, while its ends are connected
     * to each other.
     * @return \a true if the chain is unconnected and cycled onto itself,
     *         \a false otherwise.
     */
    constexpr auto is_disconnected_cycle() const noexcept -> bool;

    /**
     * @brief Checks if the chain is connected and cycled onto itself.
     * @details A chain is connected and cycled onto itself if it is
     * connected to chains other than itself, while its ends are also connected
     * to each other.
     * @return \a true if the chain is connected and cycled onto itself,
     *         \a false otherwise.
     */
    constexpr auto is_connected_cycle() const noexcept -> bool;

    /**
     * @brief Checks if the chain is cycled onto itself.
     * @details A chain is cycled onto itself if it is its ends are connected
     * to each other.
     * @return \a true if the chain is cycled onto itself, \a false otherwise.
     */
    constexpr auto is_cycle() const noexcept -> bool;

    /**
     * @brief Checks if an edge can be deleted from the chain.
     * @details An edge can be deleted from the chain if the chain length is
     * larger than the minimal length alowed for the given chain topology.
     * @return \a true if an edge can be deleted from the chain,
     *         \a false otherwise.
     */
    constexpr auto is_shrinkable() const noexcept -> bool;

    constexpr auto edge_points_forwards(EgId a) const noexcept -> bool;

    /**
     * @brief Reports the number of vertices of a given degree.
     * @tparam D Vertex degree.
     * @return the Number of vertices.
     */
    template<Degree D> requires is_implemented_degree<D>
    constexpr auto num_vertices() const noexcept -> szt;

    /**
     * @brief Reports the chain length measured in edges.
     * @details The chain length is equal to the number of edges in the chain.
     * @return Chain length measured in edges.
     */
    constexpr auto length() const noexcept -> EgId;

    /**
     * @brief Reports the total weight of the chain edges.
     * @details The chain weight is equal to the sum of the weights of all edges
     * in the chain.
     * @return Chain weight measured in units of Edge::weight_t.
     */
    constexpr auto weight() const noexcept -> typename Edge::weight_t;


    /**
     * @brief Returns the head edge of the chain.
     * @details The head edge is the last edge in the chain.
     * @return Const reference to the head edge.
     */
    constexpr auto head() const noexcept -> const Edge& { return g.back(); }

    /**
     * @brief Returns the tail edge of the chain.
     * @details The tail edge is the first edge in the chain.
     * @return Const reference to the tail edge.
     */
    constexpr auto tail() const noexcept -> const Edge& { return g.front(); }

    constexpr auto headind() const noexcept -> EgId { return head().ind; }
    constexpr auto tailind() const noexcept -> EgId { return tail().ind; }

    static constexpr auto is_tail(const EgId a) noexcept -> bool { return a == 0; }
    constexpr auto is_head(const EgId a) const noexcept -> bool { return a == length() - 1; }

    constexpr auto is_head(const Edge& eg) const noexcept -> bool { return eg.ind == head().ind; }
    constexpr auto is_tail(const Edge& eg) const noexcept -> bool { return eg.ind == tail().ind; }

    constexpr auto is_headind(const EgId ind) const noexcept -> bool { return ind == head().ind; }
    constexpr auto is_tailind(const EgId ind) const noexcept -> bool { return ind == tail().ind; }

    constexpr auto is_end(const EgId a) const noexcept -> bool { return is_head(a) || is_tail(a); }

    constexpr auto egEnd_is_head(const EgId ind,
                                 const EndId egEnd) const noexcept -> bool;
    constexpr auto egEnd_is_tail(const EgId ind,
                                 const EndId egEnd) const noexcept -> bool;

    constexpr void set_w(Id w) noexcept;
    constexpr void set_g_w(Id w) noexcept;
    constexpr void set_g_w() noexcept;

    constexpr auto dir_char(EgId a) const noexcept -> char;

    /**
     * @brief Inserts an edge at g[a].
     * @note Original edges g[a], g[a+1], ... are shifted forwards, becoming
     * g[a+1], g[a+2], ....
     * @param a Position of the inserted edge in the chain.
     * @param e Edge to be inserted.
     * @return Pointer to the newly inserted edge.
     */
    auto insert_edge(Edge&& e, EgId a) -> Edge*;

    /**
     * @brief Removes an edge at position \p a .
     * @details Removes an edge g[a] while shifting original
     * g[a+1], g[a+2], ... backwards. These then become g[a], g[a+1], ... .
     * @return Pointer to the edge g[a] after the shift if the removed edge
     * was not the at the chain head, \a nullptr otherwise.
    */
    auto remove_edge(EgId a) -> Edge*;

    /**
     * @brief Appends an edge at the chain head, i.e. at the back of g.
     * @param e Edge to be appended.
     */
    void append_edge(Edge&& e);

    /**
     * @brief Prints neighbor data on both ends.
     * @param tag Short description.
     * @param at Position of a specific edge.
     */
    template<bool endLine,
             typename... T>
    auto print_connectivity(T&&... tag) const -> szt;

    /**
     * @brief Prints chain parameters.
     * @param tag Short description.
     * @param at Position of a specific edge.
     */
    template<typename... T>
    void print(T&&... tag) const;

    /**
     * @brief Writes the chain to a binary file.
     * @param ofs std::ofstream to write to.
     */
    void write(std::ofstream& ofs) const;

    /**
     * @brief Checks that \p cond is satisfied, else terminates program.
     * @details Prints out the chain using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * @tparam T Types of the message arguments.
     * @param cond Condition to be checked.
     * @param tag Tag to the chain data to be printed if \p cond is false.
     */
    template<typename... T>
    void ensure(
        bool cond,
        const std::string& tag,
        T&&... message
    ) const;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename E4>
constexpr
Chain<E4>::
Chain(
    const Id idw
)
    : idw {idw}
{}


template<typename E4>
constexpr
Chain<E4>::
Chain(
    const EgId length,
    const Id idw,
    EgId ei
)
    : idw {idw}
{
    // append 'length' edges
    for (EgId a=0; a<length; ++a)
        append_edge(Edge{ei++, a, idw});
}


template<typename E4>
constexpr
Chain<E4>::
Chain(Chain&& other)
    : g {std::move(other.g)}
    , ngs {std::move(other.ngs)}
    , idw {std::move(other.idw)}
    , idc {std::move(other.idc)}
    , c {std::move(other.c)}
{}


template<typename E4>
constexpr
Chain<E4>::
Chain(const Chain& other)
    : g {other.g}
    , ngs {other.ngs}
    , idw {other.idw}
    , idc {other.idc}
    , c {other.c}
{}


template<typename E4>
constexpr
auto Chain<E4>::
operator=(Chain&& other) -> Chain&
{
    g = std::move(other.g);
    ngs = std::move(other.ngs);
    idw = std::move(other.idw);
    idc = std::move(other.idc);
    c = std::move(other.c);

    return *this;
}


template<typename E4>
constexpr
auto Chain<E4>::
operator=(const Chain& other) -> Chain&
{
    g = other.g;
    ngs = other.ngs;
    idw = other.idw;
    idc = other.idc;
    c = other.c;

    return *this;
}


template<typename E4>
constexpr
auto Chain<E4>::
operator==(const Chain& other) const noexcept -> bool
{
    return g == other.g &&
           ngs == other.ngs &&
           idw == other.idw &&
           idc == other.idc &&
           c == other.c;
}


// Inserts an edge at g[a] shifting original g[a], g[a+1], ... forwards.
template<typename E4>
auto Chain<E4>::
insert_edge(
    Edge&& e,
    const EgId a
) -> Edge*
{
    e.w = idw;
    const auto p = &*g.insert(g.begin() + a, std::move(e));
    for (EgId i {a}; i<length(); ++i)
        g[i].indw = i;

    return p;
}

// Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
template<typename E4>
auto Chain<E4>::
remove_edge(const EgId a) -> Edge*
{
    ASSERT(a < length(), "Attempting to erase an edge beyond chain length.");

    g.erase(g.begin() + a);

    for (EgId i {a}; i<length(); ++i)
        g[i].indw = i;

    return a < length() ? &g[a] : nullptr;
}


// Appends an edge at the chain head, i.e. at the back of g.
template<typename E4>
void Chain<E4>::
append_edge(Edge&& e)
{
    e.w = idw;
    e.indw = length();
    g.push_back(std::move(e));
}


template<typename E4>
void Chain<E4>::
reverse_g()
{
    std::reverse(g.begin(), g.end());
    for (EgId a {}; auto& o : g) {
        o.reverse();
        o.indw = a++;
    }
}


template<typename E4>
auto Chain<E4>::
set_g_cmp(
    const CmpId newc,
    EgId initindc
) noexcept -> EgId
{
    for (auto& q: g)
        q.set_cmp(newc, initindc++);

    return initindc;
}


template<typename E4>
auto Chain<E4>::
set_cmpt(
    const CmpId newc,
    const ChId newidc,
    const EgId initindc
) noexcept -> EgId
{
    c = newc;
    idc = newidc;
    return set_g_cmp(newc, initindc);
}


template<typename E4>
auto Chain<E4>::
has_edge(const EgId ind) const noexcept -> bool
{
    for (const auto& eg: g)
        if (eg.ind == ind)
            return true;

    return false;
}


template<typename E4>
constexpr
auto Chain<E4>::
end_edge(const EndId e) const noexcept -> const Edge&
{
    return e == endA ? g.front()
                     : g.back();
}


// Convert chain end to edge end
template<typename E4>
constexpr
auto Chain<E4>::
edge_end(const EndId e) const noexcept -> EndId
{
    return e == endA ? (tail().points_forwards() ? endA : endB)
                     : (head().points_forwards() ? endB : endA);
}


template<typename E4>
constexpr
auto Chain<E4>::
egEnd_is_head(const EgId ind,
              const EndId egEnd) const noexcept -> bool
{
    return is_headind(ind) &&
        (( head().points_forwards() && egEnd == Ends::B) ||
         (!head().points_forwards() && egEnd == Ends::A));
}


template<typename E4>
constexpr
auto Chain<E4>::
egEnd_is_tail(const EgId ind,
              const EndId egEnd) const noexcept -> bool
{
    return is_tailind(ind) &&
        (( tail().points_forwards() && egEnd == Ends::A) ||
         (!tail().points_forwards() && egEnd == Ends::B));
}


template<typename E4>
constexpr
auto Chain<E4>::
connected_edge(
    const EgId a,
    const EndId ege
) const noexcept -> Edge const*
{
    ASSERT(!egEnd_is_tail(g[a].ind, ege),
           "Edge end ", Ends::str(ege), " at ", a, ", w ", idw, " is chain tail");
    ASSERT(!egEnd_is_head(g[a].ind, ege),
           "Edge end ", Ends::str(ege), " at ", a, ", w ", idw, " is chain head");


    const auto pf = g[a].points_forwards();

    if (a < length() - 1) {
        if (( pf && ege == Ends::B) ||
            (!pf && ege == Ends::A))
            return &g[a+1];
    }

    if (a > 0) {
        if (( pf && ege == Ends::A) ||
            (!pf && ege == Ends::B))
            return &g[a-1];
    }

    return nullptr;
}


template<typename E4>
constexpr
auto Chain<E4>::
neig_a_of_egEnd(
    const EgId a,     // edge position inside the chain
    const EndId ee    // edge end
) const noexcept -> EgId
{
    const auto shift = g[a].points_forwards() ? (ee == endB ? 1 : -1)
                                              : (ee == endB ? -1 : 1);
    const auto b = static_cast<EgId>(a + shift);

    return b < length() ? b : undefined<EgId>;
}


// Finds edge end of the edge \p b to which the edge at
// \p a is connected via a vertex internal to the chain.
// Should only be applied to the chain internal vertices.
template<typename E4>
constexpr
auto Chain<E4>::
internal_egEnd(
    const EgId a,   // chain position of the 1st edge
    const EgId b    // chain position of the 2nd edge
) const noexcept -> EndId
{
    ASSERT(is_defined(a) && is_defined(b),
           "a = ", a, ", b = ", b, " are not chain-wide edge indexes");
    ASSERT(a == b + 1 || b == a + 1,
           "a = ", a, ", b = ", b, " do not share a vertex");

    return g[b].points_forwards() ? (a > b ? endB : endA)
                                  : (a > b ? endA : endB);
}


// Edge end of an edge a
template<typename E4>
constexpr
auto Chain<E4>::
a2egEnd(const EgId a) const noexcept -> EndId
{
    return g[a].points_forwards() ? endA : endB;
}


/*
template<typename E4>
constexpr
auto Chain<E4>::
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
        ? (egEnd == Ends::A ? (isForward ? Ends::A : Ends::B)
                            : (isForward ? Ends::B : Ends::A))
        : (isForward ? Ends::B : Ends::A);
}
*/


template<typename E4>
constexpr
auto Chain<E4>::
end2a(const EndId e) const noexcept -> EgId
{
    return e == endA ? 0 : length() - 1;
}


template<typename E4>
template<Chain<E4>::EndId E>
constexpr
auto Chain<E4>::
end2a() const noexcept -> EgId
{
    static_assert(E < Ends::num);

    return E == endA ? 0 : length() - 1;
}


template<typename E4>
constexpr
auto Chain<E4>::
a2end(const EgId a) const noexcept -> EndId
{
    ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());
    ASSERT(a == 0 || a == length() - 1, "a = ", a, " is not an end edge.");

    return length() == 1
        ? endUndef            // end is undefined for single-edge chains
        : a == 0 ? endA
                 : a == length() - 1 ? endB
                                     : endUndef;
}


template<typename E4>
constexpr
auto Chain<E4>::
egEnd_to_bulkslot(const EndId egE,
                  const EgId a) const noexcept -> BulkSlot
{
    ASSERT(a < length(), " a = ", a, " exceeds chain length ", length());

    const auto frw = g[a].points_forwards();

    return egE == endA ? (frw ? BulkSlot{idw, a}
                              : BulkSlot{idw, a + 1})
                       : (frw ? BulkSlot{idw, a + 1}
                               : BulkSlot{idw, a});
}


template<typename E4>
constexpr
auto Chain<E4>::
end2ind(const EndId e) const noexcept -> EgId
{
    return end_edge(e).ind;
}


template<typename E4>
auto Chain<E4>::
ind2end(const EgId ind,                   // graph-wide edge index
        const EndId ege) const -> EndId   // edge end
{
    if (length() == 1) {
        if (ind == g[0].ind)
            return g[0].points_forwards() ? ege : g[0].opp_end(ege);

        abort(ind, " is not an end edge ind of chain ", idw);
        return endUndef;  // unreachable
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    abort(ind, " is not an end edge ind of chain ", idw);
    return endUndef;  // unreachable
}


template<typename E4>
auto Chain<E4>::
ind2end(const EgId ind,                     // graph-wide edge index
        const EndSlot& nb) const -> EndId   // EndSlot connected to the end
{
    if (length() == 1) {
        if (ind == g.front().ind && ngs[endA].has(nb))
            return endA;
        else if (ind == g.back().ind && ngs[endB].has(nb))
            return endB;

        abort(ind, " is not an end edge ind of chain ", idw);
        return endUndef;  // unreachable
    }

    if (ind == g.front().ind)
        return endA;

    if (ind == g.back().ind)
        return endB;

    abort(ind, " is not an end edge ind of chain ", idw);
    return endUndef;  // unreachable
}


template<typename E4>
constexpr
auto Chain<E4>::
indc2end(const EgId indc,
         const EndId ege) const noexcept -> EndId
{
    if (length() == 1) {
        if (indc == g[0].indc)
            return g[0].points_forwards() ? ege : g[0].opp_end(ege);

        abort(indc, " is not an end edge indc of chain ", idw);
        return endUndef;  // unreachable
    }

    if (indc == g.front().indc)
        return endA;

    if (indc == g.back().indc)
        return endB;

    abort(indc, " is not an end edge indc of chain ", idw);
    return endUndef;  // unreachable
}


template<typename E4>
constexpr
auto Chain<E4>::
get_single_connected_end() const -> EndId
{
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endA;
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endB;

    abort("Error in Chain::get_single_connected_end(): ",
          "the connected end is not single!");

    return endUndef;  // unreachable
}

template<typename E4>
constexpr
auto Chain<E4>::
get_single_free_end() const -> EndId
{
    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    abort("Error in Chain::get_single_free_end(): ",
          "the free end is not single!");

    return endUndef;  // unreachable
}


template<typename E4>
constexpr
auto Chain<E4>::
get_single_3way_end() const -> EndId
{
    if (ngs[endA].num() == 2 && ngs[endB].num() != 2) return endA;
    if (ngs[endA].num() != 2 && ngs[endB].num() == 2) return endB;

    abort("Error in Chain::get_single_3way_end(): ",
          "the 3way end is not single!");

    return endUndef;  // unreachable
}


template<typename E4>
constexpr
auto Chain<E4>::
has_one_free_end() const noexcept -> bool
{
    const auto a = is_connected_at(endA);
    const auto b = is_connected_at(endB);

    if ((!a &&  b) ||
        ( a && !b)) return true;

    return false;
}

template<typename E4>
constexpr
auto Chain<E4>::
the_only_free_end() const noexcept -> EndId
{
    ASSERT(has_one_free_end(),
            "chain ", idw, " does not have single free end");

    if (!is_connected_at(endA) &&  is_connected_at(endB)) return endA;
    if ( is_connected_at(endA) && !is_connected_at(endB)) return endB;

    return endUndef;
}


template<typename E4>
constexpr
auto Chain<E4>::
has_a_free_end() const noexcept -> std::pair<bool, EndId>
{
    // return the end index if true

    if (!is_connected_at(endA))
        return {true, endA};

    if (!is_connected_at(endB))
        return {true, endB};

    return {false, endUndef};
}


template<typename E4>
constexpr
auto Chain<E4>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    return Ends::opp(e);
}


template<typename E4>
constexpr
auto Chain<E4>::
edge_points_forwards(const EgId a) const noexcept -> bool
{
    return g[a].points_forwards();
}


template<typename E4>
constexpr
auto Chain<E4>::
is_connected_at(const EndId e) const noexcept -> bool
{
    return ngs[e].num() > 0;
}


template<typename E4>
constexpr
auto Chain<E4>::
is_disconnected_cycle() const noexcept -> bool
{
    return ngs[endA].num() == 1 &&
           ngs[endB].num() == 1 &&
           ngs[endA][0] ==
           ngs[endB][0].opp();
}


template<typename E4>
constexpr
auto Chain<E4>::
is_connected_cycle() const noexcept -> bool
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


template<typename E4>
constexpr
auto Chain<E4>::
is_cycle() const noexcept -> bool
{
    return is_disconnected_cycle() ||
           is_connected_cycle();
}


template<typename E4>
constexpr
auto Chain<E4>::
is_shrinkable() const noexcept -> bool
{
    if (has_one_free_end())
        return true;

    const auto l = length();

    return is_cycle() ? l > minCycleLength
                      : l > minLength;
}


template<typename E4>
auto Chain<E4>::
has_such_neig(const EndId e,
              const Id n) const noexcept -> bool
{
    for (const auto& s : ngs[e]())
        if (s.w == n)
            return true;

    return false;
}


template<typename E4>
template<Degree D> requires is_implemented_degree<D>
constexpr
auto Chain<E4>::
num_vertices() const noexcept -> szt // D = 1, 2, 3, 4
{
    if constexpr (D == Deg0)  // count boundaries of disconnected cycles
        return is_disconnected_cycle() ? 1 : 0;

    if constexpr (D == Deg1) {  // count vertices of degree 1
        if ( is_connected_at(endA) &&  is_connected_at(endB)) return 0;
        if (!is_connected_at(endA) && !is_connected_at(endB)) return 2;
        /* else */                                            return 1;
    }

    if constexpr (D == Deg2)  // count vertices of degree 2
        return length() - 1;

    // count vertices of degree 3 or 4

    constexpr szt N {D - 1U};
    const auto nA = ngs[endA].num();
    const auto nB = ngs[endB].num();

    if (nA == N && nB == N) return 2;
    if (nA == N || nB == N) return 1;
    if (nA != N && nB != N) return 0;

    std::unreachable();
    return undefined<szt>;
}


template<typename E4>
constexpr
auto Chain<E4>::
length() const noexcept -> EgId
{
    return static_cast<EgId>(g.size());
}


template<typename E4>
constexpr
auto Chain<E4>::
weight() const noexcept -> typename Edge::weight_t
{
    typename Edge::weight_t res {};

    for (const auto& o: g)
        res += o.weight;

    return res;
}


template<typename E4>
constexpr
void Chain<E4>::
set_g_w(const Id w) noexcept
{
    for (EgId a {}; auto& o: g) {
        o.w = w;
        o.indw = a++;
    }
}


template<typename E4>
constexpr
void Chain<E4>::
set_g_w() noexcept
{
    set_g_w(idw);
}


template<typename E4>
constexpr
void Chain<E4>::
set_w(const Id w) noexcept
{
    idw = w;
    set_g_w();
}


template<typename E4>
template<bool endLine, typename... T>
auto Chain<E4>::
print_connectivity(T&&... tag) const -> szt
{
    std::string s {};
    if constexpr (sizeof...(T))
        s = Msgr::oss(tag...).str();

    log_<false>(colorcodes::MAGENTA, s, " ", colorcodes::BOLDRED,
                idw, colorcodes::RESET);

    ngs[endA].print();
    log_<false>("**");
    ngs[endB].print();

    if constexpr (endLine)
        log_("");

    return s.length();
}


template<typename E4>
constexpr
auto Chain<E4>::
dir_char(const EgId a) const noexcept -> char
{
    return g[a].points_forwards() ? '>' : '<';
}


template<typename E4>
template<typename... T>
void Chain<E4>::
print(T&&... tag) const
{
    const auto tag_length = print_connectivity<false>(tag...);

    log_<false>(colorcodes::BOLDCYAN, c, colorcodes::RESET, " [", idc, "]");

    log_<false>(" len ", colorcodes::GREEN, length(), colorcodes::RESET);

    for (EgId i {}; i<length(); ++i) {
        if (i && i % 10 == 0)
            log_<false>(' ');
        log_<false>(dir_char(i));
    }

    log_<false>('(');
    for (const auto& eg : g)
        log_<false>(eg.ind);
    log_(')');

    if constexpr (print_edges) {
        const std::string spaces(tag_length, ' ');
        for (const auto& eg: g)
            eg.print(spaces, dir_char(eg.indw));
    }
}


template<typename E4>
void Chain<E4>::
write(std::ofstream& ofs) const
{
    const auto len = static_cast<EgId>(length());
    ofs.write(reinterpret_cast<const char*>(&len), sizeof(EgId));
    ofs.write(reinterpret_cast<const char*>(&idw), sizeof(ChId));
    ofs.write(reinterpret_cast<const char*>(&idc), sizeof(ChId));
    ofs.write(reinterpret_cast<const char*>(&c), sizeof(CmpId));

    for (const auto& ng : ngs)
        ng.write(ofs);

    for (const auto& a : g)
        a.write(ofs);
}


template<typename E4>
template<typename... T>
void Chain<E4>::
ensure(
    bool cond,
    const std::string& tag,
    T&&... message
) const
{
    if (!cond) {
        print(tag);
        ABORT(cond, message...);
    }
}

using ChainBase = Chain<EdgeBase>;

template<typename T>
concept slot = std::is_same_v<T, ChainBase::EndSlot> ||
               std::is_same_v<T, ChainBase::BulkSlot>;




}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_CHAIN_H
