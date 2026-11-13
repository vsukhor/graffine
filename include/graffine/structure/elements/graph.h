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
 * \brief The graph: the container of all components, chains, edges and
 * vertices.
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
#include <initializer_list>
#include <memory>
#include <ostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>


namespace graffine::structure {
namespace detail {

/** Generic graph template; see the specialization Graph<CC> for the
 * implemented graph.
 */
template<typename CC,
         typename... Props>
struct Graph
{};


/** A graph made of one or more connected components.
 * \details The graph is the entry point to the structure: it owns the
 * chains (which own the edges) and the components (which own the
 * vertices), and provides graph-wide access to all of them. It keeps
 * graph-wide tables of edge and vertex descriptors, for locating an
 * element by its graph-wide index.
 *
 * The graph type is assembled from the element templates, e.g.
 * \code
 * using G = Graph<Component<Chain<Edge<Vertex>>>>;
 * \endcode
 * and makes the nested types available: G::Component, G::Chain, G::Edge,
 * G::Vertex, G::EndSlot, G::BulkSlot...
 *
 * The graph holds the topology only: it is unaware of any physical or
 * geometric properties, which client code may attach through the property
 * processors of the transformations. The graph is movable but not
 * copyable.
 * \tparam CC Type of connected components forming the graph object.
 */
template<typename CC>
struct Graph<CC>
{
    using Self = Graph<CC>;
    using Compt = CC;  ///< Connected component (short alias).
    using Component = CC;  ///< Connected component.
    using Components = structure::containers::Components<Compt>;
    using Chain = Compt::Chain;
    using Chains = Compt::Chains;
    using Edge = Chain::Edge;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using End = Chain::End;  ///< Edge ends.
    using EndId = End::Id;
    using EdgeDescr = descriptors::EdgeInGraph;
    using EdgeDescriptors = structure::containers::EdgeDescriptorsInGraph<Self>;
    using Vertex = Edge::Vertex;
    using VertexDescr = descriptors::VertexInGraph;
    using VertexDescriptors = structure::containers::VertexDescriptorsInGraph<Self>;
    /// Shortest paths over the chain ends of a component.
    using PathsOverEndSlots = paths::over_endslots::Generic<Compt>;
    using ChainClasses = Compt::Chis;  ///< Chains classified by end degrees.

    static constexpr auto Type = Elements::Graph;
    /// True iff the graph lives in a geometric space.
    static constexpr bool isSpatial {};
    /// True: a graph may hold several components.
    static constexpr bool isMulticomponent {true};
    /// Whether the graph keeps the adjacency lists in \a adj up to date on
    /// update(). Off by default: the lists are then built on demand.
    static constexpr bool useAgl {};

    template<typename, bool> friend struct IntegralTests;
    template<typename> friend struct GraphFactory;
    friend struct structure::containers::EdgeDescriptorsInGraph<Self>;
    friend struct structure::containers::VertexDescriptorsInGraph<Self>;

// VARIABLES -------------------------------------------------------------------

    /// Adjacency lists and checks over the graph-wide indexes.
    Adjacency<Graph> adj {*this};

protected:

    /// Owns the chains. Kept on the heap, so that the chains do not move
    /// when the graph is moved: the components refer to them.
    std::unique_ptr<Chains> cnStorage {std::make_unique<Chains>()};

    Chains&    cn {*cnStorage};  ///< The collection of chains.
    Components ct;               ///< The collection of components.

    std::size_t numEdges {};  ///< Current number of edges.

    /// Map of graph-wide edge indexes to chain indexes and in-chain positions.
    EdgeDescriptors egl;

    /// Graph-wide collection of vertex data.
    /// \details Rebuilt lazily, on first access after a change of the graph:
    /// transformations only mark it stale, see update_vertex_descriptors().
    mutable VertexDescriptors ved;
    mutable bool vedStale {true};  ///< Whether ved needs a rebuild.

// METHODS ---------------------------------------------------------------------

public:

    constexpr Graph() = default;
    constexpr ~Graph() = default;

