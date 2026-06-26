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

#include "common.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/vertices/collection.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_22.h"

#include <array>
#include <string>

namespace graffine::tests::vertices {

using namespace graffine::structure::vertices;

namespace elements = structure;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;
using ChSlot = Vertex::ChSlot;

/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G>
struct VertexMerger
    : public graffine::transforms::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : graffine::transforms::vertex_merger::From<D1, D2, G> {graph}
    {}
};


using Vertices = Test;

// =============================================================================


/// Tests boundary vertices of unconnected cycle chains.
TEST_F(Vertices, TypeC)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests boundary vertices of unconnected cycle chains"
        );

    constexpr std::array<EgId, 2> len {3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create two unconnected cycle chains:
    VertexMerger<1, 1, G> merge {gr};
    merge(ESlot{w0, eA}, ESlot{w0, eB});
    merge(ESlot{w1, eA}, ESlot{w1, eB});

    using V = Vertex;

    V v0 {Type::CYCLE_BOUNDARY, 0, CmpId::undefined, {{w0, eA}, {w0, eB}}, {}};
    V v1 {Type::CYCLE_BOUNDARY, 1, CmpId::undefined, {{w0, eB}, {w0, eA}}};
    V v2 {Type::CYCLE_BOUNDARY, 2, CmpId::undefined, {{w1, eB}, {w1, eA}}};

    if constexpr (profuse) {
        v0.print();
        v1.print();
        v2.print();
    }

    ASSERT_TRUE(v0 == v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
}


/// Tests boundary vertices of unconnected linear chains.
TEST_F(Vertices, TypeL)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests boundary vertices of unconnected linear chains"
        );

    constexpr std::array<EgId, 2> len {3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    using V = Vertex;

    V v0 {Type::LEAF, 0, {{w0, eA}}};
    V v1 {Type::LEAF, 1, {{w0, eB}}};
    V v2 {Type::LEAF, 2, {{w1, eB}}};

    ASSERT_TRUE(v0 == v0);
    ASSERT_TRUE(v0 != v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
}


/// Tests internal chain vertices.
TEST_F(Vertices, TypeB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests internal chain vertices");

    constexpr std::array<EgId, 2> len {3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    using V = Vertex;

    V v0 {Type::BULK, 0, {{w0, 0}, {w0, 1}}};
    V v1 {Type::BULK, 1, {{w0, 1}, {w0, 0}}};
    V v2 {Type::BULK, 2, {{w0, 1}, {w0, 2}}};
    V v3 {Type::BULK, 3, {{w1, 0}, {w1, 1}}};

    ASSERT_TRUE(v0 == v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
    ASSERT_TRUE(v0 != v3);
    ASSERT_TRUE(v2 != v3);
}


/// Tests three-way junction vertices.
TEST_F(Vertices, TypeY)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests three-way junction vertices");

    constexpr std::array<EgId, 4> len {2, 3, 2, 3};

    constexpr ChId w0 {0};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};
    constexpr ChId w4 {4};
    constexpr ChId w5 {5};

    constexpr EgId a1 {2};
    constexpr EgId a2 {1};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create two unconnected cycle chains:
    VertexMerger<1, 2, G> merge {gr};
    merge(ESlot{w0, eB}, BSlot{w1, a1});  // produces w4
    merge(ESlot{w2, eA}, BSlot{w3, a2});  // produces w5

    using V = Vertex;

    V v0 {Type::JUNCTION3, 0, {{w0, eB}, {w1, eB}, {w4, eA}}};
    V v1 {Type::JUNCTION3, 1, {{w1, eB}, {w0, eB}, {w4, eA}}};
    V v2 {Type::JUNCTION3, 2, {{w1, eB}, {w4, eA}, {w0, eB}}};
    V v3 {Type::JUNCTION3, 3, {{w0, eB}, {w4, eA}, {w1, eB}}};
    V u0 {Type::JUNCTION3, 4, {{w2, eB}, {w3, eB}, {w5, eA}}};
    V u1 {Type::JUNCTION3, 5, {{w3, eB}, {w2, eB}, {w5, eA}}};
    V u2 {Type::JUNCTION3, 6, {{w3, eB}, {w5, eA}, {w2, eB}}};
    V u3 {Type::JUNCTION3, 7, {{w2, eB}, {w5, eA}, {w3, eB}}};

    ASSERT_TRUE(v0 == v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v2 == v0);
    ASSERT_TRUE(v3 == v1);
    ASSERT_TRUE(v0 == v3);

    ASSERT_TRUE(u0 == u1);
    ASSERT_TRUE(u1 == u1);
    ASSERT_TRUE(u2 == u0);
    ASSERT_TRUE(u3 == u1);
    ASSERT_TRUE(u0 == u3);

    ASSERT_TRUE(v0 != u1);
    ASSERT_TRUE(v1 != u1);
    ASSERT_TRUE(v2 != u0);
    ASSERT_TRUE(v3 != u1);
    ASSERT_TRUE(u0 != v3);
    ASSERT_TRUE(u3 != v3);
    ASSERT_TRUE(u0 != v0);
}


/// Tests degree-specific vertex collections.
TEST_F(Vertices, CollectionsDeg)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree-specific vertex collections");

    constexpr std::array<EgId, 6> len {2, 4, 6, 3, 3, 3};

    constexpr ChId w0 {0};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};
    constexpr ChId w4 {4};
    constexpr ChId w5 {5};

    constexpr EgId a1 {1};
    constexpr EgId a2 {2};
    constexpr EgId a3 {4};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create chain configurations:

    VertexMerger<2, 2, G> merge_22 {gr};
    merge_22(BSlot{w0, a1}, BSlot{w1, a2});  // produces 6, 7
    merge_22(BSlot{w2, a2}, BSlot{w2, a3});  // produces 8, 9

    VertexMerger<1, 2, G> merge_12 {gr};
    merge_12(ESlot{w3, eB}, BSlot{w4, a1});  // produces 10

    VertexMerger<1, 1, G> merge_11 {gr};
    merge_11(ESlot{w5, eA}, ESlot{w5, eB});  // produces an unconnected cycle

    gr.print_chains("");

    /* prints:
    0 (len 1) **{ 1 B }{ 6 A }{ 7 A } 0 > ( 0 )
    1 (len 2) **{ 0 B }{ 6 A }{ 7 A } 0 >> ( 2 3 )
    2 (len 2) **{ 9 A }{ 9 B }{ 8 A } 2 >> ( 6 7 )
    3 (len 3) **{ 4 B }{ 10 A } 3 >>> ( 12 13 14 )
    4 (len 1) **{ 3 B }{ 10 A } 3 > ( 15 )
    5 (len 3) { 5 B }**{ 5 A } 1 >>> ( 18 19 20 )
    6 (len 1) { 1 B }{ 0 B }{ 7 A }** 0 > ( 1 )
    7 (len 2) { 1 B }{ 6 A }{ 0 B }** 0 >> ( 4 5 )
    8 (len 2) { 9 A }{ 9 B }{ 2 B }** 2 >> ( 10 11 )
    9 (len 2) { 2 B }{ 9 B }{ 8 A }**{ 9 A }{ 2 B }{ 8 A } 2 >> ( 8 9 )
    10 (len 2) { 3 B }{ 4 B }** 3 >> ( 16 17 )
    */

    //Collection<G> qc {&gr};
