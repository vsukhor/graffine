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
 * \brief Contains template for deletion of specific connected components.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_COMPONENT_DELETION_FUNCTOR_H
#define GRAFFINE_TRANSFORMS_COMPONENT_DELETION_FUNCTOR_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/common.h"

#include <memory>
#include <type_traits>

namespace graffine::transforms {

template<typename G>
struct Trait<TransformType::ComponentDeletion, G>
{
    using Graph = G;
    using Chain = Graph::Chain;
    using Chains = Graph::Chains;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using Vertex = Graph::Vertex;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    static constexpr const char* fullName {"component_deletion"};
    static constexpr const char* shortName {"cmp_de"};

    static constexpr auto Type = TransformType::ComponentDeletion;
};


/// Enables deletion of an unconnected component form the graph.
namespace component_deletion {

template<typename G>
using Trait = transforms::Trait<TransformType::ComponentDeletion, G>;

/**
 * Functor class deleting specific graph component.
 * \tparam G Graph to which the transformation is applied.
 */
template<typename G,
         typename PP = void>
struct Functor
{
    using Trait = component_deletion::Trait<G>;
    using Graph = Trait::Graph;
    using Chain = Graph::Chain;
    using End = Chain::End;  ///< Chain ends.
    using EndSlot = Chain::EndSlot;
    using PropertyProcessor = PP;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

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
     * \param[in] c Index of the connected component to be deleted.
     */
    auto operator()(CmpId c) noexcept -> Res;

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
operator()(const CmpId c) noexcept -> Res
{
    auto& cn = gr.chains();
    auto& ct = gr.compts();

   if constexpr (verboseF) {
        const auto nch = ct[c].num_chains();
        const auto chs = nch > 1 ? " chains, " : " chain, ";
        const auto neg = ct[c].num_edges();
        const auto egs = neg > 1 ? " edges " : " edge ";
        jot(colorcodes::GREEN, "Component deletion: ",
                   colorcodes::YELLOW, c, colorcodes::RESET,
                   " (", nch, chs, neg, egs, ")\n");
        ct[c].print("to delete ");
    }


    if constexpr (!std::is_void_v<PP>)
        updateProperties->componentDeletion.on_start();

    auto ww = ct[c].ww;  // copy
    while (ww.size()) {

        const auto w = ww.back();
        ww.pop_back();
        auto& m = cn[w];

        if constexpr (verboseF)
            m.print("deleting");

        if (!m.is_unconnected_cycle()) {
            if (m.is_connected_at(End::A))
                gr.remove_slot_from_neigs(EndSlot{w, End::A});
            if (m.is_connected_at(End::B))
                gr.remove_slot_from_neigs(EndSlot{w, End::B});
        }

        while (m.length()) {
            auto& p = m.g.back();  // edge to get removed
            if (p.ind != gr.num_edges()-1) {
                auto& plast = gr.edge_last();
                plast.ind = p.ind;
                m.g.pop_back();
                gr.set_num_edges(gr.num_edges()-1);
                if (plast.c != c) {
                    ct[plast.c].set_edges();
                    ct[plast.c].set_egl();
                }
            }
            else {
                m.g.pop_back();
                gr.set_num_edges(gr.num_edges()-1);
            }
            gr.update_edge_descriptors();
        }
        const auto last = gr.ind_last_chain();
        if (w != last) {
            if (auto ii = std::find(ww.begin(), ww.end(), last);
                ii != ww.end())
                *ii = w;
            gr.chain(last).c == c
                // gr.compt(c).ww is corrupt, so updateChis=false
                ? gr.template rename_chain<false>(last, w)
                : gr.template rename_chain<true>(last, w);
        }
        cn.pop_back();
        gr.update_edge_descriptors();
    }

    // the last chain in the component was just removed, so ct[c] is empty
    if (c != gr.ind_last_cmpt()) {
        // rename components to account for the one removed
        ct.back().set_ind(c);
        ct[c] = std::move(ct.back());
    }
    ct.pop_back();
    gr.update();

    Res res {c};

    if constexpr (!std::is_void_v<PP>)
        updateProperties->componentDeletion.on_end();

    if constexpr (verboseF) {
        jot(
            "\ncomponent ", c, " deleted  ==>  new graph size: ",
            gr.num_edges(), " edges in ",
            gr.num_chains(), " chains, ",
            gr.num_compts(), " components"
        );
        jot("");
    }

    return res;
}


}  // namespace component_deletion
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_COMPONENT_DELETION_FUNCTOR_H
