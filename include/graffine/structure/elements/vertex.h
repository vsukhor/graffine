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
 * \brief Contains implementation of various vertex types and their containers.
 * \details Due to its edge-centric graph implementation, graffine does not keep
 * instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data.
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


template <typename... Ligands>
struct Vertex {};
/**
 * A vertex in a graph.
 */


template<>
struct Vertex<>
{
    using End = descriptors::End;
    static constexpr auto Type = Elements::Vertex;
    static constexpr auto isSpatial = false;

protected:  // Data

    VIdH id;  ///< Unique identifier.

private:

    static inline VIdH h {};   ///< In-class id counter;

public:  // Functions

    constexpr Vertex();
    explicit constexpr Vertex(VIdH id);

    /// Equality operator.
    constexpr bool operator==(const Vertex& other) const noexcept;

    constexpr auto get_id() const noexcept -> VIdH { return id; }
    constexpr void set_id(const VIdH i) noexcept   { id = i;}

    /**
     * \name In/Out operations.
     * @{
     */

    /// Prints attributes to an output message processor 'os'.
    template<bool endLine=true>
    void print(std::string_view s="") const;

    ///@}  // In/Out operations.

protected: // Functions

    /// Create identifier unique over the program run.
    static constexpr auto create_id() noexcept -> VIdH
    {
        return ++h;
    }
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

constexpr
Vertex<>::
Vertex()
    : id {create_id()}
{}

constexpr
Vertex<>::
Vertex(const VIdH id)
    : id {id}
{}

constexpr
bool Vertex<>::
operator==(const Vertex& other) const noexcept
{
    return id == other.id;
}

template<bool endLine>
void Vertex<>::
print(const std::string_view s) const
{
    jot<false>(s, " ");

    jot<false>("id ", colorcodes::GREEN, id, colorcodes::RESET);

    if constexpr (endLine)
        jot("");
}

using VertexFeaturelessUnconnected = Vertex<>;

// /////////////////////////////////////////////////////////////////////////////

using ED = descriptors::EdgeInVertex<IdScope::COMPONENT>;

template<>
struct Vertex<ED>
    : public VertexFeaturelessUnconnected
{
    using Base = VertexFeaturelessUnconnected;
    template<typename T> using EgContainerTempl = containers::Neigs<T>;
    using EgDescr = ED;
    using EgDescrors = EgContainerTempl<EgDescr>;
    using FixedAttr = Property<bool>;

    static constexpr auto Type = Base::Type;
    static constexpr auto isSpatial = Base::isSpatial;

    using Base::id;
    using Base::operator==;
    using Base::get_id;
    using Base::set_id;

protected:  // Data

    CmpId     c;  ///< Identifier of the current hosting connected component.
    FixedAttr isFixed {"isFixed", "F", {}};

    /// Component-wide edge indexes and ends of incident edge descriptors.
    EgDescrors egDescrors;

public:  // Functions

    constexpr Vertex();
    explicit constexpr Vertex(VIdH id);
    explicit constexpr Vertex(VIdH id, bool isFixed);

    /**
     * Constructor for an unconnected vertex.
     * \param[in] c Id of the graph connected component hosting the vertex.
     */
    explicit constexpr Vertex(CmpId c);

    /// Vertex degree (number of incident edges).
    constexpr auto deg() const noexcept -> Degree;

    constexpr auto get_c() const noexcept -> CmpId { return c; }
    constexpr void set_c(CmpId i) noexcept   { c = i; }

    /**
     * \name Edge connectivity.
     * @{
     */

    auto edges_connected_to(const EgDescr& s) const noexcept -> const EgDescrors;
    auto edges_connected_to(const std::array<EgDescr, 2>& ss) const noexcept
        -> const EgDescrors;
    auto edges_connected_to(const EgDescrors& s) const noexcept -> const EgDescrors;

    constexpr auto edges() const noexcept -> const EgDescrors& { return egDescrors; }
    constexpr auto edges() noexcept -> EgDescrors&             { return egDescrors; }

    auto edges_in() const noexcept -> EgDescrors;
    auto edges_out() const noexcept -> EgDescrors;

    auto erase(const EgDescr& g) noexcept -> EgDescrors::size_type;

    constexpr void insert(const EgDescr& s) noexcept;
    constexpr void insert(EgDescr&& s) noexcept;

    constexpr bool is_conected_to(const EgDescr& s) noexcept;
    constexpr bool is_conected() const noexcept;

    auto common_edge(const Vertex& v) const noexcept -> EgIdC;

    bool reind_edge(EgDescr&& from,
                    EgIdC to) noexcept;

    constexpr bool is_bulk() const noexcept;
    constexpr bool is_leaf() const noexcept;
    constexpr bool is_junctionY() const noexcept;
    constexpr bool is_junctionX() const noexcept;
    constexpr bool is_junction() const noexcept;

    ///@}  // Edge connectivity.

    /**
     * \name Property operations.
     * @{
     */

    constexpr bool is_fixed() const noexcept { return isFixed(); }
    constexpr void fix() noexcept   { isFixed() = true; }
    constexpr void unfix() noexcept { isFixed() = false; }

    ///@}  // Property operations.

    /**
     * \name In/Out operations.
     * @{
     */

    /// Prints attributes to an output message processor 'os'.
    template<bool endLine=true>
    void print(std::string_view s="") const;

    ///@}  // In/Out operations.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

constexpr
Vertex<ED>::
Vertex()
    : c {CmpId::undefined}
    , isFixed {FixedAttr{"isFixed", "F", {}}}
{}


constexpr
Vertex<ED>::
Vertex(
    const VIdH id
)
    : Base {id}
    , c {CmpId::undefined}
    , isFixed {"isFixed", "F", {}}
{}

constexpr
Vertex<ED>::
Vertex(
    const VIdH id,
    const bool isFixed
)
    : Base {id}
    , c {CmpId::undefined}
    , isFixed {"isFixed", "F", isFixed}
{}

constexpr
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

    ASSERT(num_erased, "Attempt to erase missing egDescr ", g.str_short());

    return num_erased;
}

constexpr
void Vertex<ED>::
insert(const EgDescr& s) noexcept
{
    ASSERT(!egDescrors.contains(s),
            "Attempt to insert repeting egDescr ", s.str_short());

    egDescrors.insert(s);
}

constexpr
void Vertex<ED>::
insert(EgDescr&& s) noexcept
{
    ASSERT(!egDescrors.contains(s),
           "Attempt to insert repeting egDescr ", s.str_short());

    egDescrors.insert(std::move(s));
}

constexpr
bool Vertex<ED>::
is_conected_to(const EgDescr& s) noexcept
{
    return egDescrors.contains(s);
}

constexpr
bool Vertex<ED>::
is_conected() const noexcept
{
    return egDescrors.size();
}

inline
auto Vertex<ED>::
edges_connected_to(const EgDescr& s) const noexcept -> const EgDescrors
{
    if (is_leaf()) {

        ASSERT(egDescrors.contains(s),
               "Seeking neigbours of a missing LEAF edge slot", s.str_short());

        warn("Seeking neigbours of a LEAF edge end.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy

        ASSERT(ss.contains(s),
               "Seeking neigbours of a missing edge slot ", s.str_short());
        ss.erase(s);

        return ss;
    }
}

inline
auto Vertex<ED>::
edges_connected_to(
    const std::array<EgDescr, 2>& aa
) const noexcept -> const EgDescrors
{
    ASSERT(deg() >= 2, "egDescrors.size() < 2");

    if (deg() == 2) {
        warn("Neigbours of the slot pair is empty set.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy
        for (const auto& u : aa) {

            ASSERT(ss.contains(u),
                   "Seeking neigbours of a missing edge slot ", u.str_short());
            ss.erase(u);
        }
        return ss;
    }
}

inline
auto Vertex<ED>::
edges_connected_to(const EgDescrors& s) const noexcept -> const EgDescrors
{
    ASSERT(egDescrors.size() >= s.size(), "egDescrors.size() < s.size()");

    if (egDescrors.size() == s.size()) {
        warn("Edge neigbours is empty set.");

        return {};
    }
    else {
        auto ss = egDescrors;  // copy
        for (const auto& u : s) {

            ASSERT(ss.contains(u),
                   "Seeking neigbours of a missing edge slot ", u.str_short());

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
    ASSERT(id != v.id, "Called on itself.");

    for (const auto& e1 : egDescrors)
        for (const auto& e2 : v.egDescrors)
            if (e1.idh == e2.idh) {

                ASSERT(e1.e == descriptors::End::opp(e2.e),
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

using VertexFixable = detail::Vertex<detail::ED>;
using Vertex = VertexFixable;

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_VERTEX_H
