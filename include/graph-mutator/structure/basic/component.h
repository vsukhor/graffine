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
 * @file component.h
 * @brief Contains class template defining disconnected components of a graph.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_BASIC_COMPONENT_H
#define GRAPH_MUTATOR_STRUCTURE_BASIC_COMPONENT_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <vector>

#include "../../definitions.h"
#include "../../vector_container.h"
#include "chain.h"
#include "chain_collection.h"
#include "chain_indexes.h"
#include "ends.h"
#include "slot.h"
#include "vertices/collections.h"
#include "vertices/all.h"
#include "vertices/vertex.h"

namespace graph_mutator::structure::basic {

namespace components {


template<typename C>
struct Container
    : public VectorContainer<C> {

using Base = VectorContainer<C>;
using size_type = Base::size_type;

constexpr auto num() const noexcept -> size_type;

constexpr auto ind_last() const noexcept -> size_type;

void print(const std::string& tag) const;

private:

    using Base::data;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename C>
constexpr
auto Container<C>::
num() const noexcept -> size_type
{
    return this->size();
}


template<typename C>
constexpr
auto Container<C>::
ind_last() const noexcept -> size_type
{
    const auto n = num();

    return n ? num() - 1
             : undefined<size_type>;
}



template<typename C>
void Container<C>::
print(const std::string& tag) const
{
    for (const auto& c: data)
        c.print(tag);
}


// /////////////////////////////////////////////////////////////////////////////

struct Gl {

    ChId w;   ///< Chain indexes.
    EgId a;   ///< Chain::idw.
    EgId i;   ///< Edge global indexes.

    void print(const EgId j) const noexcept
    {
        log_(j, "     ", w, "  ", a, "  ", i);
    }
};


// /////////////////////////////////////////////////////////////////////////////

/**
 * @brief Disconnected component of a graph.
 * @tparam Chain Type of the Chain forming the graph.
 */
template<typename Ch>
struct DisconnectedUnit {

    using Edge = Ch::Edge;
    using Edges = Ch::Edges;
    using Chain = Ch;
    using Chains = ChainContainer<Ch>;
    using Ends = Ch::Ends;
    using EndSlot = Ch::EndSlot;
    using BulkSlot = Ch::BulkSlot;
    using EdgeWeight = typename Edge::weight_t;
    using Vertices = vertices::All<DisconnectedUnit>;

    CmpId ind {undefined<CmpId>};

    std::vector<Gl> gl;       ///< Edge descriptors.

    // cpcn
    ChIds ww;                 ///< Chain indices.

    // cpagl
    vec2<EgId> ajlg;           ///< Edge adjacency_list.
    vec2<vertices::Id> ajlev;  ///< Vertex adjacency_list for end vertices.

    vec2<ChId> ajlw;           ///< Chain adjacency_list.
    vec2<ChId> ajlwA;          ///< Chain adjacency_list in direction backwards.
    vec2<ChId> ajlwB;          ///< Chain adjacency_list in direction forwards.

    Vertices vertices;

    ChainIndexes<true, EndSlot> chis;  ///< Chain indexes according to end degrees.

    constexpr explicit DisconnectedUnit(
        CmpId ind,
        Chains& cn
    );

    constexpr explicit DisconnectedUnit(
        const ChIds vv,
        const CmpId ind,
        Chains& cn
    );

    constexpr explicit DisconnectedUnit(
        Chain& m,
        CmpId ind,
        Chains& cn
    );

    constexpr explicit DisconnectedUnit(
        Chains& mm,
        CmpId ind,
        Chains& cn
    );

    DisconnectedUnit() = delete;
    constexpr explicit DisconnectedUnit(const DisconnectedUnit& other) = default;
    constexpr explicit DisconnectedUnit(DisconnectedUnit&& other);
    constexpr auto operator=(const DisconnectedUnit& other) -> DisconnectedUnit& = default;
    constexpr auto operator=(DisconnectedUnit&& other) -> DisconnectedUnit&;
    ~DisconnectedUnit() = default;

    constexpr auto operator==(const DisconnectedUnit& other) -> bool;

    constexpr void set_ind(CmpId i) noexcept;

    void set_edges() noexcept;
    void set_gl() noexcept;
    void set_chis() noexcept;

    void rename_chain(ChId f, ChId t) noexcept;

    void clear();

    constexpr auto num_edges() const noexcept -> EgId;
    constexpr auto num_chains() const noexcept -> ChId;

    template<Degree D> requires is_implemented_degree<D>
    constexpr auto num_vertices() const noexcept -> szt;

    constexpr auto num_vertices() const noexcept -> szt;

    constexpr auto contains_chain(ChId w) const noexcept -> bool;
    constexpr auto contains(const Chain& m) const noexcept -> bool;
    constexpr auto contains_edge(EgId ei) const noexcept -> bool;
    constexpr auto contains(const Edge& eg) const noexcept -> bool;

    void append(DisconnectedUnit&& other);
    void append(DisconnectedUnit& other);
    void append(const ChIds& vv);
    void append(Chains& mm);
    void append(Chain& m);
    void append(Edges& g);
    void append(Edge& eg);

    void remove(const Edges& g);
    void remove(const Edge& eg);
    void remove(const Chain& m);
    void remove(const Chains& mm);
    void remove(const ChIds& vv);

    void move_to(DisconnectedUnit& c, ChIds&& vv);
    void move_to(DisconnectedUnit& c, Chains& mm);
    void move_to(DisconnectedUnit& c, Chain& m);

    auto find_chains(const EndSlot& source) noexcept -> ChIds;
    auto find_chains(ChId seed) noexcept -> ChIds;

    auto edge(EgId indc) const noexcept -> const Edge&;
    auto chain(ChId w) const noexcept -> const Chain&;
    auto chid(ChId idc) const noexcept -> const ChId;

    auto ind2indc(EgId ind) const noexcept -> EgId;

    void make_indma() noexcept;

    /**
     * @brief shifts an edge from f.e end of f.w to t.e end of t.w
     */
    void shift_end_edge(const EndSlot& f,
                        const EndSlot& t);

    /**
     * @brief Initializes adjacency list of edges of this component.
     */
    void adjacency_list_edges() noexcept;

    auto adjacency_list_edges() const noexcept -> vec2<EgId>;

    /**
     * @brief Initializes adjacency list of chains of this component.
     */
    template<Orientation dir>
    void adjacency_list_chains() noexcept;

    template<Orientation dir>
    auto adjacency_list_chains() const noexcept -> vec2<ChId>;

    auto ww_is_sorted() const noexcept -> bool;

    /**
     * @brief Depth-first search of the graph graph.
     * @note Performs necessary initializations of the auxiliary variables
     * before lanching the search.
     * @tparam knownSize if true enumerates over this cluster,
     * otherwise - over the whole graph
     * @param source Initial slot.
     * @param target Target slot.
     * @return True if there is a connection between \p source and \p target.
     */
    template<bool knownSize>
    auto dfs(
        const EndSlot& source,
        const EndSlot& target
    ) -> bool;

    template<bool isCycle>
    void print_classification(const ChIds& accessible,
                              const ChIds& blocked) const;

    void print_adjacency_list_edges(const std::string& tag) const;
    void print_adjacency_list_chains(const std::string& tag) const;
    void print_adjacency_list_chains(const std::string& tag,
                                     const vec2<ChId>& ajlw) const;

    template<typename... Args>
    void print(Args... args) const;

    template<typename... Args>
    void print_chains(Args... args) const;

    void print_gl() const noexcept;

    template<bool with_top=true>
    void print_ww() const noexcept;

    auto get_visited() const noexcept -> const std::vector<ChId>&
    {
        return visited;
    }

    /**
     * @brief Checks that \p cond is satisfied, else terminates program.
     * @details Prints out the component using \p tag before terminating the
     * program if \p cond is false. \p message is printed at termination.
     * @tparam T Types of the message arguments.
     * @param cond Condition to be checked.
     * @param tag Tag to the component data to be printed if \p cond is false.
     */
    template<typename... T>
    void ensure(
        bool cond,
        const std::string& tag,
        T&&... message
    ) const;


private:

    Chains& cn;  ///< Reference to the parent chains container.

    ChIds visited;  ///< Auxiliary: IDs of chains visited during a search.

    template<bool knownSize>
    void reset_search() noexcept;

    /**
     * @brief Depth-first search of the graph graph.
     * @note Assumes that initialization of the auxiliary variables is
     * already done.
     * @tparam knownSize if true enumerates over this cluster,
     * otherwise - over the whole graph
     * @param source Initial slot.
     * @param target Target slot.
     * @return True if there is a connection between \p source and \p target.
     */
    template<bool knownSize>
    auto dfs_(
        const EndSlot& source,
        const EndSlot& target
    ) -> bool;

    template<bool knownSize>
    auto bfs_(
        std::deque<EndSlot>& q,     // by reference
        const EndSlot& target,
        const EndSlot& source = EndSlot {}
    ) const -> bool;

};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Ch>
constexpr
DisconnectedUnit<Ch>::
DisconnectedUnit(
    const CmpId ind,
    Chains& cn
)
    : ind {ind}
    , vertices {*this}
    , cn {cn}
{}


template<typename Ch>
constexpr
DisconnectedUnit<Ch>::
DisconnectedUnit(
    const ChIds vv,
    const CmpId ind,
    Chains& cn
)
    : DisconnectedUnit {ind, cn}
{
    append(vv);
}


template<typename Ch>
constexpr
DisconnectedUnit<Ch>::
DisconnectedUnit(
    Chain& m,
    const CmpId ind,
    Chains& cn
)
    : DisconnectedUnit {ind, cn}
{
    append(m);
}


template<typename Ch>
constexpr
DisconnectedUnit<Ch>::
DisconnectedUnit(
    Chains& mm,
    const CmpId ind,
    Chains& cn
)
    : DisconnectedUnit {ind, cn}
{
    append(mm);
}


template<typename Ch>
constexpr
DisconnectedUnit<Ch>::
DisconnectedUnit(DisconnectedUnit&& other)
    : ind {other.ind}
    , gl {std::move(other.gl)}
    , ww {std::move(other.ww)}
    , ajlg {std::move(other.ajlg)}
    , ajlw {std::move(other.ajlw)}
    , ajlwA {std::move(other.ajlwA)}
    , ajlwB {std::move(other.ajlwB)}
    , chis {std::move(other.chis)}
    , cn {other.cn}
{}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
operator=(DisconnectedUnit&& other) -> DisconnectedUnit&
{
    ind = other.ind;
    gl = std::move(other.gl);
    ww = std::move(other.ww);
    ajlg = std::move(other.ajlg);
    ajlw = std::move(other.ajlw);
    ajlwA = std::move(other.ajlwA);
    ajlwB = std::move(other.ajlwB);
    chis = std::move(other.chis);
    cn = other.cn;

    return *this;
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
operator==(const DisconnectedUnit& other) -> bool
{
    if (num_chains() != other.num_chains())
        return false;

    if (num_chains() > 1) {
        std::sort(ww.begin(), ww.end());
        std::sort(other.ww.begin(), other.ww.end());
    }

    return ww == other.ww;
}


template<typename Ch>
constexpr
void DisconnectedUnit<Ch>::
set_ind(const CmpId i) noexcept
{
    ind = i;

    for (const auto w: ww) {
        cn[w].c = i;
        for (auto& g: cn[w].g)
            g.c = i;
    }
}


template<typename Ch>
void DisconnectedUnit<Ch>::
clear()
{
    gl.clear();
    ww.clear();
    ajlg.clear();
    ajlw.clear();
    ajlwA.clear();
    ajlwB.clear();
    chis.clear();
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
num_edges() const noexcept -> EgId
{
    return static_cast<EgId>(gl.size());
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
num_chains() const noexcept -> ChId
{
    return static_cast<ChId>(ww.size());
}


template<typename Ch>
template<Degree D> requires is_implemented_degree<D>
constexpr
auto DisconnectedUnit<Ch>::
num_vertices() const noexcept -> szt
{
    if constexpr (D == 0)
        return is_defined(chis.cn22);

    if constexpr (D == 1)
        return 2*is_defined(chis.cn11) + chis.cn13.size() + chis.cn14.size();

    if constexpr (D == 2)
        return num_edges() - num_vertices<0>() - (num_vertices<1>() +
                                                  3 * num_vertices<3>() +
                                                  4 * num_vertices<4>()) / 2;

    if constexpr (D == 3)
        return (2*chis.cn33.size() + chis.cn13.size() + chis.cn34.size()) / 3;

    if constexpr (D == 4)
        return (2*chis.cn44.size() + chis.cn14.size() + chis.cn34.size()) / 4;
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
num_vertices() const noexcept -> szt
{
    return num_vertices<0>() +
           num_vertices<1>() +
           num_vertices<2>() +
           num_vertices<3>() +
           num_vertices<4>();
}


template<typename Ch>
void DisconnectedUnit<Ch>::
set_edges() noexcept
{
    EgId indc {};
    for (const auto w: ww)
        indc = cn[w].set_g_cmp(ind, indc);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
rename_chain(const ChId f, const ChId t) noexcept
{
    std::replace(ww.begin(), ww.end(), f, t);
    std::for_each(gl.begin(), gl.end(),
                  [&](components::Gl& g) { if (g.w == f) g.w = t; });
    chis.populate(cn, ww);
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
contains_chain(const ChId w) const noexcept -> bool
{
    return std::find(ww.begin(), ww.end(), w) != ww.end();
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
contains(const Chain& m) const noexcept -> bool
{
    return contains_chain(m.idw);
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
contains_edge(const EgId ei) const noexcept -> bool
{
    return std::find_if(gl.begin(), gl.end(),
                        [&](const Gl& i) { return i.i == ei; }) != gl.end();
}


template<typename Ch>
constexpr
auto DisconnectedUnit<Ch>::
contains(const Edge& eg) const noexcept -> bool
{
    return contains_edge(eg.ind);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(DisconnectedUnit&& other)
{
    auto indc = num_edges();
    auto idc = num_chains();

    ASSERT(other.ind != ind, "appending identical compinent");

    for (const auto w: other.ww)
        indc = cn[w].set_cmpt(ind, idc++, indc);

    std::move(other.gl.begin(), other.gl.end(), std::back_inserter(gl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(DisconnectedUnit& other)
{
    auto indc = num_edges();
    auto idc = num_chains();

    ASSERT(other.ind != ind, "appending identical compinent");

    for (const auto w: other.ww)
        indc = cn[w].set_cmpt(ind, idc++, indc);

    std::move(other.gl.begin(), other.gl.end(), std::back_inserter(gl));
    std::move(other.ww.begin(), other.ww.end(), std::back_inserter(ww));

    chis.append(std::move(other.chis));
}



template<typename Ch>
void DisconnectedUnit<Ch>::
append(const ChIds& vv)
{
    for (const auto& v: vv)
        append(cn[v]);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(Chains& mm)
{
    for (auto& m: mm)
        append(m);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(Chain& m)
{
    const auto indc = num_edges();
    const auto idc = num_chains();

    ASSERT(!contains(m), "appending chain which is already included");

    m.set_cmpt(ind, idc, indc);

    for (const auto& g: m.g)
        gl.emplace_back(g.w, g.indw, g.ind);

    ww.push_back(m.idw);

    chis.include(m);

//    ASSERT(ww_is_sorted(), "ww is out of order");
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(Edges& g)
{
    for (auto& eg: g)
        append(eg);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
append(Edge& eg)
{
    ASSERT(!contains(eg), "appending edge which is already included");

    eg.set_cmp(ind, num_edges());

    set_gl();
}


template<typename Ch>
void DisconnectedUnit<Ch>::
remove(const Edges& g)
{
    for (const auto& eg: g)
        remove(eg);
}

// DisconnectedUnit-related data inside \p eg are not to be altered, hence const ref.
template<typename Ch>
void DisconnectedUnit<Ch>::
remove(const Edge& eg)
{
    if constexpr (verboseF)
        log_("-removing edge ", eg.ind, " of chain ", eg.w,
            " from component ", ind, ":");

    ASSERT(contains(eg),
           "attempt to remove a missing edge ", eg.ind, " from cmpt ", ind);
    ASSERT(cn[eg.w].length(), "removing an edge from empty chain ", eg.w);

    if (const auto& b = gl.back();
        eg.ind != b.i) {
        auto& q = cn[b.w].g[b.a];
        q.indc = eg.indc;
        gl[eg.indc] = {q.w, q.indw, q.ind};
    }

    gl.pop_back();
}


template<typename Ch>
void DisconnectedUnit<Ch>::
remove(const Chain& m)
{
    if constexpr (verboseF)
        log_("removing chain ", m.idw, " from component ", ind);

    ASSERT(contains(m),
           "attempt to remove a missing chain ", m.idw, " from cmpt ", ind);

    Edges g = m.g;  // copy

    // sort in descending order of indc
    std::ranges::sort(g, [](Edge& a, Edge& b) { return a.indc > b.indc; });

    for (const auto& eg: g)
        remove(eg);

    if (const auto last = num_chains() - 1;
        m.idc < last)
        for (const auto w: ww)
            if (cn[w].idc == last)
                cn[w].idc = m.idc;

    std::erase(ww, m.idw);

    if (m.is_disconnected_cycle())
        chis.populate(cn, ww);
    else
        chis.remove(m);

//    ASSERT(ww_is_sorted(), "ww is out of order");
}


template<typename Ch>
void DisconnectedUnit<Ch>::
remove(const Chains& mm)
{
    Edges g;
    for (const auto m: mm)  // copy
        g.append_range(std::move(m.g));

    // sort in descending order of indc
    std::ranges::sort(g, [](Edge& a, Edge& b) { return a.indc > b.indc; });

    ASSERT(std::adjacent_find(g.begin(), g.end()) == g.end(),
           "edges having identical indc found");

    for (const auto& eg: g)
        remove(eg);

    for (const auto& m: mm) {

        if (const auto last = num_chains() - 1;
            m.idc < last)
            for (const auto w: ww)
                if (cn[w].idc == last)
                    cn[w].idc = m.idc;

        std::erase(ww, m.idw);

        if (m.is_disconnected_cycle())
            chis.populate(cn, ww);
        else
            chis.remove(m);
    }

//    ASSERT(ww_is_sorted(), "ww is out of order");
}


template<typename Ch>
void DisconnectedUnit<Ch>::
remove(const ChIds& vv)
{
    for (const auto v: vv)
        remove(cn[v]);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
move_to(DisconnectedUnit& c, Chain& m)
{
    remove(m);
    c.append(m);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
move_to(DisconnectedUnit& c, Chains& mm)
{
    remove(mm);
    c.append(mm);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
move_to(DisconnectedUnit& c, ChIds&& vv)
{
    remove(vv);
    c.append(vv);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
set_gl() noexcept
{
    EgId numEdges {};
    for (const auto w: ww)
        numEdges += cn[w].length();
    gl.resize(numEdges);
    for (const auto w: ww)
        for (const auto& g: cn[w].g) {
            ASSERT(g.indc < numEdges, "In component ", ind,
                   " g.indc ", g.indc, " >= numEdges ", numEdges);
            gl[g.indc] = {g.w, g.indw, g.ind};
        }
}



template<typename Ch>
void DisconnectedUnit<Ch>::
set_chis() noexcept
{
    chis.populate(cn, ww);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
make_indma() noexcept
{
    gl.clear();
    ww.clear();

    for (const auto& m: cn)
        if (m.c == ind) {
            ww.push_back(m.idw);
            [[maybe_unused]] EgId i {};
            for (const auto& g: m.g) {
                ASSERT(i++ == g.indc,
                       "i =", i, " != g.indc = ", g.indc, " in chain ", m.idw);
                gl.emplace_back(g.w, g.indw, g.ind);
            }
        }
}


// shifts an edge from f.e end of f.w to t.e end of t.w
template<typename Ch>
void DisconnectedUnit<Ch>::
shift_end_edge(const EndSlot& f,
               const EndSlot& t)
{
    auto& g0 = cn[f.w].g;
    auto& g1 = cn[t.w].g;

    ASSERT(cn[f.w].c == cn[t.w].c, "slots belong to differrent components");

    if (f.e == Ends::B) {
        if (t.e == Ends::A) g1.insert(g1.begin(), std::move(g0.back()));
        else                g1.push_back(std::move(g0.back()));
        g0.pop_back();
    }
    else {                        // ef == 1
        if (t.e == Ends::A) g1.insert(g1.begin(), std::move(g0.front()));
        else                g1.push_back(std::move(g0.front()));
        g0.erase(g0.begin());
    }
    cn[f.w].set_g_w();
    cn[t.w].set_g_w();
    set_gl();
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
ww_is_sorted() const noexcept -> bool
{
    const auto n = num_chains();

    if (n > 1) {
        std::vector<ChId> h(n);
        std::iota(h.begin(), h.end(), ChId {});

        return std::all_of(h.begin(), h.end(),
                        [&](ChId i) { return cn[ww[i]].idc == i; });
    }
    else
        return true;
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
adjacency_list_edges() const noexcept -> vec2<EgId>
{
    vec2<EgId> a(num_edges());

    for (const auto j : ww) {
        const auto& m = cn[j];
        for (EgId k=0; k<m.length(); ++k) {
            const auto indc = m.g[k].indc;
            if (m.is_tail(k)) {

                // Connection backwards: only other chains might be found.
                for (const auto& s : m.ngs[Ends::A]())
                    a[indc].push_back(cn[s.w].g[cn[s.w].end2a(s.e)].indc);

                if (m.length() == 1)
                    // Connection forwards: to other chain.
                    for (const auto& s : m.ngs[Ends::B]())
                        a[indc].push_back(cn[s.w].g[cn[s.w].end2a(s.e)].indc);
                else
                    // Connection forwards: to the same chain.
                    a[indc].push_back(m.g[k+1].indc);
            }
            else if (m.is_head(k)) {

                // Connection backwards: to the same chain.
                a[indc].push_back(m.g[k-1].indc);

                // Connection forwards: to other chain.
                for (const auto& s : m.ngs[Ends::B]())
                    a[indc].push_back(cn[s.w].g[cn[s.w].end2a(s.e)].indc);
            }
            else {

                // Connection backwards: to the same chain.
                a[indc].push_back(m.g[k-1].indc);

                 // Connection forwards: to the same chain.
                a[indc].push_back(m.g[k+1].indc);
            }
        }
    }

    return a;
}

template<typename Ch>
void DisconnectedUnit<Ch>::
adjacency_list_edges() noexcept
{
    ajlg = adjacency_list_edges();
}

template<typename Ch>
template<Orientation dir>
auto DisconnectedUnit<Ch>::
adjacency_list_chains() const noexcept -> vec2<ChId>
{
    vec2<ChId> x (num_chains());

    for (const auto j : ww) {
        const auto& m = cn[j];

        if constexpr (dir == Orientation::Backwards)
            for (const auto& ng: m.ngs[Ends::A]())
                x[m.idc].push_back(cn[ng.w].idc);

        if constexpr (dir == Orientation::Forwards)
            for (const auto& ng: m.ngs[Ends::B]())
                x[m.idc].push_back(cn[ng.w].idc);
    }

    return x;
}


template<typename Ch>
template<Orientation dir>
void DisconnectedUnit<Ch>::
adjacency_list_chains() noexcept
{
    ajlw = adjacency_list_chains<dir>();
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
find_chains(const EndSlot& source) noexcept -> ChIds
{
    reset_search<false>();
    dfs_<false>(source, EndSlot{});

    ChIds vv {};

    std::copy_if(visited.begin(), visited.end(),
                 std::back_inserter(vv),
                 [&](ChId v) { return is_defined(v);});

    if (std::find(vv.begin(), vv.end(), source.w) == vv.end())
        vv.push_back(source.w);

   return vv;
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
find_chains(const ChId seed) noexcept -> ChIds
{
    ChIds res {seed};

    auto append = [&res](auto&& r) { for (auto rr: r) res.push_back(rr); };

    auto r1 = find_chains(EndSlot{seed, Ends::A});
    auto r2 = find_chains(EndSlot{seed, Ends::B});

    std::sort(r1.begin(), r1.end());
    std::sort(r2.begin(), r2.end());

    append(r1);
    if (r1 != r2)
        append(r2);

    return res;
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
edge(const EgId indc) const noexcept -> const Edge&
{
    const auto& h = gl[indc];

    return cn[h.w].g[h.a];
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
chain(const ChId w) const noexcept -> const Chain&
{
    ASSERT(std::find(ww.begin(), ww.end(), w) != ww.end(),
           "Chain ", w, " is not part of component ", ind);

    return cn[w];
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
chid(const ChId idc) const noexcept -> const ChId
{
    for (const auto w: ww)
        if (cn[w].idc == idc)
            return w;

    return undefined<ChId>;
}


template<typename Ch>
auto DisconnectedUnit<Ch>::
ind2indc(EgId ind) const noexcept -> EgId
{
    for (EgId i {}; i<gl.size(); ++i)
        if (gl[i].i == ind)
            return i;

    return undefined<EgId>;
}


template<typename Ch>
template<bool knownSize>
auto DisconnectedUnit<Ch>::
dfs(const EndSlot& source,
    const EndSlot& target) -> bool
{
    // Nothing is visited in the beginning of the search.
    reset_search<knownSize>();

    // This is true iff there is a connection to s2 via a path outgoing from s1.
    return dfs_<knownSize>(source, target);
}


template<typename Ch>
template<bool knownSize>
auto DisconnectedUnit<Ch>::
dfs_(const EndSlot& source,
     const EndSlot& target) -> bool
{
    if (source == target) {
        visited[knownSize ? cn[source.w].idc : source.w] = source.w;
        return true;
    }

    for (const auto& s : cn[source.w].ngs[source.e]()) {

        if (cn[s.w].ngs[Ends::opp(s.e)].num() == 0) {
            if (s.w == target.w) {
                if (s.e == target.e) {
                    return true;
                }
            }
            else {
                const auto n = knownSize ? cn[s.w].idc : s.w;
                if (is_undefined(visited[n])) {
                    visited[n] = s.w;
                    if (dfs_<knownSize>(s.opp(), target))
                        return true;
                }
            }
        }
    }

    return false;
}


template<typename Ch>
template<bool knownSize>
auto DisconnectedUnit<Ch>::
bfs_(std::deque<EndSlot>& q,     // by reference
     const EndSlot& target,
     const EndSlot& source
) const -> bool
{
    if (q.empty())
        return false;    // reached end of the search wt finding target

    auto s = std::move(q.front());
    q.pop_front();
    if (s.w == target.w)
        return true;

    for (const auto& ng : cn[s.w].ngs[s.e]()) {
        const auto n = knownSize ? cn[ng.w].idc : ng.w;
        if (is_undefined(visited[n])) {
            visited[n] = ng.w;
            q.push_back(ng.opp());
        }
    }

    return bfs_(q, target);
}


template<typename Ch>
template<bool knownSize>
void DisconnectedUnit<Ch>::
reset_search() noexcept
{
//    ASSERT(ww_is_sorted(), "ww is out of order");

    const auto n = knownSize ? num_chains() : cn.size();

    visited.resize(n);
    std::fill(visited.begin(),
              visited.begin() + static_cast<long>(n),
              undefined<ChId>);
}


template<typename Ch>
template<bool isCycle>
void DisconnectedUnit<Ch>::
print_classification(const ChIds& accessible,
                     const ChIds& blocked) const
{
    log_(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
         num_chains(), " chains");

    ChIds all(num_chains(), undefined<ChId>);
    for (const auto a: accessible) {
        const size_t i = std::distance(ww.begin(),
                                       std::find(ww.begin(), ww.end(), a));
        ASSERT(i < ww.size(), "element of 'accessible' is not in component");
        all[i] = a;
    }
    constexpr auto colorAccessible = colorcodes::GREEN;
    constexpr auto colorBlocked = colorcodes::RED;
    log_<false>(colorcodes::CYAN, "all: ", colorcodes::RESET);
    for (szt j {}; j < all.size(); ++j)
        log_<false>(is_defined(all[j]) ? colorAccessible
                                       : colorBlocked,
                    ww[j],
                    colorcodes::RESET,
                    j == all.size() - 1 ? "" : ", ");
    log_("");
    for (int i {}; auto& v: {accessible, blocked}) {
        log_<false>(colorcodes::CYAN, i == 0 ? "accessible: " : "blocked: ",
                    colorcodes::RESET);
        for (szt j {}; auto w: v)
            log_<false>(i == 0 ? colorAccessible : colorBlocked, w,
                        ++j == v.size() ? "" : ", ");
        log_(colorcodes::RESET);
        ++i;
    }
}


template<typename Ch>
void DisconnectedUnit<Ch>::
print_adjacency_list_edges(const std::string& tag) const
{
    for (szt j {}; j < num_edges(); ++j) {
        log_<false>(tag, ind, ' ', gl[j].w, ' ', gl[j].a, ' ', gl[j].i, " : ");
        for (const auto k : ajlg[j])
            log_<false>(k, ' ');
        log_("");
    }
    log_("");
}


template<typename Ch>
void DisconnectedUnit<Ch>::
print_adjacency_list_chains(const std::string& tag) const
{
    print_adjacency_list_chains(tag, ajlw);
}


template<typename Ch>
void DisconnectedUnit<Ch>::
print_adjacency_list_chains(const std::string& tag,
                            const vec2<ChId>& a) const
{
    for (szt j {}; j < num_chains(); ++j) {
        const auto q = std::find_if(ww.begin(),
                                    ww.end(),
                                    [&](const ChId w){return cn[w].idc == j; });
        ASSERT(q != ww.end(), " idc ", j, " not found in component ", ind);
        log_<false>(tag, ind, ' ', *q, " : ");
        for (const auto k : a[j])
            log_<false>(k, ' ');
        log_("");
    }
    log_("");
}


template<typename Ch>
template<typename... Args>
void DisconnectedUnit<Ch>::
print(Args... args) const
{
    print_chains(args...);
    print_ww<false>();
    print_gl();
    chis.print();
    log_("");
}


template<typename Ch>
template<typename... Args>
void DisconnectedUnit<Ch>::
print_chains(Args... args) const
{
    log_(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, ind, colorcodes::RESET, ": ",
         num_edges(), " edges in ", num_chains(), " chains");

    for (const auto j : ww)
        cn[j].print(args...);
}

template<typename Ch>
void DisconnectedUnit<Ch>::
print_gl() const noexcept
{
    log_("gl ", ind, ": w a ind");
    for (szt i {}; i < gl.size(); ++i)
        gl[i].print(i);
}


template<typename Ch>
template<bool with_top>
void DisconnectedUnit<Ch>::
print_ww() const noexcept
{
    if constexpr (with_top)
        log_<false>("cmpt ", ind, " ");
    log_<false>("ww: ");
    for (const auto w: ww)
        log_<false>(w, " ");

    log_("");
}


template<typename Ch>
template<typename... T>
void DisconnectedUnit<Ch>::
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

}  // namespace components


template<typename Ch>
using Component = components::DisconnectedUnit<Ch>;


}  // namespace graph_mutator::structure::basic

#endif  // GRAPH_MUTATOR_STRUCTURE_BASIC_COMPONENT_H
