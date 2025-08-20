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
 * @file generic.h
 * @brief Specification of the template for paths over graph edges.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H
#define GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H

#include <algorithm>  // remove, ranges::sort
#include <array>
#include <ranges>
#include <set>
#include <string>
#include <utility>  // pair
#include <vector>

#include "../../../definitions.h"
#include "../../vertices/collections.h"
#include "../../vertices/vertex.h"
#include "distance.h"


namespace graph_mutator::structure::paths::over_edges {


template<typename Component>
struct Generic {

    using Chain = Component::Chain;
    using Edge = Chain::Edge;
    using EndSlot = Chain::EndSlot;
    using Ends = Edge::Ends;  ///< Type alias for edge ends.

    /// Type alias for path over consecutively connected vertexes.
    using Path = std::deque<EgId>;

    using Dist = Distance<Component>;  ///< Distance over edges
    using Distances = std::vector<Dist>;  ///< Distances over edges.

    Component const* cmp {};   ///< Pointer to the graph component.

    explicit constexpr Generic(Component const* const cmp) noexcept;

    Generic() = default;
    ~Generic() = default;
    constexpr Generic(const Generic& all) noexcept;
    constexpr Generic(Generic&& all) noexcept;
    constexpr auto operator=(const Generic& all) -> Generic&;
    constexpr auto operator=(Generic&& all) -> Generic&;

    /**
     * \brief Computes paths connecting a vertex in the graph
     * \details Computes paths starting at vertex \p source to other vertexes
     * in the connected component of the graph;
     * \note utilizes the component adjacency list of chains.
     * \note Implements Dijkstra's algorithm
     * \ref https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm.
     * \param[in] source Path element from which the paths are computed.
     */
    void compute_from_source(EgId source);

    /**
     * \brief The shortest path between two graph vertex slots.
     * \details Calculates the shortest path from edge with a component-wide
     * index \p s1 to edge with index \p s2.
     * \param[in] s1 indc of the first edge of the path.
     * \param[in] s2 indc of the last edge of the path.
     * \return The shortest path between slots \p s1 and \p s2 .
     */
    template<bool computeFromSource>
    auto find_shortest_path(
        EgId s1,
        EgId s2
    ) -> Path;

    /**
     * \brief Converts path using component-wide EgId to graph-wide EgId.
     * \param Indexable sequence of graph-wide edge indexes.
     */
    auto from_global_ind(const Path& pg) const -> Path;

    /**
     * \brief Converts path using component-wide edge ids to graph-wide edge ids.
     * \returns Path as a sequence of graph-wide edge ids.
     */
    auto path_to_global_ind(const Path& pc) -> const Path;

    /**
     * \brief Converts distances using component-wide EgId to graph-wide EgId.
     * \returns Distances as a sequence of graph-wide edge distances.
     */
    auto distances_to_global_ind() -> const Distances;

    /**
     * \brief Extracts chain indexes of edges in the path \p path.
     * \param path Path over edges as component-wide Ids.
     * \return Sequence of graph-wide chain indexes.
     */
    auto path_chains(const Path& path) const noexcept -> std::vector<ChId>;

    template<typename... Args>
    void print_distances(Args... args) const noexcept;

    template<bool isGlobal,
             typename... Args>
    void print_path(const Path& path, Args... args) const noexcept;

    auto edge_color(const Chain& m,
                    const Edge& eg) const noexcept -> const char*;

    template<bool isGlobal,
             bool withChains>
    void print_inds(const Path& path) const noexcept;

    template<bool isGlobal,
             typename... Args>
    void print_edges(const Path& path, Args... args) const noexcept;

    template<typename... Args>
    void print_edge(EgId i, EgId indc, Args... args) const noexcept;

private:

    using Q = std::set<Dist>;
    Q q;

    Distances distances;

    decltype(Component::ajlg) ajlg;

    void set_agl();
    void reset();

