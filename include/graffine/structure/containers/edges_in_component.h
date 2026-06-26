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
 * \brief Defines classes managing collections of edges of the graph.
 * \details Adds edge-specific functionality to the Sequence container.
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

    constexpr auto operator[](const EgIdC i) -> EdgeDescr& { return m_data[i()]; }
    constexpr auto operator[](const std::size_t i) -> EdgeDescr& { return m_data[i]; }
    constexpr auto operator[](const EgIdC i) const -> const EdgeDescr& { return m_data[i()]; }
    constexpr auto operator[](const std::size_t i) const -> const EdgeDescr& { return m_data[i]; }

    void set(const Component& cmp);
    void append(const Edge& g);
    void append(const Chain& m);
    void append(Self&& egl) noexcept;
    void remove(const Edge& eg, Chains& cn, Vertices& vv);
    void clear() { m_data.clear(); }
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

            ASSERT(g.indc < numEdges, "In component ", cmp.ind,
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
    for (EgIdC k {}; const auto& gl: m_data)
        gl.check(eg, ic, k);
}

}  // namespace graffine::structure::containers

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_EDGE_DESCRIPTORS_IN_COMPONENT_H