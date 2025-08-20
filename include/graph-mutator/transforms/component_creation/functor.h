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
 * @file functor.h
 * @brief Contains template for creation of specific disconnected components.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_COMPONENT_CREATION_FUNCTOR_H
#define GRAPH_MUTATOR_COMPONENT_CREATION_FUNCTOR_H

#include "../../definitions.h"
#include "../../structure/basic/graph.h"


namespace graph_mutator::component_creation {

/**
 * @brief Functor class creating specific graph component.
 * @details The component is created as a single chain.
 * @tparam G Graph to which the transformation is applied.
 */
template<typename G>
struct Functor {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr const char* fullName {"component_creation"};
    static constexpr const char* shortName {"cmp_cr"};

    using Graph = G;
    using Chain = Graph::Chain;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit Functor(Graph& gr);

    /**
     * @brief Function call operator executing the transformation.
     * @param s Length (in edges) of the chain forming the new component.
     */
    auto operator()(EgId s) noexcept -> Res;

protected:

    Graph& gr;  ///< Reference to the graph object.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
Functor<G>::
Functor(Graph& gr)
    : gr {gr}
{}


template<typename G>
auto Functor<G>::
operator()(const EgId s) noexcept -> Res
{
   if constexpr (verboseF) {
        log_(colorcodes::GREEN, "Component creation: ",
             colorcodes::RESET, "single chain, size ", s);
        log_("");
    }

    gr.add_single_chain_component(s);     // calls gr.update() inside

    if constexpr (verboseF) {
        gr.cn[gr.ind_last_chain()].print(shortName, s, " produces");
        log_("");
    }

    return {gr.ind_last_cmpt()};
}


}  // namespace graph_mutator::component_creation

#endif  // GRAPH_MUTATOR_COMPONENT_CREATION_FUNCTOR_H
