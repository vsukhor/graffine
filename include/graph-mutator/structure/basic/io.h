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
 * @file io.h
 * @brief Contains functionality for data input-output.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_IO_H
#define GRAPH_MUTATOR_STRUCTURE_IO_H

#include <filesystem>
#include <ostream>
#include <string>

#include "../definitions.h"
#include "ends.h"
#include "graph.h"

namespace graph_mutator::structure {

/**
 * @brief Implements class template handling input-output operations.
 * @tparam World Environment to save from.
 */
template<typename World>
struct IO {

    using Chain = World::Chain;
    using Ends = Chain::Ends;

    std::filesystem::path workingDirOut;  ///< Working directory for output files.

    const std::string runName;  ///< Name of the run.

    /**
     * @brief Constructor.
     * @param workingDirOut Directory to save output files.
     * @param runName Name of the run.
     * @param world World to operate on.
     */
    explicit IO(
        const std::filesystem::path& workingDirOut,
        const std::string& runName,
        const World& world
    );

    /**
     * @brief Writes graph to a file.
     * @param startnew Start a new file vs. adding new data records.
     * @param last Is the final writeout.
     * @param itr Current simulation iteration.
     * @param t Current simulation time.
     */
    void save_graph(
        bool startnew,
        bool last,
        szt itr,
        real t,
        szt saveFrequency = 1
    ) const;

    void to_json() const;

private:

    const World& world;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename World>
IO<World>::
IO(
    const std::filesystem::path& workingDirOut,
    const std::string& runName,
    const World& world
)
    : workingDirOut {workingDirOut}
    , runName {runName}
    , world {world}
{}


template<typename World>
void IO<World>::
save_graph(
    const bool startnew,
    const bool last,
    const szt itr,
    const real t,
    const szt saveFrequency
) const
{
    const auto file {last
        ? workingDirOut / ("graph_last_"s + runName)
        : workingDirOut / ("graph_"s      + runName)};
    const auto flags = startnew ? std::ios::binary | std::ios::trunc
                                : std::ios::binary | std::ios::app;
    std::ofstream ofs {file, flags};
    if (ofs.fail())
        log_("Cannot open file: ", file);

    ofs.write(reinterpret_cast<const char*>(&t), sizeof(t));
    const ChId cn {world.graph.chain_num()};
    ofs.write(reinterpret_cast<const char*>(&cn), sizeof(ChId));

    static ChId chainummax;
    static std::array<szt, Ends::num> nnmax;
    if (!last) {
        if (startnew) {
            chainummax = 0;
            nnmax = {};
        }
        if (world.graph.chain_num() > chainummax)
            chainummax = world.graph.chain_num();
    }
    for (const auto& m: world.graph.cn) {
        m.write(ofs);
        if (!last)
            for (const auto e : Ends::Ids)
                if (m.ngs[e].num() > nnmax[e])
                    nnmax[e] = m.ngs[e].num();
    }
    ofs.write(reinterpret_cast<const char*>(&chainummax), sizeof(ChId));
    ofs.write(reinterpret_cast<const char*>(&nnmax[Ends::A]), sizeof(szt));
    ofs.write(reinterpret_cast<const char*>(&nnmax[Ends::B]), sizeof(szt));

    const szt nst2save = last
                       ? szt{}
                       : static_cast<szt>(itr / saveFrequency);
    ofs.write(reinterpret_cast<const char*>(&nst2save), sizeof(szt));
}


template<typename World>
void IO<World>::
to_json() const
{
    const auto file = workingDirOut /
                      ("graph_run_"s + runName +
                       "_it_" + std::to_string(world.it) +
                       ".json");

    std::ofstream ofs {file};
    ENSURE(!ofs.fail(), "Unable to open file for writing: ", file);

    log_("Exporting graph to ", file, " ...");

    world.to_json(ofs);
}

}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_IO_H
