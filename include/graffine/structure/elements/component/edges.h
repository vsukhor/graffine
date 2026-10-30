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
 * \file edges.h
 * \brief Insertion, removal and moves of the edges of a component.
 * \details Part of the implementation of component.h, which includes it.
 */

#ifndef GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_EDGES_H
#define GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_EDGES_H

#include "graffine/structure/elements/component.h"

namespace graffine::structure::detail {

// Reconnect edges from vA to vB
template <typename Ch>
void Component<Ch>::
reconnect_edges(Vertex* f, Vertex* t)
{
    auto& ss = f->edges();
    while (ss.size()) {

        // Copy: disconnecting erases the descriptor from 'ss'.
        const auto s = *ss.begin();
        auto& eg = edge(s.w);
        eg.disconnect_end_inner(s.e);
        eg.connect_to_inner(s.e, *t);
    }
}

// Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(const ChIdG w, // id of the chain modified by the edge removal
            const EgIdA a) -> Edge*
{
    return remove_edge(cn[w], a);
}

// Removes an edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(const EgIdC indc) -> Edge*
{
    const auto& gl = egl[indc];

    return remove_edge(cn[gl.w], gl.a);
}

// Removes edge g[a],  shifting original g[a+1], g[a+2], ... backwards.
// These then become g[a], g[a+1], ... .
// Note: does not update global edge indices.
// Note: does not remove the host chain if it gets emptied by the edge removal.
template <typename Ch>
template <bool preserveTopology>
auto Component<Ch>::
remove_edge(Chain& m, // chain modified by the edge removal
            const EgIdA a) -> Edge*
{
    auto& g = m.g;
    auto& h = g[a]; // edge to be removed

    GRAFFINE_ASSERT_CALLING(a < m.length(),
                   m.print("FAIL"),
                   "Erasing an edge at ", a, " is beyond chain length.");
    GRAFFINE_ASSERT(m.length(), "Erasing an edge in empty chain.");

    // If the chain is to get empty after removal, it needs to be intentional.
     GRAFFINE_ASSERT( //! preserveTopology ||
        m.length() > 1,
        "Erasing an edge in single-edge chain.");

    auto vA = h.vertex_at_outer(End::A);
    auto vB = h.vertex_at_outer(End::B); // to be preserved

    GRAFFINE_ASSERT(!preserveTopology ||
           is_implemented_degree(vA->deg() + vB->deg() - Deg2),
           "Erasing edge produces vertex above max implemented degree.");

    h.disconnect_end_outer(End::A);
    h.disconnect_end_outer(End::B);

    // reconnect edges from vA to vB
    if (m.is_head(a)) {
                                 // vB is at chain end B
        reconnect_edges(vA, vB); // vA to be removed, vB to be preserved
        vv.remove_disconnected(vA);
    }
    else {

        reconnect_edges(vB, vA); // vB to be removed, vA to be preserved
        vv.remove_disconnected(vB);
    }

    if (h.indc < egId_last()) {

        auto& last = edge(egId_last());
        vv.reind_edge(last, h.indc);
        last.indc = h.indc;
    }

    g.erase(g.begin() + a());

    for (EgIdA i {a}; i < m.length(); ++i)
        g[i].indw = i;

    set_egl();

    return a < m.length() ? &g[a] : nullptr;
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is not limited to the chain internal:
//   if a == 0, prepends the new edge at end A,
//   if a == length(), appends the new edge at end B.
template <typename Ch>
auto Component<Ch>::
insert_edge_into_chain(
    Edge&& eg,
    const ChIdG w,
    const EgIdA a) -> Edge&
{
    return insert_edge_into_chain(std::move(eg), cn[w], a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is not limited to the chain internal:
//   if a == 0, prepends the new edge at end A,
//   if a == length(), appends the new edge at end B.
template <typename Ch>
auto Component<Ch>::
insert_edge_into_chain(
    Edge&& eg,
    Chain& m,
    const EgIdA a) -> Edge&
{
    GRAFFINE_ASSERT_CALLING(!eg.is_connected(), eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    GRAFFINE_ASSERT_CALLING(!contains(eg), eg.print("FAIL"),
                   "Appending edge which is already included");
    GRAFFINE_ASSERT_CALLING(a <= m.length(), m.print("FAIL"),
                   "Inserting edge to pos ", a, " is beyond chain length");
    GRAFFINE_ASSERT(m.length() || (!m.ngs[End::A].num() && !m.ngs[End::B].num()),
           "Edge cannot be insetred into an empty connected chain");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    if (a == m.length())
        return append_edge_to_chain(std::move(eg), m);

    if (a == 0)
        return prepend_edge_to_chain(std::move(eg), m);

    // else: insert in between other edges of chain 'm'
    return insert_edge_inside_chain(std::move(eg), m, a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is limited to the chain internal: 0 < a < m.length()
template <typename Ch>
auto Component<Ch>::
insert_edge_inside_chain(
    Edge&& eg,
    const ChIdG w,
    const EgIdA a) -> Edge&
{
    return insert_edge_inside_chain(std::move(eg), cn[w], a);
}

// Inserts an edge at g[a] of chain 'w' shifting original g[a], g[a+1], ...
// forwards, to become g[a+1], g[a+2], .....
// Note: insertion is limited to the chain internal: 0 < a < m.length()
template <typename Ch>
auto Component<Ch>::
insert_edge_inside_chain(
    Edge&& eg,
    Chain& m,
    const EgIdA a) -> Edge&
{
    GRAFFINE_ASSERT_CALLING(!eg.is_connected(), eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    GRAFFINE_ASSERT_CALLING(!contains(eg), eg.print("FAIL"),
                   "Appending edge which is already included");
    GRAFFINE_ASSERT_CALLING(a > 0, m.print("FAIL"),
                   "Inserting edge at chain tail");
    GRAFFINE_ASSERT_CALLING(a < m.length(), m.print("FAIL"),
                   "Inserting edge to pos ", a, " is beyond chain length");
    GRAFFINE_ASSERT(m.length(), "Inserting edge inside an empty chain");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    eg.w = m.idw;
    eg.set_cmp(ind, num_edges());
    auto& g = m.g;
    auto& p = *g.insert(g.begin() + a(), std::move(eg));
    for (EgIdA i{a}; i < m.length(); ++i)
        g[i].indw = i;
    set_egl();

    auto& egA = g[a - 1];

    // 'eg' will be connected at its end B to the vertex to which egA is
    // originally connected at its end B.
    //  A new BULK vertex will be created to join end B egA to end A of 'eg'.

    auto gvB = egA.vertex_at_outer(End::B);
    GRAFFINE_ASSERT(gvB, "end B of ", egA.indw, " is not connected.");
    egA.disconnect_end_outer(End::B);
    p.connect_to_outer(End::B, *gvB);

    auto& v = vv.create_vertex(ind);
    egA.connect_to_outer(End::B, v);
    p.connect_to_outer(End::A, v);

    return p;
}

// Insert first edge into an empty chain. The chain must be disconnected.
template <typename Ch>
auto Component<Ch>::
insert_first_edge_into_chain(
    Edge&& eg,
    const ChIdG w) -> Edge&
{
    return insert_first_edge_into_chain(std::move(eg), cn[w]);
}

// Insert first edge into an empty chain. The chain must be disconnected.
template <typename Ch>
auto Component<Ch>::
insert_first_edge_into_chain(Edge&& eg,
                             Chain& m) -> Edge&
{
    GRAFFINE_ASSERT_CALLING(!eg.is_connected(),
                   eg.print("FAIL"),
                   "Passing a connected edge for insertion.");
    GRAFFINE_ASSERT_CALLING(!contains(eg),
                   eg.print("FAIL"),
                   "Appending edge which is already included");
    warn_if(eg.indw.is_defined(), " indw will be overwriten!");
    warn_if(eg.indc.is_defined(), " indc will be overwriten!");

    GRAFFINE_ASSERT_CALLING(!m.length(),
                   m.print("FAIL"),
                   "Inserting 1st edge into chain which is not empty.");
    GRAFFINE_ASSERT_CALLING(!m.is_connected(),
                   m.print("FAIL"),
                   "Chain is connected.");

    eg.w = m.idw;
    eg.indw = 0;
    eg.set_cmp(ind, num_edges());
    egl.append(eg);

    m.g.push_back(std::move(eg));
    auto& p = m.g.back();

    auto& v1 = vv.create_vertex(ind);
    auto& v2 = vv.create_vertex(ind);

    p.connect_to_outer(End::A, v1);
    p.connect_to_outer(End::B, v2);

    return p;
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
append_edge_to_chain(Edge&& eg,
                     const ChIdG w) -> Edge&
{
    return append_edge_to_chain(std::move(eg), cn[w]);
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
append_edge_to_chain(Edge&& eg,
                     Chain& m) -> Edge&
{

    GRAFFINE_ASSERT_CALLING(!eg.is_connected(),
                   eg.print("FAIL"),
                   "Passing a connected edge for appending.");
    GRAFFINE_ASSERT_CALLING(!contains(eg),
                   eg.print("FAIL"),
                   "Appending edge which is already included");
    warn_if(eg.indc.is_defined(), "indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    eg.w = m.idw;
    eg.indw = m.length();
    eg.set_cmp(ind, num_edges());
    egl.append(eg);

    m.g.push_back(std::move(eg));
    auto& p = m.g.back();

    auto& egA = m.g[p.indw - 1]; // edge preceding 'p' in chain

    // 'eg' will be connected at its end B to the vertex to which egA is
    // originally connected at its end B.
    // A new BULK vertex will be created to join end B egA to end A of 'eg'.

    auto gvB = egA.vertex_at_outer(End::B);
    GRAFFINE_ASSERT(gvB, "end B of ", egA.indw, " is not connected.");
    egA.disconnect_end_outer(End::B);
    p.connect_to_outer(End::B, *gvB);

    auto& v = vv.create_vertex(ind);
    egA.connect_to_outer(End::B, v);
    p.connect_to_outer(End::A, v);

    return p;
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
prepend_edge_to_chain(Edge&& eg,
                      const ChIdG w) -> Edge&
{
    return prepend_edge_to_chain(std::move(eg), cn[w]);
}

// Appends an edge at the chain head, i.e. at the back of g.
template <typename Ch>
auto Component<Ch>::
prepend_edge_to_chain(Edge&& eg,
                      Chain& m) -> Edge&
{
    GRAFFINE_ASSERT(!eg.is_connected(), "Passing a connected edge for appending.");
    warn_if(eg.indc.is_defined(), "indc will be overwriten!");

    if (!m.length())
        return insert_first_edge_into_chain(std::move(eg), m);

    eg.w = m.idw;
    eg.indw = 0;
    eg.set_cmp(ind, num_edges());

    auto& p = *m.g.insert(m.g.begin(), std::move(eg));
    for (EgIdA i{1}; i < m.length(); ++i)
        m.g[i].indw = i;
    set_egl();

    auto& egB = m.g[1]; // edge following 'p' in chain

    // 'eg' will be connected at its end A to the vertex to which egA is
    // originally connected at its end A.
    // A new BULK vertex will be created to join end A of egB to end B of 'eg'.

    auto gvA = egB.vertex_at_outer(End::A);
    GRAFFINE_ASSERT(gvA, "end A of ", egB.indw, " is not connected.");
    egB.disconnect_end_outer(End::A);
    p.connect_to_outer(End::A, *gvA);

    auto& v = vv.create_vertex(ind);
    egB.connect_to_outer(End::A, v);
    p.connect_to_outer(End::B, v);

    return p;
}
template <typename Ch>
constexpr void Component<Ch>::
move_edges(
    Chain& fr,
    const std::size_t posFrBegin,
    const std::size_t posFrEnd, // position after the last moved element
    Chain& to,
    const std::size_t posTo  // position of the first inserted element
)
{
    // Does not update edge connectivity.
    // Does not update chain connectivity and 'ngs'.
    // Does not remove source chain if it is emptied.

    GRAFFINE_ASSERT(fr.c == to.c,
           "Source chain component is different from that of the target");

    to.g.insert(to.g.begin() + posTo,
                std::make_move_iterator(fr.g.begin() + posFrBegin),
                std::make_move_iterator(fr.g.begin() + posFrEnd));
    fr.g.erase(fr.g.begin() + posFrBegin,
               fr.g.begin() + posFrEnd);

    for (std::size_t i{posFrBegin}; i < fr.length(); ++i) {

        fr.g[i].indw = i;
        egl[fr.g[i].indc].a = i;
    }
    for (EgIdA i{posTo}; auto& eg : to.g) {

        eg.indw = i;
        eg.w = to.idw;
        egl[eg.indc].a = i;
        egl[eg.indc].w = to.idw;
        ++i;
    }
}

template <typename Ch>
constexpr auto Component<Ch>::
edge_vertices(const EgIdC indc) const noexcept -> const Edge::Vertices&
{
    return edge(indc).vertices();
}

template <typename Ch>
constexpr auto Component<Ch>::
edge_vids(const EgIdC indc) const noexcept -> Edge::VIDs
{
    return edge(indc).vids();
}

template <typename Ch>
void Component<Ch>::
set_edges() noexcept
{
    EgIdC indc{};
    for (const auto& w : ww)
        for (auto& eg : cn[w].g) {

            vv.reind_edge(eg, indc);
            eg.set_cmp(ind, indc++);
        }
}

template <typename Ch>
void Component<Ch>::
set_edges(
    Chain& m,
    EgIdC& indc // ref
) noexcept
{
    for (auto& eg : m.g) {

        vv.reind_edge(eg, indc);
        eg.set_cmp(ind, indc++);
    }
}

template <typename Ch>
constexpr auto Component<Ch>::
common_vertex(const EgIdC e1,
              const EgIdC e2) noexcept -> Vertex*
{
    GRAFFINE_ASSERT(e1 < num_edges(),
           "Accessing non-existing edge idc ", e1, " in component ", ind);
    GRAFFINE_ASSERT(e2 < num_edges(),
           "Accessing non-existing edge idc ", e2, " in component ", ind);
    GRAFFINE_ASSERT(e1 != e2, "Equal edge ids");

    const auto& v1 = edge_vertices(e1);
    const auto& v2 = edge_vertices(e2);

    // for distinct edges at most a single common vertex is possible
    if (v1[0]->operator==(*v2[0]) || v1[0]->operator==(*v2[1])) return v1[0];
    if (v1[1]->operator==(*v2[0]) || v1[1]->operator==(*v2[1])) return v1[1];

    return nullptr;
}

}  // namespace graffine::structure::detail

#endif  // GRAFFINE_STRUCTURE_ELEMENTS_COMPONENT_EDGES_H
