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
 * \file adjacency.h
 * \brief
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_ANALYZERS_ADJACENCY_H
#define GRAFFINE_STRUCTURE_ANALYZERS_ADJACENCY_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/end.h"

#include <vector>


namespace graffine::structure {

template<typename H>
struct Adjacency
{
    using Host = H;
    using EndSlot = H::EndSlot;
    using End = descriptors::End;
    template <typename T> using vec2 = std::vector<std::vector<T>>;
    using ChId = std::conditional_t<H::isMulticomponent, ChIdG, ChIdC>;
    using EgId = std::conditional_t<H::isMulticomponent, EgIdG, EgIdC>;

    // cpagl
    vec2<EgId> lg;   ///< Edge adjacency_list.
    vec2<VIdH> lev;   ///< Vertex adjacency_list for end vertices.

    vec2<ChId> lw;   ///< Chain adjacency_list.
    vec2<ChId> lwA;  ///< Chain adjacency_list in direction backwards.
    vec2<ChId> lwB;  ///< Chain adjacency_list in direction forwards.

    Adjacency(const Host& h)
        : h {h}
    {}

    void clear();

    /**
     * Initializes adjacency list of edges of this component.
     */
    void list_edges() noexcept;

    static auto list_edges(const Host& h) noexcept -> vec2<EgId>;

    /**
     * Initializes adjacency list of chains of this component.
     */
    template<Orientation dir>
    void list_chains() noexcept;

    template<Orientation dir>
    static auto list_chains(const Host& h) noexcept -> vec2<ChId>;

    constexpr bool are_adjacent(ChId w1,
                                EgIdA a1,
                                ChId w2,
                                EgIdA a2) const noexcept;

    constexpr bool are_adjacent(const EndSlot& s1,
                                const EndSlot& s2) const noexcept;

    constexpr bool are_adjacent(const EndSlot& s1,
                                ChId w2,
                                EgIdA a2) const noexcept;



    void print_list_edges(std::string_view tag) const;
    void print_list_chains(std::string_view tag) const;
    static void print_list_chains(std::string_view tag,
                                  const vec2<ChId>& l,
                                  const Host& h);


protected:

    const Host& h;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename H>
void Adjacency<H>::
clear()
{
    lg.clear();
    lw.clear();
    lwA.clear();
    lwB.clear();
}

template<typename H>
auto Adjacency<H>::
list_edges(const Host& h) noexcept -> vec2<EgId>
{
    vec2<EgId> a(h.num_edges());
    const auto& ww = h.chids();

    for (const auto& j : ww) {
        const auto& m = h.chain(j);
        for (std::size_t k {}; k < m.length(); ++k) {
            const auto indc = m.g[k].indc();
            if (m.is_tail(k)) {

                // Connection backwards: only other chains might be found.
                for (const auto& s : m.ngs[End::A]())
                    a[indc].push_back(h.edge(s).indc);

                if (m.length() == 1)
                    // Connection forwards: to other chain.
                    for (const auto& s : m.ngs[End::B]())
                        a[indc].push_back(h.edge(s).indc);
                else
                    // Connection forwards: to the same chain.
                    a[indc].push_back(m.g[k+1].indc);
            }
            else if (m.is_head(k)) {

                // Connection backwards: to the same chain.
                a[indc].push_back(m.g[k-1].indc);

                // Connection forwards: to other chain.
                for (const auto& s : m.ngs[End::B]())
                    a[indc].push_back(h.edge(s).indc);
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

template<typename H>
void Adjacency<H>::
list_edges() noexcept
{
    lg = list_edges(h);
}

template<typename H>
template<Orientation dir>
auto Adjacency<H>::
list_chains(const Host& h) noexcept -> vec2<ChId>
{
    vec2<ChId> x (h.num_chains());

    for (const auto& j : h.chains()) {
        const auto& m = h.chain(j);

        if constexpr (dir == Orientation::Backwards)
            for (const auto& ng : m.ngs[End::A]())
                x[m.idc].push_back(h.chain(ng.w).idc);

        if constexpr (dir == Orientation::Forwards)
            for (const auto& ng : m.ngs[End::B]())
                x[m.idc].push_back(h.chain(ng.w).idc);
    }

    return x;
}


template<typename H>
template<Orientation dir>
void Adjacency<H>::
list_chains() noexcept
{
    lw = list_chains<dir>(h);
}


template<typename H>
constexpr
bool Adjacency<H>::
are_adjacent(
    const ChId w1,
    const EgIdA a1,
    const ChId w2,
    const EgIdA a2
) const noexcept
{
    ASSERT(a1 > 0, "ea or ee should be applied: a1 == 0");
    ASSERT(a1 < h.chain(w1).length(),
           "ea or ee should be applied: a1 == cn[w1].length() = ", a1);
    ASSERT(a2 > 0, "ea or ee should be applied: a2 == 0");
    ASSERT(a2 < h.chain(w2).length(),
           "ea or ee should be applied: a2 == cn[w2].length() = ", a2);

    if (w1 == w2) {
        return a1 == a2 ||
               a1 == a2 + 1 ||
               a2 == a1 + 1;
    }

    return false;
}


template<typename H>
constexpr
bool Adjacency<H>::
are_adjacent(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept
{
    const auto w1 = s1.w;
    const auto e1 = s1.e;
    const auto w2 = s2.w;
    const auto e2 = s2.e;

    const auto& m1 = h.chain(w1);

    auto is_short = [](const auto& s) noexcept
    {
        return s.length() < 2;
    };

    if (w1 == w2)
        return e1 == e2 ||
               is_short(m1) ||
               m1.has_such_neig(End::A, w2);

    if (m1.c != h.chain(w2).c)
        return false;

    for (const auto& ng1 : m1.ngs[e1]()) {

        const auto nw1 = ng1.w;
        const auto ne1 = ng1.e;
        if (nw1 == w2 &&
            (ne1 == e2 ||
             (ne1 != e2 && is_short(h.chain(nw1)))))
            return true;

        if (nw1 != w2 && is_short(h.chain(nw1))) {
            const auto oe = H::Chain::opp_end(ne1);
            for (const auto& ngo : h.chain(nw1).ngs[oe]())
                if (ngo == s2)
                    return true;
        }
    }

    const auto oe1 = H::Chain::opp_end(e1);
    for (const auto& s : m1.ngs[oe1]())
        if (s == s2)
            return true;

    return false;
}


template<typename H>
constexpr
bool Adjacency<H>::
are_adjacent(
    const EndSlot& s1,
    const ChId w2,
    const EgIdA a2
) const noexcept
{
    const auto [w1, e1] = s1.we();
    const auto& m1 = h.chain(w1);
    const auto& m2 = h.chain(w2);

    ASSERT(a2 > 0, "ea or ee should be applied: a2 == 0");
    ASSERT(a2 < m2.length(),
           "ea or ee should be applied: a2 == cn[w2].length() = ", a2);

    if (w1 == w2)
        return
            (e1 == End::A && a2 == 1) ||
            (e1 == End::B && a2 == m2.length() - 1) ||
            (m1.has_such_neig(End::A, w1) &&
             (a2 == 1 || a2 == m2.length() - 1));

    if (m1.c != m2.c)
        return false;

    if (a2 == 1 && m2.ngs[End::A].num())
        for (const auto& s : m2.ngs[End::A]())
            if (s == s1)
                return true;

    if (a2 == m2.length() - 1 && m2.ngs[End::B].num())
        for (const auto& s : m2.ngs[End::B]())
            if (s == s1)
                return true;

    return false;
}



template<typename H>
void Adjacency<H>::
print_list_edges(std::string_view tag) const
{
    for (std::size_t j {}; j < lg.size(); ++j) {

        jot<false>(tag, "[", h.edge(j).ind, "]", " : ");
        for (const auto k : lg[j])
            jot<false>(h.edge(k).ind, ' ');
        jot("");
    }
    jot("");
}


template<typename H>
void Adjacency<H>::
print_list_chains(std::string_view tag) const
{
    print_list_chains(tag, lw);
}


template<typename H>
void Adjacency<H>::
print_list_chains(std::string_view tag,
                  const vec2<ChId>& a,
                  const Host& h)
{
    for (std::size_t j {}; j < a.size(); ++j) {
        const auto q = std::find_if(h.chains().begin(),
                                    h.chains().end(),
                                    [&](const ChId w){return h.chain(w).idc == j; });
        ASSERT(q != h.chains().end(), " idc ", j, " not found in component ", h.ind);
        jot<false>(tag, h.ind, ' ', *q, " : ");
        for (const auto k : a[j])
            jot<false>(k, ' ');
        jot("");
    }
    jot("");
}


}  // namespace graffine::structure


#endif  // GRAFFINE_STRUCTURE_ANALYZERS_ADJACENCY_H