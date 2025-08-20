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
 * @brief Contains template for deletion of specific disconnected components.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_COMPONENT_DELETION_FUNCTOR_H
#define GRAPH_MUTATOR_COMPONENT_DELETION_FUNCTOR_H

#include "../../definitions.h"
#include "../../structure/basic/graph.h"


namespace graph_mutator::component_deletion {

/**
 * @brief Functor class deleting specific graph component.
 * @tparam G Graph to which the transformation is applied.
 */
template<typename G>
struct Functor {

    static_assert(std::is_base_of_v<structure::basic::GraphBase, G>);

    static constexpr const char* fullName {"component_deletion"};
    static constexpr const char* shortName {"cmp_de"};

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;  ///< Chain ends.
    using EndSlot = Chain::EndSlot;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Constructs a Functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit Functor(Graph& gr);

    /**
     * @brief Function call operator executing the transformation.
     * @param c Index of the disconnected component to be deleted.
     */
    auto operator()(CmpId c) noexcept -> Res;

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
operator()(const CmpId c) noexcept -> Res
{
    auto& cn = gr.cn;
    auto& ct = gr.ct;
    auto& edgenum = gr.edgenum;     // ref !

   if constexpr (verboseF) {
        const auto nch = ct[c].num_chains();
        const auto chs = nch > 1 ? " chains, " : " chain, ";
        const auto neg = ct[c].num_edges();
        const auto egs = neg > 1 ? " edges " : " edge ";
        log_(colorcodes::GREEN, "Component deletion: ",
                   colorcodes::YELLOW, c, colorcodes::RESET,
                   " (", nch, chs, neg, egs, ")\n");
        ct[c].print("to delete ");
    }

    auto ww = ct[c].ww;  // copy
    while (ww.size()) {

        const auto w = ww.back();
        ww.pop_back();
        auto& m = cn[w];

        if constexpr (verboseF)
            m.print("deleting");

        if (!m.is_disconnected_cycle()) {
            if (m.is_connected_at(Ends::A))
                gr.remove_slot_from_neigs(EndSlot{w, Ends::A});
            if (m.is_connected_at(Ends::B))
                gr.remove_slot_from_neigs(EndSlot{w, Ends::B});
        }

        while (m.length()) {
            auto& p = m.g.back();
            if (p.ind != edgenum-1) {
                const auto wLast = gr.glm.back();
                const auto aLast = gr.gla.back();
                auto& plast = cn[wLast].g[aLast];
                plast.ind = p.ind;
                m.g.pop_back();
                edgenum--;
                ct[plast.c].set_edges();
                ct[plast.c].set_gl();
            }
            else {
                m.g.pop_back();
                edgenum--;
            }
            gr.make_indma();
        }

        if (w != gr.ind_last_chain()) {
            if (auto ii = std::find(ww.begin(), ww.end(), gr.ind_last_chain());
                ii != ww.end())
                *ii = w;
            gr.rename_chain(gr.ind_last_chain(), w);
        }
        cn.pop_back();
        gr.make_indma();
    }

    // the last chain in the component was just removed, so ct[c] is empty
    if (c != gr.ind_last_cmpt()) {
        // rename components to account for the one removed
        ct.back().set_ind(c);
        ct[c] = std::move(ct.back());
    }
    ct.pop_back();
    gr.update();

    if constexpr (verboseF) {
        log_(
            "\ncomponent ", c, " deleted  ==>  new graph size: ",
            edgenum, " edges in ",
            gr.chain_num(), " chains, ",
            gr.cmpt_num(), " components"
        );
        log_("");
    }

    return {c};
}


}  // namespace graph_mutator::component_deletion

#endif  // GRAPH_MUTATOR_COMPONENT_DELETION_FUNCTOR_H
