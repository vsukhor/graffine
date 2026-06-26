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
 * \brief Contains template for creation of specific connected components.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_COMPONENT_CREATION_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_COMPONENT_CREATION_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"

#include <memory>
#include <type_traits>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::ComponentCreation, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    static constexpr const char* fullName {"component_creation"};
    static constexpr const char* shortName {"cmp_cr"};

    static constexpr auto Type = TransformType::ComponentCreation;
};


/// Enables creation of a new unconnected graph component.
namespace component_creation {

template<typename G>
using Trait = transforms::Trait<TransformType::ComponentCreation, G>;

/**
 * Functor class creating specific graph component.
 * \details The component is created as a single chain.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
struct Functor
{
    using Trait = component_creation::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using PropertyProcessor = PP;
    using ResT = Trait::ResT;
    using Res = Trait::Res;

    static constexpr auto fullName = Trait::fullName;
    static constexpr auto shortName = Trait::shortName;

    /**
     * Constructs a Functor object based on the Graph instance.
     * \param[in] gr Graph to which the transformation is applied.
     * \param[in] pp Processor for non-topological properties. If null,
     *               no processing is intended
     */
    explicit Functor(Graph& gr,
                     std::shared_ptr<PP> pp = nullptr);

    /**
     * Function call operator executing the transformation.
     * \param[in] s Length (in edges) of the chain forming the new component.
     */
    auto operator()(std::size_t s = 1) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the modified graph object.

    /// Processor for non-topological properties.
    std::shared_ptr<PP> updateProperties;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         typename PP>
Functor<G, PP>::
Functor(Graph& gr,
        std::shared_ptr<PP> pp)
    : gr {gr}
    , updateProperties {pp}
{
    ASSERT(!(std::is_void_v<PP> && pp), "Property processor is NULL");
}


template<typename G,
         typename PP>
auto Functor<G, PP>::
operator()(const std::size_t s) noexcept -> Res
{
   if constexpr (verboseF) {
        jot(colorcodes::GREEN, "Component creation: ",
            colorcodes::RESET, "single chain, size ", s);
        jot("");
    }

    if constexpr (!std::is_void_v<PP>)
        updateProperties->componentCreation.on_start();

    gr.add_single_chain_component(s);     // calls gr.update() inside

    Res res {gr.ind_last_cmpt()};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->componentCreation.on_end();

    if constexpr (verboseF) {
        gr.chain(gr.ind_last_chain()).print(shortName, s, " produces");
        jot("");
    }

    return res;
}

}  // namespace component_creation
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_COMPONENT_CREATION_FUNCTOR_H
