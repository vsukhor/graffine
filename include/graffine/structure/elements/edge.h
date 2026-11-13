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
 * \file edge.h
 * \brief The graph edge.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_EDGE_H
#define GRAFFINE_STRUCTURE_ELEMENTS_EDGE_H

#include "graffine/definitions.h"
#include "graffine/property.h"
#include "graffine/structure/common.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/edge_in_vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include <array>
#include <ostream>


namespace graffine::structure {
namespace detail {

/** Minimal edge: a run-wide identifier, a graph-wide index and the vertices
 * at its two ends.
 * \details Base of Edge, which adds the chain and component data and the
 * orientation; the ends of a Bond2 are its inner ends (see Edge).
 * \tparam V Vertex type.
 */
template<typename V>
struct Bond2
{
    using Vertex = V;
    using End = descriptors::End;  ///< Bond ends.
    using EndId = End::Id;
    using Vertices = std::array<Vertex*, End::num>;  ///< Vertices at ends A and B.
    using VIDs = std::array<VIdH, End::num>;         ///< Handles of the end vertices.
    using DescrInVertex = descriptors::EdgeInVertex<IdScope::GRAPH>;

    /// True iff the edge lives in a geometric space.
    static constexpr auto isSpatial = false;

// public variables

    EgIdH id;  ///< Stable handle, unique over the program run.
    EgIdG ind {EgIdG::undefined};   ///< Position in the graph (EgIdG).

protected:  // variables

    /// Vertices at ends A and B; nullptr at an unconnected end.
    Vertices vv {{nullptr, nullptr}};

public:  // functions

    constexpr Bond2() noexcept;  // Id is initialized, rest is undefined

    /** Constructor leaving chain and component data undefined.
     * \param[in] ind Index graph-wide.
     */
    explicit constexpr Bond2(EgIdG ind) noexcept;

    /// Two edges are equal if they have the same handle.
    constexpr bool operator==(const Bond2& other) const noexcept;

    /// Vertex at end \p e, or nullptr.
    constexpr auto vertex_at(EndId e) const noexcept -> const Vertex*;
    /// Vertex at end \p e, or nullptr.
    constexpr auto vertex_at(EndId e)       noexcept ->       Vertex*;
    /// Handles of the vertices at ends A and B (undefined if unconnected).
    constexpr auto vids() const noexcept -> VIDs;
    /// Handle of the vertex at end \p e, or VIdH::undefined.
    constexpr auto vid_at(const EndId e) const noexcept -> VIdH;

    /// Descriptor by which the vertex at end \p e would record this edge;
    /// its edge index is left undefined.
    constexpr auto descriptor_for_vertex_at(
        const EndId e
    ) const noexcept -> DescrInVertex;

    /** Formatted printing out of the edge data.
     * \tparam ENDL Flag to end current line after the print.
     * \param[in] tag Prefix preceding the output.
     */
    template<bool ENDL=true>
    void print(std::string_view tag) const;

protected:  // functions

