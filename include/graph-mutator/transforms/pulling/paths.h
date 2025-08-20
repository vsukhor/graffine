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
 * @file paths.h
 * @brief Contains structure for representing paths in the graph in a way
 * convenient for pulling transformations.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_PATHS_H
#define GRAPH_MUTATOR_PULLING_PATHS_H

#include <vector>

#include "../../definitions.h"
#include "../../structure/basic/component.h"
#include "../../structure/basic/slot.h"  // for IndEgInd
#include "../../structure/basic/paths/over_edges/generic.h"
#include "../../structure/basic/paths/over_endslots/generic.h"
#include "common.h"


namespace graph_mutator::pulling {


template<typename C>
struct Paths
    : public structure::basic::paths::over_edges::Generic<C> {

    using Component = C;
    using Chain = Component::Chain;
    using EndSlot = Chain::EndSlot;
    using Ends = Chain::Ends;
    using Edge =  Component::Edge;
    using Base = structure::basic::paths::over_edges::Generic<Component>;
    using Path = Base::Path;
    using PathCh = structure::basic::paths::over_endslots::Generic<Component>::Path;
//    using Skeleton = paths::Container<Driver>;
    using Source = EndSlot;

    struct Driver {

        EgId ind {undefined<EgId>};

        ChId w {undefined<ChId>};

        Ends::Id egEnd {Ends::Undefined};

        explicit Driver(
            Edge const* const eg,
            const Ends::Id egEnd
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

            return is_defined(e) ? EndSlot {w, e}
                                 : EndSlot {};
        }

        void print() const noexcept
        {
            log_("ind ", ind, " w ", w, " egEnd ", Ends::str(egEnd));
        }
    };

    template<typename> friend struct FunctorBase;

    template<Degree D,
             Orientation Dir,
             typename G> requires (is_pullable_degree<D>)
    friend struct On;

    using Base::cmp;

    Path pth;   // active path using edge ind (graph-wde)
    Path pthc;  // active path using edge indc (component-wde);

//    Skeleton skeleton;

    explicit Paths(
        Component const* const cmp,
        Driver d,
        Source s
    );

    explicit Paths(
        Component const* const cmp,
        Driver d,
        Source s,
        std::vector<Driver> internals
    );

    explicit Paths(
        Component const* const cmp,
        Driver d,
        Source s,
        Path pth
    );

    Paths() = default;
    ~Paths() = default;
    constexpr Paths(const Paths& p) noexcept;
    constexpr Paths(Paths&& p) noexcept;
    constexpr auto operator=(const Paths& p) -> Paths&;
    constexpr auto operator=(Paths&& p) -> Paths&;

    static constexpr auto end_to_a_tip(const C::Chain& m,
                                       const Ends::Id e);

//    void set_skeleton() const;

    constexpr auto driver_end() const noexcept -> Ends::Id;

    auto driver2source(const Driver& dr) const noexcept -> Source;

    void set_cmp(Component& cc) noexcept;

    constexpr auto drv() const noexcept -> const Driver& { return d; }

    constexpr auto src() const noexcept -> const Source& { return s; }
    void set_src(const Source& src) noexcept;

    constexpr auto length() const noexcept -> EgId;

    constexpr auto length_over_source_chain() const noexcept -> EgId;

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
        Component const* const cmp
    ) -> std::array<Path, 2>;

