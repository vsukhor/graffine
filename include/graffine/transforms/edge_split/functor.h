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
 * \file functor.h
 * \brief Split of an edge into two consecutive edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_SPLIT_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_EDGE_SPLIT_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/context.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/edge_creation/common.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"

#include <array>
#include <memory>
#include <type_traits>

namespace graffine::transforms {

/// Compile-time description of the edge split.
template<typename G>
struct Trait<TransformType::EdgeSplit, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using Vertex = Graph::Vertex;
    using OnStart = const Edge*;  ///< Argument of the on_start hook.
    using OnEnd = Vertex*;        ///< Argument of the on_end hook.
    using ResT = Edge*;
    /// Result of the transformation: pointer to the new edge, placed right
    /// after the split edge in its chain.
    using Res = std::array<ResT, 1>;

    static constexpr const char* fullName {"edge_split"};  ///< Name in messages.
    static constexpr const char* shortName {"eg_sp"};  ///< Abbreviated name.

    static constexpr auto Type = TransformType::EdgeSplit;  ///< Family.
};



/// Split of an edge into two consecutive edges.
namespace edge_split {

/// Compile-time description of the edge split.
template<typename G>
using Trait = transforms::Trait<TransformType::EdgeSplit, G>;

/** Splits an edge into two consecutive edges, joined by a new vertex of
 * degree 2.
 * \details Performed as an edge creation in the edge's chain (see
 * edge_creation::InExistingChain), right after the split edge: its hooks
 * are those of edgeCreation, run inside this transformation (see
 * context.h). The topology is unchanged, except that the chain gains an
 * edge. Usage:
 * \code
 * edge_split::Functor<G> split {gr};
 * const auto [eg] = split(ind);
 * \endcode
 * \tparam G Graph to which the transformation is applied.
 * \tparam PP Processor of non-topological properties, or void.
 */
template<typename G,
         typename PP = void>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
struct Functor
    : public Applicable<Functor<G, PP>>
{
    using Trait = edge_split::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto fullName = Trait::fullName;
    static constexpr auto shortName = Trait::shortName;

    /// Context of the hooks of the inner edge creation.
    static constexpr Frame frame {Trait::Type};

// public methods

    /** Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit Functor(Graph& gr,
                     std::shared_ptr<PP> pp = nullptr);

    /** Splits edge \p ind. Unchecked: see Applicable.
     * \param[in] ind Graph-wide index of the source edge.
     * \return Pointer to the new edge.
     */
    auto apply(const EgIdG ind) -> Res;

    /** Checks that edge \p ind can be split.
     * \details Rejects an invalid edge index only.
     */
    auto applicability(EgIdG ind) const -> Applicability;

    /** Outcome of the split of edge \p ind, as the edge creation performing
     * it: the end of \p ind at its end B moves to the new vertex (see plan.h).
     */
    auto plan(const EgIdG ind) const -> edge_creation::Plan<G>
    {
        const auto& eg = gr.edge(ind);
        const auto& m = gr.chain(eg.w);

        return m.is_headind(ind) ? create.plan(EndSlot {eg.w, End::B})
                                 : create.plan(BulkSlot {eg.w, eg.indw + 1});
    }

    /** Processor of non-topological properties.
     */
    auto processor() const noexcept -> PP* { return create.processor(); }

    /** The graph transformed.
     */
    auto graph() const noexcept -> const Graph& { return create.graph(); }

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Inner edge creation performing the split.
    edge_creation::InExistingChain<G, PP> create;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : gr {gr}
    , create {gr, pp}
{
    GRAFFINE_ASSERT(!(std::is_void_v<PP> && pp), "Property processor given, while PP is void");
}


template<typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto Functor<G, PP>::
applicability(const EgIdG ind) const -> Applicability
{
    if (!checks::is_valid_edge(gr, ind))
        return reject("invalid edge index");

    return {};
}


template<typename G,
         typename PP>
    requires edge_creation::Processor<PP, G> &&
             vertex_merger::Processor<PP, G>
auto Functor<G, PP>::
apply(const EgIdG ind) -> Res
{
    const auto& eg0 = gr.edge(ind);
    const auto w = eg0.w;
    const auto a = eg0.indw;
    auto& m = gr.chain(w);

    if constexpr (verboseF) {
        jot(colorcodes::GREEN, "Edge split: ");
        eg0.print("source: ");
        jot("");
    }

    // The new edge is inserted right after the source edge: either at the
    // internal vertex connecting it to the next edge of the chain, or, if the
    // source edge is the chain head, appended at chain end B. Both keep the
    // vertex at end B of the source edge in place, so that the chain
    // connectivity is preserved. Note: 'eg0' is invalidated by the insertion.
    const auto res = nested<PP>(frame, [&] {
        return m.is_headind(ind) ? create(EndSlot{w, End::B})
                                 : create(BulkSlot{w, a + 1});
    });

    if constexpr (verboseF) {
        m.print(shortName, "  produces ");
        res[0]->print("result: ");
        jot("");
    }

    return res;
}

}  // namespace edge_split
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_EDGE_SPLIT_FUNCTOR_H