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
 * \file checks.h
 * \brief Consistency checks of a component.
 * \details Part of the implementation of component.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHECKS_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHECKS_H

#include "graffine/structure/elements/component.h"

namespace graffine::structure::detail {

template <typename Ch>
template <typename... T>
void Component<Ch>::
ensure(
    bool cond,
    T&&... message) const
{
    if (!cond) [[unlikely]]
        utils::raise<invariant_error>(nullptr, std::source_location::current(),
                                      message...);
}

// -----------------------------------------------------------------------------
// INTERNAL TESTS
// -----------------------------------------------------------------------------

template <typename Ch>
template<bool beLoud>
auto Component<Ch>::
check_connectivity_vertex_edge() const -> Vertex*
{
    if constexpr (beLoud)
        jot<false>("connectivity_vertex_edge ... ");

    for (const auto& v : vv.get())
        for (const auto& s : v->edges()) {

            if (s.w >= num_edges() || !End::is_valid(s.e)) {
                if constexpr (beLoud)
                    jot("Edge descriptor of vertex ", v->get_id(),
                        " is not valid: edge ", s.w, " end ", End::str(s.e));
                return v.get();
            }

            if (edge(s.w).vertex_at_inner(s.e) != v.get()) {

                if constexpr (beLoud) {
                    jot<false>("Connection is not valid for vertex ");
                    v->print("");
                    jot<false>("and edge at inner end ", End::str(s.e),
                               "           ");
                    edge(s.w).print("");
                    if (const auto u = edge(s.w).vertex_at_inner(s.e))
                        u->print("found");
                }

                return v.get();
            }
        }
    return nullptr;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_chain_idc() const
{
    // Check that idc values are 0, 1, ... num_chains() - 1.

    if constexpr (beLoud)
        jot<false>("chain_idc ... ");

    std::vector<bool> found(num_chains());
    for (const auto& w : ww) {
        // the other checks rely on the chains being found
        if (w >= cn.size() || cn[w].c != ind) {
            if constexpr (beLoud)
                jot("chain ", w, " is not in component ", ind);
            return true;
        }
        const auto i = cn[w].idc();
        if (i >= found.size() || found[i]) {
            if constexpr (beLoud)
                jot("idc ", cn[w].idc, " of chain ", w, " is out of range ",
                    "or repeated in component ", ind);
            return true;
        }
        found[i] = true;
    }

    // As many chains as idc values, so each value is found.
    if (ww.size() != num_chains()) {
        if constexpr (beLoud)
            jot("idc values do not cover the chains of component ", ind);
        return true;
    }

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_edge_indc() const
{
    // Check that indc values are 0, 1, ... num_edges() - 1.

    if constexpr (beLoud)
        jot<false>("edge_indc ... ");

    std::vector<bool> found(num_edges());
    std::size_t n {};
    for (const auto& w : ww)
        for (const auto& eg : cn[w].g) {
            const auto i = eg.indc();
            if (i >= found.size() || found[i]) {
                if constexpr (beLoud)
                    jot("indc ", eg.indc, " of edge ", eg.ind, " is out of ",
                        "range or repeated in component ", ind);
                return true;
            }
            found[i] = true;
            ++n;
        }

    // As many edges as indc values, so each value is found.
    if (n != num_edges()) {
        if constexpr (beLoud)
            jot("indc values do not cover the edges of component ", ind);
        return true;
    }

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_egl() const
{
    if constexpr (beLoud)
        jot<false>("egl ... ");

    if (egl.size() != num_edges()) {
        if constexpr (beLoud)
            jot("egl size is incorrect for component ", ind);
        return true;
    }

    // edge(k) is found through egl[k], so egl[k] is checked to refer to an
    // edge of the component first
    for (EgIdC k{}; k < egl.size(); ++k)
        if (egl[k].w >= cn.size() || cn[egl[k].w].c != ind ||
            egl[k].a >= cn[egl[k].w].length() ||
            !egl[k].matches(edge(k))) {
            if constexpr (beLoud)
                jot("egl[", k, "] does not match its edge in component ", ind);
            return true;
        }

    for (const auto& w : ww)
        for (const auto& eg : cn[w].g)
            if (egl.size() <= eg.indc() || !egl[eg.indc].matches(eg)) {
                if constexpr (beLoud)
                    jot("egl does not match edge ", eg.ind, " of chain ", w,
                        " in component ", ind);
                return true;
            }

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_chis() const
{
    if constexpr (beLoud)
        jot<false>("chis ... ");

    /// Reports a failure if \p cond is false.
    const auto failed = [this](const bool cond, const auto&... msg)
    {
        if constexpr (beLoud)
            if (!cond)
                jot(msg..., " in component ", ind);
        return !cond;
    };

    if (failed(chis.num_chains() == num_chains(),
               "Incorrect number of chains: ",
               chis.num_chains(), " != ", num_chains()))
        return true;

    if (!num_chains())
        return false;

    warn_if(chis.num({1, 1}) > 1,
            "Found multiple unconnected linear chains in cmp ", ind);

    if (chis.num({1, 1})) {
        const auto u = chis.cn_11()[0];
        // allow several unconnected chains to handle irregular components
        if (failed(cn[u].is_unconnected_linear(), "Error 11 for w ", u))
            return true;
    }

    warn_if(chis.num({2, 2}) > 1,
            "Found multiple unconnected cycle chains in cmp ", ind);

    if (chis.num({2, 2})) {
        const auto u = chis.cn_22()[0];
        // allow several unconnected chains to handle irregular components
        if (failed(cn[u].is_unconnected_cycle(), "Error 22 for w ", u))
            return true;
    }
    for (const auto& u : chis.cn_33())
        if (failed(cn[u].ngs[End::A].num() == 2 && cn[u].ngs[End::B].num() == 2,
                   "Error 33 for w ", u))
            return true;

    for (const auto& u : chis.cn_44())
        if (failed(cn[u].ngs[End::A].num() == 3 && cn[u].ngs[End::B].num() == 3,
                   "Error 44 for w ", u))
            return true;

    for (const auto& u : chis.cn_13())
        if (failed(!cn[u.w].ngs[u.e].num() &&
                   cn[u.w].ngs[End::opp(u.e)].num() == 2,
                   "Error 13 for w ", u.str_short()))
            return true;

    for (const auto& u : chis.cn_14())
        if (failed(!cn[u.w].ngs[u.e].num() &&
                   cn[u.w].ngs[End::opp(u.e)].num() == 3,
                   "Error 14 for w ", u.str_short()))
            return true;

    for (const auto& u : chis.cn_34())
        if (failed(cn[u.w].ngs[u.e].num() == 2 &&
                   cn[u.w].ngs[End::opp(u.e)].num() == 3,
                   "Error 34 for w ", u.str_short()))
            return true;

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_chain_lengths() const
{
    if constexpr (beLoud)
        jot<false>("chain lengths ... ");

    // Empty linear chains are allowed: components are assembled from them.
    for (const auto& w : ww) {
        const auto& m = cn[w];
        if (m.is_cycle() && m.length() < Chain::minCycleLength) {
            if constexpr (beLoud)
                jot(colorcodes::BOLDRED, "cycle chain ", w, " of length ",
                    m.length(), " is shorter than ", Chain::minCycleLength,
                    colorcodes::RESET);
            return true;
        }
    }

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_vertex_data() const
{
    if constexpr (beLoud)
        jot<false>("vertex_c ... ");

    return vv.check_c(ind) ||
           check_connectivity_vertex_edge<beLoud>();
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check(const std::string_view s) const
{
    if constexpr (beLoud)
        jot<false>("Checking component ", ind, " ", s, ": ");

    if (int err = check_chain_idc<beLoud>(); err)
        return err;
    if (int err = check_edge_indc<beLoud>(); err)
        return err;
    if (int err = check_egl<beLoud>(); err)
        return err;
    if (int err = check_chis<beLoud>(); err)
        return err;
    if (int err = check_chain_lengths<beLoud>(); err)
        return err;
    if (int err = check_vertex_data<beLoud>(); err)
        return err;

    if constexpr (beLoud)
        jot("Success!");

    return false;
}

// -----------------------------------------------------------------------------
// PRINTING
// -----------------------------------------------------------------------------

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_CHECKS_H