    void set_driver_end(const Ends::Id e);
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename C>
Paths<C>::
Paths(
    Component const* const cmp,
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
Paths<C>::
Paths(
    Component const* const cmp,
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

        auto sh = set_shortest(d, driver2source(internals.front()), cmp);
        pthc.insert(pthc.end(), sh[0].cbegin(), sh[0].cend());
        pth.insert(pth.end(), sh[1].cbegin(), sh[1].cend());

        for (szt i {1}; i<internals.size(); i++) {
            ASSERT(cmp->chain(internals[i].w).c == cmp->ind,
                   "Compartment of internal ", i, " is different from cmp");
            ASSERT(internals[i].ind != d.ind, "Internal ", i, " is same as driver");
            ASSERT(internals[i].ind != cmp->chain(s.w).end2ind(s.e),
                   "Internal ", i, " is same as source");

            sh = set_shortest(internals[i-1], driver2source(internals[i]), cmp);
            pthc.insert(pthc.end(), sh[0].cbegin()+1, sh[0].cend());
            pth.insert(pth.end(), sh[1].cbegin()+1, sh[1].cend());
        }

        sh = set_shortest(internals.back(), s, cmp);
        pthc.insert(pthc.end(), sh[0].cbegin()+1, sh[0].cend());
        pth.insert(pth.end(), sh[1].cbegin()+1, sh[1].cend());
    }
    else
        set_shortest();
}


template<typename C>
Paths<C>::
Paths(
    Component const* const cmp,
    Driver d,
    Source s,
    Path pth
)
    : Base {cmp}
    , pth {pth}
    , pthc {this->from_global_ind(pth)}
    , d {d}
    , s {s}
{
    ASSERT(cmp->chain(s.w).c == cmp->ind,
           "Source compartment is different from cmp");
    ASSERT(cmp->chain(d.w).c == cmp->ind,
           "Driver compartment is different from cmp");
    ASSERT(d.ind == cmp->gl[pthc[0]].i, "Driver ind is not in cmp.gl");
}


template<typename C>
constexpr Paths<C>::
Paths(const Paths& p) noexcept
    : Base {p.cmp}
    , pth {p.pth}
    , pthc {p.pthc}
    , d {p.d}
    , s {p.s}
    , internals {p.internals}
{}


template<typename C>
constexpr Paths<C>::
Paths(Paths&& p) noexcept
    : Base {std::move(p.cmp)}
    , pth {std::move(p.pth)}
    , pthc {std::move(p.pthc)}
    , d {std::move(p.d)}
    , s {std::move(p.s)}
    , internals {std::move(p.internals)}
{}


template<typename C>
constexpr
auto Paths<C>::
operator=(const Paths& p) -> Paths&
{
    Base::operator=(p);
    pth = p.pth;
    pthc = p.pthc;
    d = p.d;
    s = p.s;
    internals = p.internals;

    return *this;
}


template<typename C>
constexpr
auto Paths<C>::
operator=(Paths&& p) -> Paths&
{
    Base::operator=(std::move(p));
    pth = std::move(p.pth);
    pthc = std::move(p.pthc);
    d = std::move(p.d);
    s = std::move(p.s);
    internals = std::move(p.internals);

    return *this;
}


template<typename C>
constexpr
auto Paths<C>::
length() const noexcept -> EgId
{
    return pth.size();
}


template<typename C>
void Paths<C>::
set_shortest()
{
    // driver is at pth.front(),
    // source is at pth.back()
    pthc = this->template find_shortest_path<true>(
        cmp->ind2indc(d.ind),
        cmp->chain(s.w).end_edge(s.e).indc
    );
    pth = this->path_to_global_ind(pthc);
}


template<typename C>
auto Paths<C>::
set_shortest(const Driver& dr,
             const Source& sr,
             Component const* const c) -> std::array<Path, 2>  // static
{
    // driver is at pth.front(),
    // source is at pth.back()
    Base b {c};

    const auto icD = b.cmp->ind2indc(dr.ind);
    const auto icS = b.cmp->chain(sr.w).end_edge(sr.e).indc;

    Path pc;
    if (dr.w != sr.w)
        pc = b.template find_shortest_path<true>(icD, icS);
    else {
        const auto w = dr.w;
        const auto aD = b.cmp->gl[icD].a;
        const auto aS = b.cmp->gl[icS].a;
        if (aS < aD)
            for (long int a {static_cast<long int>(aD)};
                 a>=static_cast<long int>(aS); --a)
                pc.push_back(b.cmp->chain(w).g[a].indc);
        else
            for (szt a {aD}; a<=aS; ++a)
                pc.push_back(b.cmp->chain(w).g[a].indc);
    }

    const auto p = b.path_to_global_ind(pc);

    return {{pc, p}};
}


template<typename C>
void Paths<C>::
set_driver_end(const Ends::Id e)
{
    d = structure::basic::IndEgEnd{pth[0], e};
}


template<typename C>
auto Paths<C>::
driver2source(const Driver& dr) const noexcept -> Source
{
    return Source {dr.w, cmp->chain(dr.w).ind2end(dr.ind, dr.egEnd)};
}


template<typename C>
constexpr
auto Paths<C>::
end_to_a_tip(const C::Chain& m,
             const Ends::Id e)  // static
{
    return e == Ends::A ? 0 : m.length();
}


template<typename C>
void Paths<C>::
set_cmp(Component& cc) noexcept
{
    cmp = cc;
}


template<typename C>
void Paths<C>::
set_src(const Source& src) noexcept
{
    s = src;
}


/*
template<typename C>
void Paths<C>::
set_skeleton() const
{
    // using path over component-wde edge indexes indc
    skeleton.clear();

    // first path element treat may be a chain inner edge, so treat it separately
    const auto& gl0 = cmp.gl[pthc[0]];
    const auto w0 = gl0.w;
    const auto ind0 = gl0.i;
    if (cmp.cn[w0].length() == 1) {
        // the first chain of the path is single-edge
        if (pthc.size() == 1) {  // single-edge path over a single_edge chain
            skeleton.emplace_back(Driver{ind0, w, 0});  // free side
            skeleton.emplace_back(Driver{ind0, w, 1});  // free side
            return;
        }
        else {  // multi-edge path starting on a single_edge chain
            const auto& gl1 = cmp.gl[pthc[1]];
            const auto ind1 = gl1.i;
            const auto es = inds_to_chain_link(ind0, gl1.i);
            // es[0] is a slot on w0 connecting to the chain that follows path
            if (es[0].e == Ends::B) {
                skeleton.emplace_back(Driver{ind0, w, 0});  // free side
                skeleton.emplace_back(Driver{ind0, w, 1});  // connected side
            }
            else if (es[0].e == Ends::A) { {
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
        const auto w = cmp.gl[p[i]].w;
            if (cmp.cn[w].length > 1) {

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
void Paths<C>::
set_clagl( Graph& gr )
{
    Path<Graph>::clagl.resize(gr.clnum);
    for (szt c=0; c<gr.clnum; ++c)
        make_adjacency_list(c, clagl[c], gr);
}

template<typename Graph>
void Paths<Graph>::
set_clagl( const szt c, Graph& gr )
{
    make_adjacency_list(c, clagl[c], gr);
}



template<typename C>
bool Paths<C>::
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
    std::deque<szt> q(1, indcl1);

    return GG.bfs(gr.clagl[c], q, vis, indcl2);
}


template<typename C>
void Paths<C>::
make_adjacency_list(
    const szt c,
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
        for (szt k=0; k<m.length(); ++k)
            if (!m.g[k].isImmobile) {
                const auto ind = m.g[k].indcl;
                if (k == 0) {
                    for (unsigned int ie=1; ie<=m.nn[1]; ++ie) {            // connection backwards: only other mitos might be found
                        const auto w2 = m.neig[1][ie];
                        const auto a2 = cn[w2].end2a(m.neen[1][ie]);
                        const auto& g2 = cn[w2].g[a2];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, one<real>));
                    }
                    if (m.length() == 1)                            // connection forwards: to other chain
                        for (unsigned int ie=1; ie<=m.nn[2]; ++ie) {
                            const auto w2 = m.neig[2][ie];
                            const auto a2 = cn[w2].end2a(m.neen[2][ie]);
                            const auto& g2 = cn[w2].g[a2];
                            if (!g2.isImmobile)
                                a[ind].push_back(typename gT::neighbour(g2.indcl, one<real>));
                        }
                    else {                                            // connection forwards: to the same chain
                        const auto& g2 = m.g[k+1];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, one<real>));
                    }
                }
                else if (k == m.length()-1) {                        // but not  a1 == 0  =>  cn[m1].length() > 1
                    a[ind].push_back(typename gT::neighbour(m.g[k-1].indcl, one<real>));    // connection backwards: to the same chain
                    for (unsigned int ie=1; ie<=m.nn[2]; ++ie) {                            // connection forwards: to other chain
                        const auto w2 = m.neig[2][ie];
                        const auto a2 = cn[w2].end2a(m.neen[2][ie]);
                        const auto& g2 = cn[w2].g[a2];
                        if (!g2.isImmobile)
                            a[ind].push_back(typename gT::neighbour(g2.indcl, one<real>));
                    }
                }
                else {                                                // edge in the bulk: a1 != 1 && a1 != cn[m1].length()
                    if (!m.g[k-1].isImmobile) a[ind].push_back(typename gT::neighbour(m.g[k-1].indcl, one<real>));    // connection backwards: to the same chain
                    if (!m.g[k+1].isImmobile) a[ind].push_back(typename gT::neighbour(m.g[k+1].indcl, one<real>));    // connection forwards: to the same chain
                }
            }
        }
}
*/


template<typename C>
constexpr
auto Paths<C>::
driver_end() const noexcept -> Ends::Id
{
    return cmp->chain(d.w).ind2end(d.ind, d.egEnd);
}


template<typename C>
constexpr
auto Paths<C>::
length_over_source_chain() const noexcept -> EgId
{
    EgId n {};

    for (auto it = pthc.crbegin(); it != pthc.rend(); it++)
        if (cmp->gl[*it].w == s.w)
            ++n;
        else
            break;

    return n;
}


template<typename C>
template<typename... Args>
void Paths<C>::
print_path(Args... args) const noexcept
{
    Base::template print_path<true>(pth, args...);
}


template<typename C>
template<bool withChains>
void Paths<C>::
print_short(const std::string& str) const
{
    if (str.length())
        log_(str);

    this->print_edge(0, pthc.front(),
                     colorcodes::YELLOW, "pth.front:", colorcodes::RESET,
                     " drv(egEnd ", Ends::str(d.egEnd), "): ");
    Base::template print_inds<true, withChains>(pth);
    this->print_edge(pthc.size() - 1, pthc.back(),
                     colorcodes::YELLOW, "pth.back:                ",
                     colorcodes::RESET);
}


template<typename C>
void Paths<C>::
print_detailed(const std::string& str) const
{
    if (str.length())
        log_(str);

    this->print_edge(0, pthc[0], colorcodes::CYAN, "driver", colorcodes::RESET,
                     " (egEnd ", Ends::str(d.egEnd), ") ");

    for (EgId i {1}; i<pthc.size()-1; ++i)
        this->print_edge(i, pthc[i], std::string(17, ' '));

    this->print_edge(pthc.size()-1, pthc.back(),
                     colorcodes::CYAN, "source", colorcodes::RESET,
                     " (end ", Ends::str(s.e), ")   ");
}


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_PATHS_H