//    Collection<G> q1 {&gr};
//    Collection<G> q2 {&gr};
//    Collection<G> q3 {&gr};
//    Collection<4, G> q4 {&gr};

//    auto& all = gr.verticesAll;

//    all.populate();
    gr.print_vertices_deg(0, "q0 ");
    /* prints:
    q0  Vertex d  0   ind:  0  chSs:  {  5 A  } {  5 B  }
    */

//    ASSERT_EQ(gr.num_vertices<0>(), 1);

    gr.print_vertices_deg(1, "q1 ");
    /* prints:
    q1  Vertex d  1   ind:  1  chSs:  {  3 A  }
    q1  Vertex d  1   ind:  2  chSs:  {  4 A  }
    q1  Vertex d  1   ind:  3  chSs:  {  10 B  }
    q1  Vertex d  1   ind:  4  chSs:  {  0 A  }
    q1  Vertex d  1   ind:  5  chSs:  {  1 A  }
    q1  Vertex d  1   ind:  6  chSs:  {  2 A  }
    q1  Vertex d  1   ind:  7  chSs:  {  6 B  }
    q1  Vertex d  1   ind:  8  chSs:  {  7 B  }
    q1  Vertex d  1   ind:  9  chSs:  {  8 B  }
    */

    ASSERT_EQ(gr.num_vertices<1>(), 9);

    gr.print_vertices_deg(2, "q1 ");
    /* prints:
    q2  Vertex d  2   ind:  10  chSs:  {  1 0  } {  1 1  }
    q2  Vertex d  2   ind:  11  chSs:  {  2 0  } {  2 1  }
    q2  Vertex d  2   ind:  12  chSs:  {  3 0  } {  3 1  }
    q2  Vertex d  2   ind:  13  chSs:  {  3 1  } {  3 2  }
    q2  Vertex d  2   ind:  14  chSs:  {  5 0  } {  5 1  }
    q2  Vertex d  2   ind:  15  chSs:  {  5 1  } {  5 2  }
    q2  Vertex d  2   ind:  16  chSs:  {  7 0  } {  7 1  }
    q2  Vertex d  2   ind:  17  chSs:  {  8 0  } {  8 1  }
    q2  Vertex d  2   ind:  18  chSs:  {  9 0  } {  9 1  }
    q2  Vertex d  2   ind:  19  chSs:  {  10 0  } {  10 1  }
    */

    ASSERT_EQ(gr.num_vertices<2>(), 10);

