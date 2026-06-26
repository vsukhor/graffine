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
 * \file from_11.h
 * \brief Contains class performing merger between vertices of degree 1.
 * \note Simplified version of the functor class was also used in
 * https://github.com/vsukhor/mitoSim/blob/master/include/ability_for_fusion.h
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_merger/reporter.h"

#include <array>
#include <memory>
#include <vector>
#include <type_traits>

namespace graffine::transforms::vertex_merger {


/**
 * Merges two vertices of degree 1 to produce a vertex of degree 2.
 * \details Adds vertex type-specific vertex merger capability and updates the
 * graph for it.
 * \tparam G Graph to which the merger operation is applied.
 */
template<typename G,
         typename PP>
struct From<Deg1, Deg1, G, PP>
{
    using Trait = vertex_merger::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto I1 = Deg1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Deg1;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Deg2;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = undefined<Degree>;  ///< No 2nd output vertex.

    const std::string dd {degrees_to_string(I1, I2)};
    const std::string shortName {make_name(Trait::shortName, dd)};
    const std::string fullName  {make_name(Trait::fullName, dd)};

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties.
     */
    explicit From(Graph& gr,
                  std::shared_ptr<PP> pp = nullptr);

    /**
     * Merges two vertices of degree 1 to produce a vertex of degree 2.
     * \details Calls its overload using chain free end slots.
     * \param[in] v1 First merged vertex.
     * \param[in] v2 Second merged vertex.
     */
    auto operator()(Vertex& v1,
                    Vertex& v2) noexcept -> Res;

    /**
     * Merges two vertices of degree 1 to produce a vertex of degree 2.
     * \attention Important: when merge.this operator is used for antiparallel
     * merger (i.e. on equal end IDs of the merged chains), one of the
     * chains is reversed:
     * then, if \p e1 is A, reverses g of w1, if \p e1 is B, reverses g of w2:
     * e1 == A: 456 + 0123 -> 6540123 (>>> + >>>> -> <<<>>>>)
     * e1 == B: 456 + 0123 -> 4563210 (>>> + >>>> -> >>><<<<)
     * \param[in] s1 Chain boundary slot at the 1st parent vertex.
     * \param[in] s2 Chain boundary slot on the 2nd parent vertex.
     */
    auto operator()(const EndSlot& s1,
                    const EndSlot& s2) noexcept -> Res;

private:

    Graph& gr;  ///< Reference to the modified graph object.

    Core<Graph, PP> merge;  ///< Low-level free-end connector.

    /// Pretty-printer of the initial and final configurations.
    Reporter<Graph> report;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
From<Deg1, Deg1, G, PP>::
From(Graph& gr,
     std::shared_ptr<PP> pp
)
    : gr {gr}
    , merge {gr, pp, shortName}
    , report {gr, dd}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
operator()(
    Vertex& v1,
    Vertex& v2
) noexcept -> Res
{
    ASSERT(v1.is_leaf(), "Vertex 1 is not Leaf");
    ASSERT(v2.is_leaf(), "Vertex 2 is not Leaf");

    const auto& egs1 = *v1.edges().begin();
    const auto& egs2 = *v2.edges().begin();

    const auto egl1 = gr.ct[v1.get_c()].get_egl(egs1.w);
    const auto egl2 = gr.ct[v2.get_c()].get_egl(egs2.w);

    const EndSlot s1 = gr.chain(egl1.w).end_slot(egl1.a, egs1.e);
    const EndSlot s2 = gr.chain(egl2.w).end_slot(egl2.a, egs2.e);

    ASSERT(s1.is_defined(), "Vertex 1 is not at chain end of", egl1.w);
    ASSERT(s2.is_defined(), "Vertex 2 is not at chain end of", egl2.w);

    return (*this)(s1, s2);
}


// Important: when merge.antiparallel is called:
// if \p e1 is A, reverses g of w1, if \p e1 is B, reverses g of w2
template<typename G,
         typename PP>
auto From<Deg1, Deg1, G, PP>::
operator()(
    const EndSlot& s1,
    const EndSlot& s2
) noexcept -> Res
{
    // Use low-level vertex merging functions directly
    // gr.update() is called in each of these:

    if (s2.w == s1.w)  return merge.to_cycle(s1.w);
    /* else */         return merge.to_linear(s1, s2);
}


// /////////////////////////////////////////////////////////////////////////////

template<typename G,
         typename PP>
struct From1<Deg1, G, PP>
    : public From<Deg1, Deg1, G, PP>
{
    using Base = From<Deg1, Deg1, G, PP>;
    using Trait = Base::Trait;
    using Graph = Base::Graph;
    using Chain = Base::Chain;
    using End = Base::End;
    using EndSlot = Base::EndSlot;
    using BulkSlot = Base::BulkSlot;
    using Vertex = Base::Vertex;
    using PropertyProcessor = PP;
    using ResT = Base::ResT;
    using Res = Base::Res;
    using Base::operator();

    static constexpr auto I1 = Base::I1;  ///< Degree of the 1st input vertex.
    static constexpr auto I2 = Base::I2;  ///< Degree of the 2nd input vertex.
    static constexpr auto J1 = Base::J1;  ///< Degree of the 1st output vertex.
    static constexpr auto J2 = Base::J2;  ///< No 2nd output vertex.

    using Base::dd;
    using Base::shortName;
    using Base::fullName;

    constexpr From1(Graph& gr,
                    std::shared_ptr<PP> pp = nullptr)
        : Base {gr, pp}
    {
        ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
    }
};


}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_FROM_11_H
