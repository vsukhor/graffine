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
 * \file plan.h
 * \brief Planning hooks: the outcome of a transformation, before it is applied.
 * \details Each transformation describes its outcome, in terms of the graph
 * before it, by a const member function plan() taking the same arguments as
 * applicability() and apply(). The plan resolves the arguments as apply()
 * does, e.g. a vertex to its slot, and the dispatching functors to the
 * transformation they dispatch to. The plan types are defined per family,
 * e.g. vertex_split::Plan.
 *
 * A property processor may provide, in the hooks of a family, an overload
 * \code
 * bool plan(const vertex_split::Plan<G>& p);
 * \endcode
 * Transformations applied with operator() or try_apply() call it after their
 * applicability check and before changing the graph. If it returns false,
 * the transformation is vetoed: the graph is left untouched, try_apply()
 * returns the rejection \ref vetoed, and operator() throws precondition_error.
 * apply(), which skips the checks, skips the planning hook too.
 *
 * The hook is called for transformations applied by the client only: the
 * inner transformations of a composed one (see context.h) are part of its
 * plan, which describes the whole outcome. Edge splits are planned as the
 * edge creations performing them, by the hooks of edgeCreation.
 *
 * Processors without planning hooks, and transformations without processor,
 * do not compute the plans.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PLAN_H
#define GRAFFINE_TRANSFORMS_PLAN_H

#include "graffine/transforms/common.h"
#include "graffine/transforms/context.h"

#include <algorithm>
#include <concepts>
#include <type_traits>
#include <vector>

namespace graffine::transforms {

/// Reason of the rejection of a transformation vetoed by a planning hook.
inline constexpr const char* vetoed {"vetoed by the property processor"};

namespace plans {

/**
 * Whether processor \p PP has the planning hook for plans of type \p P.
 * \details P::hooks(pp) selects the hooks of the family of P in \p pp.
 * A named concept, as nvcc misjudges some inline requires-expressions
 * (see hooks in context.h).
 */
template<typename PP,
         typename P>
concept Hooked = requires (PP& pp, const P& p) {
    { P::hooks(pp).plan(p) } -> std::convertible_to<bool>; };

/**
 * Calls the planning hook of the processor of transformation \p d for its
 * arguments \p args, if the processor has one and \p d is applied by the
 * client.
 * \return The rejection \ref vetoed if the hook vetoes the transformation.
 */
template<typename D,
         typename... A>
auto vet(
    const D& d,
    const A&... args
) -> Applicability
{
    using PP = typename D::PropertyProcessor;

    if constexpr (!std::is_void_v<PP>) {
        using P = decltype(d.plan(args...));
        if constexpr (Hooked<PP, P>) {
            auto* const pp = d.processor();
            if (pp && current_context().empty() && !P::hooks(*pp).plan(d.plan(args...)))
                return reject(vetoed);
        }
    }

    return {};
}

/// Vertex at chain end \p s of graph \p gr.
template<typename G>
auto vertex_at(
    const G& gr,
    const typename G::EndSlot& s
) -> const typename G::Vertex&
{
    return *gr.chain(s.w).vertex_at_end(s.e);
}

/// Vertex at position \p s inside a chain of graph \p gr; positions 0 and
/// the chain length denote the chain ends.
template<typename G>
auto vertex_at(
    const G& gr,
    const typename G::BulkSlot& s
) -> const typename G::Vertex&
{
    using End = G::Chain::End;
    const auto [w, a] = s.we();
    const auto& m = gr.chain(w);

    return a == 0 ? *m.g[0].vertex_at_outer(End::A)
                  : *m.g[a - 1].vertex_at_outer(End::B);
}

/// Boundary vertex of unconnected cycle \p w of graph \p gr.
template<typename G>
auto vertex_at(
    const G& gr,
    const ChIdG w
) -> const typename G::Vertex&
{
    return *gr.chain(w).vertex_at_end(G::Chain::End::A);
}

/// Graph-wide indexes of the edges at vertex \p v of graph \p gr.
template<typename G>
auto edges_at(
    const G& gr,
    const typename G::Vertex& v
) -> std::vector<EgIdG>
{
    const auto& cmp = gr.compt(v.get_c());

    std::vector<EgIdG> ii;
    for (const auto& egs : v.edges())
        ii.push_back(cmp.get_egl(egs.indc()).i);

    return ii;
}

/// Edges at vertex \p v of graph \p gr, other than those in range \p ii.
template<typename G,
         typename R>
auto edges_at_other_than(
    const G& gr,
    const typename G::Vertex& v,
    const R& ii
) -> std::vector<EgIdG>
{
    std::vector<EgIdG> oo;
    for (const auto i : edges_at(gr, v))
        if (std::find(ii.begin(), ii.end(), i) == ii.end())
            oo.push_back(i);

    return oo;
}

}  // namespace plans
}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_PLAN_H
