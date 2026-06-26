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
 * \file on_1.h
 * \brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 1.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_ON_1_H
#define GRAFFINE_TRANSFORMS_PULLING_ON_1_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/pulling/common.h"
#include "graffine/transforms/pulling/functor_base.h"
#include "graffine/transforms/pulling/path.h"
#include "graffine/transforms/pulling/reporter.h"

#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace graffine::transforms::pulling {

/**
 * Functor for edge pulling initiated at a vertex of degree 1.
 * \tparam Dir Direction of the pull operation.
 * \tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G,
         typename PP>
struct On<Deg1, Dir, G, PP>
    : public FunctorBase<G, PP>
{
    using Base = FunctorBase<G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Compt = Base::Compt;
    using Chain = Base::Chain;
    using Path = Base::Path;
    using PropertyProcessor = Base::PropertyProcessor;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg1;

    static_assert(is_pullable_degree<D>);

    const std::string d {std::to_string(D)};
    const std::string dir {std::to_string(Dir)};
    const std::string shortName {d + "pu_" + dir};
    const std::string fullName  {"Pulling "s + dir + " from Vertex Deg " + d};

    /**
     * Constructs functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit On(Graph& gr,
                std::shared_ptr<PP> pp = nullptr);

    /**
     * Pulls a vertex of degree 1.
     * \note The path is expected to be a valid path over the graph.
     * The path starts with a driver edge and ends with a source edge.
     * \param[inout] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps to pull over.
     * \return Result containing the component ID after the pull operation.
     */
    auto operator()(
        Path& path,
        int steps
    ) noexcept -> Res;

protected:

    using Base::cn;
    using Base::updateProperties;

    /**
     * Checks that the path is valid for use in pulling.
     * \param[in] path Path containing the string of edges to get pulled.
     * \param[in] steps Number of steps to pull over.
    */
    void check_path(
        const Path& path,
        int steps
    ) const noexcept;

private:

    /// Pretty-printer of the initial and final configurations.
    Reporter<On<D, Dir, Graph, PP>> report {*this};
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G,
         typename PP>
On<Deg1, Dir, G, PP>::
On(Graph& gr,
   std::shared_ptr<PP> pp)
    : Base {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<Orientation Dir,
         typename G,
         typename PP>
auto On<Deg1, Dir, G, PP>::
operator()(
    Path& path,
    int steps
) noexcept -> Res
{
    if constexpr (verboseF)
        report.before(path, steps);

    check_path(path, steps);

    if constexpr (!std::is_void_v<PP>)
        updateProperties->pulling.on_start(path);

    this->pull_leaf_vertex(path, steps);

    if constexpr (verboseF)
        report.after(path);

    return {path.cmp->ind};
}


template<Orientation Dir,
         typename G,
         typename PP>
void On<Deg1, Dir, G, PP>::
check_path(
    const Path& path,
    [[maybe_unused]] const int steps
) const noexcept
{
    const auto c = path.cmp;
    const auto p = path.pthw;
    const auto wS = path.s.w;
    [[maybe_unused]] const auto eS = path.s.e;
    const auto wD = path.d.w;
    [[maybe_unused]] const auto eD = path.d.end_slot(c->chain(wD)).e;

    ASSERT(c->chain(wD).ngs[eD].num() == 0, "Driver end is not disconnected");

    [[maybe_unused]] const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && c->chain(wS).ngs[eS].num() == 0),
           "Source end is not disconnected");

    [[maybe_unused]] const auto lenS = path.length_over_source_chain();

    ASSERT((!sourceIsCycle &&
            lenS >= static_cast<size_t>(steps)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<size_t>(steps)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graffine::transforms::pulling

#endif  // GRAFFINE_TRANSFORMS_PULLING_ON_1_H
