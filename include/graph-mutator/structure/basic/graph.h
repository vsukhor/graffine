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
 * @file graph.h
 * @brief Contains class template encapsulating graph structure-related features.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_GRAPH_H
#define GRAPH_MUTATOR_STRUCTURE_GRAPH_H

#include <algorithm>
#include <array>
#include <concepts>
#include <memory>
#include <ostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include "../definitions.h"
#include "paths/over_endslots/generic.h"
#include "chain_collection.h"
#include "chain_indexes.h"
#include "component.h"
#include "vertices/all.h"
#include "vertices/degrees.h"
#include "vertices/vertex.h"


namespace graph_mutator::structure {


/**
 * @brief The Graph class.
 * @details Implements the major structure-related proterties, such a
 * collections and classifications of graph Chains, but is unaware of any
 * dynamics. Forms base for clases adding graph-specific characteristics.
 * @tparam Chain Type of the Chain forming the graph.
*/
template<typename Ch>
struct Graph {

    static_assert(std::is_base_of_v<structure::ChainBase, Ch>);

    static constexpr bool useAgl {};

    enum class CoreElements {Edge, Vertex};

    using Chain = Ch;
    using Edge = Chain::Edge;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Ends = Chain::Ends;  ///< Edge ends.
    using EndId = Ends::Id;
    using Cmpt = Component<Chain>;
    using Components = components::Container<Cmpt>;
    using Chains = Cmpt::Chains;
    template<Degree D>
    using Vertex = vertices::Vertex<D, typename Ch::Slot<D>>;
    using Vertices = vertices::All<Graph>;
    using PathsOverEndSlots = paths::over_endslots::Generic<Cmpt>;

    /// The collection of chains.
    Chains cn;

    Components ct;

    ChainIndexes<false, EndSlot> chis;

    /// The vertices.
    /// Vertices are implicit: they are reconstructed from chains lasily.
    Vertices vertices;

    /// Current number of edges.
    EgId edgenum {};

    /// Mapping of graph-wide edge indexes to chain indexes.
    ChIds glm;

    /// Mapping of graph-wide edge indexes to element edge position inside chains.
    EgIds gla;

    /**
     * @brief Constructor.
     */
    constexpr Graph();

    /**
     * @brief Generate initial set of the graph components.
     * @details The components are single chains of equal length 'len'.
     * @param num Desired number of components.
     * @param len Component length.
     */
    void generate_single_chain_components(
        CmpId num,
        EgId len
    );

    /// Appends a disconnected chain to the graph.
    void add_single_chain_component(
        EgId len,
        ChId ind=undefined<ChId>
    );

    void add_component(Chains&& cg);

    auto mark_branch(const EndSlot& source) -> bool;

    /**
     * @brief Updates the disconnected component indexes.
     * @param f Initial index.
     * @param t Final index.
     */
//    void change_cmpt(CmpId f, CmpId t) noexcept;

    /**
     * @brief Updates the disconnected component indexes.
     * @param c Initial index.
     */
//    void update_indc(CmpId c) noexcept;

    /**
     * @brief Appends \p donor component to \p acceptor component.
     * @note If necessary, lhe last component in \a ct is moved to the donor
     * place and reindexed.
     * @param acceptor Component to which \p donor component is appended.
     * @param donor Component merged to the \p acceptor.
     */
    void merge_components(
        Cmpt& acceptor,
        Cmpt& donor
    ) noexcept;

    /**
     * @brief Appends \p don component to \p acc component.
     * @note If necessary, lhe last component in \a ct is moved to the donor
     * place and reindexed.
     * @param acceptor Component to which \p don component is appended.
     * @param donor Component merged to the \p acc.
     */
    void merge_components(
        CmpId acc,
        CmpId don
    ) noexcept;

    auto cut_component_at(const EndSlot& s) -> bool;

    template<bool withSourceChain>
    void split_component(
        Cmpt& cmp, const
        PathsOverEndSlots& pp,
        const EndSlot& s
    );

    void split_component(Cmpt& cmp, ChIds&& rm);

    /// Current number of components.
    constexpr auto cmpt_num() const noexcept -> CmpId { return ct.num(); };

    /// Current number of chains.
    constexpr auto chain_num() const noexcept -> ChId { return cn.num(); };

