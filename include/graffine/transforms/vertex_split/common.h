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
 * \brief Declarations shared by the vertex splits: trait, processor
 * concept, plan, and the degree-specific functor templates.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H

#include "graffine/definitions.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/context.h"
#include "graffine/transforms/plan.h"
#include "graffine/transforms/vertex_merger/common.h"

#include <type_traits>

#include <array>
#include <vector>


namespace graffine::transforms {

/// Compile-time description of the vertex split.
template<typename G>
struct Trait<TransformType::VertexSplit, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    /// Result of the transformation: pointers to the two vertices produced
    /// by the split.
    using Res = std::array<ResT, 2>;

    static constexpr auto Type = TransformType::VertexSplit;  ///< Family.

    static constexpr const char* fullName  {"vertex_split_"};  ///< Name prefix in messages.
    static constexpr const char* shortName {"v_s_"};  ///< Abbreviated name prefix.
};



/** Split of a vertex into two.
 * \details The inverse of the vertex merger: a vertex of degree D1 + D2
 * becomes two vertices of degrees D1 and D2, sharing out its edges. The
 * edges going to the first resulting vertex are given by their chain end
 * slots, or by their descriptors at the vertex; a vertex of degree 2 is
 * given by its BulkSlot. Splitting may disconnect a component into two.
 *
 * To<D1, D2> produces vertices of degrees D1 and D2; the implemented pairs
 * are 1+1 (cutting a chain at an internal vertex), 1+2 and 1+3 (detaching
 * one chain end from a junction) and 2+2 (separating an X junction into two
 * pairs of chain ends). Functor dispatches to the right one for any vertex.
 * Usage:
 * \code
 * vertex_split::To<Deg1, Deg2, G> detach {gr};
 * detach(G::EndSlot {w, G::End::A});  // detach end A of w from its Y junction
 * \endcode
 */
namespace vertex_split {


/// Compile-time description of the vertex split.
template<typename G>
using Trait = transforms::Trait<TransformType::VertexSplit, G>;

/** Processor of non-topological properties usable by the vertex splits.
 * \details Either void, for no processing, or a type providing the hooks
 * called by the splits: with the split chain or slots before a split, and
 * with the resulting vertices after it.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept Processor =
    std::is_void_v<PP> ||
    requires (PP& pp,
              const ChIdG w,
              const typename G::EndSlot& es,
              const typename G::BulkSlot& bs,
              const typename Trait<G>::Res& res)
    {
        hooks::on_start(pp.vertexSplit, w);
        hooks::on_start(pp.vertexSplit, es);
        hooks::on_start(pp.vertexSplit, bs);
        hooks::on_start(pp.vertexSplit, es, es);
        hooks::on_end(pp.vertexSplit, res);
    };


/** Outcome of a vertex split, in terms of the graph before it.
 * \details See plan.h.
 */
template<typename G>
struct Plan {
    using Vertex = G::Vertex;

    const Vertex* vertex {};  ///< The vertex split.

    /// Edges of the resulting vertices, in the order of the result.
    /// \details Either resulting vertex may be \a vertex itself, or a new
    /// one: the plan does not tell.
    std::array<std::vector<EgIdG>, 2> edges;

    /// The hooks of this family in processor \p pp.
    template<typename PP>
    static constexpr auto hooks(PP& pp) -> decltype((pp.vertexSplit))
    {
        return pp.vertexSplit;
    }
};

/** Plan of the split of vertex \p v of graph \p gr, giving edges \p ii to
 * the first resulting vertex and the others to the second one.
 */
template<typename G>
auto make_plan(
    const G& gr,
    const typename G::Vertex& v,
    std::vector<EgIdG> ii
) -> Plan<G>
{
    auto oo = plans::edges_at_other_than(gr, v, ii);

    return {&v, {std::move(ii), std::move(oo)}};
}


/** Generic template of the degree-specific vertex splits; specialized for
 * the implemented pairs of resulting degrees.
 * \tparam D1 Degree of the 1st resulting vertex.
 * \tparam D2 Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D1,
         Degree D2,
         typename G,
         typename PP = void>
    requires vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct To
{};

/** Split producing a vertex of degree 1 and one of degree \p D: the same
 * functor as To<Deg1, D>, addressed by the second degree only.
 * \tparam D Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D,
         typename G,
         typename PP = void>
    requires vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct To1
{};

/** Split producing a vertex of degree 2 and one of degree \p D: the same
 * functor as To<Deg2, D>, addressed by the second degree only.
 * \tparam D Degree of the 2nd resulting vertex.
 * \tparam G Graph to which the split operation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<Degree D,
         typename G,
         typename PP = void>
    requires vertex_split::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct To2
{};


}  // namespace vertex_split
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_COMMON_H