    /** Moves the graph.
     * \details The chains, components and vertices keep their addresses, so
     * references to them remain valid. Objects referring to the graph itself,
     * such as transformation functors, keep referring to the moved-from
     * graph, which may only be destroyed.
     */
    constexpr Graph(Graph&& gr) noexcept;

    // Not assignable: the reference to the chains cannot be rebound.
    constexpr auto operator=(Graph&& gr) -> Graph& = delete;

    // Non-copyable because vertices are owned by std::unique_ptr;
    // state this explicitly for clarity.
    constexpr Graph(const Graph& gr) = delete;
    constexpr auto operator=(const Graph& gr) -> Graph& = delete;

    /** Constructor taking over the chains \p gr, without components.
     * \details Used by graph builders, which then assemble the components
     * (see GraphFactory).
     */
    explicit constexpr Graph(Chains&& gr);

    /** Adds \p num new components, each a single unconnected chain of
     * \p len edges.
     * \note The count of components to reach is based on the number of
     * chains already present, so it is exact only if each existing
     * component is a single chain.
     * \param[in] num Desired number of components.
     * \param[in] len Number of edges of each chain.
     */
    void generate_single_chain_components(
        std::size_t num,
        std::size_t len
    );

    /** Adds a new component made of an unconnected chain of \p len edges.
     */
    void add_single_chain_component(std::size_t len);

    /** Appends component \p cmp to the graph and updates the graph data.
     */
    void add_component(Compt&& cmp);

    /** Removes edge \p ind from the graph.
     * \details Removes the edge from its chain as Component::remove_edge()
     * does, merging the two vertices of the edge, and updates the graph data.
     * \note The formerly last edge of the graph takes over index \p ind:
     * graph-wide edge indexes of the other edges remain valid.
     * \param[in] ind Graph-wide index of the edge to be removed.
     */
    void remove_edge(EgIdG ind);

    /** Appends \p donor component to \p acceptor component.
     * \details The place of \p donor is filled by the last component of
     * \a ct, which is reindexed, unless \p donor is the last one itself.
     * Hence, if \p acceptor is the last component, the merged component
     * ends up in the place, and with the index, of \p donor; otherwise it
     * keeps the place and index of \p acceptor.
     * \note References to the last component are invalidated: use the
     * returned reference rather than \p acceptor.
     * \param[in] acceptor Component to which \p donor component is appended.
     * \param[in] donor Component merged to the \p acceptor.
     * \return Reference to the merged component.
     */
    auto merge_components(
        Compt& acceptor,
        Compt& donor
    ) -> Compt&;

    /** Appends \p donor component to \p acceptor component.
     * \details The place of \p donor is filled by the last component of
     * \a ct, which is reindexed, unless \p donor is the last one itself.
     * Hence, if \p acceptor is the last component, the merged component
     * ends up in the place, and with the index, of \p donor; otherwise it
     * keeps the place and index of \p acceptor.
     * \note References to the last component are invalidated: use the
     * returned reference rather than \p acceptor.
     * \param[in] acceptor Component to which \p donor component is appended.
     * \param[in] donor Component merged to the \p acceptor.
     * \return Reference to the merged component.
     */
    auto merge_components(
        CmpId acceptor,
        CmpId donor
    ) -> Compt&;

    /** Moves the chains of \p cmp unreachable from slot \p s to a new
     * component.
     * \details The chains are classified by
     * PathsOverEndSlots::classify_chains_by_connectivity(), so that \p pp
     * must hold the distances computed from \p s, e.g. by
     * pp.compute_from_source(s) or pp.are_connected(s, ...).
     * \note See split_component(Compt&, ChIdGs&&) on invalidation.
     * \return Reference to the new component.
     */
    auto split_component(
        Compt& cmp,
        const PathsOverEndSlots& pp,
        const EndSlot& s
    )-> Compt&;

    /** Moves chains \p rm of \p cmp to a new component.
     * \details The new component is appended to the components: this may
     * reallocate them, invalidating references to the components,
     * \p cmp included.
     * \note Does not mark the graph-wide vertex descriptors stale: the
     * caller does so once its changes are complete.
     * \return Reference to the new component.
     */
    auto split_component(
        Compt& cmp,
        ChIdGs&& rm
    ) -> Compt&;