    constexpr auto ind_last_cmpt() const noexcept -> CmpId;
    constexpr auto ind_last_chain() const noexcept -> ChId;

    constexpr auto vertices_are_adjacent_aa(
        ChId w1, EgId a1, ChId w2, EgId a2) const noexcept -> bool;
    constexpr auto vertices_are_adjacent_ee(
        const EndSlot& s1, const EndSlot& s2) const noexcept -> bool;
    constexpr auto vertices_are_adjacent_ea(
        const EndSlot& s1, ChId w2, EgId a2) const noexcept -> bool;

    constexpr auto slot2a(const EndSlot& s) const noexcept -> EgId;
    constexpr auto edge(const BulkSlot& s) noexcept -> Edge&;
    constexpr auto edge(EgId ind) noexcept -> Edge&;
    constexpr auto edge(const EgId ind) const noexcept -> const Edge&;
    constexpr auto chain(const EndSlot& s) const noexcept -> Chain;

    constexpr auto slot2ind(const EndSlot& s) const noexcept -> EgId;
    constexpr auto ind2bslot(EgId ind) const noexcept -> BulkSlot;

    constexpr auto edge_end(const EndSlot& s) const noexcept -> EndId;

    /// Updates internal data.
    /// Calls make_indma() and populate_component_vectors().
    void update_books() noexcept;

    /// Updates internal data for component c.
//    void update_adjacency_cmpt(CmpId c) noexcept;

    /// Updates internal data related to edge 'ind'.
    void update_adjacency_edges(EgId ind) noexcept;

    /// Updates internal data.
    void update_adjacency() noexcept;

    /// Updates internal vectors.
    /// Calls update_books() and update_adjacency().
    void update() noexcept;

    /// Initializes or updates glm and gla vectors.
    /// Sets 'glm' and 'gla':
    void make_indma() noexcept;

    /**
     * @brief Initializes or updates adjacency list.
     * @param a The adjacency list.
     */
    template<typename F>
    auto adjacency_list_edges(F(*f)(const Edge&)) const noexcept -> vec2<F>;

    auto adjacency_list_chains() const noexcept -> vec2<ChId>;

//    auto make_adjacency_list_old() const noexcept -> Base::adjLT;
//    auto make_adjacency_list_old(szt c) noexcept -> vec2<szt>;

    /**
     * @brief Number of vertices for the specific vertex degree.
     * @tparam I Vertex degree.
     */
    template<Degree D> requires is_implemented_degree<D>
    auto num_vertices() const noexcept -> szt;

    /// Total number of vertices.
    auto num_vertices() const noexcept -> szt;

    constexpr auto ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&;
    constexpr auto ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&;

    constexpr auto ng_inds_at(const EndSlot& s) const noexcept -> EgIds;

    constexpr auto edge_vertices(EgId ind) const noexcept ->
        std::array<vertices::Id, Ends::num>;

    constexpr auto edge_vertices(const BulkSlot& s) const noexcept ->
        std::array<vertices::Id, Ends::num>;

    auto inds_to_chain_link(
        EgId ind1,
        EgId ind2
    ) const noexcept -> std::array<EndSlot, 2>;

    /**
     * @brief Prints the graph chains using prefix \p tag.
     * @param tag Prefix.
     */
    void print_chains(const std::string& tag) const;

    /**
     * @brief Prints the graph components using prefix \p tag.
     * @param tag Prefix.
     */
    void print_components(const std::string& tag="") const;

    void print_component_chains(const std::string& tag="") const;

    void print_adjacency_lists(const std::string& tag="") const;

    /**
     * @brief Outputs the graph components to a text-formatted stream.
     * @param ofs Output stream.
     */
    void report(std::ostream& ofs) const;

    /**
     * @brief Exports graph components to a json-formatted file stream.
     * @param ofs Output file stream.
     */
    void to_json(std::ofstream& ofs) const;

    /**
     * @brief Updates graph chain indexes.
     * @details The indexes are updated so that the chain indexed as source
     * will acquire the identity of the target:
     * - connections of the source become connections of the target (end to end).
     * - the source edges are moved to the target.
     * - target acquires component index of the source.
     * Source now devoid of edges retains its neigs which are now identical to
     * those of the target.
     * @param f Source chain index.
     * @param t Target chain index.
     */
    void rename_chain(ChId f, ChId t);

