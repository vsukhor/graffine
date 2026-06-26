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
 * \file path.h
 * \brief Contains structure for representing paths in the graph in a way
 * convenient for pulling transformations.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_PATH_H
#define GRAFFINE_TRANSFORMS_PULLING_PATH_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/paths/over_edges/generic.h"
#include "graffine/structure/paths/over_endslots/generic.h"
#include "graffine/structure/descriptors/slot.h"  // for IndEgInd
#include "graffine/transforms/pulling/common.h"

#include <array>
#include <string>
#include <utility>  // move
#include <vector>
#include <tuple>

namespace graffine::transforms::pulling {


template<typename C>
struct Path
    : public structure::paths::over_edges::Generic<C>
{
    using Base = structure::paths::over_edges::Generic<C>;
    using Component = C;
    using Chain = Component::Chain;
    using EndSlot = Chain::EndSlot;
    using End = Chain::End;
    using Edge =  Component::Edge;
    using Vertex = Component::Vertex;
    using PathC = Base::PathC;
    using PathW = Base::PathW;
    using PathCh = structure::paths::over_endslots::Generic<C>::Path;
//    using Skeleton = paths::Container<Driver>;
    using Source = EndSlot;

    /// Descriptor of the Path driver.
    struct Driver
    {
        EgIdG ind {EgIdG::undefined};  ///< Graph-wide ind of the driving edge.
        ChIdG w {ChIdG::undefined};    ///< Chain id

        /// Edge inner end at the driving vertex.
        End::Id egEnd {End::undefined};

        explicit constexpr Driver(
            const Edge* const eg,
            const End::Id egEnd
        )
            : ind {eg->ind}
            , w {eg->w}
            , egEnd {egEnd}
        {}

        Driver() = default;
        ~Driver() = default;
        constexpr Driver(const Driver& d) noexcept = default;
        constexpr Driver(Driver&& d) noexcept = default;
        constexpr auto operator=(const Driver& d) -> Driver& = default;
        constexpr auto operator=(Driver&& d) -> Driver& = default;

        constexpr auto operator==(const Driver& d)
        {
            return ind == d.ind &&
                   egEnd == d.egEnd;
        }

        constexpr auto end_slot(const Chain& m) const noexcept -> EndSlot
        {
            const auto e = m.ind2end(ind, egEnd);

            return End::is_defined(e) ? EndSlot {w, e}
                                      : EndSlot {};
        }

        void print() const noexcept
        {
            jot("ind ", ind, " w ", w, " egEnd ", End::str(egEnd));
        }
    };

    template<typename, typename> friend struct FunctorBase;

    template<Degree D,
             Orientation Dir,
             typename G,
             typename PP> requires pullable_degree<D>
    friend struct On;

    using Base::cmp;

    PathW pthw;  // active path using edge ind (graph-wde EgIdG)
    PathC pthc;  // active path using edge indc (component-wde EgIdC);

//    Skeleton skeleton;

    explicit constexpr Path(
        const Component* const cmp,
        Driver d,
        Source s
    );

    explicit constexpr Path(
        const Component* const cmp,
        Driver d,
        Source s,
        std::vector<Driver> internals
    );

    explicit constexpr Path(
        const Component* const cmp,
        Driver d,
        Source s,
        PathW pthw
    );

    explicit constexpr Path(
        const Component* const cmp,
        Driver d,
        Source s,
        PathC pthc
    );

    Path() = default;
    ~Path() = default;
    constexpr Path(const Path& p) noexcept;
    constexpr Path(Path&& p) noexcept;
    constexpr auto operator=(const Path& p) -> Path&;
    constexpr auto operator=(Path&& p) -> Path&;

    static constexpr auto end_to_a_tip(const C::Chain& m,
                                       const End::Id e);

//    void set_skeleton() const;

    /// Id of the chain at the leading path end.
    constexpr auto driver_chid() const noexcept -> ChIdG;

    /// Id of the chain boundary at the leading path end.
    constexpr auto driver_chain_end() const noexcept -> End::Id;

    /// Vertex at the leading path end.
    constexpr auto driver_vertex() const noexcept -> const Vertex&;
    constexpr auto driver_vertex()       noexcept ->       Vertex&;

    /// Id of the chain at the trailing path end.
    constexpr auto source_chid() const noexcept -> ChIdG;

    /// Id of the chain boundary at the trailing path end.
    constexpr auto source_end() const noexcept -> End::Id;

    /// Vertex at the trailing path end.
    constexpr auto source_vertex() const noexcept -> const Vertex&;
    constexpr auto source_vertex()       noexcept ->       Vertex&;

    auto driver2source(const Driver& dr) const noexcept -> Source;

    void set_cmp(Component& cc) noexcept;

    constexpr auto drv() const noexcept -> const Driver& { return d; }

    constexpr auto src() const noexcept -> const Source& { return s; }
    void set_src(const Source& src) noexcept;

    constexpr auto length() const noexcept -> std::size_t;

    constexpr auto length_over_source_chain() const noexcept -> std::size_t;

    constexpr auto vertices() const noexcept -> std::vector<Vertex*>;

    template<typename... Args>
    void print_path(Args... args) const noexcept;

    template<bool withChains>
    void print_short(const std::string& str) const;

    void print_detailed(const std::string& str) const;

private:

    Driver d;  // driver
    Source s;  // source
    std::vector<Driver> internals {};

    void set_shortest();
    static auto set_shortest(
        const Driver& dr,
        const Source& sr,
        const Component* const cmp
    ) -> std::tuple<PathC, PathW>;

    void set_driver_end(const End::Id e);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename C>
constexpr
Path<C>::
Path(
    const Component* const cmp,
    Driver d,
    Source s
)
    : Base {cmp}
    , d {d}
    , s {s}
{
    ASSERT(cmp->chain(s.w).ngs[s.e].num() == 0,
           "Source end is not disconnected");
    ASSERT(cmp->chain(s.w).c == cmp->ind,
           "Source compartment is different from cmp");
    ASSERT(cmp->chain(d.w).c == cmp->ind,
           "Driver compartment is different from cmp");

    set_shortest();
}


template<typename C>
constexpr
Path<C>::
Path(
    const Component* const cmp,
    Driver d,
    Source s,
    std::vector<Driver> internals
)
    : Base {cmp}
    , d {d}
    , s {s}
    , internals {internals}
{
//    ASSERT(cmp->chain(s.w).ngs[s.e].num() == 0,
//           "Source end is not disconnected");
    ASSERT(cmp->chain(s.w).c == cmp->ind,
           "Source compartment is different from cmp");
    ASSERT(cmp->chain(d.w).c == cmp->ind,
           "Driver compartment is different from cmp");

    if (internals.size()) {

        ASSERT(cmp->chain(internals[0].w).c == cmp->ind,
               "Compartment of internal 0 is different from cmp");
        ASSERT(internals[0].ind != d.ind, "Internal 0 is same as driver");
        ASSERT(internals[0].ind != cmp->chain(s.w).end2ind(s.e),
               "Internal 0 is same as source");

        auto [pc0, pw0] = set_shortest(d, driver2source(internals.front()), cmp);
        pthc.insert(pthc.end(), pc0.cbegin(), pc0.cend());
        pthw.insert(pthw.end(), pw0.cbegin(), pw0.cend());

        for (std::size_t i {1}; i<internals.size(); i++) {
            ASSERT(cmp->chain(internals[i].w).c == cmp->ind,
                   "Compartment of internal ", i, " is different from cmp");
            ASSERT(internals[i].ind != d.ind, "Internal ", i, " is same as driver");
            ASSERT(internals[i].ind != cmp->chain(s.w).end2ind(s.e),
                   "Internal ", i, " is same as source");

            auto [pc, pw] = set_shortest(internals[i-1], driver2source(internals[i]), cmp);
            pthc.insert(pthc.end(), pc.cbegin()+1, pc.cend());
            pthw.insert(pthw.end(),pw.cbegin()+1, pw.cend());
        }

        auto [pc, pw] = set_shortest(internals.back(), s, cmp);
        pthc.insert(pthc.end(), pc.cbegin()+1, pc.cend());
        pthw.insert(pthw.end(), pw.cbegin()+1, pw.cend());
    }
    else
        set_shortest();
}


template<typename C>
constexpr
Path<C>::
Path(
    const Component* const cmp,
    Driver d,
    Source s,
    PathW pthw
)
    : Base {cmp}
    , pthw {pthw}
    , pthc {this->from_global_ind(pthw)}
    , d {d}
    , s {s}
{
    ASSERT(cmp->chain(s.w).c == cmp->ind,
           "Source compartment is different from cmp");
    ASSERT(cmp->chain(d.w).c == cmp->ind,
           "Driver compartment is different from cmp");
    ASSERT(d.ind == cmp->get_egl(pthc[0]).i, "Driver ind is not in cmp.egl");
}

template<typename C>
constexpr
Path<C>::
Path(
    const Component* const cmp,
    Driver d,
    Source s,
    PathC pthc
)
    : Base {cmp}
    , pthw {this->path_to_global_ind(pthc)}
    , pthc {pthc}
    , d {d}
    , s {s}
{
    ASSERT(cmp->chain(s.w).c == cmp->ind,
           "Source compartment is different from cmp");
    ASSERT(cmp->chain(d.w).c == cmp->ind,
           "Driver compartment is different from cmp");
    ASSERT(d.ind == cmp->get_egl(pthc[0]).i, "Driver ind is not in cmp.egl");
}


//path_to_global_ind
template<typename C>
constexpr
Path<C>::
Path(const Path& p) noexcept
    : Base {p.cmp}
    , pthw {p.pthw}
    , pthc {p.pthc}
    , d {p.d}
    , s {p.s}
    , internals {p.internals}
{}


template<typename C>
constexpr
Path<C>::
Path(Path&& p) noexcept
    : Base {std::move(p.cmp)}
    , pthw {std::move(p.pthw)}
    , pthc {std::move(p.pthc)}
    , d {std::move(p.d)}
    , s {std::move(p.s)}
    , internals {std::move(p.internals)}
{}


template<typename C>
constexpr
auto Path<C>::
operator=(const Path& p) -> Path&
{
    Base::operator=(p);
    pthw = p.pthw;
    pthc = p.pthc;
    d = p.d;
    s = p.s;
    internals = p.internals;

    return *this;
}


template<typename C>
constexpr
auto Path<C>::
operator=(Path&& p) -> Path&
{
    Base::operator=(std::move(p));
    pthw = std::move(p.pthw);
    pthc = std::move(p.pthc);
    d = std::move(p.d);
    s = std::move(p.s);
    internals = std::move(p.internals);

    return *this;
}


template<typename C>
constexpr
auto Path<C>::
length() const noexcept -> std::size_t
{
    return pthw.size();
}


template<typename C>
void Path<C>::
set_shortest()
{
    // driver is at pthc.front(),
    // source is at pthc.back()
    pthc = this->template find_shortest_path<true>(
        cmp->ind2indc(d.ind),
        cmp->chain(s.w).end_edge(s.e).indc
    );
    pthw = this->path_to_global_ind(pthc);
}


template<typename C>
auto Path<C>::
set_shortest(const Driver& dr,
             const Source& sr,
             const Component* const c) -> std::tuple<PathC, PathW>  // static
{
    // driver is at pthw.front(),
    // source is at pthw.back()
    Base b {c};

    const auto icD = b.cmp->ind2indc(dr.ind);
    const auto icS = b.cmp->chain(sr.w).end_edge(sr.e).indc;

    PathC pc;
    if (dr.w != sr.w)
        pc = b.template find_shortest_path<true>(icD, icS);
    else {
        const auto w = dr.w;
        const auto aD = b.cmp->get_egl(icD).a;
        const auto aS = b.cmp->get_egl(icS).a;
        if (aS < aD)
            for (long int a {static_cast<long int>(aD())};
                 a>=static_cast<long int>(aS()); --a)
                pc.push_back(b.cmp->chain(w).g[a].indc);
        else
            for (std::size_t a {aD()}; a<=aS(); ++a)
                pc.push_back(b.cmp->chain(w).g[a].indc);
    }

    const auto p = b.path_to_global_ind(pc);

    return {pc, p};
}


template<typename C>
void Path<C>::
set_driver_end(const End::Id e)
{
    d = structure::descriptors::EdgeSlotC{pthw[0], e};
}


template<typename C>
auto Path<C>::
driver2source(const Driver& dr) const noexcept -> Source
{
    return Source {dr.w, cmp->chain(dr.w).ind2end(dr.ind, dr.egEnd)};
}


template<typename C>
constexpr
auto Path<C>::
end_to_a_tip(const C::Chain& m,
             const End::Id e)  // static
{
    return e == End::A ? 0 : m.length();
}


template<typename C>
void Path<C>::
set_cmp(Component& cc) noexcept
{
    cmp = cc;
}


template<typename C>
void Path<C>::
set_src(const Source& src) noexcept
{
    s = src;
}


/*
template<typename C>
void Path<C>::
set_skeleton() const
{
    // using path over component-wde edge indexes indc
    skeleton.clear();

    // first path element treat may be a chain inner edge, so treat it separately
    const auto& gl0 = cmp.egl[pthc[0]];
    const auto w0 = gl0.w;
    const auto ind0 = gl0.i;
    if (cmp.chain(w0).length() == 1) {
        // the first chain of the path is single-edge
        if (pthc.size() == 1) {  // single-edge path over a single_edge chain
            skeleton.emplace_back(Driver{ind0, w, 0});  // free side
            skeleton.emplace_back(Driver{ind0, w, 1});  // free side
            return;
        }
        else {  // multi-edge path starting on a single_edge chain
            const auto& gl1 = cmp.egl[pthc[1]];
            const auto ind1 = gl1.i;
            const auto es = inds_to_chain_link(ind0, gl1.i);
            // es[0] is a slot on w0 connecting to the chain that follows path
            if (es[0].e == End::B) {
                skeleton.emplace_back(Driver{ind0, w, 0});  // free side
                skeleton.emplace_back(Driver{ind0, w, 1});  // connected side
            }
            else if (es[0].e == End::A) { {
                skeleton.emplace_back(Driver{ind0, w, 1});  // free side
                skeleton.emplace_back(Driver{ind0, w, 0});  // connected side
            }
            else
                abort("edges with inds", ind0,", " gl1.i,
                      "are not on the same junction");
        }
    }
    else { // the first chain of the path is multi-edge
        // the second edge of the path is guaranteed to be on the chain end
    }

    for (EgId i {1}; i < pthc.length(); ++i) {
        const auto w = cmp.egl[p[i]].w;
            if (cmp.chain(w).length > 1) {

                pes.push_back(EndSlot{w, e});
            }
            else {
                pes.push_back(EndSlot{w, e});
                pes.push_back(EndSlot{w, e});
            }
        }
        else
            pes.push_back(p);
    }
}
*/


/*
template<typename C>
void Path<C>::
set_clagl( Graph& gr )
{
    Path<Graph>::clagl.resize(gr.clnum);
    for (std::size_t c=0; c<gr.clnum; ++c)
        make_adjacency_list(c, clagl[c], gr);
}

template<typename Graph>
void Path<Graph>::
set_clagl( const std::size_t c, Graph& gr )
{
    make_adjacency_list(c, clagl[c], gr);
}



template<typename C>
bool Path<C>::
hasPath(Graph& gr,
    const EgId ind1,
    const EgId ind2
)  // static
{
    const auto indcl1 = gr.cn[gr.glm[ind1]].g[gr.gla[ind1]].indcl;
    const auto indcl2 = gr.cn[gr.glm[ind2]].g[gr.gla[ind2]].indcl;

    auto& vis = gr.clvisited[c];
    std::fill(vis.begin(), vis.end(), false);
    vis[indcl1] = true;
    std::deque<size_t> q(1, indcl1);

    return GG.bfs(gr.clagl[c], q, vis, indcl2);
}


template<typename C>
void Path<C>::
make_adjacency_list(
    const std::size_t c,
    typename gT::aglT& a,
    Graph& gr
)
{
    gr.clvisited[c].resize(gr.cls[c]);

    a.resize(gr.cls[c]);
    for (auto& o : a)
        o.clear();

    for (const auto j : gr.clmt[c]) {
        auto& m = cn[j];
        for (std::size_t k=0; k<m.length(); ++k)
            if (!m.g[k].isImmobile) {
                const auto ind = m.g[k].indcl;
                if (k == 0) {
                    for (unsigned int ie=1; ie<=m.nn[1]; ++ie) {            // connection backwards: only other mitos might be found
                        const auto w2 = m.neig[1][ie];
                        const auto a2 = cn[w2].end2a(m.neen[1][ie]);
                        const auto& g2 = cn[w2].g[a2];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, 1));
                    }
                    if (m.length() == 1)                            // connection forwards: to other chain
                        for (unsigned int ie=1; ie<=m.nn[2]; ++ie) {
                            const auto w2 = m.neig[2][ie];
                            const auto a2 = cn[w2].end2a(m.neen[2][ie]);
                            const auto& g2 = cn[w2].g[a2];
                            if (!g2.isImmobile)
                                a[ind].push_back(typename gT::neighbour(g2.indcl, 1));
                        }
                    else {                                            // connection forwards: to the same chain
                        const auto& g2 = m.g[k+1];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, 1));
                    }
                }
                else if (k == m.length()-1) {                        // but not  a1 == 0  =>  cn[m1].length() > 1
                    a[ind].push_back(typename gT::neighbour(m.g[k-1].indcl, 1));    // connection backwards: to the same chain
                    for (unsigned int ie=1; ie<=m.nn[2]; ++ie) {                            // connection forwards: to other chain
                        const auto w2 = m.neig[2][ie];
                        const auto a2 = cn[w2].end2a(m.neen[2][ie]);
                        const auto& g2 = cn[w2].g[a2];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, 1));
                    }
                }
                else {                                                // chain internal edge: a1 != 1 && a1 != cn[m1].length()
                    if (!m.g[k-1].isImmobile) a[ind].push_back(typename gT::neighbour(m.g[k-1].indcl, 1));    // connection backwards: to the same chain
                    if (!m.g[k+1].isImmobile) a[ind].push_back(typename gT::neighbour(m.g[k+1].indcl, 1));    // connection forwards: to the same chain
                }
            }
        }
}
*/


template<typename C>
constexpr
auto Path<C>::
driver_chid() const noexcept -> ChIdG
{
    return d.w;
}


template<typename C>
constexpr
auto Path<C>::
driver_chain_end() const noexcept -> End::Id
{
    const auto m = cmp->chain(d.w);

    return m.ind_is_end(d.ind) ? m.ind2end(d.ind, d.egEnd)
                               : End::undefined;
}


template<typename C>
constexpr
auto Path<C>::
driver_vertex() const noexcept -> const Vertex&
{
    const auto vp = cmp->edge(pthc[0]).vertex_at_inner(d.egEnd);

    ASSERT(vp, "Driver edge leading end is not connected");

    return *vp;
}

template<typename C>
constexpr
auto Path<C>::
driver_vertex() noexcept -> Vertex&
{
    Vertex* vp = const_cast<Vertex*>(cmp->edge(pthc[0]).vertex_at_inner(d.egEnd));

    ASSERT(vp, "Driver edge leading end is not connected");

    return *vp;
}



template<typename C>
constexpr
auto Path<C>::
source_chid() const noexcept -> ChIdG
{
    return s.w;
}


template<typename C>
constexpr
auto Path<C>::
source_end() const noexcept -> End::Id
{
    return s.e;
}

template<typename C>
constexpr
auto Path<C>::
source_vertex() const noexcept -> const Vertex&
{
    const auto vp = cmp->edge(pthc.back()).vertex_at_outer(s.e);

    ASSERT(vp, "Driver edge leading end is not connected");

    return *vp;
}

template<typename C>
constexpr
auto Path<C>::
source_vertex() noexcept -> Vertex&
{
    auto vp = const_cast<Vertex*>(cmp->edge(pthc.back()).vertex_at_outer(s.e));

    ASSERT(vp, "Driver edge leading end is not connected");

    return *vp;
}


template<typename C>
constexpr
auto Path<C>::
length_over_source_chain() const noexcept -> std::size_t
{
    std::size_t n {};

    for (auto it = pthc.crbegin(); it != pthc.rend(); it++)
        if (cmp->get_egl(*it).w == s.w)
            ++n;
        else
            break;

    return n;
}


template<typename C>
constexpr
auto Path<C>::
vertices() const noexcept -> std::vector<Vertex*>
{
    return Base::vertices(pthc, d.egEnd);
}

template<typename C>
template<typename... Args>
void Path<C>::
print_path(Args... args) const noexcept
{
    Base::template print_path<true>(pthw, args...);
}


template<typename C>
template<bool withChains>
void Path<C>::
print_short(const std::string& str) const
{
    if (str.length())
        jot(str);

    this->print_edge(0, pthc.front(),
                     colorcodes::YELLOW, "pthw.front:", colorcodes::RESET,
                     " drv(egEnd ", End::str(d.egEnd), "): ");
    Base::template print_inds<true, withChains>(pthw);
    this->print_edge(pthc.size() - 1, pthc.back(),
                     colorcodes::YELLOW, "pthw.back:                ",
                     colorcodes::RESET);
}


template<typename C>
void Path<C>::
print_detailed(const std::string& str) const
{
    if (str.length())
        jot(str);

    this->print_edge(0, pthc[0], colorcodes::CYAN, "driver", colorcodes::RESET,
                     " (egEnd ", End::str(d.egEnd), ") ");

    for (std::size_t i {1}; i<pthc.size()-1; ++i)
        this->print_edge(i, pthc[i], std::string(17, ' '));

    this->print_edge(pthc.size()-1, pthc.back(),
                     colorcodes::CYAN, "source", colorcodes::RESET,
                     " (end ", End::str(s.e), ")   ");
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_PATHS_H
