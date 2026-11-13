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
 * \file edges_in_component.h
 * \brief Table of the edge descriptors of a component.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_COMPONENT_H
#define GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_COMPONENT_H


#include "graffine/definitions.h"
#include "graffine/structure/containers/sequence.h"
#include "graffine/structure/descriptors/edge_in_component.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

#include <string>
#include <string_view>
#include <vector>


namespace graffine::structure::containers {

/** Table of the edge descriptors of a component, indexed by EgIdC.
 * \details The edges themselves are stored in the chains. This table maps
 * the component-wide index of each edge to its location (chain and
 * position) and to its other ids, see descriptors::EdgeInComponent.
 * \tparam C Component type.
 */
template<typename C>
struct EdgeDescriptorsInComponent
    : Sequence<std::vector<descriptors::EdgeInComponent<typename C::Edge>>>
{
    using Component = C;
    using Edge = Component::Edge;
    using EdgeDescr = descriptors::EdgeInComponent<Edge>;
    using Base = Sequence<std::vector<EdgeDescr>>;
    using Self = EdgeDescriptorsInComponent;
    using Vertices = Component::Vertices;
    using Chain = Component::Chain;
    using Chains = Component::Chains;

    /// Descriptor of the edge with component-wide index \p i.
    constexpr auto operator[](const EgIdC i) -> EdgeDescr& { return m_data[i()]; }
    /// Descriptor of the edge with component-wide index \p i.
    constexpr auto operator[](const std::size_t i) -> EdgeDescr& { return m_data[i]; }
    /// Descriptor of the edge with component-wide index \p i.
    constexpr auto operator[](const EgIdC i) const -> const EdgeDescr& { return m_data[i()]; }
    /// Descriptor of the edge with component-wide index \p i.
    constexpr auto operator[](const std::size_t i) const -> const EdgeDescr& { return m_data[i]; }

    /// Rebuilds the table from the edges of the chains of component \p cmp.
    void set(const Component& cmp);

    /// Appends the descriptor of edge \p g; its EgIdC must equal the
    /// current table size.
    void append(const Edge& g);
    /// Appends the descriptors of the edges of chain \p m, in chain order.
    void append(const Chain& m);
    /// Appends the descriptors of another table, e.g. when merging
    /// components.
    void append(Self&& egl) noexcept;

    /** Removes the descriptor of edge \p eg.
     * \details The last edge of the component takes over the EgIdC of
     * \p eg: its descriptor moves into the vacated entry, and the edge and
     * its vertices are reindexed accordingly.
     * \param eg Edge to remove.
     * \param cn Chains of the graph.
     * \param vv Vertices of the component.
     */
    void remove(const Edge& eg, Chains& cn, Vertices& vv);

    /// Removes all the descriptors.
    void clear() { m_data.clear(); }

    /** Checks that the descriptor at the index of edge \p eg matches it.
     * \param eg Edge.
     * \param ic Host component, for the error message.
     * \throws invariant_error If the descriptor disagrees with the edge,
     * or the index is out of range.
     */
    void check(const Edge& eg, CmpId ic) const;

private:

    using Base::m_data;
};

// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename C>
void EdgeDescriptorsInComponent<C>::
set(const Component& cmp)
{
    std::size_t numEdges {};
    for (const auto& w : cmp.ww)
        numEdges += cmp.chain(w).length();

    this->resize(numEdges);

    for (const auto& w : cmp.ww)
        for (const auto& g : cmp.chain(w).g) {

            GRAFFINE_ASSERT(g.indc < numEdges, "In component ", cmp.ind,
                   " g.indc ", g.indc, " >= numEdges ", numEdges);

            m_data[g.indc()] = EdgeDescr{g};
        }
}

template<typename C>
void EdgeDescriptorsInComponent<C>::
append(Self&& egl) noexcept
{
    std::move(egl.begin(), egl.end(), std::back_inserter(m_data));
}

template<typename C>
void EdgeDescriptorsInComponent<C>::
append(const Chain& m)
{
    for (const auto& g : m.g)
        append(g);
}

template<typename C>
void EdgeDescriptorsInComponent<C>::
append(const Edge& g)
{
        m_data.push_back(EdgeDescr{g});
}

template<typename C>
void EdgeDescriptorsInComponent<C>::
remove(const Edge& eg,
       Chains& cn,
       Vertices& vv)
{
    if (const auto& b = this->back();
        eg.ind != b.i) {

        auto& last = cn[b.w].g[b.a];
        vv.reind_edge(last, eg.indc);
        last.indc = eg.indc;
        m_data[eg.indc()] = EdgeDescr{last};
    }

    this->pop_back();
}

template<typename C>
void EdgeDescriptorsInComponent<C>::
check(const Edge& eg,
      const CmpId ic) const
{
    GRAFFINE_ENSURE(eg.indc < m_data.size(), "indc ", eg.indc, " of edge ",
                    eg.ind, " out of range in component ", ic);

    m_data[eg.indc()].check(eg, ic, eg.indc);
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_COMPONENT_H