    /**
     * @brief Copies connection partners to a new chain.
     * @details Copies assigning the connected slots of \p f to \p t and
     * updates neigs of \p f to become neigs of \p t . Previous neigs of \p t
     * are deleted; \p f neigs are not cleared.
     * @param f Source slot.
     * @param t Target slot.
     */
    void copy_neigs(
        const EndSlot& f,
        const EndSlot& t
    );

    /**
     * @brief Removes \p s from list of \a neigs in slots connected to it.
     */
    void remove_slot_from_neigs(const EndSlot& s);

    /**
     * @brief Replaces \p old with \p nov in slots connected to \p nov .
     * @details Slots connected to \p nov will be checked, and if there is
     * \p old, it will be replaced by \p nov .
     * @note Important: \p old retains list of slots formerly connected to it.
     */
    void replace_slot_in_neigs(
        const EndSlot& old,
        const EndSlot& nov
    );

    template<Degree D> requires (D == 0 || D == 3 || D == 4)
    auto zero_cycles_are_at_slot(const EndSlot& s) const noexcept -> bool;

    template<Degree D> requires (D == 0 || D == 3 || D == 4)
    auto one_cycle_chain_at_slot(const EndSlot& s) const noexcept -> ChId;

    auto two_cycles_are_at_slot(const EndSlot& s) const noexcept -> bool;

    constexpr auto is_same_cycle(
        const EndSlot& s1,
        const EndSlot& s2
    ) const noexcept -> bool;