    /** Creates a handle unique over the program run, by incrementing a
     * counter shared by all edges of this type.
     */
    static auto create_id() noexcept -> EgIdH;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename V>
constexpr
Bond2<V>::
Bond2() noexcept
    : id {create_id()}
{}


template<typename V>
constexpr
Bond2<V>::
Bond2(const EgIdG ind) noexcept
    : id {create_id()}
    , ind {ind}
{}


template<typename V>
constexpr
bool Bond2<V>::
operator==(const Bond2& other) const noexcept
{
    return id == other.id;
}

template<typename V>
auto Bond2<V>::
create_id() noexcept -> EgIdH
{
    static EgIdH h;
    return ++h;
}

template<typename V>
constexpr
auto Bond2<V>::
vertex_at(EndId e) const noexcept -> const Vertex*
{
    return vv[e];
}

template<typename V>
constexpr
auto Bond2<V>::
vertex_at(EndId e) noexcept -> Vertex*
{
    return vv[e];
}

template<typename V>
constexpr
auto Bond2<V>::
vids() const noexcept -> VIDs
{
    return {vid_at(End::A),
            vid_at(End::B)};
}


template<typename V>
constexpr
auto Bond2<V>::
vid_at(const EndId e) const noexcept -> VIdH
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[e] ? vv[e]->id : VIdH::undefined;
}

template<typename V>
constexpr
auto Bond2<V>::
descriptor_for_vertex_at(
    const EndId e
) const noexcept -> DescrInVertex
{
    return DescrInVertex {id, undefined<std::size_t>, e};
}

template<typename V>
template<bool ENDL>
void Bond2<V>::
print(const std::string_view prefix) const
{
    jot<false>(prefix);
    jot<false>("id ", colorcodes::BOLDBLUE, id, colorcodes::RESET);
    jot<false>("ind ", colorcodes::BOLDGREEN, ind, colorcodes::RESET);
    jot<false>("v [",
        colorcodes::BOLDYELLOW, vid_at(End::A), " -> ",
                                vid_at(End::B), colorcodes::RESET,
        "]");
    jot<ENDL>("");
}




// /////////////////////////////////////////////////////////////////////////////

/** Generic template for edges with additional properties \p Props;
 * see the specialization Edge<V> for the implemented edge.
 */
template<typename V,
         typename... Props>
struct Edge
{
    using Vertex = V;
    static constexpr auto Type = Elements::Edge;
};


template<typename, typename...> struct Chain;

/** The graph edge.
 * \details An edge is the smallest structural unit of a graph. It joins
 * the vertices at its two ends, belongs to exactly one chain, and through
 * it to one component. Besides its own handle and graph-wide index, it
 * records its positions in the component and in the chain, and the ids of
 * its chain and component; the containers keep these up to date when the
 * graph changes.
 *
 * The ends of an edge are labelled in two coordinate systems:
 * - \a inner ends are the edge's own ends A and B: vertex \a vv[e] is
 *   connected at inner end \a e, and the vertices record the inner end in
 *   their descriptors of the edge (Vertex::edges());
 * - \a outer ends are the edge ends in the coordinates of its host chain:
 *   outer end A faces chain end A.
 *
 * The orientation \a ornt relates the two: if the edge points forwards, the
 * inner and outer ends coincide, otherwise they are opposite. Accordingly,
 * the \c *_inner and \c *_outer members take an end in the respective
 * coordinates. inner_endId() and outer_endId() convert between them.
 * \see descriptors::ids for the meaning of the edge identifiers.
 * \tparam V Vertex the edge is designed to operate with.
 */
template<typename V>
struct Edge<V>
    : public Bond2<V>
{
    using Base = Bond2<V>;
    using Vertex = Base::Vertex;
    using End = Base::End;  ///< Edge ends.
    using EndId = End::Id;
    using weight_t = Property<real>::value_t;  ///< Type alias for edge weight.
    using Vertices = Base::Vertices;  ///< Container holding vertices.
    using VIDs = Base::VIDs;          ///< Handles of the end vertices.
    /// Descriptor by which a vertex records this edge.
    using DescrInVertex = descriptors::EdgeInVertex<IdScope::COMPONENT>;

    friend struct Chain<Edge>;

    static constexpr auto Type = Elements::Edge;

    /// Maximal edge weight.
    static constexpr auto maxWeight = huge<weight_t>;

    /// True iff the edge lives in a geometric space.
    static constexpr auto isSpatial = false;

// public variables

    using Base::id;   ///< Stable handle, unique over the program run.
    using Base::ind;  ///< Position in the graph (EgIdG).


    EgIdC indc {EgIdC::undefined};  ///< Position in the host component.
    EgIdA indw {EgIdA::undefined};  ///< Position in the host chain, from 0 at chain end A.

    ChIdG w {ChIdG::undefined};  ///< Identifier of the hosting chain.
    CmpId c {CmpId::undefined};  ///< Identifier of the hosting component.

    /// Edge weight, 1 by default: the length of the edge for the shortest
    /// path searches (see structure/paths), read and written by the PLY I/O.
    /// The transformations do not change it.
    Property<real> weight {"weight", "wt", weight_t(1.0)};

protected:  // variables

    /// Orientation relative to the host chain (see the class description).
    Orientation ornt {Orientation::Undefined};

    using Base::vv;  ///< Vertices connected to ends.

public:  // functions

    constexpr Edge() : Base{} {}  // Id is initialized, rest is undefined

    /** Constructor leaving chain and component data undefined.
     * \param[in] ind Index graph-wide.
     */
    explicit constexpr Edge(EgIdG ind) noexcept;

    /** Constructor leaving chain and component data undefined.
     * \param[in] ind Index graph-wide.
     * \param[in] ornt Orientation relative to the future host chain.
     */
    explicit constexpr Edge(
        EgIdG ind,
        Orientation ornt
    ) noexcept;

    /** Constructor leaving chain and component data undefined.
     * \param[in] ind Index graph-wide.
     * \param[in] ornt Orientation relative to the future host chain.
     * \param[in] weight Edge weight.
     */
    explicit constexpr Edge(
        EgIdG ind,
        Orientation ornt,
        weight_t weight
    ) noexcept;

    /** Constructor of a chain-embedded edge without component specs.
     * \param[in] ind Index graph-wide.
     * \param[in] indw Index in the host chain.
     * \param[in] w Index of the host chain.
     */
    explicit constexpr Edge(
        EgIdG ind,
        EgIdA indw,
        ChIdG w
    ) noexcept;

    /** Constructor setting all data members.
     * \param[in] ind Index graph-wide.
     * \param[in] indc Index component-wide.
     * \param[in] indw Index in the host chain.
     * \param[in] w Index of the host chain.
     * \param[in] c Index of the host component.
     */
    explicit constexpr Edge(
        EgIdG ind,
        EgIdC indc,
        EgIdA indw,
        ChIdG w,
        CmpId c
    ) noexcept;

    using Base::operator==;

    /** Assigns the edge to component \p cc at component-wide index
     * \p indcc, updating the descriptors held by its vertices.
     */
    constexpr void set_cmp(CmpId cc, EgIdC indcc) noexcept;

    /** Reverses the edge orientation.
     * \details With \p swapVV, also swaps the inner ends: the vertices and
     * their descriptors of the edge are relabelled, so that each outer end
     * keeps its vertex. Without, only \a ornt is flipped, which swaps the
     * vertices at the outer ends: used when the host chain is reversed.
     * \tparam swapVV Whether to swap the inner ends.
     */
    template<bool swapVV> void reverse();

    /** Edge orientation relative to the host chain.
     */
    constexpr auto orientation() const noexcept -> Orientation;

    /** Whether the inner and outer ends of the edge coincide.
     */
    constexpr bool points_forwards() const noexcept;

    /** Outer end corresponding to inner end \p innerE.
     */
    constexpr auto outer_endId(EndId innerE) const noexcept -> EndId;
    /** Inner end corresponding to outer end \p outerE.
     */
    constexpr auto inner_endId(EndId outerE) const noexcept -> EndId;

    /** The end opposite to \p e.
     */
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    /** \name Connection to vertices.
     * \details Connecting an end to a vertex also records the edge in the
     * vertex's descriptors; disconnecting removes the record. An end must
     * be free before it is connected.
     * @{
     */
    /// Connects inner end \p e to vertex \p v.
    constexpr void connect_to_inner(EndId e, Vertex& v);
    /// Connects outer end \p e to vertex \p v.
    constexpr void connect_to_outer(EndId e, Vertex& v);
    /// Disconnects inner end \p e from its vertex.
    constexpr void disconnect_end_inner(EndId e) noexcept;
    /// Disconnects outer end \p e from its vertex.
    constexpr void disconnect_end_outer(EndId e) noexcept;
    /// Checks if inner end \p e is connected to a vertex.
    constexpr bool is_connected_inner(EndId e) const noexcept;
    /// Checks if outer end \p e is connected to a vertex.
    constexpr bool is_connected_outer(EndId e) const noexcept;
    /// Checks if at least one end is connected to a vertex.
    constexpr bool is_connected() const noexcept;
    /// Checks if both ends are connected to vertices.
    constexpr bool is_connected_both_ends() const noexcept;
    /** Checks if edge \p eg shares a vertex with this edge.
     */
    constexpr bool is_neig_of(const Edge& eg) const noexcept;
    ///@}

    /// The vertices, indexed by inner end.
    constexpr auto vertices() const noexcept -> const Vertices& { return vv; }
    /// Vertex at outer end \p e, or nullptr.
    constexpr auto vertex_at_outer(EndId e) const noexcept -> const Vertex*;
    /// Vertex at outer end \p e, or nullptr.
    constexpr auto vertex_at_outer(EndId e) noexcept -> Vertex*;
    /// Vertex at inner end \p e, or nullptr.
    constexpr auto vertex_at_inner(EndId e) const noexcept -> const Vertex*;
    /// Vertex at inner end \p e, or nullptr.
    constexpr auto vertex_at_inner(EndId e) noexcept -> Vertex*;
    /// The vertex at the other end from vertex \p v, which must be at one
    /// end of this edge.
    constexpr auto vertex_opposite_to(Vertex* v) noexcept -> Vertex*;

    /// Handles of the vertices at inner ends A and B.
    constexpr auto vids_inner() const noexcept -> VIDs;
    /// Handle of the vertex at outer end \p e, or VIdH::undefined.
    constexpr auto vid_at_outer(const EndId e) const noexcept -> VIdH;
    /// Handle of the vertex at inner end \p e, or VIdH::undefined.
    constexpr auto vid_at_inner(const EndId e) const noexcept -> VIdH;

    /// Descriptor by which the vertex at inner end \p e records this edge.
    constexpr auto descriptor_for_vertex_at(
        const EndId e
    ) const noexcept -> DescrInVertex;


    /** Formatted printing out of the edge data.
     * \tparam ENDL Flag to end current line after the print.
     * \param[in] tag Prefix preceding the output.
     */
    template<bool ENDL=true>
    void print(std::string_view tag) const;

    // Testing

    /** Ensures that the edge indexes are within the bounds of their
     * containers, and the weight within its range.
     * \throws invariant_error If a check fails.
     * \param[in] nInd Total number of edges in the graph.
     * \param[in] nIndC Number of edges in the component.
     * \param[in] nIndW Number of edges in the chain.
     * \param[in] nCh Number of chains in the graph.
     * \param[in] nCmpt Number of connected components in the graph.
     */
    void check(std::size_t nInd,
               std::size_t nIndC,
               std::size_t nIndW,
               ChIdG nCh,
               CmpId nCmpt) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename V>
constexpr
Edge<V>::
Edge(const EgIdG ind) noexcept
    : Base {ind}
    , ornt {Orientation::Forwards}
{}


template<typename V>
constexpr
Edge<V>::
Edge(const EgIdG ind,
     const Orientation ornt
) noexcept
    : Base {ind}
    , ornt {ornt}
{}


template<typename V>
constexpr
Edge<V>::
Edge(const EgIdG ind,
     const Orientation ornt,
     const weight_t weight
) noexcept
    : Base {ind}
    , weight {"weight", "wt", weight}
    , ornt {ornt}
{}


template<typename V>
constexpr
Edge<V>::
Edge(
    const EgIdG ind,
    const EgIdA indw,
    const ChIdG w
) noexcept
    : Base {ind}
    , indw {indw}
    , w {w}
    , ornt {Orientation::Forwards}
{}


template<typename V>
constexpr
Edge<V>::
Edge(
    const EgIdG ind,
    const EgIdC indc,
    const EgIdA indw,
    const ChIdG w,
    const CmpId c
) noexcept
    : Base {ind}
    , indc {indc}
    , indw {indw}
    , w {w}
    , c {c}
    , ornt {Orientation::Forwards}
{}


template<typename V>
template<bool swapVV>
void Edge<V>::
reverse()
{
    if constexpr (swapVV) {
        for (const auto& e : End::Ids)
            if (is_connected_inner(e)) {

                const typename Vertex::EgDescr s {id, indc, e};
                auto v = vertex_at_inner(e);

                GRAFFINE_ASSERT(v->is_connected_to(s),
                    "Edge slot ", s.str_short(), " in connected vertex");

                auto& egSs = v->edges();
                auto node = egSs.extract(s);
                if (node) {
                    node.value().e = End::opp(e);
                    egSs.insert(std::move(node));
                }
            }
        std::swap(vv[End::A], vv[End::B]);
    }

    ornt = (ornt == Orientation::Forwards) ? Orientation::Backwards
                                           : Orientation::Forwards;
}


template<typename V>
constexpr
auto Edge<V>::
orientation() const noexcept -> Orientation
{
    return points_forwards() ? Orientation::Forwards
                             : Orientation::Backwards;
}


template<typename V>
constexpr
bool Edge<V>::
points_forwards() const noexcept
{
    return ornt == Orientation::Forwards;
}

template<typename V>
constexpr
auto Edge<V>::
inner_endId(const EndId outerE) const noexcept -> EndId
{
    GRAFFINE_ASSERT(End::is_valid(outerE), "outerE is not valid.");

    return points_forwards() ? outerE
                             : End::opp(outerE);
}

template<typename V>
constexpr
auto Edge<V>::
outer_endId(const EndId innerE) const noexcept -> EndId
{
    GRAFFINE_ASSERT(End::is_valid(innerE), "innerE is not valid.");

    return points_forwards() ? innerE
                             : End::opp(innerE);
}

template<typename V>
constexpr
auto Edge<V>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return End::opp(e);
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_outer(EndId e) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[inner_endId(e)];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_outer(EndId e) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[inner_endId(e)];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_inner(EndId e) const noexcept -> const Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_inner(EndId e) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_opposite_to(Vertex* v) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(vv[End::A]->operator==(*v) ||
           vv[End::B]->operator==(*v),
          "Vertex is not valid.");

    return vv[End::A]->operator==(*v) ? vv[End::B]
                                      : vv[End::A];
}

template<typename V>
constexpr
auto Edge<V>::
vids_inner() const noexcept -> VIDs
{
    return {vid_at_inner(End::A),
            vid_at_inner(End::B)};
}

template<typename V>
constexpr
auto Edge<V>::
vid_at_outer(const EndId e) const noexcept -> VIdH
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    const auto eInner = inner_endId(e);