    /**
     * \brief Computes element indexes of the \a min_distance and \a previous.
     */
    constexpr auto element_ind(EgId s) const noexcept -> szt;

    constexpr auto element(szt i) const noexcept -> EgId;

    void update(
        const Q::value_type& ud,
        EgId v
    );
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Component>
constexpr
Generic<Component>::
Generic(Component const* const cmp) noexcept
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
    ajlg = cmp->adjacency_list_edges();
    for (szt i {}; i<ajlg.size(); ++i) {
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
element_ind(const EgId s) const noexcept -> szt
{
    return static_cast<szt>(s);
}


template<typename Component>
constexpr
auto Generic<Component>::
element(const szt i) const noexcept -> EgId
{
    return static_cast<EgId>(i);
}


template<typename Component>
void Generic<Component>::
update(
    const Q::value_type& ud,
    const EgId v
)
{
    const auto d = ud.get_dist() + cmp->edge(v).weight;
    const auto vi = element_ind(v);

    if (d < distances[vi].get_dist()) {
        q.erase(Dist {v, distances[vi].get_dist()});
        distances[vi].set(ud.get_prev(), d);
        q.emplace(v, distances[vi].get_dist());
    }
}


template<typename Component>
void Generic<Component>::
compute_from_source(const EgId source)
{
    reset();

    distances[element_ind(source)].set_dist(Dist::zero);
    q.emplace(source, Dist::zero);

    do {
        const auto ud = std::move(*q.begin());
        q.erase(q.begin());

        // Visit each edge adjacent to u
        for (const auto nb: ajlg[ud.get_prev()])
            update(ud, nb);
    } while (!q.empty());
}


template<typename Component>
template<bool computeFromSource>
auto Generic<Component>::
find_shortest_path(
    const EgId s1,  // starting edge indc
    const EgId s2   // final edge indc
) -> Path
{
    if constexpr (computeFromSource)
        compute_from_source(s1);  // populate distances

    if (distances[element_ind(s2)].is_finite()) {

        // The shortest path edge sequence from s1 to s2:
        Path path {s2};
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
from_global_ind(const Path& pg) const -> Path
{
    Path pc;  // Path using component-wide EgIds.

    for (const auto ind : pg) {
        is_defined(ind) ? pc.push_back(cmp->ind2indc(ind))
                        : pc.push_back(ind);
    }

    return pc;
}


template<typename Component>
auto Generic<Component>::
path_to_global_ind(const Path& pc) -> const Path
{
    Path pg;  // Path using graph-wide EgIds.

    for (const auto p: pc) {
        if (is_defined(p))
            pg.push_back(cmp->edge(p).ind);
        else
            pg.push_back(p);
    }

    return pg;
}


template<typename Component>
auto Generic<Component>::
distances_to_global_ind() -> const Distances
{
    // !!! the output has size is cmp.num_edges(), not gr.numedges,
    // and elements are still indexed as indc
    Distances dg;

    for (const auto p: distances) {
        if (is_defined(p))
            dg.push_back(cmp->edge(p).ind);
        else
            dg.push_back(p);
    }

    return dg;
}


template<typename Component>
auto Generic<Component>::
path_chains(const Path& path) const noexcept -> std::vector<ChId>
{
    std::set<ChId> s;  // set elements are unique by definition

    for (const auto indc: path)
        s.insert(s.end(), cmp->gl[indc].w);

    return std::vector<ChId>(s.begin(), s.end());
}


template<typename Component>
template<typename... Args>
void Generic<Component>::
print_distances(Args... args) const noexcept
{
    log_(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, cmp->ind, colorcodes::RESET, ": Distances ",
         args...);

    for (szt i {}; i<distances.size(); ++i)
        distances[i].print(element(i));
    log_("");
}


template<typename Component>
template<bool isGlobal,
         typename... Args>
void Generic<Component>::
print_path(const Path& path,
           Args... args) const noexcept
{
    const auto ne = path.size();
    const auto chs = path_chains(path);
    log_(colorcodes::YELLOW, "Component ",
         colorcodes::BOLDCYAN, cmp->ind, colorcodes::RESET, ": ",
         args..., " path  from ", path.front(), " to ", path.back(),
         " has length ", ne, (ne == 1 ? " edge" : " edges"), " over ", chs.size(),
         (chs.size() == 1 ? " chain: " : " chains: "));

    print_inds<isGlobal, true>(path);
}


template<typename Component>
auto Generic<Component>::
edge_color(const Chain& m,
           const Edge& eg) const noexcept -> const char*
{
    return m.is_tail(eg) && !m.is_head(eg)
           ? colorcodes::GREEN
           : !m.is_tail(eg) && m.is_head(eg)
           ? colorcodes::RED
           : m.is_tail(eg) && m.is_head(eg)
           ? colorcodes::YELLOW
           : colorcodes::WHITE;
}


template<typename Component>
template<bool isGlobal,
         bool withChains>
void Generic<Component>::
print_inds(const Path& path) const noexcept
{
    std::string indcaption = isGlobal ? "ind:  "
                                      : "indc: ";
    std::string wcaption = withChains ? "w  :  " : "";

    szt len = std::max(indcaption.length(), wcaption.length());

    auto empty = [](const szt maxn, const std::string& s)
    {
        return std::string(maxn - s.length() + 1, ' ');
    };

    std::vector<std::string> inds {indcaption + empty(len, indcaption)};
    std::vector<std::string> ws {wcaption + empty(len, wcaption)};
    std::vector<const char*> colors {colorcodes::WHITE};

    for (const auto& p: path) {
        const auto istr = std::to_string(p);

        const auto indc = isGlobal ? cmp->ind2indc(p) : p;
        const auto w = cmp->gl[indc].w;
        const auto a = cmp->gl[indc].a;
        const auto& m = cmp->chain(cmp->gl[indc].w);

        std::string slotstr {};

        if constexpr (withChains) {
            slotstr = m.is_headind(p) && !m.is_tailind(p)
                    ? EndSlot{w, Ends::B}.str_short()
                    : !m.is_headind(p) && m.is_tailind(p)
                    ? EndSlot{w, Ends::A}.str_short()
                    : m.is_headind(p) && m.is_tailind(p)
                    ? std::to_string(w) + " AB"
                    : "";
        }

        len = std::max(istr.length(), slotstr.length());

        inds.push_back(istr + empty(len, istr));
        colors.push_back(edge_color(m, m.g[a]));
        if constexpr (withChains)
            ws.push_back(slotstr + empty(len, slotstr));
    }


    for (szt i {}; i<inds.size(); ++i)
        log_<false>(colors[i], inds[i], colorcodes::RESET);
    log_("");
    if constexpr (withChains) {
        for (szt i {}; i<ws.size(); ++i)
            log_<false>(colors[i], ws[i], colorcodes::RESET);
        log_("");
    }
}


template<typename Component>
template<bool isGlobal,
         typename... Args>
void Generic<Component>::
print_edges(const Path& path,
            Args... args) const noexcept
{
    const auto pthc = isGlobal ? from_global_ind(path)
                               : path;

    for (EgId i {}; i<pthc.size(); ++i)
        print_edge(i, pthc[i], args...);
}

template<typename Component>
template<typename... Args>
void Generic<Component>::
print_edge(const EgId i,
           const EgId indc,
           Args... args) const noexcept
{
    const auto gl = cmp->gl[indc];
    const auto& m = cmp->chain(gl.w);
    const auto& eg = m.g[gl.a];
    log_<false>(args..., edge_color(m, eg), i, colorcodes::RESET, ": ");
    eg.print("", m.dir_char(gl.a));
}


}  // namespace graph_mutator::structure::paths::over_edges

#endif  // GRAPH_MUTATOR_STRUCTURE_PATHS_OVER_EDGES_GENERIC_H