    /** Moves chain \p m of \p cmp to a new component.
     * \details As split_component(Compt&, ChIdGs&&), for a single chain.
     * \return Reference to the new component.
     */
    auto split_component(
        Compt& cmp,
        Chain& m
    ) -> Compt&;

    /**
     * \name Access to the components.
     * @{
     */
    /// All the components.
    constexpr auto compts() const noexcept -> const Components& { return ct; }
    /// All the components.
    constexpr auto compts()       noexcept ->       Components& { return ct; }
    /// Component \p c.
    constexpr auto compt(const CmpId c) const noexcept -> const Component& { return ct[c]; }
    /// Component \p c.
    constexpr auto compt(const CmpId c)       noexcept ->       Component& { return ct[c]; }
    /// Component hosting chain \p w.
    constexpr auto compt(const ChIdG w) const noexcept -> const Component& { return ct[cn[w].c]; }
    /// Component hosting chain \p w.
    constexpr auto compt(const ChIdG w)       noexcept ->       Component& { return ct[cn[w].c]; }
    /// Component hosting edge \p i.
    constexpr auto compt(const EgIdG i) const noexcept -> const Component& { return ct[cmpid(i)]; }
    /// Component hosting edge \p i.
    constexpr auto compt(const EgIdG i)       noexcept ->       Component& { return ct[cmpid(i)]; }
    /// The last component.
    constexpr auto compt_last() const noexcept -> const Component& { return ct.back(); }
    /// The last component.
    constexpr auto compt_last()       noexcept ->       Component& { return ct.back(); }
    /// Id of the component hosting chain \p w.
    constexpr auto cmpid(const ChIdG w) const noexcept -> CmpId { return cn[w].c; }
    /// Id of the component hosting edge \p i.
    constexpr auto cmpid(const EgIdG i) const noexcept -> CmpId { return edge(i).c; }
    ///@}

    /**
     * \name Access to the chains.
     * @{
     */
    /// Ids of all the chains: 0, 1, ..., num_chains()-1.
    constexpr auto chids() const noexcept -> ChIdGs { return cn.chids(); }
    /// Id of the chain hosting edge \p ind.
    constexpr auto chid(const EgIdG ind) const noexcept -> ChIdG;
    /// All the chains.
    constexpr auto chains() const noexcept -> const Chains& { return cn; }
    /// All the chains.
    constexpr auto chains()       noexcept ->       Chains& { return cn; }
    /// Chain \p w.
    constexpr auto chain(const ChIdG& w) const noexcept -> const Chain& { return cn[w]; }
    /// Chain \p w.
    constexpr auto chain(const ChIdG& w)       noexcept ->       Chain& { return cn[w]; }
    /// The chain of end slot \p s.
    constexpr auto chain(const EndSlot& s) const noexcept -> const Chain&;
    /// The chain of end slot \p s.
    constexpr auto chain(const EndSlot& s)       noexcept ->       Chain&;
    /// The chain hosting edge \p ind.
    constexpr auto chain(const EgIdG ind) const noexcept -> const Chain&;
    /// The chain hosting edge \p ind.
    constexpr auto chain(const EgIdG ind)       noexcept ->       Chain&;
    /// The last chain.
    constexpr auto chain_last() const noexcept -> const Chain& { return cn.back(); }
    /// The last chain.
    constexpr auto chain_last()       noexcept ->       Chain& { return cn.back(); }
    ///@}

    /**
     * \name Access to the edges.
     * @{
     */
    /// Edge \p ind.
    constexpr auto edge(EgIdG ind) const noexcept -> const Edge&;
    /// Edge \p ind.
    constexpr auto edge(EgIdG ind)       noexcept ->       Edge&;
    /// The edge at chain end \p s.
    constexpr auto edge(const EndSlot& s) noexcept -> Edge&;
    /// The edge following internal vertex \p s along its chain, i.e. g[a].
    constexpr auto edge(const BulkSlot& s) noexcept -> Edge&;
    /// The edge with the highest graph-wide index.
    constexpr auto edge_last() const noexcept -> const Edge& { return edge(numEdges-1); };
    /// The edge with the highest graph-wide index.
    constexpr auto edge_last()       noexcept ->       Edge& { return edge(numEdges-1); };
    ///@}

