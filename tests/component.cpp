/* =============================================================================

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

#include "common.h"
//#include "graffine/properties/processors.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/transforms/vertex_merger/from_12.h"


#include <array>
#include <iostream>
#include <numeric>  // accumulate
#include <string>


namespace graffine::tests::graph {

namespace elements = structure;
namespace trs = transforms;

using Cmp = elements::Component<
            elements::Chain<
            elements::Edge<elements::Vertex>>>;
using Chain = Cmp::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G> requires (isImplementedDegree<D1> &&
                               isImplementedDegree<D2>)
struct VertexMerger
    : public trs::vertex_merger::From<D1, D2, G, void> {

    explicit VertexMerger(G& graph)
        : trs::vertex_merger::From<D1, D2, G, void> {graph}
    {}
};


class ComponentTest
    : public Test {

public:
};

// =============================================================================

// Tests constructor of an empty component object.
TEST_F(ComponentTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests constructor of an empty component object");

    constexpr CmpId c {1};
    Cmp::Chains cn;

    Cmp cmp {c, cn};

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 0);
    ASSERT_EQ(cmp.num_chains(), 0);
    ASSERT_EQ(cmp.ww.size(), 0);
    ASSERT_EQ(cmp.vv.num(), 0);
    ASSERT_EQ(cmp.get_egl().size(), 0);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_FALSE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 0);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);
}


// Tests appending an empty chain.
TEST_F(ComponentTest, AppendChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests appending an empty chain");

    constexpr ChIdG w{};
    constexpr CmpId c{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Cmp cmp {c, cn};

    cmp.append(cn.back());

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 0);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 0);
    ASSERT_EQ(cmp.get_egl().size(), 0);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests insertion of the first edge into an empty chain using proper function.
TEST_F(ComponentTest, InsertFirstEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of the first ",
                          "edge into an empty chain using proper function");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e), w);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 1);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 2);
    ASSERT_EQ(cmp.get_egl().size(), 1);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests insertion of the first edge into an empty chain using append_edge
// function.
TEST_F(ComponentTest, InsertFirstEdgeAppend)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of the first ",
                          "edge into an empty chain using append_edge function");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.append_edge_to_chain(std::move(e), w);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 1);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 2);
    ASSERT_EQ(cmp.get_egl().size(), 1);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests insertion of the first edge into an empty chain using prepend_edge
// function.
TEST_F(ComponentTest, InsertFirstEdgePrepend)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of the first edge into an ",
                          "empty chain using prepend_edge function");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.prepend_edge_to_chain(std::move(e), w);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 1);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 2);
    ASSERT_EQ(cmp.get_egl().size(), 1);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests insertion of the first edge into an empty chain using
// insert_edge_into_chain function.
TEST_F(ComponentTest, InsertFirstEdgeByInsert)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of the first edge into an empty ",
                          "chain using insert_edge_into_chain function");

    constexpr EgIdA a0{};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_edge_into_chain(std::move(e), w, a0);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 1);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 2);
    ASSERT_EQ(cmp.get_egl().size(), 1);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests appending an edge to the chain head using proper function.
TEST_F(ComponentTest, AppendEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests appending an edge to the chain head using ",
                          "proper function");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.append_edge_to_chain(std::move(e2), w);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests appending an edge to the chain head using insert.
TEST_F(ComponentTest, AppendEdgeByInsert)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests appending an edge to the chain head ",
                          "using insert");

    constexpr std::size_t a{1};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests prepending an edge to the chain tail using proper function.
TEST_F(ComponentTest, PrependEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests prepending an edge to the chain tail using ",
                          "proper function");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.prepend_edge_to_chain(std::move(e2), w);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind(), cmp.egId_last());
    ASSERT_EQ(cn[w].g[0].indc, cmp.egId_last());
    ASSERT_EQ(cn[w].g[0].indw, 0);
    ASSERT_EQ(cn[w].g[0].c, c);
    ASSERT_EQ(cn[w].g[0].w, w);

    for (std::size_t i{1}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i - 1);
        ASSERT_EQ(cn[w].g[i].indc, i - 1);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests prepending an edge to the chain tail using insert.
TEST_F(ComponentTest, PrependEdgeByInsert)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests prepending an edge to the chain tail using ",
                          "insert");

    constexpr std::size_t a{};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind(), cmp.egId_last());
    ASSERT_EQ(cn[w].g[0].indc, cmp.egId_last());
    ASSERT_EQ(cn[w].g[0].indw, 0);
    ASSERT_EQ(cn[w].g[0].c, c);
    ASSERT_EQ(cn[w].g[0].w, w);

    for (std::size_t i{1}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i - 1);
        ASSERT_EQ(cn[w].g[i].indc, i - 1);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests inserting an edge into the chain internal.
TEST_F(ComponentTest, InsertEdgeIntoChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests inserting an edge into the chain internal");

    constexpr std::size_t a{0};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices
    Edge e3{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);
    cmp.insert_edge_into_chain(std::move(e3), w, a+1);

    if constexpr (profuse)
        cmp.print("");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 3);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 4);
    ASSERT_EQ(cmp.get_egl().size(), 3);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind, 1);
    ASSERT_EQ(cn[w].g[0].indc, 1);
    ASSERT_EQ(cn[w].g[1].ind, 2);
    ASSERT_EQ(cn[w].g[1].indc, 2);
    ASSERT_EQ(cn[w].g[2].ind, 0);
    ASSERT_EQ(cn[w].g[2].indc, 0);

    for (EgIdA i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests removing an edge from the chain internal.
TEST_F(ComponentTest, RmEdgeFromChainInternal)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests removing an edge from the chain internal");

    constexpr std::size_t a{};
    constexpr std::size_t arem{1};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices
    Edge e3{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);
    cmp.insert_edge_into_chain(std::move(e3), w, a+1);

    if constexpr (profuse)
        cmp.print("before");

    cmp.remove_edge(w, arem);

    if constexpr (profuse)
        cmp.print("after");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind, 1);
    ASSERT_EQ(cn[w].g[0].indc, 1);
    ASSERT_EQ(cn[w].g[1].ind, 0);
    ASSERT_EQ(cn[w].g[1].indc, 0);

    for (EgIdA i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}


// Tests removing an edge from the chain tail.
TEST_F(ComponentTest, RmEdgeFromChainTail)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests removing an edge from the chain tail");

    constexpr std::size_t a{};
    constexpr std::size_t arem{};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices
    Edge e3{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);
    cmp.insert_edge_into_chain(std::move(e3), w, a+1);

    if constexpr (profuse)
        cmp.print("before");

    cmp.remove_edge(w, arem);

    if constexpr (profuse)
        cmp.print("after");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind, 2);  // is not expected to update global indexing
    ASSERT_EQ(cn[w].g[0].indc, 1);
    ASSERT_EQ(cn[w].g[1].ind, 0);
    ASSERT_EQ(cn[w].g[1].indc, 0);

    for (EgIdA i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests removing an edge from the chain head.
TEST_F(ComponentTest, RmEdgeFromChainHead)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests removing an edge from the chain head");

    constexpr std::size_t a{0};
    constexpr std::size_t arem{2};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd{};

    Cmp::Chains cn;
    cn.push_back(Chain{w});

    Edge e1{egInd++};  // edge not connected to vertices
    Edge e2{egInd++};  // edge not connected to vertices
    Edge e3{egInd++};  // edge not connected to vertices

    Cmp cmp {c, cn};
    cmp.append(cn.back());
    cmp.insert_first_edge_into_chain(std::move(e1), w);
    cmp.insert_edge_into_chain(std::move(e2), w, a);
    cmp.insert_edge_into_chain(std::move(e3), w, a+1);

    if constexpr (profuse)
        cmp.print("before");

    cmp.remove_edge(w, arem);

    if constexpr (profuse)
        cmp.print("after");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), 2);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), 3);
    ASSERT_EQ(cmp.get_egl().size(), 2);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    ASSERT_EQ(cn[w].g[0].ind, 1);
    ASSERT_EQ(cn[w].g[0].indc, 1);
    ASSERT_EQ(cn[w].g[1].ind, 2);  // is not expected to update global indexing
    ASSERT_EQ(cn[w].g[1].indc, 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (std::size_t i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests creation of a chain of specific length.
// Result: a single-chain component.
TEST_F(ComponentTest, CreateChain1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests creation of a chain of specific length");

    constexpr ChIdG w{};
    constexpr CmpId c{};
    EgIdG egInd0{3};
    EgIdG egInd{egInd0};
    constexpr std::size_t len{5};

    Cmp::Chains cn;
    Cmp cmp {c, cn};

    if constexpr (profuse)
        cmp.print("before");

    cmp.create_chain(len, egInd);

    if constexpr (profuse)
        cmp.print("after");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), len);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), len+1);
    ASSERT_EQ(cmp.get_egl().size(), len);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i + egInd0);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (std::size_t i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

// Tests creation of a chain of specific length.
// Result: a single-chain component.
TEST_F(ComponentTest, CreateChain2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests creation of a chain of specific length");

    constexpr EgIdG egidIni0 {};
    EgIdG egidIni {egidIni0};
    constexpr ChIdG w{};
    constexpr CmpId c{};
    constexpr std::size_t len{5};

    Cmp::Chains cn;
    Cmp cmp {c, cn};

    if constexpr (profuse)
        cmp.print("before");

    cmp.create_chain(len, egidIni);

    if constexpr (profuse)
        cmp.print("after");

    ASSERT_EQ(cmp.ind, c);
    ASSERT_EQ(cmp.num_edges(), len);
    ASSERT_EQ(cmp.num_chains(), 1);
    ASSERT_EQ(cmp.ww.size(), 1);
    ASSERT_EQ(cmp.vv.num(), len+1);
    ASSERT_EQ(cmp.get_egl().size(), len);
    ASSERT_EQ(cmp.trs.num(), 0);
    ASSERT_TRUE(cmp.is_single_unconnected_chain());
    ASSERT_FALSE(cmp.is_unconnected_cycle());
    ASSERT_EQ(cmp.chis.num({1, 1}), 1);
    ASSERT_EQ(cmp.chis.num({2, 2}), 0);
    ASSERT_EQ(cmp.chis.num({3, 3}), 0);
    ASSERT_EQ(cmp.chis.num({4, 4}), 0);
    ASSERT_EQ(cmp.chis.num({1, 3}), 0);
    ASSERT_EQ(cmp.chis.num({1, 4}), 0);
    ASSERT_EQ(cmp.chis.num({3, 4}), 0);

    for (std::size_t i{}; i<cn[w].length(); ++i) {
        ASSERT_EQ(cn[w].g[i].ind, i);
        ASSERT_EQ(cn[w].g[i].indc, i);
        ASSERT_EQ(cn[w].g[i].indw, i);
        ASSERT_EQ(cn[w].g[i].c, c);
        ASSERT_EQ(cn[w].g[i].w, w);
        ASSERT_TRUE(cn[w].g[i].is_connected_both_ends());
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::A)->get_c(), c);
        ASSERT_EQ(cn[w].g[i].vertex_at_outer(End::B)->get_c(), c);
    }
    for (EgIdA i{}; i<cn[w].length()-1; ++i)
        ASSERT_EQ(cn[w].g[i  ].vertex_at_outer(End::B),
                  cn[w].g[i+1].vertex_at_outer(End::A));
    ASSERT_FALSE(cmp.template check<profuse>());
}

}
