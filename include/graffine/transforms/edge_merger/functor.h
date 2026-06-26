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
 * \brief Contains class performing merger of two incident edges.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_EDGE_MERGER_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_EDGE_MERGER_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"
#include "graffine/transforms/edge_deletion/in_multi_edge_chain.h"
#include "graffine/transforms/edge_deletion/in_single_edge_chain.h"

#include <array>
#include <memory>
#include <type_traits>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::EdgeMerger, G>
{
    using Graph = G;
    using Compt = Graph::Compt;
    using Chain = Graph::Chain;
    using Edge = Graph::Edge;
    using OnStart = std::array<const Edge*, 2>;
    using OnEnd = Edge*;
    using ResT = Edge*;
    using Res = std::array<ResT, 1>;

    static constexpr const char* fullName {"edge_merger"};
    static constexpr const char* shortName {"eg_mg"};

    static constexpr auto Type = TransformType::EdgeMerger;
};


/// Enables merger of graph edges.
namespace edge_merger {

template<typename G>
using Trait = transforms::Trait<TransformType::EdgeMerger, G>;

/// Performs merger of two incident edges.
template<typename G,
         typename PP = void>
struct Functor
{
    using Trait = edge_merger::Trait<G>;
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

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit Functor(Graph& gr,
                     std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the transformation.
     * \details \p ind1 will be merged into \p ind2 and disappear if 'ind2' has
     * other edges besides it; if it does not, 'ind2' will be merged into 'ind1'
     * and disappear.
     * \param[in] ind1 Graph_wide index of the 1st edge merged.
     * \param[in] ind2 Graph_wide index of the 2nd edge merged.
     */
    auto operator()(const EgIdG ind1,
                    const EgIdG ind2) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;

    /// Auxiliary functors. Utilized for internal operation.
    edge_deletion::InSingleEdgeChain<G, PP> remSE;
    edge_deletion::InMultiEdgeChain<G, PP>  remME;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
    , remSE {gr, pp}
    , remME {gr, pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}

// 'ind1' will be merged into 'ind2' and disappear if chain of 'ind2' has
// other edges besides it; if it does not, 'ind2' will be merged into 'ind1'
// and disappear.
template<typename G,
         typename PP>
auto Functor<G, PP>::
operator()(const EgIdG ind1,
           const EgIdG ind2) noexcept -> Res
{
    auto& eg1 = gr.edge(ind1);
    auto& m1 = gr.chain(eg1.w);
    auto& eg2 = gr.edge(ind2);
    auto& m2 = gr.chain(eg2.w);

    ASSERT(eg1.is_neig_of(eg2), "Edges are not mergeable: Edges are not adjacent");
    ASSERT(m1.is_shrinkable() || m2.is_shrinkable(),
          "Edges are not mergeable: Hosting chain(s) are not shrinkable.");

    if constexpr (verboseF) {
        jot(colorcodes::GREEN, "Edge merger: ");
        eg1.print("Edge 1: ");
        eg2.print("edge 2: ");
        m1.print("");
        if (m1.idw != m2.idw)
            m2.print("");
        jot("");
    }

    if constexpr (!std::is_void_v<PP>)
    gr.template on_transform<Trait>.at_start({eg1, eg2});

    Edge* eg {};
    if (m1.is_shrinkable()) {
        m1.length() == 1 && m1.has_single_leaf_vertex()
            ? remSE(ind1)
            : remME(ind1);
        eg = &eg2;
    }
    else if (m2.is_shrinkable()) {
        m2.length() == 1 && m2.has_single_leaf_vertex()
            ? remSE(ind2)
            : remME(ind2);
        eg = &eg1;
    }

    Res res {eg};

    if constexpr (!std::is_void_v<PP>)
    gr.template on_transform<Trait>.at_end(eg);

    if constexpr (verboseF) {
        m1.print(shortName, "  produces ");
        if (m1.idw != m2.idw)
            m2.print("       and");
        eg.print("result: ");
        jot("");
    }

    return res;
}

}  // namespace edge_merger
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_EDGE_MERGER_FUNCTOR_H