    /**
     * \name Access to the vertices.
     * \details The lookups by VertexDescr, VIdG or VIdH go through the
     * graph-wide vertex descriptors, rebuilt first if stale (see
     * refresh_vertex_descriptors()).
     * @{
     */
    /// Descriptors of the vertices of degree \p d.
    auto vertices(Degree d) const -> VertexDescriptors;
    /// The vertex described by \p vd.
    constexpr auto vertex(const VertexDescr& vd) const noexcept -> const Vertex&;
    /// The vertex described by \p vd.
    constexpr auto vertex(const VertexDescr& vd)       noexcept ->       Vertex&;
    /// The vertex at position \p ind of the graph-wide descriptors.
    auto vertex(VIdG ind) const -> const Vertex&;
    /// The vertex at position \p ind of the graph-wide descriptors.
    auto vertex(VIdG ind)       ->       Vertex&;
    /// The vertex with handle \p h, or nullptr.
    auto vertex(VIdH h) const -> const Vertex*;
    /// The vertex with handle \p h, or nullptr.
    auto vertex(VIdH h)       ->       Vertex*;
    /// The vertex at chain end \p s, or nullptr if unconnected.
    constexpr auto vertex(const EndSlot& s) const noexcept -> const Vertex*;
    /// The vertex at chain end \p s, or nullptr if unconnected.
    constexpr auto vertex(const EndSlot& s)       noexcept ->       Vertex*;
    /// The internal vertex at slot \p s.
    constexpr auto vertex(const BulkSlot& s) const noexcept -> const Vertex*;
    /// The internal vertex at slot \p s.
    constexpr auto vertex(const BulkSlot& s)       noexcept ->       Vertex*;
    ///@}

    /** Exchanges the places of vertices \p a and \p b in the graph: each takes
     * over the edges, and the component, of the other.
     * \details Changes the topology only up to vertex identity: the vertex
     * objects, with their ids and other attributes, remain valid. Used by
     * the transformations keeping given vertex objects (see Keep).
     */
    void exchange_vertices(Vertex& a, Vertex& b);

    /// The graph-wide edge descriptors.
    constexpr auto get_egl() const noexcept -> const EdgeDescriptors& { return egl; }
    /// The graph-wide edge descriptors.
    constexpr auto get_egl()       noexcept ->       EdgeDescriptors& { return egl; }
    /// Descriptor of edge \p ind.
    constexpr auto get_egl(EgIdG ind) const noexcept -> const EdgeDescr&;
    /// Descriptor of edge \p ind.
    constexpr auto get_egl(EgIdG ind)       noexcept ->       EdgeDescr&;

    // The vertex descriptors are refreshed if stale, see update_vertex_descriptors().
    /// The graph-wide vertex descriptors.
    auto get_ved() const -> const VertexDescriptors& { refresh_vertex_descriptors(); return ved; }
    /// The graph-wide vertex descriptors.
    auto get_ved()       ->       VertexDescriptors& { refresh_vertex_descriptors(); return ved; }
    /// The vertex descriptor at position \p ind.
    auto get_ved(VIdG ind) const -> const VertexDescr&;
    /// The vertex descriptor at position \p ind.
    auto get_ved(VIdG ind)       ->       VertexDescr&;
    /// Descriptor of the vertex with handle \p h, which must exist.
    auto get_ved(VIdH h) const -> const VertexDescr& { refresh_vertex_descriptors(); return ved[h]; }
    /// Descriptor of the vertex with handle \p h, which must exist.
    auto get_ved(VIdH h)       ->       VertexDescr& { refresh_vertex_descriptors(); return ved[ved.find(h)]; }

