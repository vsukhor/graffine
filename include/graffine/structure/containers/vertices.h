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
 * \file vertices.h
 * \brief Container of the vertices of a component.
 * \details Each graph component owns its vertices through a Vertices
 * collection, which holds them by std::unique_ptr: vertex addresses remain
 * stable while the collection changes, so that edges can point to them.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_VERTICES_H
#define GRAFFINE_STRUCTURE_CONTAINERS_VERTICES_H

#include <algorithm>  // adjacent_find
#include <array>
#include <map>
#include <memory>
#include <utility>  // pair, make_pair
#include <vector>

#include "graffine/definitions.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/structure/descriptors/vertex_degrees.h"

#include "graffine/utils/constants.h"

namespace graffine::structure::containers {

/** Container owning the vertices of a component.
 * \details The vertices are not ordered: lookups by handle, by address or
 * by incident edge scan the container, at a cost linear in its size.
 * \tparam V Vertex class.
 * \tparam H Graph component class hosting the vertices.
 */
template<typename V,
         typename H>
struct Vertices
{
    using Host = H;  ///< Host component type.
    using Chain = Host::Chain;
    using Edge = Chain::Edge;
    using End = Host::End;
    using EndSlot = Host::EndSlot;  ///< Slot type for the edge ends.
    using EndId = End::Id;
    using EdgeSlotC = descriptors::EdgeSlotC;  ///< Edge end, by EgIdC.
    using Vertex = V;
    using Element = std::unique_ptr<Vertex>;  ///< Owning pointer to a vertex.
    using Container = std::vector<Element>;   ///< Underlying container.
    using Elements = Container;

protected:  // Data

    Container vv;  ///< Container holding vertices.

public:  // Functions

    Vertices() = default;
    ~Vertices() = default;
    constexpr Vertices(const Vertices& other) = delete;
    constexpr Vertices(Vertices&& other) = default;
    constexpr auto operator=(const Vertices& other) -> Vertices& = delete;
    constexpr auto operator=(Vertices&& other) -> Vertices& = default;

    /// The underlying container.
    constexpr auto get() const noexcept -> const Container&;
    /// The underlying container.
    constexpr auto get() noexcept -> Container&;
    /// Pointers to the vertices of degree \p d.
    constexpr auto get(Degree d) const noexcept -> std::vector<Vertex*>;

    /// Deletes all the vertices.
    constexpr void clear() noexcept;

    /// Number of vertices.
    constexpr auto num() const noexcept -> std::size_t;
    /// Number of vertices of degree \p d.
    constexpr auto num(Degree d) const noexcept -> std::size_t;

    /** Creates a new, unconnected vertex.
     * \param c Component to assign the vertex to.
     * \return Reference to the new vertex.
     */
    constexpr auto create_vertex(CmpId c = CmpId::undefined) -> Vertex&;

    /** Moves the vertices of chain \p m from this container to that of
     * component \p h.
     * \details The vertices not found here, e.g. already moved, are
     * skipped.
     * \return Number of the vertices moved.
     */
    constexpr auto move_to(Host& h, const Chain& m) -> std::size_t;
    /** Moves vertex \p v from this container to that of component \p h,
     * and assigns it to \p h.
     * \return False if \p v is not in this container.
     */
    constexpr bool move_to(Host& h, const Vertex* const v);

    /// Takes over vertex \p v.
    void insert(std::unique_ptr<Vertex>&& v);

    /** Takes over all the vertices of \p other, leaving it empty.
     * \note The vertices must already be assigned to the right component.
     */
    constexpr void append(Container& other);

    /// Deletes vertex \p vp, which must have no edges attached.
    constexpr void remove_disconnected(Vertex* vp) noexcept;
    /// Detaches end \p e of edge \p eg from its vertex, which must have no
    /// other edges attached, and deletes that vertex.
    constexpr void remove_connected_to_outer(Edge& eg,
                                             End::Id e) noexcept;
    /** Disconnects vertex \p id from its edges, which are hosted by
     * \p hst , and deletes it.
     */
    constexpr void remove(VIdH id, Host& hst) noexcept;

    /** Updates the vertices at the ends of edge \p from to refer to the
     * edge by the new component-wide index \p to.
     */
    constexpr void reind_edge(const Edge& from,
                              EgIdC to) noexcept;

    /// Assigns all the vertices to component \p c.
    void set_c(CmpId c) noexcept;

