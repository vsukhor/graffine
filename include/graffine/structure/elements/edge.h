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
 * \brief The graph graffine::Edge struct template.
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
#include <fstream>
#include <ostream>


namespace graffine::structure {
namespace detail {

template<typename V>
struct Bond2
{
    using Vertex = V;
    using End = descriptors::End;  ///< Bond ends.
    using EndId = End::Id;
    using Vertices = std::array<Vertex*, End::num>;
    using VIDs = std::array<VIdH, End::num>;
    using DescrInVertex = descriptors::EdgeInVertex<IdScope::GRAPH>;

    /// True iff the edge lives in a geometric space.
    static constexpr auto isSpatial = false;

// public variables

    EgIdH id;  ///< Unique identifier.
    EgIdG ind {EgIdG::undefined};   ///< Identifier container-wide.

protected:  // variables

    Vertices vv {{nullptr, nullptr}};  ///< Vertices.

public:  // functions

    constexpr Bond2() noexcept;  // Id is initialized, rest is undefined

    /**
     * Constructor with both chain all specs unset.
     * \param[in] ind Index graph-wide.
     */
    explicit constexpr Bond2(EgIdG ind) noexcept;

    /**
     * Import constructor.
     * \param[in] ifs Input file stream for reading in edge attributes.
     */
    explicit Bond2(std::ifstream& ifs);

    constexpr bool operator==(const Bond2& other) const noexcept;

    constexpr auto vertex_at(EndId e) const noexcept -> const Vertex*;
    constexpr auto vertex_at(EndId e)       noexcept ->       Vertex*;
    constexpr auto vids() const noexcept -> VIDs;
    constexpr auto vid_at(const EndId e) const noexcept -> VIdH;

    constexpr auto descriptor_for_vertex_at(
        const EndId e
    ) const noexcept -> DescrInVertex;

    // In/Out

    /**
     * Read the edge from a binary file.
     * \param[in] ofs Output file stream.
     */
    void read(std::ifstream& ofs);

    /**
     * Write the edge to a binary file.
     * \param[in] ofs Output file stream.
     */
    void write(std::ofstream& ofs) const;

    /**
     * Formatted printing out of the edge data.
     * \tparam ENDL Flag to end current line after the print.
     * \param[in] tag Prefix preceding the output.
    */
    template<bool ENDL=true>
    void print(std::string_view tag) const;

protected:  // functions