    /** Rotates the edges of chain \p m by \p n positions, as
     * Component::rotate_chain(), also updating the graph-wide descriptors.
     */
    constexpr void rotate_chain(Chain& m, std::size_t n);

    /** Moves edges between two chains of a component, as
     * Component::move_edges(), also updating the graph-wide descriptors.
     * \note Does not update edge connectivity.
     *       Does not update chain connectivity and 'ngs'.
     *       Does not remove source chain if it is emptied.
     *       Only \p posTo = 0 is handled correctly.
     */
    constexpr void move_edges(
        Chain& fr,
        std::size_t posFrBegin,
        std::size_t posFrEnd,  // position AFTER the last moved element
        Chain& to,
        std::size_t posTo  // position of the first inserted element
    );

    /** Position in its chain of the edge at chain end \p s .
     */
    constexpr auto slot2a(const EndSlot& s) const noexcept -> EgIdA;

    /** Graph-wide index of the edge at chain end \p s .
     */
    constexpr auto slot2ind(const EndSlot& s) const noexcept -> EgIdG;
    /** BulkSlot {chain, position} of edge \p ind : the internal vertex at
     * the outer end A of the edge, if the edge is not the chain tail.
     */
    constexpr auto ind2bslot(EgIdG ind) const noexcept -> BulkSlot;

    /** Inner end, of the edge at chain end \p s , facing that chain end.
     */
    constexpr auto edge_end(const EndSlot& s) const noexcept -> EndId;

    /** Rebuilds the edge adjacency of the component hosting edge \p ind.
     * \note Only compiled in use if \a useAgl is set.
     */
    void update_adjacency_edges(EgIdG ind) noexcept;

    /** Rebuilds the edge adjacency of all the components.
     * \note Only compiled in use if \a useAgl is set.
     */
    void update_adjacency() noexcept;

    /** Updates the graph-wide records after a change of the structure.
     * \details Calls update_edge_descriptors(), then update_adjacency() if
     * the graph keeps an adjacency list (\a useAgl), and finally
     * update_vertex_descriptors(), which only marks the vertex descriptors
     * stale for a lazy rebuild.
     * \tparam withVertices Whether to mark the vertex descriptors stale.
     */
    template<bool withVertices = true>
    void update();

    /** Updates the graph-wide records after changes limited to chains \p ww.
     * \details Like update(), but updates only the descriptors of the edges
     * of chains \p ww: these must comprise every chain whose edges changed
     * position, chain or index. Chains no longer in the graph are skipped.
     */
    template<bool withVertices = true>
    void update(std::initializer_list<ChIdG> ww);

    /** Rebuilds the graph-wide edge descriptors from all the chains.
     */
    void update_edge_descriptors();

    /** Updates the graph-wide edge descriptors of the edges of chains \p ww.
     * \details In DEBUG builds, the result is checked against a full update.
     */
    void update_edge_descriptors(std::initializer_list<ChIdG> ww);

    /** Marks the graph-wide vertex descriptors as stale.
     * \details Called by the transformations: the descriptors are rebuilt,
     * at linear cost, on their first access after a change, rather than
     * after every transformation.
     */
    void update_vertex_descriptors() noexcept;

    /** Rebuilds the graph-wide vertex descriptors if they are stale.
     * \note The accessors of the descriptors call it, so that reading them
     * from a const graph may modify this cache: call it beforehand to read
     * the descriptors of a graph from several threads.
     */
    void refresh_vertex_descriptors() const;

    /** Builds an edge adjacency list: edges are adjacent if they share a
     * vertex.
     * \tparam F Type of element of the generated list.
     * \param[in] f Function mapping an edge to an element of the list, e.g.
     * to its index.
     * \return For each graph-wide edge index, the elements generated by \p f
     * from the edges adjacent to it.
     */
    template<typename F>
    auto adjacency_list_edges(F(*f)(const Edge&)) const noexcept -> vec2<F>;