    /// Position in the container of the vertex with handle \p id, or
    /// \ref utils::undefined if absent.
    auto find_container_coord(VIdH id) const noexcept -> std::size_t;
    /// Position in the container of vertex \p v, or \ref utils::undefined
    /// if absent.
    auto find_container_coord(const Vertex* v) const noexcept -> std::size_t;
    /// The vertex to which edge end \p s is attached, or nullptr.
    auto find(const EdgeSlotC& s)       noexcept ->       Vertex*;
    /// The vertex to which edge end \p s is attached, or nullptr.
    auto find(const EdgeSlotC& s) const noexcept -> const Vertex*;
    /// The vertex with handle \p id, or nullptr.
    auto find(VIdH id)       noexcept ->       Vertex*;
    /// The vertex with handle \p id, or nullptr.
    auto find(VIdH id) const noexcept -> const Vertex*;

    /// The first vertex not assigned to component \p c, which is also
    /// printed, or nullptr if all are.
    auto check_c(CmpId c) const noexcept -> Vertex*;

    /// \name Formatted printing
    ///@{

    /// Writes the number of vertices to \p ofs.
    constexpr void report(std::ostream& ofs) const noexcept;

    /// Prints the vertices grouped by degree; \p args prefix each vertex.
    template<typename... Args>
    void print(Args&&... args) const;

    /// Prints the vertices of degree \p d; \p args prefix each vertex.
    template<typename... Args>
    void print_degree(
        Degree d,
        Args&&... args
    ) const;

