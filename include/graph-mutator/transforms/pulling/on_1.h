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
 * @file on_1.h
 * @brief Contains specialization of the functor for pulling graph
 * transformation to vertices of degree 1.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_PULLING_FUNCTOR_1_H
#define GRAPH_MUTATOR_PULLING_FUNCTOR_1_H

#include "../../definitions.h"
#include "../../string_ops.h"
#include "../../structure/basic/graph.h"
#include "common.h"
#include "functor_base.h"
#include "log.h"
#include "paths.h"


namespace graph_mutator::pulling {

/**
 * @brief Functor for the edge pulling initiated from a vertex of degree 1.
 * @tparam Dir Direction of the pull operation.
 * @tparam G Graph to which the pull operation is applied.
 */
template<Orientation Dir,
         typename G>
struct On<Deg1, Dir, G>
    : public FunctorBase<G> {

    static constexpr auto Direction = Dir;

    /// Degree of the vertex from which the pull operation is initiated.
    static constexpr auto D = Deg1;
    static_assert(is_pullable_degree<D>);

    static constexpr auto d = string_ops::str1<D>;

    static constexpr auto shortName
        {string_ops::shName<d, 1, 'p', 'u', '_', signchar<Dir>>};
    static constexpr auto fullName
        {string_ops::shName<d, 1,
         'P', 'u', 'l', 'l', 'i', 'n', 'g', ' ', 'f', 'r', 'o', 'm', ' ',
         'V', 'e', 'r', 't', 'e', 'x', ' ', 'D', 'e', 'g', ' ', signchar<Dir>>};

    using Graph = G;
    using Cmpt = Graph::Cmpt;
    using Chain = Graph::Chain;
    using Base = FunctorBase<G>;
    using L = Log<On<D, Dir, Graph>>;
    using ResT = CmpId;
    using Res = std::array<ResT, 1>;

    /**
     * @brief Constructs this functor object based on the Graph instance.
     * @param gr Graph to which the transformation is applied.
     */
    explicit On(Graph& gr);

    /**
     * @brief Pulls a vertex of degree 1.
     * The path starts with a driver edge and ends with a source edge.
     * @note The path is expected to be a valid path over the graph.
     * @param pp Path to the vertex to pull.
     * @param n Number of steps to pull over.
     * @return Result containing the component ID after the pull operation.
     */
    auto operator()(
        Paths<Cmpt>& pp,
        int n
    ) noexcept -> Res;

protected:

    using Base::cn;

    void check_path(
        const Paths<Cmpt>& pp,
        int n
    ) const noexcept;

private:

    L log;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<Orientation Dir,
         typename G>
On<Deg1, Dir, G>::
On(Graph& gr)
    : Base {gr}
    , log {*this}
{}


template<Orientation Dir,
         typename G>
auto On<Deg1, Dir, G>::
operator()(
    Paths<Cmpt>& pp,
    int n
) noexcept -> Res
{
    if constexpr (verboseF)
        log.before(pp, n);

    check_path(pp, n);

    this->pull_free_end(pp, n);

    if constexpr (verboseF)
        log.after(pp);

    return {pp.cmp->ind};
}


template<Orientation Dir,
         typename G>
void On<Deg1, Dir, G>::
check_path(
    const Paths<Cmpt>& pp,
    const int n
) const noexcept
{
    const auto c = pp.cmp;
    const auto p = pp.pth;
    const auto wS = pp.s.w;
    const auto eS = pp.s.e;
    const auto wD = pp.d.w;
    const auto eD = pp.d.end_slot(c->chain(wD)).e;

    ASSERT(c->chain(wD).ngs[eD].num() == 0, "Driver end is not disconnected");

    const auto sourceIsCycle = cn[wS].is_cycle();

    ASSERT(sourceIsCycle ||
           (!sourceIsCycle && c->chain(wS).ngs[eS].num() == 0),
           "Source end is not disconnected");

    const auto lenS = pp.length_over_source_chain();

    ASSERT((!sourceIsCycle &&
            lenS >= static_cast<EgId>(n)) ||
           (sourceIsCycle &&
            lenS >= Chain::minCycleLength + static_cast<EgId>(n)),
           "Pulling over the track longer than the source chain");
}


}  // namespace graph_mutator::pulling

#endif  // GRAPH_MUTATOR_PULLING_FUNCTOR_1_H