    /** Generates the chain adjacency list of the graph.
     * \details Chains are adjacent if they are joined at their ends. Each
     * chain end joined at an end of chain \a w contributes one entry to
     * the list of \a w, first those at end A of \a w, then at end B. So a
     * chain joined to another at both ends lists it twice, and a cycle,
     * having its ends joined to each other, lists itself twice.
     * \return For each graph-wide chain id, the ids of the chains joined
     * to its ends.
     */
    auto adjacency_list_chains() const noexcept -> vec2<ChIdG>;

    /** Maximal vertex degree in the graph.
     */
    constexpr auto max_degree() const noexcept -> Degree;

    /** Number of vertices of degree \p d.
     */
    constexpr auto num_vertices(Degree d) const noexcept -> std::size_t;

    /** Total number of vertices.
     */
    constexpr auto num_vertices() const noexcept -> std::size_t;

    /** Current number of components.
     */
    constexpr auto num_compts() const noexcept -> std::size_t { return ct.size(); };
    /** Current number of components; the same as num_compts().
     */
    constexpr auto num_components() const noexcept -> std::size_t { return num_compts(); };

    /** Current number of chains.
     */
    constexpr auto num_chains() const noexcept -> std::size_t { return cn.num(); };

    /** Current number of chains having end degrees \p k.
     * \param k End degrees in ascending order, see ChainClasses::Key.
     */
    constexpr auto num_chains(
        const ChainClasses::Key& k
    ) const noexcept -> std::size_t;

    /** Current number of edges.
     */
    constexpr auto num_edges() const noexcept -> std::size_t { return numEdges; };
    /// Recounts the edges from the chains.
    constexpr auto set_num_edges() noexcept { numEdges = cn.num_edges(); }
    /// Sets the recorded number of edges to \p n.
    constexpr auto set_num_edges(const std::size_t n) noexcept { numEdges = n; }

    /// Id of the last component.
    constexpr auto ind_last_cmpt() const noexcept -> CmpId;
    /// Id of the last chain.
    constexpr auto ind_last_chain() const noexcept -> ChIdG;

    /** Records chain ends \p s1 and \p s2 as joined.
     * \details Adds each slot to the neighbour list (Chain::ngs) of the
     * other.
     * \note Only updates the chain connectivity: the vertices at the two
     * ends are not merged.
     */
    constexpr void connect(const EndSlot& s1,
                           const EndSlot& s2);

    /// The neighbours of chain end \p s.
    constexpr auto ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&;
    /// The neighbours of chain end \p s.
    constexpr auto ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&;

    /// Graph-wide indexes of the end edges of the chain ends joined to \p s.
    constexpr auto ng_inds_at(const EndSlot& s) const noexcept -> EgIdGs;

    /// The vertices of edge \p ind, by inner end.
    constexpr auto edge_vertices(EgIdG ind) const noexcept -> const Edge::Vertices&;
    /// Handles of the vertices of edge \p ind, by inner end.
    constexpr auto edge_vids(EgIdG ind) const noexcept -> Edge::VIDs;

    /** Chain end slots by which the end edges \p ind1 and \p ind2 of two
     * chains are joined.
     * \return The end slots of the chains of \p ind1 and \p ind2 at their
     * common vertex, or a pair of undefined slots if they are not joined.
     */
    auto inds_to_chain_link(
        EgIdG ind1,
        EgIdG ind2
    ) const noexcept -> std::array<EndSlot, 2>;

    /** Transfers the identity of chain \p f to chain \p t.
     * \details Used, e.g., to fill the place of a removed chain with the
     * last chain:
     * - the connections of the source become those of the target, end to
     *   end, and the neighbours refer to the target;
     * - the source edges are moved to the target;
     * - the target takes over the component and component-wide index of
     *   the source, and replaces it in the records of the component.
     *
     * The source, now devoid of edges, retains its neighbours, which are
     * now identical to those of the target.
     * \tparam updateChis Also update the chain classification.
     * \param[in] f Source chain index.
     * \param[in] t Target chain index.
     */
    template<bool updateChis=true>
    void rename_chain(ChIdG f, ChIdG t);

