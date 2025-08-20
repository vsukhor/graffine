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
 * @file collections.h
 * @brief Contains functor classes holding and updating vertex collections.
 * @details Due to its edge-centric graph implementation, graph_mutator does not keep
 * instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data. The Updaters are
 * defined for each if vertex degrees.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_VERTICES_COLLECTIONS_H
#define GRAPH_MUTATOR_STRUCTURE_VERTICES_COLLECTIONS_H

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "../../definitions.h"
#include "degrees.h"
#include "vertex.h"

namespace graph_mutator::structure::vertices {

/**
 * @brief Template for classes updating degree-specific vertex collections.
 * @tparam D Vertex degree.
 * @tparam G Graph containing the vertices.
 */
template<Degree D,
         typename G> requires is_implemented_degree<D>
class Collection {};


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Functor class updating vertex collection for vertices of degree 2.
 * @details Partial template specialization for vertex container updater
 * operating on vertices of degree 2.
 * @tparam G Graph class supplying the vertices.
 */
template<typename G>
struct Collection<0, G> {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using S = Chain::EndSlot;  ///< Slot type for the edge ends.
    using V = Vertex<0, S>;
    using Container = std::vector<V>;

    const Graph& gr;  ///< Reference to the graph object.

    Container vv;  ///< Boundary vertices of disconnected cycles.

    /// Constructs an Updater from the Graph class instance.
    explicit constexpr Collection(const Graph& gr)
        : gr {gr}
    {};

    constexpr auto num() const noexcept -> szt
    {
        return vv.size();
    }

    constexpr void report(std::ostream& ofs) const noexcept
    {
        ofs << num() << " ";
    }

    /**
     * @brief Populates the vector of vertices of degree D.
     * @param indini [in, out] Inicial vertex index in the whole_graph count.
     * @param c Compartment index. If undefined, all compartments are included.
     * @return The vector of vertices of degree D.
     */
    auto populate(
        Id& indini,
        CmpId c=undefined<CmpId>
    ) noexcept -> Container;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
auto Collection<0, G>::
populate(
    Id& index,
    const CmpId c
) noexcept -> Container
{
    vv.clear();

    for (const auto w: gr.chis.cn22)
        if (is_undefined(c) ||
            (is_defined(c) && gr.cn[w].c == c))
            vv.emplace_back(index++, typename V::ArS{S{w, Ends::A}, S{w, Ends::B}});

    return vv;
}


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Functor class updating vertex collection for vertices of degree 1.
 * @details Partial template specialization for vertex container updater
 * operating on vertices of degree 1.
 * @tparam G Graph class supplying the vertices.
 */
template<typename G>
struct Collection<1, G> {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using S = Chain::EndSlot;  ///< Slot type for the edge ends.
    using V = Vertex<1, S>;
    using Container = std::vector<V>;

    const Graph& gr;  ///< Reference to the graph object.

    Container vv;   ///< Vertices of degree 1.

    /// Constructs an Updater from the Graph class instance.
    explicit constexpr Collection(const Graph& gr)
        : gr {gr}
    {};

    constexpr auto num() const noexcept -> szt
    {
        return vv.size();
    }

    constexpr void report(std::ostream& ofs) const noexcept
    {
        ofs << num() << " ";
    }

    /**
     * @brief Populates the vector of vertices of degree D.
     * @param indini [in, out] Inicial vertex index in the whole_graph count.
     * @param c Compartment index. If undefined, all compartments are included.
     * @return The vector of vertices of degree D.
     */
    auto populate(
        Id& indini,
        CmpId c=undefined<CmpId>
    ) noexcept -> Container;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
auto Collection<1, G>::
populate(
    Id& indini,
    const CmpId c
) noexcept -> Container
{
    vv.clear();

    // '11': both ends of the unconnected segments represent distinct vertices
    for (const auto w : gr.chis.cn11)
        if (is_undefined(c) ||
            (is_defined(c) && gr.cn[w].c == c))
            for (const auto e: Ends::Ids)
                vv.emplace_back(indini++, typename V::ArS{S {w, e}});

    // segments '13'
    for (const auto& we: is_defined(c)
                        ? gr.ct[c].chis.cn13
                        : gr.chis.cn13)
            vv.emplace_back(indini++, typename V::ArS{we});

    // segments '14'
    for (const auto& we: is_defined(c)
                        ? gr.ct[c].chis.cn14
                        : gr.chis.cn14)
            vv.emplace_back(indini++, typename V::ArS{we});

    return vv;
}


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Functor class updating vertex collection for vertices of degree 2.
 * @details Partial template specialization for vertex container updater
 * operating on vertices of degree 2.
 * @tparam G Graph class supplying the vertices.
 */
template<typename G>
struct Collection<2, G> {

