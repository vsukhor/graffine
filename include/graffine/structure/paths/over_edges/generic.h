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
 * \file generic.h
 * \brief Specification of the template for paths over graph edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H
#define GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H

#include "graffine/definitions.h"
#include "distance.h"
#include "graffine/structure/containers/vertices.h"
#include "graffine/structure/elements/vertex.h"

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <deque>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>


namespace graffine::structure::paths::over_edges {


template<typename Component>
struct Generic
{
    using Chain = Component::Chain;
    using Edge = Chain::Edge;
    using Vertex = Component::Vertex;
    using EndSlot = Chain::EndSlot;
    using End = Edge::End;  ///< Type alias for edge ends.

    /// Type alias for path over consecutively connected vertexes.
    using PathC = std::deque<EgIdC>;
    using PathW = std::deque<EgIdG>;

    using Dist = Distance<Component>;     ///< Distance over edges
    using Distances = std::vector<Dist>;  ///< Distances over edges.

    const Component* cmp {};  ///< Pointer to the graph component.

    explicit constexpr Generic(const Component* const cmp) noexcept;

    Generic() = default;
    ~Generic() = default;
    constexpr Generic(const Generic& all) noexcept;
    constexpr Generic(Generic&& all) noexcept;
    constexpr auto operator=(const Generic& all) -> Generic&;
    constexpr auto operator=(Generic&& all) -> Generic&;

    /**
     * Computes paths connecting a vertex in the graph
     * \details Computes paths starting at vertex \p source to other vertexes
     * in the connected component of the graph;
     * \note utilizes the component adjacency list of chains.
     * \note Implements Dijkstra's algorithm
     * \see https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm.
     * \param[in] source Path element from which the paths are computed.
     */
    void compute_from_source(EgIdC source);

    /**
     * The shortest path between two graph vertex slots.
     * \details Calculates the shortest path from edge with a component-wide
     * index \p s1 to edge with index \p s2.
     * \param[in] s1 indc of the first edge of the path.
     * \param[in] s2 indc of the last edge of the path.
     * \return The shortest path between slots \p s1 and \p s2 .
     */
    template<bool computeFromSource>
    auto find_shortest_path(
        EgIdC s1,
        EgIdC s2
    ) -> PathC;

    /**
     * Converts path using component-wide EgId to graph-wide EgId.
     * \param[in] pg Indexable sequence of graph-wide edge indexes.
     */
    auto from_global_ind(const PathW& pg) const -> PathC;

    /**
     * Converts path using component-wide edge ids to graph-wide edge ids.
     * \returns Path as a sequence of graph-wide edge ids.
     */
    auto path_to_global_ind(const PathC& pc) -> const PathW;

    /**
     * Converts distances using component-wide EgId to graph-wide EgId.
     * \returns Distances as a sequence of graph-wide edge distances.
     */
    auto distances_to_global_ind() -> const Distances;

    /**
     * Extracts chain indexes of edges in the path \p path.
     * \param[in] path Path over edges as component-wide Ids.
     * \return Sequence of graph-wide chain indexes.
     */
    auto chains(const PathC& path) const noexcept -> std::vector<ChIdG>;

    /**
     * \name Printing.
     * @{
     */

    template<typename... Args>
    void print_distances(Args... args) const noexcept;

    template<bool isGlobal,
             typename... Args>
    void print_path(const auto& path, Args... args) const noexcept;

    template<bool isGlobal,
             bool withChains>
    void print_inds(const auto& path) const noexcept;

    template<bool isGlobal,
             typename... Args>
    void print_edges(const auto& path, Args... args) const noexcept;

    template<typename... Args>
    void print_edge(EgIdG i, EgIdC indc, Args... args) const noexcept;

    ///@}  // Printing.

protected:

    /// These are just for printing: text colorcodes.
    auto edge_color(const Chain& m,
                    const Edge& eg) const noexcept -> const char*;

private:

    using Q = std::set<Dist>;
    Q q;

    Distances distances;

    decltype(Component::adj.lg) ajlg;  ///< Adjacency list.

    void set_agl();
    void reset();

    /**
     * Computes element indexes of the \a min_distance and \a previous.
     */
    constexpr auto element_ind(EgIdC s) const noexcept -> std::size_t;

    constexpr auto element(std::size_t i) const noexcept -> EgIdC;

