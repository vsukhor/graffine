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
 * \file vertex.h
 * \brief The graph vertex.
 * \details Vertices are owned by the containers::Vertices collection of their
 * host component, which holds them by std::unique_ptr, so that their
 * addresses remain stable while the graph is transformed. A vertex records
 * the descriptors of its incident edges, while each edge points back to the
 * vertices at its ends.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_VERTEX_H
#define GRAFFINE_STRUCTURE_ELEMENTS_VERTEX_H

#include "graffine/definitions.h"
#include "graffine/property.h"
#include "graffine/structure/common.h"
#include "graffine/structure/descriptors/edge_in_vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/structure/containers/neigs.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include <algorithm>
#include <array>
#include <memory>
#include <ostream>
#include <set>
#include <unordered_set> // ordered set is not suitable because of operator==
#include <vector>

namespace graffine::structure {
namespace detail {


/** Generic vertex template, specialized by the kinds of data a vertex
 * holds: Vertex<> (handle only) and Vertex<ED> (with incident edges).
 */
template <typename... Ligands>
struct Vertex {};


/** A vertex having only an identifier, unique over the program run.
 */
template<>
struct Vertex<>
{
    using End = descriptors::End;
    static constexpr auto Type = Elements::Vertex;
    /// True iff the vertex lives in a geometric space.
    static constexpr auto isSpatial = false;

protected:  // Data

    VIdH id;  ///< Stable handle, unique over the program run.

private:

    static inline VIdH h {};   ///< Counter issuing the handles.

public:  // Functions

    /// Constructor issuing a new handle.
    constexpr Vertex()
        : id {create_id()}
    {}

    /// Constructor taking a given handle \p id, e.g. read from a file.
    explicit constexpr Vertex(const VIdH id)
        : id {id}
    {}

    /** Two vertices are equal if they have the same handle.
     */
    constexpr bool operator==(const Vertex& other) const noexcept
    {
        return id == other.id;
    }

    /// The stable handle.
    constexpr auto get_id() const noexcept -> VIdH { return id; }
    /// Sets the stable handle.
    constexpr void set_id(const VIdH i) noexcept   { id = i;}

    /**
     * \name In/Out operations.
     * @{
     */

    /** Prints attributes, preceded by \p s , to the message processor.
     */
    template<bool endLine=true>
    void print(const std::string_view s="") const
    {
        jot<false>(s, " ");

        jot<false>("id ", colorcodes::GREEN, id, colorcodes::RESET);

        if constexpr (endLine)
            jot("");
    }

    ///@}  // In/Out operations.

protected: // Functions