    /**
     * @brief Checks if the graph is equilibrated.
     * @return True if equilibrated, false otherwise.
     */
    auto is_equilibrated() const noexcept -> bool;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Ch>
constexpr
Graph<Ch>::
Graph()
    : vertices {*this}
{}


template<typename Ch>
void Graph<Ch>::
generate_single_chain_components(
    const CmpId num,
    const EgId len
)
{
    const auto n {chain_num()};      // number of chains already present

    while (chain_num() < num + n)
        add_single_chain_component(len, chain_num());

    log_("Generated ", colorcodes::GREEN, num, colorcodes::RESET,
         " identical single-chain components, to ", edgenum, " edges total");
}


template<typename Ch>
void Graph<Ch>::
add_single_chain_component(
    const EgId len,
    const ChId idw
)
{
    cn.emplace_back(len,
                    is_defined(idw) ? idw : chain_num(),
                    edgenum);
    edgenum += len;

    ct.emplace_back(cn.back(), cmpt_num(), cn);

    update();
}


template<typename Ch>
void Graph<Ch>::
add_component(Chains&& mm)
{
    ct.emplace_back(cmpt_num(), cn);  // empty

    for (auto&& m : mm) {

        for (auto& g : m.g)
            g.ind = edgenum++;

        if (is_undefined(m.idw))
            m.set_w(chain_num());
        cn.push_back(std::move(m));

        ct.back().append(cn.back());
    }

    update();
}


template<typename Ch>
auto Graph<Ch>::
cut_component_at(const EndSlot& s) -> bool
{
    auto& cmp = ct[cn[s.w].c];

    // Find if the vertex split has resulted in a component split, and if so,
    // update the the disconnected component.

    paths::over_endslots::Generic<Cmpt> pp {cmp};

    // this is true iff there is a connection to s2 via a path outgoing from 's'
    // knownSize is true: enumertate over the cluster
    const auto is_cycle = pp.template are_connected<false>(s, s.opp());

    if constexpr (verboseF) {
        const auto ckl = is_cycle ? "" : "not ";
        log_("Component ", cmp.ind, " is ", ckl, "cycled over chain ", s.w);
    }

    if (!is_cycle)
        // create a new component from 's' neigs and beyond
        // (excluding s.w itself):

        split_component<false>(cmp, pp, s);

    return is_cycle;
}


template<typename Ch>
auto Graph<Ch>::
mark_branch(const EndSlot& source) -> bool
{
//    static_assert(knownSize);
    auto& cmp = ct[cn[source.w].c];

    PathsOverEndSlots pp {cmp};

    return pp.template are_connected<false>(source, source.opp());
}


template<typename Ch>
template<bool withSourceChain>
void Graph<Ch>::
split_component(
    Cmpt& cmp,
    const PathsOverEndSlots& pp,
    const EndSlot& s
)
{
    auto [accessible, blocked] =
        pp.template classify_chains_by_connectivity<withSourceChain>(s);

    if constexpr (verboseF)
        cmp.template print_classification<false>(accessible, blocked);

    const auto newc = cmpt_num();
    auto& newcmp = ct.emplace_back(newc, cn);

    newcmp.ww = accessible;
    for (EgId indc {}, j {}; const auto w: accessible)
        indc = cn[w].set_cmpt(newc, j++, indc);
    newcmp.set_gl();
    newcmp.set_chis();


    cmp.ww = blocked;
    for (EgId indc {}, j {}; const auto w: blocked)
        indc = cn[w].set_cmpt(cmp.ind, j++, indc);
    cmp.set_gl();
    cmp.set_chis();
}


template<typename Ch>
void Graph<Ch>::
split_component(Cmpt& cmp, ChIds&& rm)
{
    const auto newc = cmpt_num();
    auto& newcmp = ct.emplace_back(newc, cn);

    cmp.move_to(newcmp, std::move(rm));
}


template<typename Ch>
void Graph<Ch>::
merge_components(
    const CmpId acc,
    const CmpId don
) noexcept
{
    merge_components(ct[acc], ct[don]);
}


template<typename Ch>
void Graph<Ch>::
merge_components(
    Cmpt& acceptor,
    Cmpt& donor
) noexcept
{
    acceptor.ww.insert(acceptor.ww.end(), donor.ww.begin(), donor.ww.end());
    acceptor = Cmpt {acceptor.ww, acceptor.ind, cn};
    if (donor.ind != ind_last_cmpt()) {
        ct.back().set_ind(donor.ind);
        donor = std::move(ct.back());
    }
    ct.pop_back();
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ind_last_cmpt() const noexcept -> CmpId
{
    return ct.ind_last();
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ind_last_chain() const noexcept -> ChId
{
    return cn.ind_last();
}


template<typename Ch>
constexpr
auto Graph<Ch>::
vertices_are_adjacent_aa(
    const ChId w1,
    const EgId a1,
    const ChId w2,
    const EgId a2
) const noexcept -> bool
{
    ASSERT(a1, "ea or ee should be applied: a1 == 0");
    ASSERT(a1 < cn[w1].length(),
           "ea or ee should be applied: a1 == cn[w1].length() = ", a1);
    ASSERT(a2, "ea or ee should be applied: a2 == 0");
    ASSERT(a2 < cn[w2].length(),
           "ea or ee should be applied: a2 == cn[w2].length() = ", a2);

    if (w1 == w2) {
        return a1 == a2 ||
               a1 == a2 + 1 ||
               a2 == a1 + 1;
    }

    return false;
}


template<typename Ch>
constexpr
auto Graph<Ch>::
vertices_are_adjacent_ee(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept -> bool
{
    const auto w1 = s1.w;
    const auto e1 = s1.e;
    const auto w2 = s2.w;
    const auto e2 = s2.e;

    const auto& m1 = cn[w1];

    auto is_short = [](const auto& s) noexcept
    {
        return s.length() < 2;
    };

    if (w1 == w2)
        return e1 == e2 ||
               is_short(m1) ||
               m1.has_such_neig(Ends::A, w2);

    if (m1.c != cn[w2].c)
        return false;

    for (const auto& ng1 : m1.ngs[e1]()) {
        const auto nw1 = ng1.w;
        const auto ne1 = ng1.e;
        if (nw1 == w2 &&
            (ne1 == e2 ||
             (ne1 != e2 && is_short(cn[nw1]))))
            return true;
        if (nw1 != w2 && is_short(cn[nw1])) {
            const auto oe = Ch::opp_end(ne1);
            for (const auto& ngo : cn[nw1].ngs[oe]())
                if (ngo == s2)
                    return true;
        }
    }

    const auto oe1 = Ch::opp_end(e1);
    for (const auto& s : m1.ngs[oe1]())
        if (s == s2)
            return true;

    return false;
}


template<typename Ch>
constexpr
auto Graph<Ch>::
vertices_are_adjacent_ea(
    const EndSlot& s1,
    const ChId w2,
    const EgId a2
) const noexcept -> bool
{
    const auto [w1, e1] = s1.we();
    const auto& m1 = cn[w1];
    const auto& m2 = cn[w2];

    ASSERT(a2, "ea or ee should be applied: a2 == 0");
    ASSERT(a2 < m2.length(),
            "ea or ee should be applied: a2 == cn[w2].length() = ", a2);

    if (w1 == w2)
        return
            (e1 == Ends::A && a2 == 1) ||
            (e1 == Ends::B && a2 == m2.length() - 1) ||
            (m1.has_such_neig(Ends::A, w1) &&
             (a2 == 1 || a2 == m2.length() - 1));

    if (m1.c != m2.c)
        return false;

    if (a2 == 1 && m2.ngs[Ends::A].num())
        for (const auto& s : m2.ngs[Ends::A]())
            if (s == s1)
                return true;

    if (a2 == m2.length() - 1 && m2.ngs[Ends::B].num())
        for (const auto s : m2.ngs[Ends::B]())
            if (s == s1)
                return true;

    return false;
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge(const BulkSlot& s) noexcept -> Edge&
{
    return cn[s.w].g[s.a()];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge(const EgId ind) noexcept -> Edge&
{
    return cn[glm[ind]].g[gla[ind]];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge(const EgId ind) const noexcept -> const Edge&
{
    return cn[glm[ind]].g[gla[ind]];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
chain(const EndSlot& s) const noexcept -> Chain
{
    return cn[s.w];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
slot2a(const EndSlot& s) const noexcept -> EgId
{
    return cn[s.w].end2a(s.e);
}


template<typename Ch>
constexpr
auto Graph<Ch>::
slot2ind(const EndSlot& s) const noexcept -> EgId
{
    return cn[s.w].g[slot2a(s)].ind;
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ind2bslot(const EgId ind) const noexcept -> BulkSlot
{
    return BulkSlot{glm[ind], gla[ind]};
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge_end(const EndSlot& s) const noexcept -> EndId
{
    return cn[s.w].edge_end(s.e);
}


template<typename Ch>
void Graph<Ch>::
rename_chain(const ChId f, const ChId t)
{
    for (const auto e : Ends::Ids)
        copy_neigs(EndSlot{f, e}, EndSlot{t, e});

    cn[f].set_g_w(t);            // should be called before the move of g
    cn[t].g = std::move(cn[f].g);
    cn[t].c = cn[f].c;
    cn[t].idc = cn[f].idc;
    ct[cn[f].c].rename_chain(f, t);
}


template<typename Ch>
void Graph<Ch>::
copy_neigs(
    const EndSlot& f,
    const EndSlot& t
)
{
    ngs_at(t) = ngs_at(f);

   // Substitute f in f's neig's neigs for t:
    replace_slot_in_neigs(f, t);
}


template<typename Ch>
void Graph<Ch>::
remove_slot_from_neigs(const EndSlot& s)
{
    const auto sc = ngs_at(s);  // copy
    auto&      sr = ngs_at(s);  // ref

    for (auto& ne : sc()) {

        auto& ngs = ngs_at(ne);

        // Remove oldn from the neig list of its j-th neig
        const auto ngsOK = ngs.remove(s);

        ENSURE(ngsOK, "EndSlot {", s.w, " ", s.e, "} not found among",
                      "ngs of the slot {", ne.w, " ", ne.e, " connected to it");

        // Remove the j-th neig from the oldn's list of neigs
        const auto neOK = sr.remove(ne);

        ENSURE(neOK, "EndSlot {", ne.w, " ", ne.e, "} not found among",
                     "ngs of the slot {", s.w, " ", s.e, " connected to it");
    }
}


/// slots connected to 'nov' will be checked, and if there is 'old',
/// it will be replaced by 'nov'
template<typename Ch>
void Graph<Ch>::
replace_slot_in_neigs(
    const EndSlot& old,
    const EndSlot& nov
)
{
    const auto& nov_ngs = ngs_at(nov);

    for (auto& ne : nov_ngs()) {

        auto& ne_ngs = ngs_at(ne);

        const auto ok = ne_ngs.replace(old, nov);

        ENSURE(ok, "EndSlot {", old.w, " ", old.e, "} not found among",
                    "ngs of the slot {", ne.w, " ", ne.e, " connected to it");
    }
}


template<typename Ch>
void Graph<Ch>::
update_books() noexcept
{
    make_indma();
    chis.populate(cn);
    vertices.create();
//    std::cout << "num 0 " << vertices.template num<0>() << std::endl;
//    std::cout << "num 1 " << vertices.template num<1>() << std::endl;
//    std::cout << "num 2 " << vertices.template num<2>() << std::endl;
//    std::cout << "num 3 " << vertices.template num<3>() << std::endl;
//    std::cout << "num 4 " << vertices.template num<4>() << std::endl;
//    std::cout << "num all " << vertices.num() << std::endl;
//    const auto cmp0 = vertices.template for_compartment<1>(0);
//    vertices::print<1, Graph>(cmp0, "cmp0 deg 1: ");

//    vertices.print("on upd: ");
}


template<typename Ch>
void Graph<Ch>::
update_adjacency_edges(const EgId ind) noexcept
{
    const auto c = cn[glm[ind]].c;
    ct[c].adjacency_list_edges();
}


template<typename Ch>
void Graph<Ch>::
update_adjacency() noexcept
{
    for (auto& c: ct)
        c.adjacency_list_edges();
}


template<typename Ch>
void Graph<Ch>::
update() noexcept
{
    update_books();
    if constexpr (useAgl)
        update_adjacency();
}


template<typename Ch>
void Graph<Ch>::
make_indma() noexcept
{
//    for (auto& c: ct)
//        c.make_indma();

    glm.resize(edgenum);
    gla.resize(edgenum);
    for (const auto& m: cn)
        for (const auto& g: m.g) {
            glm[g.ind] = m.idw;
            gla[g.ind] = g.indw;
        }
}


template<typename Ch>
template<typename F>
auto Graph<Ch>::
adjacency_list_edges(F(*f)(const Edge&)) const noexcept -> vec2<F>
{
    vec2<F> x(edgenum);

    for (const auto& m: cn) {

        for (EgId a=0; a<m.length(); ++a) {

            const auto ind = m.g[a].ind;

            if (m.is_tail(a)) {
                for (const auto& s : m.ngs[Ends::A]())
                    // Connection backwards: only other chains might be found:
                    x[ind].push_back(f(edge(slot2ind(s))));

                if (m.length() == 1)
                    // Connection forwards: to other chain:
                    for (const auto& s : m.ngs[Ends::B]())
                        x[ind].push_back(f(edge(slot2ind(s))));

                else
                    // Connection forwards: to the same chain:
                    x[ind].push_back(f(m.g[a+1]));
            }
            else if (m.is_head(a)) {
                // but not  a1 == 0  =>  m.length() > 1
                // Connection backwards: to the same chain:
                x[ind].push_back(f(m.g[a-1]));
                // Connection forwards: to other chains:
                for (const auto& s : m.ngs[Ends::B]())
                    x[ind].push_back(f(edge(slot2ind(s))));
            }
            else {
                // Edge in the bulk: a > 0 && a < m.length() - 1:
                // Connection backwards: to the same chain:
                x[ind].push_back(f(m.g[a-1]));
                // Connection forwards: to the same chain:
                x[ind].push_back(f(m.g[a+1]));
            }
        }
    }

    return x;
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ngs_at(const EndSlot& s) noexcept -> Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ngs_at(const EndSlot& s) const noexcept -> const Chain::Neigs&
{
    return cn[s.w].ngs[s.e];
}


template<typename Ch>
constexpr
auto Graph<Ch>::
ng_inds_at(const EndSlot& s) const noexcept -> EgIds
{
    const auto& ngs = ngs_at(s);

    EgIds inds(ngs.num());

    for (szt i {}; i<ngs.num(); ++i)
        inds[i] = slot2ind(ngs[i]);

    return inds;
}


template<typename Ch>
template<Degree D> requires is_implemented_degree<D>
auto Graph<Ch>::
num_vertices() const noexcept -> szt
{
/*
    auto count_vertices = [&]() noexcept
    {
        szt k {};
        for (const auto& m: cn)
             k += m.template num_vertices<D>();

        return D == 3 ? k/3 :
               D == 4 ? k/4 : k;
    };

    return count_vertices();
*/
    return vertices.template num<D>();
}


template<typename Ch>
auto Graph<Ch>::
num_vertices() const noexcept -> szt
{
    return num_vertices<0>() +
           num_vertices<1>() +
           num_vertices<2>() +
           num_vertices<3>() +
           num_vertices<4>();
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge_vertices(const EgId ind) const noexcept -> std::array<vertices::Id, Ends::num>
{

    return edge_vertices(ind2bslot(ind));
}


template<typename Ch>
constexpr
auto Graph<Ch>::
edge_vertices(const BulkSlot& s) const noexcept -> std::array<vertices::Id, Ends::num>
{
    const auto [w, a] = s.we();

    const auto& m = cn[w];

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    if (!a) {
        auto i1 = vertices.from_end_slot(m.ngs[eA].num()+1, EndSlot{w, eA});
        auto i2 = m.length() == 1
                 ? vertices.from_end_slot(m.ngs[eB].num()+1, EndSlot{w, eB})
                 : vertices.from_bulk_slot(s);
        return {i1, i2};
    }
    else if (a == m.length() - 1) {
        const auto d = m.ngs[eB].num() + 1;
        return {vertices.from_bulk_slot(s),
                vertices.from_end_slot(d, EndSlot{w, eB})};
    }
    else
        return {vertices.from_bulk_slot(s),
                vertices.from_bulk_slot(BulkSlot{w, a + 1})};
}


template<typename Ch>
template<Degree D> requires (D == 0 || D == 3 || D == 4)
auto Graph<Ch>::
zero_cycles_are_at_slot(const EndSlot& s) const noexcept -> bool
{
    const auto& m = cn[s.w];

    const auto& ngs = m.ngs[s.e];

    if constexpr (D == 0)

        return !m.is_disconnected_cycle();

    if constexpr (D == 3) {

        ASSERT(ngs.num() == D - 1,
               "num connections ", ngs.num(), " is different from D-1", D - 1);

        const auto& n = cn[ngs[0].w];
        const auto& q = cn[ngs[1].w];

        return !m.is_connected_cycle() &&
               !n.is_connected_cycle() &&
               !q.is_connected_cycle();
    }
    if constexpr (D == 4) {

        ASSERT(ngs.num() == D - 1,
               "num connections ", ngs.num(), " is different from D-1", D - 1);

        const auto& n = cn[ngs[0].w];
        const auto& q = cn[ngs[1].w];
        const auto& p = cn[ngs[2].w];

        return !m.is_connected_cycle() &&
               !n.is_connected_cycle() &&
               !q.is_connected_cycle() &&
               !p.is_connected_cycle();
    }

    return false;
}


template<typename Ch>
template<Degree D> requires (D == 0 || D == 3 || D == 4)
auto Graph<Ch>::
one_cycle_chain_at_slot(const EndSlot& s) const noexcept -> ChId
{
    const auto& m = cn[s.w];

    const auto& ngs = m.ngs[s.e];

    if constexpr (D == 0)

        if (m.is_disconnected_cycle())
            return s.w;

    if constexpr (D == 3) {

        ASSERT(ngs.num() == D - 1,
               "num connections ", ngs.num(), " is different from D-1", D - 1);

        const auto& n = cn[ngs[0].w];
        const auto& q = cn[ngs[1].w];

        const auto mc = m.is_connected_cycle();
        const auto nc = n.is_connected_cycle();
        const auto qc = q.is_connected_cycle();

        if ( mc &&  nc && !qc) return s.w;       // same as ngs[0].w
        if ( mc && !nc &&  qc) return s.w;       // same as ngs[1].w
        if (!mc &&  nc &&  qc) return ngs[0].w;  // same as ngs[1].w
    }
    if constexpr (D == 4) {

        ASSERT(ngs.num() == D - 1,
               "num connections ", ngs.num(), " is different from D-1", D - 1);

        const auto& n = cn[ngs[0].w];
        const auto& q = cn[ngs[1].w];
        const auto& p = cn[ngs[2].w];

        const auto mc = m.is_connected_cycle();
        const auto nc = n.is_connected_cycle();
        const auto qc = q.is_connected_cycle();
        const auto pc = p.is_connected_cycle();

        if ( mc &&  nc && !qc && !pc) return s.w;       // same as ngs[0].w;
        if ( mc && !nc &&  qc && !pc) return s.w;       // same as ngs[1].w;
        if (!mc &&  nc &&  qc && !pc) return ngs[0].w;  // same as ngs[1].w;
        if ( mc && !nc && !qc &&  pc) return s.w;       // same as ngs[2].w;
        if (!mc &&  nc && !qc &&  pc) return ngs[0].w;  // same as ngs[2].w;
        if (!mc && !nc &&  qc &&  pc) return ngs[1].w;  // same as ngs[2].w;
    }

    return undefined<ChId>;
}


template<typename Ch>
auto Graph<Ch>::
two_cycles_are_at_slot(const EndSlot& s) const noexcept -> bool
{
    const auto& m = cn[s.w];

    const auto& ngs = m.ngs[s.e];

    ASSERT(ngs.num() == 3,
           "num connections ", ngs.num(), " is different from 3");

    const auto& n = cn[ngs[0].w];
    const auto& q = cn[ngs[1].w];
    const auto& p = cn[ngs[2].w];

    return m.is_connected_cycle() &&
           n.is_connected_cycle() &&
           q.is_connected_cycle() &&
           p.is_connected_cycle();
}


template<typename Ch>
constexpr
auto Graph<Ch>::
is_same_cycle(
    const EndSlot& s1,
    const EndSlot& s2
) const noexcept -> bool
{
    return cn[s1.w].is_cycle() && s1 == s2.opp();
}


template<typename Ch>
auto Graph<Ch>::
inds_to_chain_link(
    const EgId ind1,
    const EgId ind2
) const noexcept -> std::array<EndSlot, 2>
{
    const auto w = glm[ind1];

    for (const auto e : Ends::Ids) {
        if (cn[w].end2ind(e) == ind1) {
            auto s1 = EndSlot{w, e};
            for (const auto& s2: cn[w].ngs[e]())
                if (cn[s2.w].end2ind(s2.e) == ind2)
                    for (const auto& n2: cn[s2.w].ngs[s2.e]())
                        if (n2 == s1)
                            return {s1, s2};

        }
    }
    return {{}};
}


template<typename Ch>
auto Graph<Ch>::
is_equilibrated() const noexcept -> bool
{
    return true;  // stub
}


template<typename Ch>
void Graph<Ch>::
print_chains(const std::string& tag) const
{
    cn.print(tag);
}


template<typename Ch>
void Graph<Ch>::
print_components(const std::string& tag) const
{
    log_("");
    for (const auto& c : ct)
        c.print(tag);

    if (!ct.size())
        log_("No components: the graph is empty!");
}


template<typename Ch>
void Graph<Ch>::
print_component_chains(const std::string& tag) const
{
    log_("");
    for (const auto& c : ct) {
        c.print_chains(tag);
        log_("");
    }

    if (!ct.size())
        log_("No components: the graph is empty!");
}



template<typename Ch>
void Graph<Ch>::
print_adjacency_lists(const std::string& tag) const
{
    for (const auto& c : ct)
        c.print_adjacency_list_edges(tag);
}


template<typename Ch>
void Graph<Ch>::
report(std::ostream& ofs) const
{
    vertices.report(ofs);
    chis.report(ofs);
    ofs << " egn " << edgenum
        << " cnn " << chain_num()
        << " ctn " << cmpt_num();
}


template<typename Ch>
void Graph<Ch>::
to_json(std::ofstream& ofs) const
{
    ofs << "{\n\"vertices\": [\n";
    vertices.to_json(ofs);
    ofs << "],\n\"links\": [\n";

    for (EgId i=0; i<edgenum; ++i) {

        const auto link = edge_vertices(i);

        ASSERT(is_defined(link[0]),
               "Vertex 0 for edge ind ", i, ", w ", glm[i],", a ", gla[i],
               " is undefined");
        ASSERT(is_defined(link[1]),
               "Vertex 1 for edge ind ", i, ", w ", glm[i],", a ", gla[i],
               " is undefined");

        ofs << "{"
            << "\"source_id\": " << link[0] << ", "
            << "\"target_id\": " << link[1]
            << "}" << (i < edgenum - 1 ? "," : "") << '\n';
    }
    ofs << "]\n}\n";
}


using GraphBase = Graph<ChainBase>;


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_GRAPH_H
