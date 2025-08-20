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
 * @file all.h
 * @brief Contains implementation of various vertex types and their containers.
 * @details Due to its edge-centric graph implementation, graph_mutator does not keep
 * instances of vertices in allocated memory throughout the graph lifetime.
 * On relatively rear occasions when explicit vertex instances are needed, these
 * are reconstructed from edge connectivity data.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_VERTICES_ALL_H
#define GRAPH_MUTATOR_STRUCTURE_VERTICES_ALL_H

#include <algorithm>
#include <array>
#include <memory>
#include <functional>
#include <ostream>
#include <tuple>
#include <vector>

#include "../../definitions.h"
#include "collections.h"
#include "degrees.h"
#include "vertex.h"


namespace graph_mutator::structure::vertices {

/**
 * @brief The Collection of vertices classified by degree.
 * @details Encapsulates vertex containers and the uppper-level machinery for
 * updating and exporting their contents.
 * @tparam G Graph containing the vertices.
*/
template<typename G>
struct All {

    static_assert(maxDegree == 4);

    template<Degree D> requires is_implemented_degree<D>

    /// Tuple of vertex collections classified by vertex degree.
    using Container = std::tuple<Collection<0, G>,
                                 Collection<1, G>,
                                 Collection<2, G>,
                                 Collection<3, G>,
                                 Collection<4, G>>;

    using EndsContainer = std::tuple<Collection<1, G>,
                                     Collection<3, G>,
                                     Collection<4, G>>;

    using BulksContainer = std::tuple<Collection<0, G>,
                                      Collection<2, G>>;

    const G& gr;

    /// Tuple of vertex collections classified by vertex degree.
    std::tuple<Collection<0, G>,
               Collection<1, G>,
               Collection<2, G>,
               Collection<3, G>,
               Collection<4, G>> all;

    /// Tuple of collections of non-bulk vertices by vertex degree.
    std::tuple<Collection<1, G>,
               Collection<3, G>,
               Collection<4, G>> ends;

    /// Tuple of collections of non-bulk vertices by vertex degree.
    std::tuple<Collection<0, G>,
               Collection<2, G>> bulks;

    /// Constructs a Collection from the Graph class instance.
    explicit constexpr All(const G& gr);

    /// Updates all the containers.
    void create() noexcept;

    void create_on_ends() noexcept;
    void create_on_bulks() noexcept;

    /**
     * @brief Updates container content for the verted degree specified.
     * @tparam D Vertex degree.
     * @param index Reference to global vertex counter.
     * graph compartment is being processed.
     * */
    template<Degree D> requires is_implemented_degree<D>
    auto create(
        Id& index,
        CmpId c
    ) noexcept;

    template<Degree D> requires is_implemented_degree<D>
    constexpr auto num() const noexcept -> szt;

    constexpr auto num() const noexcept -> szt;

    template<Degree D> requires is_implemented_degree<D>
    auto for_compartment(CmpId c) const noexcept -> Collection<D, G>;

    constexpr auto from_end_slot(const Degree d,
                                 const G::EndSlot& s) const noexcept -> Id;

    constexpr auto from_bulk_slot(const G::BulkSlot& s) const noexcept -> Id;

    template<Degree D> requires is_implemented_degree<D>
    void print(const std::string& s = "") const noexcept;

    void print(const std::string& s = "") const noexcept ;

    void report(std::ostream& ofs) const;

    /**
     * @brief Exports to a json-formatted file.
     * @param ofs File stream to output the data.
     * */
    void to_json(std::ofstream& ofs) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
All<G>::
All(
    const G& gr
)
    : gr {gr}
    , all {Collection<0, G> {gr},
           Collection<1, G> {gr},
           Collection<2, G> {gr},
           Collection<3, G> {gr},
           Collection<4, G> {gr}}
    , ends {Collection<1, G> {gr},
            Collection<3, G> {gr},
            Collection<4, G> {gr}}
    , bulks {Collection<0, G> {gr},
             Collection<2, G> {gr}}
{}


template<typename G>
void All<G>::
create() noexcept
{
    Id ind {};
    std::apply([&](auto&... ns){ (ns.populate(ind), ...); }, all);
}


template<typename G>
void All<G>::
create_on_ends() noexcept
{
    Id ind {};
    std::apply([&](auto&... ns){ (ns.populate(ind), ...); }, ends);
}


template<typename G>
void All<G>::
create_on_bulks() noexcept
{
    Id ind {};
    std::apply([&](auto&... ns){ (ns.populate(ind), ...); }, bulks);
}


template<typename G>
template<Degree D> requires is_implemented_degree<D>
auto All<G>::
create(Id& index, const CmpId c) noexcept
{
    std::get<D>(all).populate(index, c);
}


template<typename G>
constexpr
auto All<G>::
num() const noexcept -> szt
{

    return std::apply([](const auto&... e){ return (e.num() + ...); }, all);
}


template<typename G>
template<Degree D> requires is_implemented_degree<D>
constexpr
auto All<G>::
num() const noexcept -> szt
{
    return std::get<D>(all).num();
}


template<typename G>
template<Degree D> requires is_implemented_degree<D>
auto All<G>::
for_compartment(const CmpId c) const noexcept -> Collection<D, G>
{
    Collection<D, G> vs {gr};

    for (const auto& v: std::get<D>(all).vv)
        if (Vertex<D, typename Collection<D, G>::S>::cmpt(v, gr) == c)
            vs.vv.push_back(v);

    return vs;
}


template<typename G>
constexpr
auto All<G>::
from_end_slot(const Degree d,
              const G::EndSlot& s) const noexcept -> Id
{

    switch (d) {
        case 0:
            return find_vertex<0, G>(s, std::get<0>(all));

        case 1:
            return find_vertex<1, G>(s, std::get<1>(all));

        case 3:
            return find_vertex<3, G>(s, std::get<3>(all));

        case 4:
            return find_vertex<4, G>(s, std::get<4>(all));
    }

    return undefined<Id>;
}


template<typename G>
constexpr
auto All<G>::
from_bulk_slot(const G::BulkSlot& s) const noexcept -> Id
{
    return find_vertex<2, G>(s, std::get<2>(all));
}


template<typename G>
template<Degree D> requires is_implemented_degree<D>
void All<G>::
print(const std::string& s) const noexcept
{
    log_<false>(s);
    for (const auto& v: std::get<D>(all).vv)
        v.print();
}


template<typename G>
void All<G>::
print(const std::string& s) const noexcept
{
    log_<false>(s);
    if (s.length())
        log_("");

    std::apply([&](const auto&... ns){ (vertices::print(ns), ...); },
               all);

    log_("");
}


template<typename G>
void All<G>::
report(std::ostream& ofs) const
{
    ofs << " X ";
    std::apply([&](const auto&... ns){ (ns.report(ofs), ...); },
               all);
}


template<typename G>
void All<G>::
to_json(std::ofstream& ofs) const
{
    const auto nn = num();

    auto jsn = [&](const auto& e)
    {
        for (const auto& n : e.vv)
            n.to_json(ofs, nn);
    };

    std::apply([&](auto const&... e){ (jsn(e), ...); },
               all);
}


}  // namespace graph_mutator::structure::vertices

#endif  // GRAPH_MUTATOR_STRUCTURE_VERTICES_ALL_H