    /** Creates a handle unique over the program run, by incrementing a
     * counter shared by all vertices.
     */
    static constexpr auto create_id() noexcept -> VIdH
    {
        return ++h;
    }
};

using VertexFeaturelessUnconnected = Vertex<>;  ///< Vertex holding only a handle.

// /////////////////////////////////////////////////////////////////////////////

/// Descriptor of an incident edge, as recorded by a vertex.
using ED = descriptors::EdgeInVertex<IdScope::COMPONENT>;

/** A vertex knowing its incident edges and its host component.
 * \details The edges are recorded by descriptors (Vertex::edges()) holding
 * their component-wide index and the inner end connected to the vertex
 * (see Edge for inner and outer ends); the vertex degree is the number of
 * descriptors. A vertex can be \a fixed:
 * a flag for the user, stored and saved to PLY files, but not interpreted
 * by graffine.
 */
template<>
struct Vertex<ED>
    : public VertexFeaturelessUnconnected
{
    using Base = VertexFeaturelessUnconnected;
    template<typename T> using EgContainerTempl = containers::Neigs<T>;
    using EgDescr = ED;  ///< Descriptor of an incident edge.
    using EgDescrors = EgContainerTempl<EgDescr>;  ///< Set of descriptors.
    using FixedAttr = Property<bool>;  ///< Type of the \a fixed flag.

    static constexpr auto Type = Base::Type;
    static constexpr auto isSpatial = Base::isSpatial;

    using Base::id;
    using Base::get_id;
    using Base::set_id;

protected:  // Data

    CmpId     c;  ///< Identifier of the current hosting connected component.
    /// Whether the vertex is fixed; see is_fixed().
    FixedAttr isFixed {"isFixed", "F", {}};

    /// Descriptors of the incident edges: component-wide edge index and
    /// the inner end connected to this vertex (see Edge).
    EgDescrors egDescrors;

public:  // Functions

    /// Constructor of an unconnected vertex with a new handle, not
    /// assigned to a component.
    Vertex();
    /// Constructor of an unconnected vertex with handle \p id, not
    /// assigned to a component.
    explicit Vertex(VIdH id);
    /// Constructor of an unconnected vertex with handle \p id and the
    /// \a fixed flag \p isFixed, not assigned to a component.
    explicit Vertex(VIdH id, bool isFixed);

    /** Constructor of an unconnected vertex with a new handle.
     * \param[in] c Id of the graph connected component hosting the vertex.
     */
    explicit Vertex(CmpId c);

    /** Equality operator.
     * \details Declared here rather than taken from the base, whose operator
     * makes `a == b` ambiguous with its reversed form `b == a` in C++20.
     */
    constexpr bool operator==(const Vertex& other) const noexcept
    {
        return Base::operator==(other);
    }

    /** Vertex degree (number of incident edges).
     */
    constexpr auto deg() const noexcept -> Degree;

    /// Id of the host component.
    constexpr auto get_c() const noexcept -> CmpId { return c; }
    /// Sets the id of the host component.
    constexpr void set_c(CmpId i) noexcept   { c = i; }

    /**
     * \name Edge connectivity.
     * @{
     */

    /**
     * \name Edges connected to the given ones at this vertex.
     * \details Return the descriptors of the incident edges other than the
     * given ones, which must be incident; the result is empty, with a warning,
     * if there are no others.
     * @{
     */
    auto edges_connected_to(const EgDescr& s) const -> const EgDescrors;
    auto edges_connected_to(const std::array<EgDescr, 2>& ss) const
        -> const EgDescrors;
    auto edges_connected_to(const EgDescrors& s) const noexcept -> const EgDescrors;

    ///@}  // Edges connected to the given ones.

    /// Descriptors of the incident edges.
    constexpr auto edges() const noexcept -> const EgDescrors& { return egDescrors; }
    /// Descriptors of the incident edges.
    constexpr auto edges() noexcept -> EgDescrors&             { return egDescrors; }

    /** Descriptors of the edges connected at their inner end B, i.e.
     * pointing into this vertex by their own orientation.
     */
    auto edges_in() const noexcept -> EgDescrors;
    /** Descriptors of the edges connected at their inner end A, i.e.
     * pointing out of this vertex by their own orientation.
     */
    auto edges_out() const noexcept -> EgDescrors;

    /** Removes the descriptor \p g of an incident edge.
     * \return 1 if \p g was present, 0 otherwise (an assertion failure in
     * DEBUG builds).
     */
    auto erase(const EgDescr& g) noexcept -> EgDescrors::size_type;

    /** Records the incident edge described by \p s, which must be new.
     */
    constexpr void insert(const EgDescr& s) noexcept;
    /**
     * \overload
     */
    constexpr void insert(EgDescr&& s);

    /** Whether the edge described by \p s is incident.
     */
    constexpr bool is_connected_to(const EgDescr& s) const noexcept;
    /** Whether any edge is incident.
     */
    constexpr bool is_connected() const noexcept;

    /** Component-wide index of an edge joining this vertex and \p v, or
     * undefined if there is none.
     */
    auto common_edge(const Vertex& v) const noexcept -> EgIdC;

    /** Changes the component-wide index recorded for an incident edge.
     * \param[in] from Descriptor of the edge, holding its former index.
     * \param[in] to New component-wide index of the edge.
     * \return \a true if the edge was found.
     */
    bool reind_edge(EgDescr&& from,
                    EgIdC to) noexcept;

    /// Checks if the vertex is inside a chain: degree 2.
    constexpr bool is_bulk() const noexcept;
    /// Checks if the vertex is a free end: degree 1.
    constexpr bool is_leaf() const noexcept;
    /// Checks if the vertex is a Y-shaped junction: degree 3.
    constexpr bool is_junctionY() const noexcept;
    /// Checks if the vertex is an X-shaped junction: degree 4.
    constexpr bool is_junctionX() const noexcept;
    /// Checks if the vertex is a junction: degree above 2.
    constexpr bool is_junction() const noexcept;

    ///@}  // Edge connectivity.

    /**
     * \name Property operations.
     * @{
     */

    /** Whether the vertex is fixed (see the class description).
     */
    constexpr bool is_fixed() const noexcept { return isFixed(); }
    /// Sets the \a fixed flag.
    constexpr void fix() noexcept   { isFixed() = true; }
    /// Clears the \a fixed flag.
    constexpr void unfix() noexcept { isFixed() = false; }

    ///@}  // Property operations.

    /**
     * \name In/Out operations.
     * @{
     */

    /** Prints attributes, preceded by \p s , to the message processor.
     */
    template<bool endLine=true>
    void print(std::string_view s="") const;

    ///@}  // In/Out operations.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline
Vertex<ED>::
Vertex()
    : c {CmpId::undefined}
    , isFixed {FixedAttr{"isFixed", "F", {}}}
{}

inline
Vertex<ED>::
Vertex(
    const VIdH id
)
    : Base {id}
    , c {CmpId::undefined}
    , isFixed {"isFixed", "F", {}}
{}

inline
Vertex<ED>::
Vertex(
    const VIdH id,
    const bool isFixed
)
    : Base {id}
    , c {CmpId::undefined}
    , isFixed {"isFixed", "F", isFixed}
{}

inline
Vertex<ED>::
Vertex(
    const CmpId c
)
    : Base {}
    , c {c}
    , isFixed {"isFixed", "F", {}}
{}

constexpr
auto Vertex<ED>::
deg() const noexcept -> Degree
{
    return static_cast<Degree>(egDescrors.size());
}

constexpr
bool Vertex<ED>::
is_bulk() const noexcept
{
    return deg() == Deg2;
}

constexpr
bool Vertex<ED>::
is_leaf() const noexcept
{
    return deg() == Deg1;
}

constexpr
bool Vertex<ED>::
is_junctionY() const noexcept
{
    return deg() == Deg3;
}

constexpr
bool Vertex<ED>::
is_junctionX() const noexcept
{
    return deg() == Deg4;
}

constexpr
bool Vertex<ED>::
is_junction() const noexcept
{
    return deg() > Deg2;
}

inline
auto Vertex<ED>::
erase(const EgDescr& g) noexcept -> EgDescrors::size_type
{
    const auto num_erased = egDescrors.erase(g);

    GRAFFINE_ASSERT(num_erased, "Attempt to erase missing egDescr ", g.str_short());

    return num_erased;
}

constexpr
void Vertex<ED>::
insert(const EgDescr& s) noexcept
{
    GRAFFINE_ASSERT(!egDescrors.contains(s),
            "Attempt to insert repeating egDescr ", s.str_short());

    egDescrors.insert(s);
}

constexpr
void Vertex<ED>::
insert(EgDescr&& s)
{
    GRAFFINE_ASSERT(!egDescrors.contains(s),
           "Attempt to insert repeating egDescr ", s.str_short());

    egDescrors.insert(std::move(s));
}

constexpr
bool Vertex<ED>::
is_connected_to(const EgDescr& s) const noexcept
{
    return egDescrors.contains(s);
}

constexpr
bool Vertex<ED>::
is_connected() const noexcept
{
    return egDescrors.size();
}

inline
auto Vertex<ED>::
edges_connected_to(const EgDescr& s) const -> const EgDescrors
{
    if (is_leaf()) {

        GRAFFINE_ASSERT(egDescrors.contains(s),
               "Seeking neighbours of a missing LEAF edge slot", s.str_short());

        warn("Seeking neighbours of a LEAF edge end.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy

        GRAFFINE_ASSERT(ss.contains(s),
               "Seeking neighbours of a missing edge slot ", s.str_short());
        ss.erase(s);

        return ss;
    }
}

inline
auto Vertex<ED>::
edges_connected_to(
    const std::array<EgDescr, 2>& aa
) const -> const EgDescrors
{
    GRAFFINE_ASSERT(deg() >= 2, "egDescrors.size() < 2");

    if (deg() == 2) {
        warn("Neighbours of the slot pair is empty set.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy
        for (const auto& u : aa) {

            GRAFFINE_ASSERT(ss.contains(u),
                   "Seeking neighbours of a missing edge slot ", u.str_short());
            ss.erase(u);
        }
        return ss;
    }
}

inline
auto Vertex<ED>::
edges_connected_to(const EgDescrors& s) const noexcept -> const EgDescrors
{
    GRAFFINE_ASSERT(egDescrors.size() >= s.size(), "egDescrors.size() < s.size()");

    if (egDescrors.size() == s.size()) {
        warn("Edge neighbours is empty set.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy
        for (const auto& u : s) {

            GRAFFINE_ASSERT(ss.contains(u),
                   "Seeking neighbours of a missing edge slot ", u.str_short());

            ss.erase(u);
        }
        return ss;
    }
}

inline
auto Vertex<ED>::
edges_in() const noexcept -> EgDescrors
{
    EgDescrors ss;
    for (const auto& s : egDescrors)
        if (s.e == End::B)
            ss.insert(s);

    return ss;
}

inline
auto Vertex<ED>::
edges_out() const noexcept -> EgDescrors
{
    EgDescrors ss;
    for (const auto& s : egDescrors)
        if (s.e == End::A)
            ss.insert(s);

    return ss;
}

inline
bool Vertex<ED>::
reind_edge(EgDescr&& from,
           const EgIdC to) noexcept
{
    auto node = egDescrors.extract(from);
    if (node) {
       node.value().w = to;
        egDescrors.insert(std::move(node));
        return true;
    }

    return false;
}

inline
auto Vertex<ED>::
common_edge(const Vertex& v) const noexcept -> EgIdC
{
    GRAFFINE_ASSERT(id != v.id, "Called on itself.");

    for (const auto& e1 : egDescrors)
        for (const auto& e2 : v.egDescrors)
            if (e1.idh == e2.idh) {

                GRAFFINE_ASSERT(e1.e == descriptors::End::opp(e2.e),
                       "not e1.e == End::opp(e2.e)");

                return e1.w;
            }

    return EgIdC::undefined;
}

template<bool endLine>
void Vertex<ED>::
print(const std::string_view s) const
{
    Base::print<false>(s);

    jot<false>("d ", colorcodes::BOLDBLUE, deg(), colorcodes::RESET);
    jot<false>("c ", colorcodes::CYAN, c, colorcodes::RESET);
    jot<false>(" egDescrors: ");
    for (const auto& s : egDescrors)
        s.print(colorcodes::MAGENTA);

    isFixed.print();

    if constexpr (endLine)
        jot("");
}

}  // namespace::detail

using VertexFixable = detail::Vertex<detail::ED>;  ///< Vertex with incident edges and a \a fixed flag.
using Vertex = VertexFixable;  ///< The vertex type used by graffine.

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_VERTEX_H