    return vv[eInner] ? vv[eInner]->id : VIdH::undefined;
}

template<typename V>
constexpr
auto Edge<V>::
vid_at_inner(const EndId e) const noexcept -> VIdH
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[e] ? vv[e]->id : VIdH::undefined;
}

template<typename V>
constexpr
void Edge<V>::
connect_to_inner(
    const EndId e,   // inner end id, i.e. taking into account edge reversal
    Vertex& v
)
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");
    GRAFFINE_ASSERT(!is_connected_inner(e),
           "Vertex ", v.id, " cmp ", v.get_c(),
           " is already connected to inner end ", End::str(e),
           " of edge ind ", ind);

    typename Vertex::EgDescr s {id, indc, e};

    GRAFFINE_ASSERT(!v.is_connected_to(s), "EgDescr ", s.str_short(),
                                 " already present in vertex ", v.id);

    v.insert(std::move(s));

    vv[e] = &v;
}


template<typename V>
constexpr
void Edge<V>::
connect_to_outer(
    const EndId e,   // outer end id, i.e. in chain coordinates
    Vertex& v
)
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    const auto eInner = inner_endId(e);

    typename Vertex::EgDescr s {id, indc, eInner};

    GRAFFINE_ASSERT(!is_connected_outer(e), "Vertex ", v.id, " cmp ", v.get_c(),
                                    " is already connected to outer slot ",
                                    s.str_short());

    GRAFFINE_ASSERT(!v.is_connected_to(s),
           "EgDescr ", s.str_short(), " already present in vertex ", v.id);

    v.insert(std::move(s));

    vv[eInner] = &v;
}