    ///@}  // Formatted printing
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
get() const noexcept -> const Container&
{
    return vv;
}


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
get() noexcept -> Container&
{
    return vv;
}

template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
get(const Degree d) const noexcept -> std::vector<Vertex*>
{
    std::vector<Vertex*> w;
    for (auto& v : vv)
        if (v->deg() == d)
            w.push_back(v.get());

    return w;
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
clear() noexcept
{
    vv.clear();
}


template<typename V,
         typename H>
constexpr
bool Vertices<V, H>::
move_to(Host& h,
        const Vertex* const v)
{
    GRAFFINE_ASSERT(v, "Vertex not found");

    const auto i = find_container_coord(v);

    if (utils::is_defined(i)) {
        vv[i]->set_c(h.ind);
        h.vv.vv.push_back(std::move(vv[i]));
        vv.erase(vv.begin() + i);
        return true;
    }

    return false;
}


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
move_to(Host& h,
        const Chain& m) -> std::size_t
{
    std::size_t n {};

    n += move_to(h, m.g.front().vertex_at_outer(End::A));
    for (std::size_t a {1}; a < m.length(); ++a) {
        n += move_to(h, m.g[a-1].vertex_at_outer(End::B));
        n += move_to(h, m.g[a  ].vertex_at_outer(End::A));
    }
    n += move_to(h, m.g.back().vertex_at_outer(End::B));

    return n;
}


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
create_vertex(const CmpId c) -> Vertex&
{
    vv.push_back(std::move(std::make_unique<Vertex>(c)));

    return *vv.back();
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
remove_disconnected(Vertex* vp) noexcept
{
    const auto vid = vp->id();

    GRAFFINE_ASSERT(!vp->is_connected(), "Vertex is connected, indc: ", vid);

    const auto i = find_container_coord(vid);

    GRAFFINE_ASSERT(utils::is_defined(i), "Vertex not found, indc: ", vid);

    vv.erase(vv.begin() + i);
    vp = nullptr;
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
remove_connected_to_outer(Edge& eg,
                          const End::Id e) noexcept
{
    auto v1 = eg.vertex_at_outer(e);
    eg.disconnect_end_outer(e);
    remove_disconnected(v1);
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
remove(const VIdH id, Host& hst) noexcept
{
    const auto i = find_container_coord(id);

    GRAFFINE_ASSERT(utils::is_defined(i), "Vertex not found, id: ", id);

    // Copy: disconnecting an edge erases its descriptor from the vertex.
    const auto ee = vv[i]->edges();
    for (const auto& s : ee)
        hst.edge(s.indc()).disconnect_end_inner(s.e);

    vv.erase(vv.begin() + i);
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
reind_edge(const Edge& from,
           const EgIdC to) noexcept
{
    // Only the vertices at the ends of the edge refer to it. They are owned,
    // non-const, by this container.
    for (const auto e : End::Ids)
        if (const auto v = from.vertex_at_inner(e))
            const_cast<Vertex*>(v)->reind_edge(
                typename Vertex::EgDescr {from.id, from.indc, e}, to);
}


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
num() const noexcept -> std::size_t
{
    return vv.size();
}


template<typename V,
         typename H>
constexpr
auto Vertices<V, H>::
num(const Degree d) const noexcept -> std::size_t
{
    std::size_t n {};
    for (auto& v : vv)
        if (v->deg() == d)
            ++n;

    return n;
}


template<typename V,
         typename H>
void Vertices<V, H>::
set_c(const CmpId c) noexcept
{
    for (auto& v : vv)
        v->set_c(c);
}


template<typename V,
         typename H>
auto Vertices<V, H>::
find_container_coord(const VIdH vid) const noexcept -> std::size_t
{
    for (std::size_t i {}; i < vv.size(); ++i)
        if (vv[i]->id == vid)
            return i;

    return utils::undefined<size_t>;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find_container_coord(const Vertex* v) const noexcept -> std::size_t
{
    for (std::size_t i {}; i < vv.size(); ++i)
        if (vv[i].get() == v)
            return i;

    return utils::undefined<size_t>;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find(const EdgeSlotC& s) noexcept -> Vertex*
{
    for (auto& v : vv)
        for (const auto& d : v->edges())
            if (d.indc() == s.w && d.e == s.e)
                return v.get();

    return nullptr;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find(const EdgeSlotC& s) const noexcept -> const Vertex*
{
    for (auto& v : vv)
        for (const auto& d : v->edges())
            if (d.indc() == s.w && d.e == s.e)
                return v.get();

    return nullptr;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find(const VIdH id) noexcept -> Vertex*
{
    for (auto& v : vv)
        if (v->id == id)
            return v.get();

    return nullptr;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find(const VIdH id) const noexcept -> const Vertex*
{
    for (const auto& v : vv)
        if (v->id() == id)
            return v.get();

    return nullptr;
}

// Assumes that the vertices were already assigned to this container's
// component and connected as needed.
template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
append(Container& other)
{
    std::move(other.begin(), other.end(), std::back_inserter(vv));
    other.clear();
}

template<typename V,
         typename H>
void Vertices<V, H>::
insert(std::unique_ptr<Vertex>&& v)
{
    vv.push_back(std::move(v));
}

template<typename V,
         typename H>
auto Vertices<V, H>::
check_c(const CmpId c) const noexcept -> Vertex*
{
    for (const auto& v : vv)
        if (v->get_c() != c) {
            v->print("wrong c: must be "s + c.as_string());
            return v.get();
        }

    return nullptr;
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
report(std::ostream& ofs) const noexcept
{
    ofs << num() << " ";
}

template<typename V,
         typename H>
template<typename... Args>
void Vertices<V, H>::
print(Args&&... args) const
{
    std::set<Degree> degrees;
    for (const auto& v : vv)
        degrees.insert(v->deg());

    for (const auto d : degrees)
        print_degree(d, args...);
}

template<typename V,
         typename H>
template<typename... Args>
void Vertices<V, H>::
print_degree(
    const Degree d,
    Args&&... args) const
{
    jot(colorcodes::BOLDBLUE, "D ", colorcodes::RED, d,
        colorcodes::RESET, " (", num(d), ")");

    for (const auto& v : vv)
        if (v->deg() == d)
            v->print(to_string(args...));
}



// /////////////////////////////////////////////////////////////////////////////
//
// Namespace functions acting on Vertices.
//
// /////////////////////////////////////////////////////////////////////////////

/** Deletes all elements.
 * \tparam V Class encapsulating vertex object.
 * \tparam H Host graph component.
 */
template<typename V,
         typename H>
constexpr
void clear(Vertices<V, H>& vcl) noexcept
{
    vcl.clear();
}


/** Finds a vertex inside a collection by one of its incident edge ends.
 * \tparam V Class encapsulating vertex object.
 * \tparam H Host graph component.
 * \param[in] s Edge end attached to the vertex to find, by component-wide
 * edge index.
 * \param[in] ns Vertices to search.
 * \return Handle of the vertex found, or VIdH::undefined.
 */
template<typename V,
         typename H>
auto find_vertex(
    const descriptors::EdgeSlotC& s,
    const Vertices<V, H>& ns
) noexcept -> VIdH
{
    for (const auto& v : ns.get())
        for (const auto& d : v->edges())
            if (d.indc() == s.w && d.e == s.e)
                return v->get_id();

    return VIdH::undefined;
}

/** Prints the vertices of degree \p D held in the collection.
 * \tparam D Vertex degree.
 * \tparam V Class encapsulating vertex object.
 * \tparam H Host graph component.
 * \param[in] ns Vertices to print.
 * \param[in] s Optional string prefix for each vertex printout.
 */
template<Degree D,
         typename V,
         typename H>
void print(const Vertices<V, H>& ns,
           const std::string& s="") noexcept
{
    for (const auto& v : ns.get())
        if (v->deg() == D)
            v->print(s);
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_VERTICES_H