    using Graph = G;
    using Chain = Graph::Chain;
    using S = Chain::BulkSlot;  ///< Slot type for the edge ends.
    using V = Vertex<2, S>;
    using Container = std::vector<V>;

    const Graph& gr;  ///< Reference to the graph object.

    Container vv;   ///< Vertices of degree 2.

    /// Constructs an Updater from the Graph class instance.
    explicit constexpr Collection(const Graph& gr)
        : gr {gr}
    {};

    constexpr auto num() const noexcept -> szt
    {
        return vv.size();
    }

    constexpr void report(std::ostream& ofs) const noexcept
    {
        ofs << num() << " ";
    }

    /**
     * @brief Populates the vector of vertices of degree D.
     * @param indini [in, out] Inicial vertex index in the whole_graph count.
     * @param c Compartment index. If undefined, all compartments are included.
     * @return The vector of vertices of degree D.
     */
    auto populate(
        Id& indini,
        CmpId c=undefined<CmpId>
    ) noexcept -> Container;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
auto Collection<2, G>::
populate(
    Id& index,
    const CmpId c
) noexcept -> Container
{
    vv.clear();

    const auto allcn = [&]()
    {
        std::vector<ChId> u(gr.chain_num());
        std::iota(u.begin(), u.end(), 0);
        return u;
    };

    for (const auto w : is_undefined(c) ? allcn()
                                        : gr.ct[c].ww)
        for (EgId a=1; a<gr.cn[w].length(); ++a)
            vv.emplace_back(index++, typename V::ArS{S {w, a-1}, S {w, a}});

    return vv;
}


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Functor class updating vertex collection for vertices of degree 3.
 * @details Partial template specialization for vertex container updater
 * operating on vertices of degree 3.
 * @tparam G Graph class supplying the vertices.
 */
template<typename G>
struct Collection<3, G> {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using S = Chain::EndSlot;  ///< Slot type for the edge ends.
    using V = Vertex<3, S>;
    using Container = std::vector<V>;

    const Graph& gr;  ///< Reference to the graph object.

    Container vv;   ///< Vertices of degree 3.

    /// Constructs an Updater from the Graph class instance.
    explicit constexpr Collection(const Graph& gr)
        : gr {gr}
    {};

    constexpr auto num() const noexcept -> szt
    {
        return vv.size();
    }

    constexpr void report(std::ostream& ofs) const noexcept
    {
        ofs << num() << " ";
    }

    /**
     * @brief Populates the vector of vertices of degree D.
     * @param indini [in, out] Inicial vertex index in the whole_graph count.
     * @param c Compartment index. If undefined, all compartments are included.
     * @return The vector of vertices of degree D.
     */
    auto populate(
        Id& indini,
        CmpId c=undefined<CmpId>
    ) noexcept -> Container;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
auto Collection<3, G>::
populate(
    Id& index,
    const CmpId c
) noexcept -> Container
{
    vv.clear();

    auto attempt_new_vertex = [&](const S& s)
    {
        const auto& ng = gr.ngs_at(s);

        V vertex {index, typename V::ArS{s, ng[0], ng[1]}};

        if (!std::any_of(vv.begin(),
                         vv.end(),
                         [&](const auto& nup) { return nup == vertex; })) {
            vv.push_back(std::move(vertex));
            ++index;
        }
    };

    for (const auto w : is_defined(c) ? gr.ct[c].chis.cn33
                                      : gr.chis.cn33)
        for (const auto e: Ends::Ids)
            attempt_new_vertex(S{w, e});

    for (const auto& s: is_defined(c) ? gr.ct[c].chis.cn13
                                      : gr.chis.cn13)
        attempt_new_vertex(s.opp());

    for (const auto& s: is_defined(c) ? gr.ct[c].chis.cn34
                                      : gr.chis.cn34)
        attempt_new_vertex(s);

    return vv;
}


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Functor class updating vertex collection for vertices of degree 4.
 * @details Partial template specialization for vertex container updater
 * operating on vertices of degree 4.
 * @tparam G Graph class supplying the vertices.
 */
template<typename G>
struct Collection<4, G> {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using S = Chain::EndSlot;  ///< Slot type for the edge ends.
    using V = Vertex<4, S>;
    using Container = std::vector<V>;