template<typename V>
constexpr
void Edge<V>::
disconnect_end_outer(
    const EndId e   // outer end id, i.e. in chain coordinates
) noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");
    GRAFFINE_ASSERT(is_connected_outer(e),
           "Edge ", ind, " is not connected to outer end ", End::str(e));

    const auto eInner = inner_endId(e);

    typename Vertex::EgDescr s {id, indc, eInner};

    auto v = vertex_at_outer(e);

    GRAFFINE_ASSERT_CALLING(v->is_connected_to(s), v->print("ERROR"),
                   "EgDescr ", s.str_short(), " not found in vertex ", v->id);

    v->erase(s);

    vv[eInner] = nullptr;
}


template<typename V>
constexpr
void Edge<V>::
disconnect_end_inner(
    const EndId e   // inner end id, i.e. taking into account edge reversal
) noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");
    GRAFFINE_ASSERT(is_connected_inner(e),
           "Edge ", ind, " is not connected at inner end ", End::str(e));

    typename Vertex::EgDescr s {id, indc, e};

    auto v = vertex_at_inner(e);

    GRAFFINE_ASSERT(v->is_connected_to(s), "EgDescr ", s.str_short(),
                              " not found in vertex ", v->id);

    v->erase(s);

    vv[e] = nullptr;
}

