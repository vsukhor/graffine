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
 * @file world.h
 * @brief Implements container for graphs and auxiliary components and fields.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_WORLD_H
#define GRAPH_MUTATOR_STRUCTURE_WORLD_H

#include <ostream>
#include <string>

#include "../definitions.h"

namespace graph_mutator::structure {

/**
 * @brief The Network class template.
 * @details Represents a dynamic graph, subjected to transformations. An
 * initial set of graph components is created at construction.
 * @tparam Graph Type of Graph used to populate the World.
 */
template<typename G>
struct World {

    friend class IO<World>;

    using Graph = G;
    using Chain = Graph::Chain;
    using Edge = Chain::Edge;
    using Chains = Graph::Chains;

    Graph graph;

    double time;  ///< Current time.
    itT    it;    ///< Iteration counter.

    /**
     * @brief Constructor.
     * @param num Initial number of components.
     * @param len Initial component length.
     * @param rnd Random number factory.
     */
    explicit World(
        CmpId num,
        EgId len,
        RandFactory& rnd
    );

    /**
     * @brief Checks if the world is equilibrated.
     * @return True if equilibrated, false otherwise.
     */
    auto is_equilibrated() const noexcept -> bool;

    /**
     * @brief Outputs components of the universe to a text-formatted stream.
     * @param os Output stream.
     */
    void report(std::ostream& os) const;

    /**
     * @brief Exports components to a json-formatted file stream.
     * @param ofs Output file stream.
     */
    void to_json(std::ofstream& ofs) const;

private:
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename Chain>
World<Chain>::
World(
    const CmpId num,
    const EgId len,
    RandFactory& rnd
)
    : time {}
    , it {}
{
    graph.generate_single_chain_components(num, len);
    graph.update_books();
}


template<typename Chain>
auto World<Chain>::
is_equilibrated() const noexcept -> bool
{
    return graph.is_equilibrated();
}


template<typename Chain>
void World<Chain>::
report(std::ostream& os) const
{
    graph.report(os);
}


template<typename Chain>
void World<Chain>::
to_json(std::ofstream& ofs) const
{
    graph.to_json(ofs);
}


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_WORLD_H
