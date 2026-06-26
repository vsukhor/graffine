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
 * \brief Contains classes holding and updating vertex collections.
 * \details Due to its edge-centric graph implementation, graffine does not
 * keep instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data. The Updaters are
 * defined for each if vertex degrees.
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

/**
 * Collection of veertices.
 * \tparam V Vertex class
 * \tparam H Graph compartment class hosting the vertices.
 */
template<typename V,
         typename H>
struct Vertices
{
    using Host = H;
    using Chain = Host::Chain;
    using Edge = Chain::Edge;
    using End = Host::End;
    using EndSlot = Host::EndSlot;  ///< Slot type for the edge ends.
    using EndId = End::Id;
    using EdgeSlotC = descriptors::EdgeSlotC;
    using Vertex = V;
    using Element = std::unique_ptr<Vertex>;
    using Container = std::vector<Element>;
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

    constexpr auto get() const noexcept -> const Container&;
    constexpr auto get() noexcept -> Container&;
    constexpr auto get(Degree d) const noexcept -> std::vector<Vertex*>;

    constexpr void clear() noexcept;

    constexpr auto num() const noexcept -> std::size_t;
    constexpr auto num(Degree d) const noexcept -> std::size_t;

    constexpr auto create_vertex(CmpId c = CmpId::undefined) noexcept -> Vertex&;

    constexpr auto move_to(Host& h, const Chain& m) noexcept -> VIdH;
    constexpr bool move_to(Host& h, const Vertex* const v) noexcept;

    void insert(std::unique_ptr<Vertex>&& v);

    constexpr void append(Container& other) noexcept;

    constexpr void remove_disconnected(Vertex* vp) noexcept;
    constexpr void remove_connected_to_outer(Edge& eg,
                                             End::Id e) noexcept;
    constexpr void remove(VIdH indc, Host& hst) noexcept;

    constexpr void reind_edge(const Edge& from,
                              EgIdC to) noexcept;

    void set_c(CmpId c) noexcept;

    auto find_container_coord(VIdH id) const noexcept -> std::size_t;
    auto find_container_coord(const Vertex* v) const noexcept -> std::size_t;
    auto find(const EdgeSlotC& s)       noexcept ->       Vertex*;
    auto find(const EdgeSlotC& s) const noexcept -> const Vertex*;
    auto find(VIdH id)       noexcept ->       Vertex*;
    auto find(VIdH id) const noexcept -> const Vertex*;

    //    auto check_types() const noexcept -> Vertex*;
    auto check_c(CmpId c) const noexcept -> Vertex*;

    /// \name Formatted printing
    ///@{

    constexpr void report(std::ostream& ofs) const noexcept;

    template<typename... Args>
    void print(Args&&... args) const;

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
        const Vertex* const v) noexcept
{
    ASSERT(v, "Vertex not found");

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
        const Chain& m) noexcept -> VIdH
{
    VIdH n {};

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
create_vertex(const CmpId c) noexcept -> Vertex&
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

    ASSERT(!vp->is_conected(), "Vertex is connected, indc: ", vid);

    const auto i = find_container_coord(vid);

    ASSERT(utils::is_defined(i), "Vertex not found, indc: ", vid);

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
remove(const VIdH indc, Host& hst) noexcept
{
    const auto i = find_container_coord(indc);

    ASSERT(is_defined(i), "Vertex not found, indc: ", indc);

    for (const auto& ee : vv[i].edges())
        hst.edge(ee.w).disconnect_end(ee.e);

    vv.erase(vv.begin() + i);
}

template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
reind_edge(const Edge& from,
           const EgIdC to) noexcept
{
    for (auto& v : vv) {
        v->reind_edge(typename Vertex::EgDescr{from.id, from.indc, End::A}, to);
        v->reind_edge(typename Vertex::EgDescr{from.id, from.indc, End::B}, to);
    }
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
        if (v->is_conected_to(s))
            return v.get();

    return nullptr;
}

template<typename V,
         typename H>
auto Vertices<V, H>::
find(const EdgeSlotC& s) const noexcept -> const Vertex*
{
    for (auto& v : vv)
        if (v->is_conected_to(s))
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

// Assumes that issues related to vertex types and connectivity were solved
// elsewhere so that the vertex can be placed into proper container
template<typename V,
         typename H>
constexpr
void Vertices<V, H>::
append(Container& other) noexcept
{
    std::move(other.begin(), other.end(), std::back_inserter(vv));
    other.clear();
//        vv.append_range(std::move(other));
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

/**
 * Deletes all elements.
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


/**
 * Finds a vertex inside a collection by one of its incident slots.
 * \tparam V Class encapsulating vertex object.
 * \tparam H Host graph component.
 * \param[in] s Slot of the vertex to find.
 * \param[in] ns Vertices of vertices to search.
 * \return Index of the found vertex or an undefined Id.
 */
template<typename V,
         typename H>
auto find_vertex(
    const typename Vertices<V, H>::EndSlot& s,
    const Vertices<V, H>& ns
) noexcept -> VIdH
{
    for (const auto& v : ns.get())
        if (v.contains(s))
            return v->id();

    return VIdH::undefined;
}

/**
 * Prints all the vertices held in the collection.
 * \tparam D Vertex degree.
 * \tparam V Class encapsulating vertex object.
 * \tparam H Host graph component.
 * \param[in] ns Vertices of vertices to print.
 * \param[in] s Optional string prefix for each vertex printout.
 */
template<Degree D,
         typename V,
         typename H>
void print(const Vertices<V, H>& ns,
           const std::string& s="") noexcept
{
    for (const auto& v : ns.get()[D])
        v.print(s);
}


}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_VERTICES_H