    /** Transfers the neighbours of chain end \p f to chain end \p t.
     * \details Copies the neighbour list of \p f to \p t, replacing the
     * previous neighbours of \p t, and makes each neighbour refer to \p t
     * instead of \p f. The neighbour list of \p f is not cleared.
     * \param[in] f Source slot.
     * \param[in] t Target slot.
     */
    void copy_neigs(
        const EndSlot& f,
        const EndSlot& t
    );

    /** Removes chain end slot \p s from list of \a neigs in slots connected
     * to it.
     * \param[in] s Chain end slot to be removed.
     */
    void remove_slot_from_neigs(const EndSlot& s);

    /** Replaces \p old with \p nov in the neighbour lists of the slots
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
        const EndSlot& nov
    );

    /** Checks if \p s1 and \p s2 are the two ends of the same cycle chain.
     */
    constexpr bool is_same_cycle(
        const EndSlot& s1,
        const EndSlot& s2
    ) const noexcept;

    /** Prints the graph chains using prefix \p tag.
     * \param[in] tag Human-readable description common to all records.
     */
    void print_chains(const std::string& tag) const;

    /** Prints the graph components using prefix \p tag.
     * \param[in] tag Human-readable description common to all records.
     */
    void print_components(std::string_view tag="") const;

    /// Prints the chains of each component, labelled by \p tag.
    void print_component_chains(std::string_view tag="") const;
    /// Prints the edge adjacency lists of the components.
    void print_adjacency_lists(std::string_view tag="") const;
    /// Prints the vertices of each component, each preceded by \p tag.
    void print_vertices(std::string_view tag=" ") const;
    /// Prints the vertices of degree \p d of each component, each preceded
    /// by \p tag.
    void print_vertices_deg(
        Degree d,
        std::string_view tag=" "
    ) const;

    /** Writes a one-line summary of the graph to \p ofs: the numbers of
     * vertices by degree, of edges, of chains and of components.
     * \note The current implementation iterates degrees 0 to 3, thus
     * omitting degree 4.
     * \param[in] ofs Output stream.
     */
    void report(std::ostream& ofs) const;

};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename CC>
constexpr
Graph<CC>::
Graph(Graph&& gr) noexcept
    : adj {std::move(gr.adj), *this}  // must refer to this, not to 'gr'
    , cnStorage {std::move(gr.cnStorage)}
    , cn {*cnStorage}
    , ct {std::move(gr.ct)}
    , numEdges {gr.numEdges}
    , egl {std::move(gr.egl)}
    , ved {std::move(gr.ved)}
    , vedStale {gr.vedStale}
{}

template<typename CC>
constexpr
Graph<CC>::
Graph(Chains&& chains)
    : cnStorage {std::make_unique<Chains>(std::move(chains))}
{}

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
auto Graph<CC>::
get_egl(const EgIdG ind) const noexcept -> const EdgeDescr&
{
    GRAFFINE_ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind];
}

template<typename CC>
constexpr
auto Graph<CC>::
get_egl(const EgIdG ind) noexcept -> EdgeDescr&
{
    GRAFFINE_ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind];
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
    GRAFFINE_ASSERT(ind < egl.size(), "ind exceeds the number of edges");

    return egl[ind].w;
}

template<typename CC>
constexpr
auto Graph<CC>::
chain(const EgIdG ind) const noexcept -> const Chain&
{
    GRAFFINE_ASSERT(ind < egl.size(), "ind exceeds the number of edges");
    return cn[egl[ind].w];
}

template<typename CC>
constexpr
auto Graph<CC>::
num_chains(
    const ChainClasses::Key& kk
) const noexcept -> std::size_t
{
    std::size_t n {};
    for (const auto& c : ct)
        n += c.chis.num(kk);

    return n;
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


}  // namespace detail

template<typename CC> using Graph = detail::Graph<CC>;


}  // namespace graffine::structure

// Parts of the implementation.
#include "graffine/structure/elements/graph/components.h"
#include "graffine/structure/elements/graph/neighbours.h"
#include "graffine/structure/elements/graph/descriptors.h"
#include "graffine/structure/elements/graph/vertices.h"
#include "graffine/structure/elements/graph/print.h"

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_GRAPH_H
