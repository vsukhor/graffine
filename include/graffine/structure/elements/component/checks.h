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
check_connectivity_vetrex_edge() const -> Vertex*
{
    if constexpr (beLoud)
        jot<false>("connectivity_vetrex_edge ... ");

    for (const auto& v : vv.get())
        for (const auto& s : v->edges()) {

            GRAFFINE_ASSERT(s.w < num_edges(),
                   "EgDescr.w", s.w, " >= num_edges", num_edges());
            GRAFFINE_ASSERT(End::is_valid(s.e), "End is not valid: ", s.e);

            if (edge(s.w).vertex_at_inner(s.e) != v.get()) {

                jot<false>("Connection is not valid for vertex ");
                v->print("");
                jot<false>("and edge at inner end ", End::str(s.e),
                           "           ");
                edge(s.w).print("");
                edge(s.w).vertex_at_inner(s.e)->print("found");

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
    // Check continuity of idc values.

    if constexpr (beLoud)
        jot<false>("chain_idc ... ");

    std::vector<ChIdC> idcs(num_chains());
    std::iota(idcs.begin(), idcs.end(), 0);

    for (const auto& i : idcs) {

        [[maybe_unused]] bool found{};
        for (const auto& w : ww)
            if (cn[w].idc == i) {

                found = true;
                break;
            }

        GRAFFINE_ASSERT(found, "idc ", i, " not found in component ", ind);
    }

    // Check max idc.

    ChIdC maxval{};
    for (const auto& w : ww)
        if (cn[w].idc > maxval)
            maxval = cn[w].idc;

    GRAFFINE_ASSERT(!num_chains() || maxval < num_chains(),
           "Max idc ", maxval, " >= num_chains ", num_chains());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_edge_indc() const
{
    // Check continuity of indc values.

    if constexpr (beLoud)
        jot<false>("edge_indc ... ");

    std::vector<ChIdC> idcs(num_edges());
    std::iota(idcs.begin(), idcs.end(), 0);

    for (const auto& i : idcs) {

        bool found{};
        for (const auto& w : ww)
            for (const auto& eg : cn[w].g)
                if (eg.indc() == i()) {

                    found = true;
                    break;
                }
        GRAFFINE_ENSURE(found, "indc ", i, " not found in component ", ind);
    }

    // Check max indc.

    ChIdC maxval{};
    for (const auto& w : ww)
        for (const auto& eg : cn[w].g)
            if (eg.indc() > maxval())
                maxval = cn[w].idc;

    GRAFFINE_ENSURE(!num_edges() || maxval < num_edges(),
           "Max indc ", maxval, " >= num_edges ", num_edges());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_egl() const
{
    if constexpr (beLoud)
        jot<false>("egl ... ");

    GRAFFINE_ASSERT(egl.size() == num_edges(),
           "egl size is incorrect for component ", ind);

    for (EgIdC k{}; k < egl.size(); ++k)
        egl[k].check(edge(k), ind, k);

    for (const auto& w : ww)
        for (const auto& eg : cn[w].g) {

            GRAFFINE_ASSERT(egl.size() > eg.indc(),
                   "egl size", egl.size(), " is <= for component ", eg.indc);

            egl[eg.indc].check(eg, ind, eg.indc);
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

    GRAFFINE_ENSURE(chis.num_chains() == num_chains(),
                    "Incorrect number of chains: ",
                    chis.num_chains(), " != ", num_chains());

    if (!num_chains())
        return false;

    warn_if(chis.num({1, 1}) > 1,
            "Found multiple unconnected linear chains in cmp ", ind);

    if (chis.num({1, 1})) {
        [[maybe_unused]] const auto u = chis.cn_11()[0];
        // allow several unconnected chains to handle irregular components
        GRAFFINE_ASSERT(cn[u].is_unconnected_linear(),
               "Error 11 for w ", u);
    }

    warn_if(chis.num({2, 2}) > 1,
            "Found multiple unconnected cycle chains in cmp ", ind);

    if (chis.num({2, 2})) {
        [[maybe_unused]] const auto u = chis.cn_22()[0];
        // allow several unconnected chains to handle irregular components
        GRAFFINE_ASSERT(cn[u].is_unconnected_cycle(),
               "Error 22 for w ", u);
    }
    for ([[maybe_unused]] const auto& u : chis.cn_33())
        GRAFFINE_ASSERT(cn[u].ngs[End::A].num() == 2 && cn[u].ngs[End::B].num() == 2,
               "Error 33 for w ", u);

    for ([[maybe_unused]] const auto& u : chis.cn_44())
        GRAFFINE_ASSERT(cn[u].ngs[End::A].num() == 3 && cn[u].ngs[End::B].num() == 3,
               "Error 44 for w ", u);

    for ([[maybe_unused]] const auto& u : chis.cn_13())
        GRAFFINE_ASSERT(!cn[u.w].ngs[u.e].num() && cn[u.w].ngs[End::opp(u.e)].num() == 2,
               "Error 13 for w ", u.str_short());

    for ([[maybe_unused]] const auto& u : chis.cn_14())
        GRAFFINE_ASSERT(!cn[u.w].ngs[u.e].num() && cn[u.w].ngs[End::opp(u.e)].num() == 3,
               "Error 14 for w ", u.str_short());

    for ([[maybe_unused]] const auto& u : chis.cn_34())
        GRAFFINE_ASSERT(cn[u.w].ngs[u.e].num() == 2 &&
                   cn[u.w].ngs[End::opp(u.e)].num() == 3,
               "Error 34 for w ", u.str_short());

    return false;
}

template <typename Ch>
template<bool beLoud>
bool Component<Ch>::
check_vertex_data() const
{
    if constexpr (beLoud)
        jot<false>("vertex_c ... ");

    GRAFFINE_ASSERT(!vv.check_c(ind));
    GRAFFINE_ASSERT(!check_connectivity_vetrex_edge<beLoud>());

    return false;
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
