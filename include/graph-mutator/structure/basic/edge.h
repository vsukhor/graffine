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
 * @file edge.h
 * @brief The graph graph_mutator::Edge class template.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_EDGE_H
#define GRAPH_MUTATOR_STRUCTURE_EDGE_H

#include <array>
#include <fstream>
#include <ostream>

#include "../definitions.h"
#include "ends.h"
#include "vertices/degrees.h"


namespace graph_mutator::structure {

template<typename> class Chain;

/**
 * @brief The Graph Edge class.
 * @details Edge is a minimal structural unit of the graph.
 * The class implements the tasks and properties specific to a single edge
 * and its relation to other components.
 * @tparam D Max vertex degree the edge is desined to operate with.
 */
template<Degree D>
struct Edge {

    static_assert(is_implemented_degree<D>);

    static constexpr auto maxVertexDegree = maxDegree;

    using Id = EgId;
    using Ends = structure::Ends;  ///< Edge ends.
    using EndId = Ends::Id;
    using weight_t = real;    ///< Type alias for edge weight.

    friend Chain<Edge>;

    /// Maximal edge weight allowed.
    static constexpr auto maxWeight = huge<weight_t>;

    Id ind {undefined<Id>};         ///< Id graph-wide.
    Id indc {undefined<Id>};        ///< Id component-wide.
    Id indw {undefined<Id>};        ///< Id chain-wide.

    ChId w {undefined<ChId>};       ///< Currend chain id.
    CmpId c {undefined<CmpId>};     ///< Current component id.

    weight_t weight {one<weight_t>};    ///< Weight of the edge.

    /**
     * @brief Constructor with both chain and component specs not set.
     * @param ind Index graph-wide.
     */
    explicit constexpr Edge(
        Id ind
    );

    /**
     * @brief Constructor of a chain-embedded edge without component specs.
     * @param ind Index graph-wide.
     * @param indw Index in the host chain.
     * @param w Index of the host chain.
     */
    explicit constexpr Edge(
        Id ind,
        Id indw,
        ChId w
    );

    /**
     * @brief Full constructor.
     * @param ind Index graph-wide.
     * @param indc Index component-wide.
     * @param indw Index in the host chain.
     * @param w Index of the host chain.
     * @param c Index of the host component.
     */
    explicit constexpr Edge(
        Id ind,
        Id indc,
        Id indw,
        ChId w,
        CmpId c
    );

    /**
     * @brief Import constructor.
     * @param ifs Input file stream supplying edge attributes.
     */
    explicit Edge(std::ifstream& ifs);

    constexpr auto operator==(const Edge& e) const noexcept -> bool;

    /// Swap the edge ends.
    void reverse();

    /// Get edge orientation.
    constexpr auto orientation() const noexcept -> Orientation;
    constexpr auto points_forwards() const noexcept -> bool;

    constexpr auto oriented_end(Ends::Id side_in_chain) const noexcept -> Ends::Id;

    /// Return index of the chain end opposite to the end 'e'.
    static constexpr auto opp_end(EndId e) noexcept -> EndId;

    template<EndId E> requires End<E>
    static consteval auto opp_end() noexcept -> EndId;

    constexpr void set_cmp(CmpId cc, Id indcc) noexcept;

    /**
     * @brief Read the edge from a binary file.
     * @param ofs Output file stream.
     */
    void read(std::ifstream& ofs);

    /**
     * @brief Write the edge to a binary file.
     * @param ofs Output file stream.
     */
    void write(std::ofstream& ofs) const;

    /**
     * @brief Print the edge to a stream.
     * @tparam ENDL Flag to end current line.
     * @param tag Prefix preceding the output.
     * @param a Position inside chain.
     * @param dir Char indicating orientation of this edge relative the
     *            direction of the host chain.
    */
    template<bool ENDL=true>
    void print(const std::string& tag,
               char dir) const;

    /**
     * @brief Ensures that the edge indexes are consistant within the graph.
     * @param nInd Total number of edges in the graph.
     * @param nIndC Number of edges in the component.
     * @param nIndW Number of edges in the chain.
     * @param nCh Number of chains in the graph.
     * @param nCmpt Number of disconnected components in the graph.
    */
    void check(Id nInd,
               Id nIndC,
               Id nIndW,
               ChId nCh,
               CmpId nCmpt) const;

private:

    std::array<Ends::Id, Ends::num> dir {Ends::Ids};
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Degree D>
constexpr
Edge<D>::
Edge(const Id ind)
    : ind {ind}
{}


template<Degree D>
constexpr
Edge<D>::
Edge(
    const Id ind,
    const Id indw,
    const ChId w
)
    : ind {ind}
    , indw {indw}
    , w {w}
{}


template<Degree D>
constexpr
Edge<D>::
Edge(
    const Id ind,
    const Id indc,
    const Id indw,
    const ChId w,
    const CmpId c
)
    : ind {ind}
    , indc {indc}
    , indw {indw}
    , w {w}
    , c {c}
{}


template<Degree D>
Edge<D>::
Edge(std::ifstream& ifs)
{
    read(ifs);
}


template<Degree D>
constexpr
auto Edge<D>::
operator==(const Edge& other) const noexcept -> bool
{
    return ind == other.ind &&
           indc == other.indc &&
           indw == other.indw &&
           c == other.c &&
           w == other.w;
}


template<Degree D>
void Edge<D>::
reverse()
{
    std::swap(dir[Ends::A], dir[Ends::B]);
}


template<Degree D>
constexpr
auto Edge<D>::
orientation() const noexcept -> Orientation
{
    return points_forwards() ? Orientation::Forwards
                             : Orientation::Backwards;
}


template<Degree D>
constexpr
auto Edge<D>::
points_forwards() const noexcept -> bool
{

    return dir[Ends::A] < dir[Ends::B];
}


template<Degree D>
constexpr
auto Edge<D>::
oriented_end(Ends::Id side_in_chain) const noexcept -> Ends::Id
{
    return points_forwards() ? side_in_chain
                             : Ends::opp(side_in_chain);
}


template<Degree D>
template<Ends::Id E> requires End<E>
consteval
auto Edge<D>::
opp_end() noexcept -> EndId   // static
{
    return Ends::opp<E>();
}

template<Degree D>
constexpr
auto Edge<D>::
opp_end(const EndId e) noexcept -> EndId   // static
{
    return Ends::opp(e);
}


template<Degree D>
constexpr
void Edge<D>::
set_cmp(const CmpId cc,
        const Id ic) noexcept
{
    c = cc;
    indc = ic;
}


template<Degree D>
void Edge<D>::
read(std::ifstream& ifs)
{
    ifs.read(reinterpret_cast<char*>(&ind), sizeof(ind));
    ifs.read(reinterpret_cast<char*>(&indc), sizeof(indc));
    ifs.read(reinterpret_cast<char*>(&indw), sizeof(indw));
    ifs.read(reinterpret_cast<char*>(&w), sizeof(w));
    ifs.read(reinterpret_cast<char*>(&c), sizeof(c));
    ifs.read(reinterpret_cast<char*>(&dir[Ends::A]), sizeof(Ends::Id));
    ifs.read(reinterpret_cast<char*>(&dir[Ends::B]), sizeof(Ends::Id));
    ifs.read(reinterpret_cast<char*>(&weight), sizeof(weight));
}


template<Degree D>
void Edge<D>::
write(std::ofstream& ofs) const
{
    ofs.write(reinterpret_cast<const char*>(&ind), sizeof(ind));
    ofs.write(reinterpret_cast<const char*>(&indc), sizeof(indc));
    ofs.write(reinterpret_cast<const char*>(&indw), sizeof(indw));
    ofs.write(reinterpret_cast<const char*>(&w), sizeof(w));
    ofs.write(reinterpret_cast<const char*>(&c), sizeof(c));
    ofs.write(reinterpret_cast<const char*>(&dir[Ends::A]), sizeof(Ends::Id));
    ofs.write(reinterpret_cast<const char*>(&dir[Ends::B]), sizeof(Ends::Id));
    ofs.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
}


template<Degree D>
template<bool ENDL>
void Edge<D>::
print(const std::string& prefix,
      const char dir) const
{
    log_<false>(prefix);
    log_<false>("[", indw, "]");
    if (dir != ' ')
        log_<false>(colorcodes::MAGENTA, dir, colorcodes::RESET);
    log_<false>("ind ", colorcodes::BOLDGREEN, ind, colorcodes::RESET);
    log_<false>("indc ", colorcodes::BOLDYELLOW, indc, colorcodes::RESET);
    log_<false>("w ", colorcodes::BOLDYELLOW, w, colorcodes::RESET);
    log_<false>("c ", colorcodes::BOLDYELLOW, c, colorcodes::RESET);
    log_<false>("weight ", colorcodes::BOLDYELLOW, weight, colorcodes::RESET);
    log_<ENDL>("");
}


template<Degree D>
void Edge<D>::
check(const Id nInd,
      const Id nIndC,
      const Id nIndW,
      const ChId nCh,
      const CmpId nCmpt) const
{
    ENSURE(ind < nInd,
           "Edge.cEdge.ind is out of range: ",
           "ind = ", ind, ", edgenum = ", nInd);

    ENSURE(indc < nIndC,
           "Edge.indc is out of range for ",
           "ind ", ind, ": ", "indc = ", indc, ", nIndC = ", nIndC);

    ENSURE(indw < nIndW,
           "EdgEdge.indw is out of range for ",
           "ind ", ind, ": ", "indw = ", indw, ", nIndW = ", nIndW);

    ENSURE(w < nCh,
           "Edge.w is out of range for ",
           "ind ", ind, ": ", "w = ", w, ", nCh = ", nCh);

    ENSURE(c < nCmpt,
           "Edge.c is out of range for ",
           "ind ", ind, ": ", "c = ", c, ", nCmpt = ", nCmpt);

    ENSURE(weight <= maxWeight,
           "Edge.weight is out of range for ",
           "ind ", ind, ": ", "weight = ", weight, ", nWeight = ", maxWeight);
}

using EdgeBase = Edge<maxDegree>;

}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_EDGE_H
