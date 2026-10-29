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
 * \file common.h
 * \brief Contains the interface shared by the transformation functors.
 * \details Defines the generic template for transformation traits, the
 * Applicable base providing checked application of the transformations,
 * the Rejection reported for inapplicable arguments, and the argument
 * checks shared by the transformations.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_COMMON_H
#define GRAFFINE_TRANSFORMS_COMMON_H

#include "graffine/definitions.h"
#include "graffine/utils/errors.h"

#include <expected>
#include <source_location>
#include <string>
#include <utility>

/// Functionality enabling transformations of the graph topology
namespace graffine::transforms {

/**
 * Reason why a transformation is not applicable to its arguments.
 * \details Holds a static string, so that rejections cost no allocation:
 * they are part of the regular flow of simulations proposing random moves.
 */
struct Rejection {
    const char* reason;
};

/// Outcome of checking whether a transformation can be applied.
using Applicability = std::expected<void, Rejection>;

/// Rejection of a transformation for \p reason.
constexpr auto reject(const char* reason) noexcept -> std::unexpected<Rejection>
{
    return std::unexpected(Rejection {reason});
}

/**
 * Interface shared by the transformation functors.
 * \details A transformation \p D implements, for each of its argument lists:
 * - \c applicability(args) const: checks the preconditions, at a cost
 *   independent of the graph size;
 * - \c apply(args): performs the transformation without checking them.
 *
 * On top of these, this base provides:
 * - \c operator()(args): checks, then applies; throws precondition_error
 *   if the transformation is not applicable;
 * - \c is_applicable(args): checks only;
 * - \c try_apply(args): checks, then applies; returns the rejection instead
 *   of throwing if the transformation is not applicable.
 *
 * Calling \c apply directly is only valid for arguments for which
 * \c is_applicable is true: it spares the check when this is known.
 * \tparam D Transformation functor type.
 */
template<typename D>
struct Applicable
{
    /// Applies the transformation, after checking its preconditions.
    /// \throws precondition_error If the transformation is not applicable.
    // DD defers the use of D, incomplete when this class is instantiated.
    template<typename... A,
             typename DD = D>
    auto operator()(A&&... args)
        -> decltype(std::declval<DD&>().apply(std::forward<A>(args)...))
    {
        if (const auto ok = self().applicability(std::as_const(args)...);
            !ok) [[unlikely]]
            utils::raise<precondition_error>(
                nullptr, std::source_location::current(),
                name(), ": ", ok.error().reason);

        return self().apply(std::forward<A>(args)...);
    }

    /// Whether the transformation is applicable to \p args.
    template<typename... A>
    auto is_applicable(const A&... args) const -> bool
    {
        return self().applicability(args...).has_value();
    }

    /// Applies the transformation if it is applicable to \p args.
    /// \return The result of the transformation, or the reason of rejection.
    template<typename... A,
             typename DD = D>
    auto try_apply(A&&... args)
        -> std::expected<decltype(std::declval<DD&>().apply(std::forward<A>(args)...)),
                         Rejection>
    {
        if (const auto ok = self().applicability(std::as_const(args)...); !ok)
            return std::unexpected(ok.error());

        return self().apply(std::forward<A>(args)...);
    }

private:

    constexpr auto self() noexcept -> D& { return static_cast<D&>(*this); }
    constexpr auto self() const noexcept -> const D&
    {
        return static_cast<const D&>(*this);
    }

    auto name() const -> std::string
    {
        if constexpr (requires { self().fullName; })
            return std::string {self().fullName};
        else if constexpr (requires { D::Trait::fullName; })
            return D::Trait::fullName;
        else
            return "transformation";
    }
};

/// Checks of the transformation arguments, shared by the transformations.
namespace checks {

/// Whether \p s refers to an end of an existing chain of \p gr.
template<typename G>
constexpr auto is_valid(
    const G& gr,
    const typename G::EndSlot& s
) noexcept -> bool
{
    return s.w < gr.num_chains() && G::End::is_valid(s.e);
}

/// Whether \p s refers to an internal vertex of an existing chain of \p gr.
template<typename G>
constexpr auto is_valid(
    const G& gr,
    const typename G::BulkSlot& s
) noexcept -> bool
{
    const auto [w, a] = s.we();
    return w < gr.num_chains() && a > 0 && a < gr.chain(w).length();
}

/// Whether \p w refers to an existing chain of \p gr.
template<typename G>
constexpr auto is_valid(
    const G& gr,
    const ChIdG w
) noexcept -> bool
{
    return w < gr.num_chains();
}

/// Whether \p ind refers to an existing edge of \p gr.
template<typename G>
constexpr auto is_valid_edge(
    const G& gr,
    const EgIdG ind
) noexcept -> bool
{
    return ind < gr.num_edges();
}

/**
 * Whether vertex \p v belongs to graph \p gr.
 * \details Follows the first edge of \p v back to its vertex: constant cost.
 */
template<typename G>
constexpr auto is_in_graph(
    const G& gr,
    const typename G::Vertex& v
) noexcept -> bool
{
    const auto c = v.get_c();
    if (!c.is_defined() || c >= gr.num_compts() || v.edges().num() == 0)
        return false;

    const auto& egs = *v.edges().begin();
    const auto& cmp = gr.compt(c);
    if (egs.indc() >= cmp.num_edges())
        return false;

    const auto& egl = cmp.get_egl(egs.indc());
    if (egl.w >= gr.num_chains() || egl.a >= gr.chain(egl.w).length())
        return false;

    return gr.chain(egl.w).g[egl.a()].vertex_at_inner(egs.e) == &v;
}

}  // namespace checks


/// Slots of vertices. The vertices must belong to the graph.
namespace locate {

/**
 * Chain end slot at vertex \p v, through the first edge of \p v.
 * \return Undefined slot if \p v is not at an end of that edge's chain.
 */
template<typename G>
constexpr auto end_slot(
    const G& gr,
    const typename G::Vertex& v
) -> typename G::EndSlot
{
    const auto& egs = *v.edges().begin();
    const auto& egl = gr.compt(v.get_c()).get_egl(egs.indc());

    return gr.chain(egl.w).end_slot(egl.a, egs.e);
}

/**
 * Bulk slot at vertex \p v, through the first edge of \p v.
 * \details Accounts for the orientation of the edge. Positions 0 and the
 * chain length denote the boundary vertex of an unconnected cycle.
 */
template<typename G>
constexpr auto bulk_slot(
    const G& gr,
    const typename G::Vertex& v
) -> typename G::BulkSlot
{
    const auto& egs = *v.edges().begin();
    const auto& egl = gr.compt(v.get_c()).get_egl(egs.indc());

    return gr.chain(egl.w).egEnd_to_bulkslot(egs.e, egl.a);
}

}  // namespace locate

enum TransformType
{
    ComponentCreation,
    ComponentDeletion,
    EdgeCreation,
    EdgeDeletion,
    EdgeMerger,
    EdgeSplit,
    VertexMerger,
    VertexSplit,
    Pulling
};

template<TransformType TT,
         typename Element>
struct Trait
{};

inline
auto degrees_to_string(
    const Degree d1,
    const Degree d2
) -> std::string
{
    return std::to_string(d1) + '+' + std::to_string(d2);
}

constexpr
auto make_name(
    const char* nameTrait,
    const std::string& dd = ""
) -> std::string
{
    return std::string(nameTrait) + dd;
}


}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_COMMON_H