    const Graph& gr;  ///< Reference to the graph object.

    Container vv;   ///< Vertices of degree 4.

    /// Constructs an Updater from the Graph class instance.
    explicit constexpr Collection(const Graph& gr)
        : gr {gr}
    {};

    constexpr auto num() const noexcept -> szt
    {
        return vv.size();
    }

    constexpr void report(std::ostream& ofs) const noexcept
    {
        ofs << num() << " ";
    }

    /**
     * @brief Populates the vector of vertices of degree D.
     * @param indini [in, out] Inicial vertex index in the whole_graph count.
     * @param c Compartment index. If undefined, all compartments are included.
     * @return The vector of vertices of degree D.
     */
    auto populate(
        Id& indini,
        CmpId c=undefined<CmpId>
    ) noexcept -> Container;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
auto Collection<4, G>::
populate(
    Id& index,
    const CmpId c
) noexcept -> Container
{
    vv.clear();

    auto attempt_new_vertex = [&](const S& s)
    {
        const auto& ng = gr.ngs_at(s);

        V vertex {index, typename V::ArS{s, ng[0], ng[1], ng[2]}};

        if (!std::any_of(vv.begin(),
                         vv.end(),
                         [&](const auto& nup) {return nup == vertex;})) {
            vv.push_back(std::move(vertex));
            ++index;
        }
    };

    for (const auto w: is_defined(c) ? gr.ct[c].chis.cn44
                                     : gr.chis.cn44)
        for (const auto e: Ends::Ids)
            attempt_new_vertex(S{w, e});

    for (const auto& s: is_defined(c) ? gr.ct[c].chis.cn14
                                      : gr.chis.cn14)
        attempt_new_vertex(s.opp());

    for (const auto& s: is_defined(c) ? gr.ct[c].chis.cn34
                                      : gr.chis.cn34)
        attempt_new_vertex(s.opp());

    return vv;
}


////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Returns a collection of vertices in a given compartment.
 * @details This function filters the vertices in the provided collection
 * based on the specified compartment id. It returns a new collection containing
 * only those vertices that belong to the specified compartment.
 * @note The function is designed to work with vertex collections of any degree
 * up to the maximum degree defined in the graph_mutator library.
 * It uses the `Vertex::cmpt` method to check the compartment of each vertex.
 * The function is a template that can be instantiated for different vertex degrees
 * and graph types, allowing for flexibility in handling various graph structures.
 * @tparam D Vertex degree.
 * @tparam G Graph class supplying the vertices.
 * @param c Compartment id.
 * @param ns Collection of vertices to filter.
 * @return Collection of vertices in the specified compartment.
 */
template<Degree D,
         typename G> requires is_implemented_degree<D>
auto for_compartment(
    const CmpId c,
    const Collection<D, G>& ns
) noexcept -> Collection<D, G>
{
    Collection<D, G> vs {ns.gr};

    for (const auto& v: ns.vv)
        if (Vertex<D, typename Collection<D, G>::S>::cmpt(v, vs.gr) == c)
            vs.vv.push_back(v);

    return vs;
}

/**
 * @brief Finds a vertex in the collection by its slot.
 * @tparam D Vertex degree.
 * @tparam G Graph class supplying the vertices.
 * @param s Slot of the vertex to find.
 * @param ns Collection of vertices to search.
 * @return Index of the found vertex or an undefined Id.
 */
template<Degree D,
         typename G> requires is_implemented_degree<D>
auto find_vertex(
    const typename Collection<D, G>::S& s,
    const Collection<D, G>& ns
) noexcept -> Id
{

    for (const auto& v: ns.vv)
        if (v.contains(s))
            return v.ind;

    return undefined<Id>;
}

/**
 * @brief Prints the vertices in the collection.
 * @tparam D Vertex degree.
 * @tparam G Graph class supplying the vertices.
 * @param ns Collection of vertices to print.
 * @param s Optional string prefix for each vertex printout.
 */
template<Degree D,
         typename G> requires is_implemented_degree<D>
void print(const Collection<D, G>& ns,
           const std::string& s="") noexcept
{
    for (const auto& v: ns.vv)
        v.print(s);
}


}  // namespace graph_mutator::structure::vertices

#endif  // GRAPH_MUTATOR_STRUCTURE_VERTICES_COLLECTIONS_H