//    all.template print_degree<3>("q3 ");
    gr.print_vertices_deg(3, "q3 ");
    /* prints:
    q3  Vegr.allVerticesrtex d  3   ind:  20  chSs:  {  3 B  } {  4 B  } {  10 A  }
    */

    ASSERT_EQ(gr.num_vertices<3>(), 1);

//    all.template print_degree<4>("q4 ");
    gr.print_vertices_deg(4, "q4 ");
    /* prints:
    q4  Vertex d  4   ind:  21  chSs:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    q4  Vertex d  4   ind:  22  chSs:  {  0 B  } {  1 B  } {  6 A  } {  7 A  }
    */

    ASSERT_EQ(gr.num_vertices<4>(), 2);

    gr.compts(2).vertices.print_degree(4, "q4_c2 ");
    /* prints:
    q4_c2  Vertex d  4   ind:  21  chSs:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    */

    ASSERT_EQ(gr.compts(2).vertices.num(4), 1);
}


/// Tests collection of all vertices.
TEST_F(Vertices, CollectionAll)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests collection of all vertices");

    constexpr std::array<EgId, 6> len {2, 4, 6, 3, 3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};
    constexpr ChId w4 {4};
    constexpr ChId w5 {5};

    constexpr EgId a1 {1};
    constexpr EgId a2 {2};
    constexpr EgId a3 {4};

    constexpr auto eA = End::A;
    constexpr auto eB = End::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create chain configurations:

    VertexMerger<2, 2, G> merge_22 {gr};
    merge_22(BSlot{w0, a1}, BSlot{w1, a2});  // produces 6, 7
    merge_22(BSlot{w2, a2}, BSlot{w2, a3});  // produces 8, 9

    VertexMerger<1, 2, G> merge_12 {gr};
    merge_12(ESlot{w3, eB}, BSlot{w4, a1});  // produces 10

    VertexMerger<1, 1, G> merge_11 {gr};
    merge_11(ESlot{w5, eA}, ESlot{w5, eB});  // produces an unconnected cycle

    gr.print_chains("");

    /* prints:
    0 (len 1) **{ 1 B }{ 6 A }{ 7 A } 0 > ( 0 )
    1 (len 2) **{ 0 B }{ 6 A }{ 7 A } 0 >> ( 2 3 )
    2 (len 2) **{ 9 A }{ 9 B }{ 8 A } 2 >> ( 6 7 )
    3 (len 3) **{ 4 B }{ 10 A } 3 >>> ( 12 13 14 )
    4 (len 1) **{ 3 B }{ 10 A } 3 > ( 15 )
    5 (len 3) { 5 B }**{ 5 A } 1 >>> ( 18 19 20 )
    6 (len 1) { 1 B }{ 0 B }{ 7 A }** 0 > ( 1 )
    7 (len 2) { 1 B }{ 6 A }{ 0 B }** 0 >> ( 4 5 )
    8 (len 2) { 9 A }{ 9 B }{ 2 B }** 2 >> ( 10 11 )
    9 (len 2) { 2 B }{ 9 B }{ 8 A }**{ 9 A }{ 2 B }{ 8 A } 2 >> ( 8 9 )
    10 (len 2) { 3 B }{ 4 B }** 3 >> ( 16 17 )
    */

