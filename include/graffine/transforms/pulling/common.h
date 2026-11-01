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
 * \file common.h
 * \brief Contains generic templates for degree-specific vertex pulling.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PULLING_COMMON_H
#define GRAFFINE_TRANSFORMS_PULLING_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/context.h"
#include "graffine/transforms/plan.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_split/common.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_creation/common.h"

#include <type_traits>

#include <algorithm>  // find
#include <array>
#include <vector>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::Pulling, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using ResT = CmpId;
    /// Result of the transformation: index of the component hosting the path after the pull.
    using Res = std::array<ResT, 1>;

    static constexpr auto Type = TransformType::Pulling;
};


/// Enables pulling of a graph edge holding it at specific end.
namespace pulling {

template<typename C>
struct Path;

/**
 * Processor of non-topological properties usable by the pullings.
 * \details Either void, for no processing, or a type providing the hooks
 * called with the path: on_start once before pulling along it, and on_end
 * after each of the steps of the pull.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              Path<typename G::Compt>& path)
    {
        hooks::on_start(pp.pulling, path);
        hooks::on_end(pp.pulling, path);
    };


template<typename G>
using Trait = transforms::Trait<TransformType::Pulling, G>;

/**
 * Outcome of a pulling, in terms of the graph before it.
 * \details The path keeps its vertices and edges, in number and order: the
 * driver gains an edge and the source loses one. In each of the first
 * \a shifts steps, the edges off the path at its vertices other than the
 * source (the branches at its junctions, and at the driver if it is not a
 * leaf) move by one vertex towards the source: after step k, the branches at
 * path vertex i < n, n indexing the source, are at path vertex
 * i + min(k, shifts). The steps after the first \a shifts leave the topology
 * unchanged: the source chain is dissolved in step \a shifts if a branch
 * then reaches the source. Edges off the path at the source stay there.
 *
 * The path keeps its vertex objects, and so their ids: after each step,
 * path vertex i is the same object as before the pulling, the driver
 * included. A junction moves along the path by its branches moving to the
 * next path vertex, not by the vertex objects moving. This holds for every
 * driver degree, and when the source chain is dissolved, the source then
 * becoming the junction. The pulling hooks, and those of the inner splits
 * and mergers, receive the path vertices as these objects. See plan.h.
 */
template<typename G>
struct Plan {
    using Vertex = G::Vertex;

    /// Vertices of the path, from the driver to the source.
    std::vector<const Vertex*> vertices;

    /// Edges off the path at each of its vertices.
    std::vector<std::vector<EgIdG>> branches;

    int steps {};   ///< Number of steps.
    int shifts {};  ///< Number of leading steps shifting the branches.

    template<typename PP>
    static constexpr auto hooks(PP& pp) -> decltype((pp.pulling))
    {
        return pp.pulling;
    }
};

/**
 * Plan of pulling \p path of graph \p gr by \p steps.
 * \details The branch nearest to the source, at path vertex j, reaches the
 * source in n - j steps.
 */
template<typename G,
         typename P>
auto make_plan(
    const G& gr,
    const P& path,
    const int steps
) -> Plan<G>
{
    Plan<G> p {{}, {}, steps, 0};
    for (const auto* v : path.vertices()) {
        p.vertices.push_back(v);
        p.branches.push_back(plans::edges_at_other_than(gr, *v, path.pthw));
    }

    const auto n = static_cast<int>(p.vertices.size()) - 1;
    for (int j {n - 1}; j >= 0; --j)
        if (!p.branches[static_cast<std::size_t>(j)].empty()) {
            p.shifts = std::min(steps, n - j);
            break;
        }

    return p;
}

inline constexpr std::array pullableDegrees {Deg1, Deg2, Deg3};

template <Degree D>
constexpr auto is_pullable_degree =
    std::find(pullableDegrees.begin(), pullableDegrees.end(), D) !=
    pullableDegrees.end();

template <Degree D>
concept pullable_degree =
    std::find(pullableDegrees.begin(), pullableDegrees.end(), D) !=
    pullableDegrees.end();

/// Whether pulling from a degree 2 driver on an unconnected cycle is rejected.
inline constexpr bool omitUnconnectedCycles = true;

/**
 * Generic template for specifying vertex degree-specific pulling classes.
 * \details See FunctorBase for the pulling transformation, and Path for its
 * driver and source.
 * \tparam D Degree of the driver vertex, from which the pull operation is
 * initiated.
 * \tparam G Graph to which the transformation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D,
         typename G,
         typename PP = void>
    requires pullable_degree<D> &&
             pulling::Processor<PP, G> &&
             edge_creation::Processor<PP, G> &&
             edge_deletion::Processor<PP, G> &&
             vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct On {};


}  // namespace pulling
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_PULLING_COMMON_H