template<typename V>
constexpr
bool Edge<V>::
is_connected_inner(
    const EndId e   // inner end id, i.e. taking into account edge reversal
) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
bool Edge<V>::
is_connected_outer(
    const EndId e   // outer end id, i.e. in chain coordinates
) const noexcept
{
    GRAFFINE_ASSERT(End::is_valid(e), "End not valid.");

    const auto eInner = inner_endId(e);

    return vv[eInner];
}


template<typename V>
constexpr
bool Edge<V>::
is_connected() const noexcept
{
    return vv[End::A] || vv[End::B];
}


template<typename V>
constexpr
bool Edge<V>::
is_connected_both_ends() const noexcept
{
    return vv[End::A] && vv[End::B];
}

template<typename V>
constexpr
bool Edge<V>::
is_neig_of(const Edge& eg) const noexcept
{
    for (const auto e : End::Ids) {

        const auto v = vertex_at_inner(e);

        if (!v)
            continue;

        if (v->is_connected_to(typename Vertex::EgDescr{eg.id, eg.indc, End::A}) ||
            v->is_connected_to(typename Vertex::EgDescr{eg.id, eg.indc, End::B}))

            return true;
    }
    return false;
}


template<typename V>
constexpr
void Edge<V>::
set_cmp(const CmpId cc,
        const EgIdC ic) noexcept
{
    c = cc;
    for (const auto e : End::Ids)
        if (vv[e])
            vv[e]->reind_edge(typename Vertex::EgDescr{id, indc, e}, ic);
    indc = ic;
}