//    auto& all = gr.verticesAll;

//    all.populate();

    gr.print_vertices("all ");
    /* prints:
    Vertex d  0   ind:  0  chSs:  {  5 A  } {  5 B  }
    Vertex d  1   ind:  1  chSs:  {  3 A  }
    Vertex d  1   ind:  2  chSs:  {  4 A  }
    Vertex d  1   ind:  3  chSs:  {  10 B  }
    Vertex d  1   ind:  4  chSs:  {  0 A  }
    Vertex d  1   ind:  5  chSs:  {  1 A  }
    Vertex d  1   ind:  6  chSs:  {  2 A  }
    Vertex d  1   ind:  7  chSs:  {  6 B  }
    Vertex d  1   ind:  8  chSs:  {  7 B  }
    Vertex d  1   ind:  9  chSs:  {  8 B  }
    Vertex d  2   ind:  10  chSs:  {  1 0  } {  1 1  }
    Vertex d  2   ind:  11  chSs:  {  2 0  } {  2 1  }
    Vertex d  2   ind:  12  chSs:  {  3 0  } {  3 1  }
    Vertex d  2   ind:  13  chSs:  {  3 1  } {  3 2  }
    Vertex d  2   ind:  14  chSs:  {  5 0  } {  5 1  }
    Vertex d  2   ind:  15  chSs:  {  5 1  } {  5 2  }
    Vertex d  2   ind:  16  chSs:  {  7 0  } {  7 1  }
    Vertex d  2   ind:  17  chSs:  {  8 0  } {  8 1  }
    Vertex d  2   ind:  18  chSs:  {  9 0  } {  9 1  }
    Vertex d  2   ind:  19  chSs:  {  10 0  } {  10 1  }
    Vertex d  3   ind:  20  chSs:  {  3 B  } {  4 B  } {  10 A  }
    Vertex d  4   ind:  21  chSs:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    Vertex d  4   ind:  22  chSs:  {  0 B  } {  1 B  } {  6 A  } {  7 A  }
    */

    ASSERT_EQ(gr.num_vertices(), 23);
    ASSERT_EQ(gr.num_vertices<1>(), 9);
    ASSERT_EQ(gr.num_vertices<2>(), 11);
    ASSERT_EQ(gr.num_vertices<3>(), 1);
    ASSERT_EQ(gr.num_vertices<4>(), 2);

    const auto& all_c3 = gr.ct[3].vertices;

    all_c3.print_degree(3, "all_c3_d3");
    /* prints:
    all_c3_d3  Vertex d  3   ind:  20  chSs:  {  3 B  } {  4 B  } {  10 A  }
    */

    ASSERT_EQ(all_c3.num(3), 1);
}


}  // namespace graffine::tests::vertices