    void update(
        const Q::value_type& ud,
        EgIdC v
    );
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Component>
constexpr
Generic<Component>::
Generic(const Component* const cmp) noexcept
    : cmp {cmp}
{
    ASSERT(cmp, "Pointer to Component is null");
}


template<typename Component>
constexpr
Generic<Component>::
Generic(const Generic& all) noexcept
    : cmp {all.cmp}
    , q {all.q}
    , distances {all.distances}
    , ajlg {all.ajlg}
{}


template<typename Component>
constexpr
Generic<Component>::
Generic(Generic&& all) noexcept
    : cmp {all.cmp}
    , q {std::move(all.q)}
    , distances {std::move(all.distances)}
    , ajlg {std::move(all.ajlg)}
{}


template<typename Component>
constexpr
auto Generic<Component>::
operator=(const Generic& all) -> Generic&
{
    cmp = all.cmp;
    q = all.q;
    distances = all.distances;
    ajlg = all.ajlg;
    return *this;
}


template<typename Component>
constexpr
auto Generic<Component>::
operator=(Generic&& all) -> Generic&
{
    cmp = all.cmp;
    q = std::move(all.q);
    distances = std::move(all.distances);
    ajlg = std::move(all.ajlg);
    return *this;
}


template<typename Component>
void Generic<Component>::
set_agl()
{
    ajlg = cmp->adj.list_edges(*cmp);
    for (std::size_t i {}; i<ajlg.size(); ++i) {

        auto last = std::unique(ajlg[i].begin(), ajlg[i].end());
        ajlg[i].erase(last, ajlg[i].end());
    }
}


template<typename Component>
void Generic<Component>::
reset()
{
    set_agl();

    distances.clear();
    distances.resize(cmp->num_edges());
    q.clear();
}


template<typename Component>
constexpr
auto Generic<Component>::
element_ind(const EgIdC s) const noexcept -> std::size_t
{
    return s();
}


template<typename Component>
constexpr
auto Generic<Component>::
element(const std::size_t i) const noexcept -> EgIdC
{
    return EgIdC {i};
}


template<typename Component>
void Generic<Component>::
update(
    const Q::value_type& ud,
    const EgIdC v
)
{
    const auto d = ud.get_dist() + cmp->edge(v).weight();
    const auto vi = element_ind(v);

    if (d < distances[vi].get_dist()) {

        q.erase(Dist {v, distances[vi].get_dist()});
        distances[vi].set(ud.get_prev(), d);
        q.emplace(v, distances[vi].get_dist());
    }
}


template<typename Component>
void Generic<Component>::
compute_from_source(const EgIdC source)
{
    reset();

    distances[element_ind(source)].set_dist(Dist::zero);
    q.emplace(source, Dist::zero);

    do {
        const auto ud = std::move(*q.begin());
        q.erase(q.begin());

        // Visit each edge adjacent to u
        for (const auto& nb : ajlg[ud.get_prev()()])
            update(ud, nb);
    } while (!q.empty());
}


template<typename Component>
template<bool computeFromSource>
auto Generic<Component>::
find_shortest_path(
    const EgIdC s1,  // starting edge indc
    const EgIdC s2   // final edge indc
) -> PathC
{
    if constexpr (computeFromSource)
        compute_from_source(s1);  // set distances

    if (distances[element_ind(s2)].is_finite()) {

        // The shortest path edge sequence from s1 to s2:
        PathC path {s2};
        auto u {s2};
        while(u != s1) {
            u = distances[element_ind(u)].get_prev();
            path.push_front(u);
        }

        return path;
    }

    return {};
}


template<typename Component>
auto Generic<Component>::
from_global_ind(const PathW& pg) const -> PathC
{
    PathC pc;  // Path using component-wide EgIdCs.

    for (const auto& ind : pg) {
        ind.is_defined() ? pc.push_back(cmp->ind2indc(ind))
                         : pc.push_back(static_cast<EgIdC>(ind()));
    }

    return pc;
}


template<typename Component>
auto Generic<Component>::
path_to_global_ind(const PathC& pc) -> const PathW
{
    PathW pg;  // Path using graph-wide EgIdGs.

    for (const auto& p : pc)

        p.is_defined()
            ? pg.push_back(cmp->edge(p).ind)
            : pg.push_back(p());

    return pg;
}


template<typename Component>
auto Generic<Component>::
distances_to_global_ind() -> const Distances
{
    // !!! the output has size is cmp.num_edges(), not gr.numedges,
    // and elements are still indexed as indc
    Distances dg;

    for (const auto p : distances)

        p.is_defined()
            ? dg.push_back(cmp->edge(p).ind)
            : dg.push_back(p);

    return dg;
}


template<typename Component>
auto Generic<Component>::
chains(const PathC& path) const noexcept -> std::vector<ChIdG>
{
    std::set<ChIdG> s;  // set elements are unique by definition

    for (const auto& indc : path)
        s.insert(s.end(), cmp->get_egl(indc).w);

    return std::vector<ChIdG>(s.begin(), s.end());
}

template<typename Component>
template<typename... Args>
void Generic<Component>::
print_distances(Args... args) const noexcept
{
    jot(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, cmp->ind, colorcodes::RESET, ": Distances ",
         args...);

    for (std::size_t i {}; i<distances.size(); ++i)
        distances[i].print(element(i));
    jot("");
}


template<typename Component>
template<bool isGlobal,
         typename... Args>
void Generic<Component>::
print_path(const auto& path,
           Args... args) const noexcept
{
    const auto ne = path.size();
    const auto chs = chains(path);
    jot(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, cmp->ind, colorcodes::RESET, ": ",
         args..., " path  from ", path.front(), " to ", path.back(),
         " has length ", ne, (ne == 1 ? " edge" : " edges"), " over ", chs.size(),
         (chs.size() == 1 ? " chain: " : " chains: "));

    print_inds<isGlobal, true>(path);
}

/// These are just for printing: text colorcodes.
template<typename Component>
auto Generic<Component>::
edge_color(const Chain& m,
           const Edge& eg) const noexcept -> const char*
{
    return (m.is_tail(eg) && !m.is_head(eg))
           ? colorcodes::GREEN
           : (!m.is_tail(eg) && m.is_head(eg))
           ? colorcodes::RED
           : (m.is_tail(eg) && m.is_head(eg))
           ? colorcodes::YELLOW
           : colorcodes::WHITE;
}


template<typename Component>
template<bool isGlobal,
         bool withChains>
void Generic<Component>::
print_inds(const auto& path) const noexcept
{
    std::string indcaption = (isGlobal ? "ind:  "
                                       : "indc: ");
    std::string wcaption = (withChains ? "w  :  " : "");

    std::size_t len = std::max(indcaption.length(), wcaption.length());

    auto empty = [](const std::size_t maxn, const std::string& s)
    {
        return std::string(maxn - s.length() + 1, ' ');
    };

    std::vector<std::string> inds {indcaption + empty(len, indcaption)};
    std::vector<std::string> ws {wcaption + empty(len, wcaption)};
    std::vector<const char*> colors {colorcodes::WHITE};

    for (const auto& p : path) {
        const auto istr = p.as_string();

        EgIdC indc {};
        if constexpr (isGlobal)
            indc = cmp->ind2indc(p());
        else
            indc = p;
        const auto w = cmp->get_egl(indc).w;
        const auto a = cmp->get_egl(indc).a;
        const auto& m = cmp->chain(cmp->get_egl(indc).w);

        std::string slotstr {};

        if constexpr (withChains) {
            slotstr = m.is_headind(p()) && !m.is_tailind(p())
                    ? EndSlot{w, End::B}.str_short()
                    : !m.is_headind(p()) && m.is_tailind(p())
                    ? EndSlot{w, End::A}.str_short()
                    : m.is_headind(p()) && m.is_tailind(p())
                    ? w.as_string() + " AB"
                    : "";
        }

        len = std::max(istr.length(), slotstr.length());

        inds.push_back(istr + empty(len, istr));
        colors.push_back(edge_color(m, m.g[a]));
        if constexpr (withChains)
            ws.push_back(slotstr + empty(len, slotstr));
    }

    for (std::size_t i {}; i<inds.size(); ++i)
        jot<false>(colors[i], inds[i], colorcodes::RESET);
    jot("");
    if constexpr (withChains) {
        for (std::size_t i {}; i<ws.size(); ++i)
            jot<false>(colors[i], ws[i], colorcodes::RESET);
        jot("");
    }
}


template<typename Component>
template<bool isGlobal,
         typename... Args>
void Generic<Component>::
print_edges(const auto& path,
            Args... args) const noexcept
{
    const auto pthc = isGlobal ? from_global_ind(path)
                               : path;

    for (std::size_t i {}; i<pthc.size(); ++i)
        print_edge(i, pthc[i], args...);
}

template<typename Component>
template<typename... Args>
void Generic<Component>::
print_edge(const EgIdG i,
           const EgIdC indc,
           Args... args) const noexcept
{
    const auto egl = cmp->get_egl(indc);
    const auto& m = cmp->chain(egl.w);
    const auto& eg = m.g[egl.a];
    jot<false>(args..., edge_color(m, eg), i, colorcodes::RESET, ": ");
    eg.print("");
}


}  // namespace graffine::structure::paths::over_edges

#endif  // GRAFFINE_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H
