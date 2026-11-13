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
 * \brief Declarations shared by the vertex mergers: trait, processor
 * concept, plan, and the degree-specific functor templates.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/context.h"
#include "graffine/transforms/plan.h"

#include <type_traits>
#include <utility>  // swap

#include <array>

namespace graffine::transforms {

/// Compile-time description of the vertex merger.
template<typename G>
struct Trait<TransformType::VertexMerger, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    /// Result of the transformation: pointer to the vertex resulting from the merger.
    using Res = std::array<ResT, 1>;

    static constexpr auto Type = TransformType::VertexMerger;  ///< Family.

    static constexpr const char* shortName {"v_m_"};  ///< Abbreviated name prefix.
    static constexpr const char* fullName  {"vertex_merger"};  ///< Name in messages.
};

/** Merger of two vertices into one.
 * \details The two vertices, of degrees D1 and D2, become a single vertex
 * of degree D1 + D2, which must not exceed the maximal degree 4. The
 * vertices are given by their slots: an EndSlot for a vertex at a chain end
 * (degrees 1 and 3), a BulkSlot for a vertex inside a chain (degree 2), or
 * the chain id for the boundary vertex of an unconnected cycle. Merging
 * vertices of different components joins the components; merging vertices
 * of the same component closes a cycle.
 *
 * From<D1, D2> merges vertices of degrees D1 and D2; the implemented pairs
 * are 1+1, 1+2, 1+3 and 2+2. Functor dispatches to the right one for any
 * two vertices. Usage:
 * \code
 * vertex_merger::From<Deg1, Deg1, G> merge {gr};
 * merge(G::EndSlot {w1, G::End::B}, G::EndSlot {w2, G::End::A});
 * \endcode
 */
namespace vertex_merger {

/** Processor of non-topological properties usable by the vertex mergers.
 * \details Either void, for no processing, or a type providing the hooks
 * called by the mergers: with the slots of the preserved and the removed
 * vertex before a merger, and with the resulting vertex after it.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              const typename G::EndSlot& es,
              const typename G::BulkSlot& bs,
              typename G::Vertex& v)
    {
        hooks::on_start(pp.vertexMerger, es, es);
        hooks::on_start(pp.vertexMerger, bs, es);
        hooks::on_start(pp.vertexMerger, bs, bs);
        hooks::on_end(pp.vertexMerger, v);
    };


/// Compile-time description of the vertex merger.
template<typename G>
using Trait = transforms::Trait<TransformType::VertexMerger, G>;

/** Outcome of a vertex merger, in terms of the graph before it.
 * \details See plan.h.
 */
template<typename G>
struct Plan {
    using Vertex = G::Vertex;

    /// Merged vertex which is the result, keeping its identity.
    const Vertex* preserved {};
    /// Merged vertex which is removed.
    const Vertex* removed {};

    /// The hooks of this family in processor \p pp.
    template<typename PP>
    static constexpr auto hooks(PP& pp) -> decltype((pp.vertexMerger))
    {
        return pp.vertexMerger;
    }
};

/** Keeping of a given one of the merged vertices (see Keep).
 * \details The merger exchanges the two vertices, if it would keep the
 * other one, right after its on_start hook: its hooks see the vertices in
 * place before the merger, and the kept one as the result.
 * \tparam G Graph to which the merger is applied.
 */
template<typename G>
struct Keeper {
    using Vertex = G::Vertex;

    /// The vertex to keep and the other one, if to exchange.
    std::array<Vertex*, 2> vv {};

    /** Plan \p p of the merger of \p v1 and \p v2, keeping vertex \p k.
     */
    static auto plan(Plan<G> p,
                     const Vertex& v1,
                     const Vertex& v2,
                     const Keep k) -> Plan<G>
    {
        if (p.preserved != (k == Keep::First ? &v1 : &v2))
            std::swap(p.preserved, p.removed);

        return p;
    }

    /** Prepares the merger of \p v1 and \p v2, planned \p p, to keep vertex
     * \p k.
     */
    void ask(const Plan<G>& p,
             Vertex& v1,
             Vertex& v2,
             const Keep k) noexcept
    {
        auto* const kept = k == Keep::First ? &v1 : &v2;
        if (p.preserved != kept)
            vv = {kept, kept == &v1 ? &v2 : &v1};
    }

    /** Exchanges the vertices, if asked for.
     */
    void operator()(G& gr)
    {
        if (vv[0])
            gr.exchange_vertices(*vv[0], *vv[1]);
        vv = {};
    }
};

/** Generic template of the degree-specific vertex mergers; specialized for
 * the implemented pairs of degrees.
 * \tparam D1 Degree of the 1st input vertex.
 * \tparam D2 Degree of the 2nd input vertex.
 * \tparam G Graph to which the transformation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D1,
         Degree D2,
         typename G,
         typename PP = void>
    requires areCompatibleDegrees<D1, D2> &&
             vertex_merger::Processor<PP, G>
struct From {};

/** Merger of a vertex of degree 1 with a vertex of degree \p D: the same
 * functor as From<Deg1, D>, addressed by the degree of the second vertex
 * only.
 * \tparam D Degree of the 2nd input vertex.
 * \tparam G Graph to which the transformation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D,
         typename G,
         typename PP = void>
    requires vertex_merger::Processor<PP, G>
struct From1 {};

/// Chain end standing for the boundary vertex of an unconnected cycle,
/// which lies at both ends.
inline constexpr auto UnconnectedCycleEnd = structure::descriptors::End::A;


}  // namespace vertex_merger
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_COMMON_H
