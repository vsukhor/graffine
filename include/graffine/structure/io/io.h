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
 * \file io.h
 * \brief Contains functionality for data input-output.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_IO_H
#define GRAFFINE_STRUCTURE_IO_H

#include <filesystem>
#include <ostream>
#include <string>

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/descriptors/end.h"
#include "tinyply.h"

namespace graffine::structure {

/**
 * Implements class template handling input-output operations.
 * \tparam World Environment to save from.
 */
template<typename World>
struct IO
{
    using Graph = World::Graph;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using Vertex = Graph::Vertex;
    using End = Chain::End;

    std::filesystem::path workingDirOut;  ///< Working directory for output files.

    const std::string runName;  ///< Name of the run.

    /**
     * Constructor.
     * \param[in] workingDirOut Directory to save output files.
     * \param[in] runName Human-readable name of the current run.
     * \param[in] world World to operate on.
     */
    explicit IO(
        const std::filesystem::path& workingDirOut,
        const std::string& runName,
        const World& world
    );

    void write_graph_to_PLY(const std::filesystem::path& file,
                            const bool asBinary);

    /**
     * Writes graph to a file.
     * \param[in] startnew SrunNametart a new file vs. adding new data records.
     * \param[in] last Is the final writeout.
     * \param[in] itr Current simulation iteration.
     * \param[in] t Current simulation time.
     * \param[in] saveFreq The graph is saved every \a saveFreq iteration.
     */
    void save_graph(
        bool startnew,
        bool last,
        std::size_t itr,
        real t,
        std::size_t saveFreq=1
    ) const;

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
    const std::size_t itr,
    const real t,
    const std::size_t saveFreq
) const
{
    const auto file {
        last ? workingDirOut / ("graph_last_"s + runName)
             : workingDirOut / ("graph_"s      + runName)
    };
    const auto flags =
        startnew ? std::ios::binary | std::ios::trunc
                 : std::ios::binary | std::ios::app;
    std::ofstream ofs {file, flags};
    if (ofs.fail())
        jot("Cannot open file: ", file);

    ofs.write(reinterpret_cast<const char*>(&t), sizeof(t));
    const ChId chnum {world.graph.num_chains()};
    ofs.write(reinterpret_cast<const char*>(&chnum), sizeof(ChId));

    static std::size_t chainummax;
    static std::array<size_t, End::num> nnmax;
    if (!last) {
        if (startnew) {
            chainummax = {};
            nnmax = {};
        }
        if (world.graph.num_chains() > chainummax)
            chainummax = world.graph.num_chains();
    }
    for (const auto& m : world.graph.chains()) {
        m.write(ofs);
        if (!last)
            for (const auto e : End::Ids)
                if (m.ngs[e].num() > nnmax[e])
                    nnmax[e] = m.ngs[e].num();
    }
    ofs.write(reinterpret_cast<const char*>(&chainummax), sizeof(std::size_t));
    ofs.write(reinterpret_cast<const char*>(&nnmax[End::A]), sizeof(std::size_t));
    ofs.write(reinterpret_cast<const char*>(&nnmax[End::B]), sizeof(std::size_t));

    const std::size_t nst2save = last
                       ? std::size_t{}
                       : static_cast<size_t>(itr / saveFreq);
    ofs.write(reinterpret_cast<const char*>(&nst2save), sizeof(std::size_t));
}

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_IO_H