template<typename V>
constexpr
auto Edge<V>::
descriptor_for_vertex_at(
    const EndId e
) const noexcept -> DescrInVertex
{
    return DescrInVertex {id, indc, e};
}


template<typename V>
template<bool ENDL>
void Edge<V>::
print(const std::string_view prefix) const
{
    jot<false>(prefix);
    jot<false>("[", indw, "]");
    jot<false>(colorcodes::MAGENTA, points_forwards()?'>':'<', colorcodes::RESET);
    jot<false>("id ", colorcodes::BOLDBLUE, id, colorcodes::RESET);
    jot<false>("ind ", colorcodes::BOLDGREEN, ind, colorcodes::RESET);
    jot<false>("indc ", colorcodes::MAGENTA, indc, colorcodes::RESET);
    jot<false>("w ", colorcodes::BOLDYELLOW, w, colorcodes::RESET);
    jot<false>("c ", colorcodes::CYAN, c, colorcodes::RESET);
    jot<false>("weight ", colorcodes::BOLDYELLOW, weight(), colorcodes::RESET);
    jot<false>("v [",
        colorcodes::BOLDYELLOW, vid_at_outer(End::A),
                                (points_forwards() ? " -> " : " <- "),
                                vid_at_outer(End::B), colorcodes::RESET,
        "]");
    jot<ENDL>("");
}


template<typename V>
void Edge<V>::
check(const std::size_t nInd,
      const std::size_t nIndC,
      const std::size_t nIndW,
      const ChIdG nCh,
      const CmpId nCmpt) const
{
    GRAFFINE_ENSURE(ind < nInd,
           "Edge.ind is out of range: ",
           "ind = ", ind, ", numEdges = ", nInd);

    GRAFFINE_ENSURE(indc < nIndC,
           "Edge.indc is out of range for ",
           "ind ", ind, ": ", "indc = ", indc, ", nIndC = ", nIndC);

    GRAFFINE_ENSURE(indw < nIndW,
           "Edge.indw is out of range for ",
           "ind ", ind, ": ", "indw = ", indw, ", nIndW = ", nIndW);

    GRAFFINE_ENSURE(w < nCh,
           "Edge.w is out of range for ",
           "ind ", ind, ": ", "w = ", w, ", nCh = ", nCh);

    GRAFFINE_ENSURE(c < nCmpt,
           "Edge.c is out of range for ",
           "ind ", ind, ": ", "c = ", c, ", nCmpt = ", nCmpt);

    GRAFFINE_ENSURE(weight() <= maxWeight,
           "Edge.weight is out of range for ",
           "ind ", ind, ": ", "weight = ", weight(), ", nWeight = ", maxWeight);
}


}  // namespace detail

template<typename V> using Edge = detail::Edge<V>;

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_EDGE_H
