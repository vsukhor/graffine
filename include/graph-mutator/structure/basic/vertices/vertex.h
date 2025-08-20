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
 * @file vertex.h
 * @brief Contains implementation of various vertex types and their containers.
 * @details Due to its edge-centric graph implementation, graph_mutator does not keep
 * instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_VERTICES_VERTEX_H
#define GRAPH_MUTATOR_STRUCTURE_VERTICES_VERTEX_H

#include <algorithm>
#include <array>
#include <memory>
#include <ostream>
#include <vector>

#include "../../definitions.h"
#include "degrees.h"


namespace graph_mutator::structure::vertices {


using Id = szt;


/**
 * @brief Class template encapsulating a graph vertex of specific degree.
 * @details Due to its edge-centric graph implementation, graph_mutator does not
 *  keep instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data.
 * @tparam D_ Vertex degree.
 */
template<Degree D_,
         typename S>
struct Vertex {

    /// Vertex degree.
    static constexpr auto D = D_;

    /// Number of edges incident to the vertex.
    static constexpr unsigned int numEdges {D == Deg0 ? 2 : D};

    /// Type of the slot incident to the vertex.
    static constexpr auto isBulk = is_bulk_degree<D>;

    static_assert(is_implemented_degree<D>);

//    using S = Slot<D>;
    using ArS = std::array<S, numEdges>;
    using EgdeInds = std::array<EgId, numEdges>;

    Id ind;   ///< Vertex graph-wide index.

    /// Chain indexes and ends of incident chains.
    ArS ars;

    /**
     * @brief Constructor.
     * @param ind Vertex graph-wide index.
     * @param ars Array of slots: incident chain indexes and in chain positions.
     */
    explicit constexpr Vertex (
        Id ind,
        const ArS& ars={}
    );

    /// Equality operator.
    constexpr auto operator==(const Vertex& p) const noexcept -> bool;

    template<typename G>
    static constexpr auto inds(const Vertex& v,
                               const G& gr) noexcept -> const EgdeInds;

    template<typename G>
    static constexpr auto cmpt(const Vertex& v,
                               const G& gr) noexcept -> CmpId;


    constexpr auto contains(const S& s) const noexcept -> bool;

    /// Prints attributes to an output message processor 'os'.
    void print(const std::string& s="") const;

    /**
     * @brief Exports the vertex instance to a json-formatted file.
     * @param ofs std::ofstream to write to.
     * @param numtotal Total number of vertices exported.
     */
    void to_json(std::ofstream& ofs, szt numtotal) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D_,
         typename S>
constexpr
Vertex<D_, S>::
Vertex(
    const Id ind,
    const ArS& ars
)
    : ind {ind}
    , ars {ars}
{}


template<Degree D_,
         typename S>
constexpr
auto Vertex<D_, S>::
operator==(const Vertex& p) const noexcept -> bool
{
    if constexpr (D == 1)
        return ars == p.ars;

    if constexpr (D == 0 || D == 2)
        return ars == p.ars ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[0]);

    if constexpr (D == 3)
        return ars == p.ars ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[2] && ars[2] == p.ars[0]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[0] && ars[2] == p.ars[1]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[0] && ars[2] == p.ars[2]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[2] && ars[2] == p.ars[1]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[1] && ars[2] == p.ars[0]);

    if constexpr (D == 4) {
        return
        (ars[0] == p.ars[0] && ars[1] == p.ars[1] && ars[2] == p.ars[2] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[1] && ars[2] == p.ars[3] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[2] && ars[2] == p.ars[1] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[2] && ars[2] == p.ars[3] && ars[3] == p.ars[1]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[3] && ars[2] == p.ars[1] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[0] && ars[1] == p.ars[3] && ars[2] == p.ars[2] && ars[3] == p.ars[1]) ||

        (ars[0] == p.ars[1] && ars[1] == p.ars[0] && ars[2] == p.ars[2] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[0] && ars[2] == p.ars[3] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[2] && ars[2] == p.ars[0] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[2] && ars[2] == p.ars[3] && ars[3] == p.ars[0]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[3] && ars[2] == p.ars[0] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[1] && ars[1] == p.ars[3] && ars[2] == p.ars[2] && ars[3] == p.ars[0]) ||

        (ars[0] == p.ars[2] && ars[1] == p.ars[0] && ars[2] == p.ars[1] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[0] && ars[2] == p.ars[3] && ars[3] == p.ars[1]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[1] && ars[2] == p.ars[0] && ars[3] == p.ars[3]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[1] && ars[2] == p.ars[3] && ars[3] == p.ars[0]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[3] && ars[2] == p.ars[0] && ars[3] == p.ars[1]) ||
        (ars[0] == p.ars[2] && ars[1] == p.ars[3] && ars[2] == p.ars[1] && ars[3] == p.ars[0]) ||

        (ars[0] == p.ars[3] && ars[1] == p.ars[0] && ars[2] == p.ars[1] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[3] && ars[1] == p.ars[0] && ars[2] == p.ars[2] && ars[3] == p.ars[1]) ||
        (ars[0] == p.ars[3] && ars[1] == p.ars[1] && ars[2] == p.ars[0] && ars[3] == p.ars[2]) ||
        (ars[0] == p.ars[3] && ars[1] == p.ars[1] && ars[2] == p.ars[2] && ars[3] == p.ars[0]) ||
        (ars[0] == p.ars[3] && ars[1] == p.ars[2] && ars[2] == p.ars[0] && ars[3] == p.ars[1]) ||
        (ars[0] == p.ars[3] && ars[1] == p.ars[2] && ars[2] == p.ars[1] && ars[3] == p.ars[0]);
    }

    std::unreachable();
    return false;
}


template<Degree D_,
         typename S>
void Vertex<D_, S>::
print(const std::string& s) const
{
    log_<false>(s, "Vertex d ", D, " ind: ", ind, " ars: ");
    for (const auto& s: ars)
        s.print();
    log_("");
}


template<Degree D_,
         typename S>
void Vertex<D_, S>::
to_json(std::ofstream& ofs,
        const szt numtotal) const
{
    ofs << "{"
        << "\"d\": " << D << ", "
        << "\"ind\": " << ind << ", ";
    ofs << "\"w\": [";
    for (szt i{}; i<D; ++i)
        ofs << ars[i].w << (i < D-1 ? ", " : "], ");
    ofs << "\"e\": [";
    for (szt i{}; i<D; ++i)
        ofs << ars[i].e << (i < D-1 ? ", " : "]");
    ofs << "}" << (ind < numtotal - 1 ? "," : "") << std::endl;
}


template<Degree D_,
         typename S>
template<typename G>
constexpr
auto Vertex<D_, S>::
inds(const Vertex& v,
     const G& gr) noexcept -> const EgdeInds  // static
{
    EgdeInds egs;
    for (EgId i{}; i<numEdges; ++i)
        egs[i] = gr.slot2ind(v.ars[i]);

    return egs;
}

template<Degree D_,
         typename S>
template<typename G>
constexpr
auto Vertex<D_, S>::
cmpt(const Vertex& v,
     const G& gr) noexcept -> CmpId  // static
{
    return gr.cn[v.ars[0].w].c;
}

template<Degree D_,
         typename S>
constexpr
auto Vertex<D_, S>::
contains(const S& s) const noexcept -> bool
{
    return std::find(ars.begin(), ars.end(), s) != ars.end();
}


}  // namespace graph_mutator::structure::vertices

#endif  // GRAPH_MUTATOR_STRUCTURE_VERTICES_VERTEX_H