    /// Create identifier unique over the program run.
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
Bond2<V>::
Bond2(std::ifstream& ifs)
{
    read(ifs);
}


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
    ASSERT(End::is_valid(e), "End not valid.");

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
void Bond2<V>::
read(std::ifstream& ifs)
{
    ifs.read(reinterpret_cast<char*>(&ind), sizeof(ind));
    ifs.read(reinterpret_cast<char*>(&vv[End::A]), sizeof(VIdH));
    ifs.read(reinterpret_cast<char*>(&vv[End::B]), sizeof(VIdH));
}

template<typename V>
void Bond2<V>::
write(std::ofstream& ofs) const
{
    ofs.write(reinterpret_cast<const char*>(&ind), sizeof(ind));
    ofs.write(reinterpret_cast<const char*>(&vv[End::A]), sizeof(VIdH));
    ofs.write(reinterpret_cast<const char*>(&vv[End::B]), sizeof(VIdH));
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

template<typename V,
         typename... Props>
struct Edge
{
    using Vertex = V;
    static constexpr auto Type = Elements::Edge;
};


template<typename, typename...> struct Chain;

/**
 * The graph Edge.
 * \details Edge is a minimal structural unit of a graph.
 * The struct implements the tasks and properties specific to a single edge
 * and its relation to other components.
 * \tparam V Vertex the edge is desined to operate with.
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
    using VIDs = Base::VIDs;
    using DescrInVertex = descriptors::EdgeInVertex<IdScope::COMPONENT>;

    friend struct Chain<Edge>;

    static constexpr auto Type = Elements::Edge;

    /// Maximal edge weight.
    static constexpr auto maxWeight = huge<weight_t>;

    /// True iff the edge lives in a geometric space.
    static constexpr auto isSpatial = false;

// public variables

    using Base::id;   ///< Unique identifier.
    using Base::ind;  ///< Identifier container-wide.


    EgIdC indc {EgIdC::undefined};  ///< Identifier component-wide.
    EgIdA indw {EgIdA::undefined};  ///< Identifier chain-wide.

    ChIdG w {ChIdG::undefined};  ///< Identifier of the hosting chain.
    CmpId c {CmpId::undefined};  ///< Identifier of the hosting component.

    /// Example of an edge property: weight real variable.
    Property<real> weight {"weight", "wt", weight_t(1.0)};

protected:  // variables

    Orientation ornt {Orientation::Undefined};

    using Base::vv;  ///< Vertices connected to ends.

public:  // functions

    constexpr Edge() : Base{} {}  // Id is initialized, rest is undefined

    /**
     * Constructor with both chain all specs unset.
     * \param[in] ind Index graph-wide.
     */
    explicit constexpr Edge(EgIdG ind) noexcept;

    explicit constexpr Edge(
        EgIdG ind,
        Orientation ornt
    ) noexcept;

    explicit constexpr Edge(
        EgIdG ind,
        Orientation ornt,
        weight_t weight
    ) noexcept;

    /**
     * Constructor of a chain-embedded edge without component specs.
     * \param[in] ind Index graph-wide.
     * \param[in] indw Index in the host chain.
     * \param[in] w Index of the host chain.
     */
    explicit constexpr Edge(
        EgIdG ind,
        EgIdA indw,
        ChIdG w
    ) noexcept;

    /**
     * Constructor setting all data members.
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

    /**
     * Import constructor.
     * \param[in] ifs Input file stream for reading in edge attributes.
     */
    explicit Edge(std::ifstream& ifs);

    using Base::operator==;

    constexpr void set_cmp(CmpId cc, EgIdC indcc) noexcept;

    /// Swap the edge ends.
    template<bool swapVV> void reverse();

    /// Get edge orientation.
    constexpr auto orientation() const noexcept -> Orientation;

    constexpr bool points_forwards() const noexcept;

    constexpr auto outer_endId(EndId innerE) const noexcept -> EndId;
    constexpr auto inner_endId(EndId outerE) const noexcept -> EndId;

    /// Return index of the edge end opposite to 'e'.
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    constexpr void connect_to_inner(EndId e, Vertex& v) noexcept;
    constexpr void connect_to_outer(EndId e, Vertex& v) noexcept;
    constexpr void disconnect_end_inner(EndId e) noexcept;
    constexpr void disconnect_end_outer(EndId e) noexcept;
    constexpr bool is_connected_inner(EndId e) const noexcept;
    constexpr bool is_connected_outer(EndId e) const noexcept;
    constexpr bool is_connected() const noexcept;
    constexpr bool is_connected_both_ends() const noexcept;
    constexpr bool is_neig_of(const Edge& eg) const noexcept;

    constexpr auto vertices() const noexcept -> const Vertices& { return vv; }
    constexpr auto vertex_at_outer(EndId e) const noexcept -> const Vertex*;
    constexpr auto vertex_at_outer(EndId e) noexcept -> Vertex*;
    constexpr auto vertex_at_inner(EndId e) const noexcept -> const Vertex*;
    constexpr auto vertex_at_inner(EndId e) noexcept -> Vertex*;
    constexpr auto vertex_opposite_to(Vertex* v) noexcept -> Vertex*;

    constexpr auto vids_inner() const noexcept -> VIDs;
    constexpr auto vid_at_outer(const EndId e) const noexcept -> VIdH;
    constexpr auto vid_at_inner(const EndId e) const noexcept -> VIdH;

    constexpr auto descriptor_for_vertex_at(
        const EndId e
    ) const noexcept -> DescrInVertex;


    // In/Out

    /**
     * Read the edge from a binary file.
     * \param[in] ofs Output file stream.
     */
    void read(std::ifstream& ofs);

    /**
     * Write the edge to a binary file.
     * \param[in] ofs Output file stream.
     */
    void write(std::ofstream& ofs) const;

    /**
     * Formatted printing out of the edge data.
     * \tparam ENDL Flag to end current line after the print.
     * \param[in] tag Prefix preceding the output.
    */
    template<bool ENDL=true>
    void print(std::string_view tag) const;

    // Testing

    /**
     * Ensures that the edge indexes are consistant within the graph.
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
Edge<V>::
Edge(std::ifstream& ifs)
{
    read(ifs);
}


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

                ASSERT(v->is_conected_to(s),
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
    ASSERT(End::is_valid(outerE), "outerE is not valid.");

    return points_forwards() ? outerE
                             : End::opp(outerE);
}

template<typename V>
constexpr
auto Edge<V>::
outer_endId(const EndId innerE) const noexcept -> EndId
{
    ASSERT(End::is_valid(innerE), "innerE is not valid.");

    return points_forwards() ? innerE
                             : End::opp(innerE);
}

template<typename V>
constexpr
auto Edge<V>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    ASSERT(End::is_valid(e), "End not valid.");

    return End::opp(e);
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_outer(EndId e) const noexcept -> const Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[inner_endId(e)];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_outer(EndId e) noexcept -> Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[inner_endId(e)];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_inner(EndId e) const noexcept -> const Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_at_inner(EndId e) noexcept -> Vertex*
{
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
auto Edge<V>::
vertex_opposite_to(Vertex* v) noexcept -> Vertex*
{
    ASSERT(vv[End::A]->operator==(*v) ||
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
    ASSERT(End::is_valid(e), "End not valid.");

    const auto eInner = inner_endId(e);

    return vv[eInner] ? vv[eInner]->id : VIdH::undefined;
}

template<typename V>
constexpr
auto Edge<V>::
vid_at_inner(const EndId e) const noexcept -> VIdH
{
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[e] ? vv[e]->id : VIdH::undefined;
}

template<typename V>
constexpr
void Edge<V>::
connect_to_inner(
    const EndId e,   // inner end id, i.e. taking into account edge reversal
    Vertex& v
) noexcept
{
    ASSERT(End::is_valid(e), "End not valid.");
    ASSERT(!is_connected_inner(e),
           "Vertex ", v.id, " cmp ", v.get_c(),
           " is already connected to inner end ", End::str(e),
           " of edge ind ", ind);

    typename Vertex::EgDescr s {id, indc, e};

    ASSERT(!v.is_conected_to(s), "EgDescr ", s.str_short(),
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
) noexcept
{
    ASSERT(End::is_valid(e), "End not valid.");

    const auto eInner = inner_endId(e);

    typename Vertex::EgDescr s {id, indc, eInner};

    ASSERT(!is_connected_outer(e), "Vertex ", v.id, " cmp ", v.get_c(),
                                    " is already connected to outer slot ",
                                    s.str_short());

    ASSERT(!v.is_conected_to(s),
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
    ASSERT(End::is_valid(e), "End not valid.");
    ASSERT(is_connected_outer(e),
           "Edge ", ind, " is not connected to outer end ", End::str(e));

    const auto eInner = inner_endId(e);

    typename Vertex::EgDescr s {id, indc, eInner};

    auto v = vertex_at_outer(e);

    ASSERT_CALLING(v->is_conected_to(s), v->print("ERROR"),
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
    ASSERT(End::is_valid(e), "End not valid.");
    ASSERT(is_connected_inner(e),
           "Edge ", ind, " is not connected at inner end ", End::str(e));

    typename Vertex::EgDescr s {id, indc, e};

    auto v = vertex_at_inner(e);

    ASSERT(v->is_conected_to(s), "EgDescr ", s.str_short(),
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
    ASSERT(End::is_valid(e), "End not valid.");

    return vv[e];
}

template<typename V>
constexpr
bool Edge<V>::
is_connected_outer(
    const EndId e   // outer end id, i.e. in chain coordinates
) const noexcept
{
    ASSERT(End::is_valid(e), "End not valid.");

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
            return false;

        if (v->is_conected_to(typename Vertex::EgDescr{eg.id, eg.indc, End::A}) ||
            v->is_conected_to(typename Vertex::EgDescr{eg.id, eg.indc, End::B}))

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
void Edge<V>::
read(std::ifstream& ifs)
{
    ifs.read(reinterpret_cast<char*>(&ind), sizeof(ind));
    ifs.read(reinterpret_cast<char*>(&indc), sizeof(indc));
    ifs.read(reinterpret_cast<char*>(&indw), sizeof(indw));
    ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
    ifs.read(reinterpret_cast<char*>(&c), sizeof(c));
    ifs.read(reinterpret_cast<char*>(&weight), sizeof(weight));
    ifs.read(reinterpret_cast<char*>(&ornt), sizeof(ornt));
    ifs.read(reinterpret_cast<char*>(&vv[End::A]), sizeof(VIdH));
    ifs.read(reinterpret_cast<char*>(&vv[End::B]), sizeof(VIdH));
}


template<typename V>
void Edge<V>::
write(std::ofstream& ofs) const
{
    ofs.write(reinterpret_cast<const char*>(&ind), sizeof(ind));
    ofs.write(reinterpret_cast<const char*>(&indc), sizeof(indc));
    ofs.write(reinterpret_cast<const char*>(&indw), sizeof(indw));
    ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));
    ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
    ofs.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
    ofs.write(reinterpret_cast<const char*>(&ornt), sizeof(ornt));
    ofs.write(reinterpret_cast<const char*>(&vv[End::A]), sizeof(VIdH));
    ofs.write(reinterpret_cast<const char*>(&vv[End::B]), sizeof(VIdH));
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
    ENSURE(ind < nInd,
           "Edge.cEdge.ind is out of range: ",
           "ind = ", ind, ", numEdges = ", nInd);

    ENSURE(indc < nIndC,
           "Edge.indc is out of range for ",
           "ind ", ind, ": ", "indc = ", indc, ", nIndC = ", nIndC);

    ENSURE(indw < nIndW,
           "EdgEdge.indw is out of range for ",
           "ind ", ind, ": ", "indw = ", indw, ", nIndW = ", nIndW);

    ENSURE(w < nCh,
           "Edge.w is out of range for ",
           "ind ", ind, ": ", "w = ", w, ", nCh = ", nCh);

    ENSURE(c < nCmpt,
           "Edge.c is out of range for ",
           "ind ", ind, ": ", "c = ", c, ", nCmpt = ", nCmpt);

    ENSURE(weight() <= maxWeight,
           "Edge.weight is out of range for ",
           "ind ", ind, ": ", "weight = ", weight(), ", nWeight = ", maxWeight);
}


}  // namespace detail

template<typename V> using Edge = detail::Edge<V>;

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_EDGE_H
