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

#include <array>
#include <string>

#include "common.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/structure/basic/vertices/all.h"
#include "graph-mutator/structure/basic/vertices/collections.h"
#include "graph-mutator/structure/basic/vertices/vertex.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"
#include "graph-mutator/transforms/vertex_merger/from_22.h"


namespace graph_mutator::tests::vertices {

using namespace graph_mutator::structure::basic::vertices;

using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using Ends = Chain::Ends;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;


/// Subclass to make protected members accessible for testing:
template<unsigned D1,
         unsigned D2,
         typename G>
struct VertexMerger
    : public graph_mutator::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : graph_mutator::vertex_merger::From<D1, D2, G> {graph}
    {}
};


using VerticesTest = Test;

// =============================================================================


/// Tests boundary vertices of disconnected cycle chains
TEST_F(VerticesTest, Degree0)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests boundary vertices of disconnected cycle chains");

    constexpr std::array<EgId, 2> len {3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create two disconnected cycle chains:
    VertexMerger<1, 1, G> merge {gr};
    merge(ESlot{w0, eA}, ESlot{w0, eB});
    merge(ESlot{w1, eA}, ESlot{w1, eB});

    using V = Vertex<0, ESlot>;

    V v0 {0, {ESlot{w0, eA}, ESlot{w0, eB}}};
    V v1 {1, {ESlot{w0, eB}, ESlot{w0, eA}}};
    V v2 {2, {ESlot{w1, eB}, ESlot{w1, eA}}};

    ASSERT_TRUE(v0 == v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
}


/// Tests boundary vertices of disconnected linear chains
TEST_F(VerticesTest, Degree1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests boundary vertices of disconnected linear chains");

    constexpr std::array<EgId, 2> len {3, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    using V = Vertex<1, ESlot>;

    V v0 {0, {ESlot{w0, eA}}};
    V v1 {1, {ESlot{w0, eB}}};
    V v2 {2, {ESlot{w1, eB}}};

    ASSERT_TRUE(v0 == v0);
    ASSERT_TRUE(v0 != v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
}


/// Tests internal chain vertices
TEST_F(VerticesTest, Degree2)
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

    using V = Vertex<2, BSlot>;

    V v0 {0, {BSlot{w0, 0}, BSlot{w0, 1}}};
    V v1 {1, {BSlot{w0, 1}, BSlot{w0, 0}}};
    V v2 {2, {BSlot{w0, 1}, BSlot{w0, 2}}};
    V v3 {3, {BSlot{w1, 0}, BSlot{w1, 1}}};

    ASSERT_TRUE(v0 == v1);
    ASSERT_TRUE(v1 == v1);
    ASSERT_TRUE(v0 != v2);
    ASSERT_TRUE(v1 != v2);
    ASSERT_TRUE(v0 != v3);
    ASSERT_TRUE(v2 != v3);
}


/// Tests degree 3 vertex class.
TEST_F(VerticesTest, Degree3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree 3 vertex class");

    constexpr std::array<EgId, 4> len {2, 3, 2, 3};

    constexpr ChId w0 {};
    constexpr ChId w1 {1};
    constexpr ChId w2 {2};
    constexpr ChId w3 {3};
    constexpr ChId w4 {4};
    constexpr ChId w5 {5};

    constexpr EgId a1 {2};
    constexpr EgId a2 {1};

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;

    for (const auto o : len)
        gr.add_single_chain_component(o);

    // create two disconnected cycle chains:
    VertexMerger<1, 2, G> merge {gr};
    merge(ESlot{w0, eB}, BSlot{w1, a1});  // produces w4
    merge(ESlot{w2, eA}, BSlot{w3, a2});  // produces w5

    using V = Vertex<3, ESlot>;

    V v0 {0, {ESlot{w0, eB}, ESlot{w1, eB}, ESlot{w4, eA}}};
    V v1 {1, {ESlot{w1, eB}, ESlot{w0, eB}, ESlot{w4, eA}}};
    V v2 {2, {ESlot{w1, eB}, ESlot{w4, eA}, ESlot{w0, eB}}};
    V v3 {3, {ESlot{w0, eB}, ESlot{w4, eA}, ESlot{w1, eB}}};
    V u0 {4, {ESlot{w2, eB}, ESlot{w3, eB}, ESlot{w5, eA}}};
    V u1 {5, {ESlot{w3, eB}, ESlot{w2, eB}, ESlot{w5, eA}}};
    V u2 {6, {ESlot{w3, eB}, ESlot{w5, eA}, ESlot{w2, eB}}};
    V u3 {7, {ESlot{w2, eB}, ESlot{w5, eA}, ESlot{w3, eB}}};

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
TEST_F(VerticesTest, CollectionsDeg)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests degree-specific vertex collections");

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

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

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
    merge_11(ESlot{w5, eA}, ESlot{w5, eB});  // produces a disconnected cycle

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

    Collection<0, G> q0 {gr};
    Collection<1, G> q1 {gr};
    Collection<2, G> q2 {gr};
    Collection<3, G> q3 {gr};
    Collection<4, G> q4 {gr};

    Id ind {};
    q0.populate(ind);
    q1.populate(ind);
    q2.populate(ind);
    q3.populate(ind);
    q4.populate(ind);

    print<0>(q0, "q0 ");
    /* prints:
    q0  Vertex d  0   ind:  0  ars:  {  5 A  } {  5 B  }
    */

    ASSERT_EQ(q0.num(), 1);

    print<1>(q1, "q1 ");
    /* prints:
    q1  Vertex d  1   ind:  1  ars:  {  3 A  }
    q1  Vertex d  1   ind:  2  ars:  {  4 A  }
    q1  Vertex d  1   ind:  3  ars:  {  10 B  }
    q1  Vertex d  1   ind:  4  ars:  {  0 A  }
    q1  Vertex d  1   ind:  5  ars:  {  1 A  }
    q1  Vertex d  1   ind:  6  ars:  {  2 A  }
    q1  Vertex d  1   ind:  7  ars:  {  6 B  }
    q1  Vertex d  1   ind:  8  ars:  {  7 B  }
    q1  Vertex d  1   ind:  9  ars:  {  8 B  }
    */

    ASSERT_EQ(q1.num(), 9);

    print<2>(q2, "q2 ");
    /* prints:
    q2  Vertex d  2   ind:  10  ars:  {  1 0  } {  1 1  }
    q2  Vertex d  2   ind:  11  ars:  {  2 0  } {  2 1  }
    q2  Vertex d  2   ind:  12  ars:  {  3 0  } {  3 1  }
    q2  Vertex d  2   ind:  13  ars:  {  3 1  } {  3 2  }
    q2  Vertex d  2   ind:  14  ars:  {  5 0  } {  5 1  }
    q2  Vertex d  2   ind:  15  ars:  {  5 1  } {  5 2  }
    q2  Vertex d  2   ind:  16  ars:  {  7 0  } {  7 1  }
    q2  Vertex d  2   ind:  17  ars:  {  8 0  } {  8 1  }
    q2  Vertex d  2   ind:  18  ars:  {  9 0  } {  9 1  }
    q2  Vertex d  2   ind:  19  ars:  {  10 0  } {  10 1  }
    */

    ASSERT_EQ(q2.num(), 10);

    print<3>(q3, "q3 ");
    /* prints:
    q3  Vertex d  3   ind:  20  ars:  {  3 B  } {  4 B  } {  10 A  }
    */

    ASSERT_EQ(q3.num(), 1);

    print<4>(q4, "q4 ");
    /* prints:
    q4  Vertex d  4   ind:  21  ars:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    q4  Vertex d  4   ind:  22  ars:  {  0 B  } {  1 B  } {  6 A  } {  7 A  }
    */

    ASSERT_EQ(q4.num(), 2);

    const auto q4_c2 = for_compartment<4>(2, q4);

    print<4>(q4_c2, "q4_c2 ");
    /* prints:
    q4_c2  Vertex d  4   ind:  21  ars:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    */

    ASSERT_EQ(q4_c2.num(), 1);
}


/// Tests collection of all vertices.
TEST_F(VerticesTest, CollectionAll)
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

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

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
    merge_11(ESlot{w5, eA}, ESlot{w5, eB});  // produces a disconnected cycle

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

    All<G> all {gr};

    all.create();

    all.print("all ");
    /* prints:
    Vertex d  0   ind:  0  ars:  {  5 A  } {  5 B  }
    Vertex d  1   ind:  1  ars:  {  3 A  }
    Vertex d  1   ind:  2  ars:  {  4 A  }
    Vertex d  1   ind:  3  ars:  {  10 B  }
    Vertex d  1   ind:  4  ars:  {  0 A  }
    Vertex d  1   ind:  5  ars:  {  1 A  }
    Vertex d  1   ind:  6  ars:  {  2 A  }
    Vertex d  1   ind:  7  ars:  {  6 B  }
    Vertex d  1   ind:  8  ars:  {  7 B  }
    Vertex d  1   ind:  9  ars:  {  8 B  }
    Vertex d  2   ind:  10  ars:  {  1 0  } {  1 1  }
    Vertex d  2   ind:  11  ars:  {  2 0  } {  2 1  }
    Vertex d  2   ind:  12  ars:  {  3 0  } {  3 1  }
    Vertex d  2   ind:  13  ars:  {  3 1  } {  3 2  }
    Vertex d  2   ind:  14  ars:  {  5 0  } {  5 1  }
    Vertex d  2   ind:  15  ars:  {  5 1  } {  5 2  }
    Vertex d  2   ind:  16  ars:  {  7 0  } {  7 1  }
    Vertex d  2   ind:  17  ars:  {  8 0  } {  8 1  }
    Vertex d  2   ind:  18  ars:  {  9 0  } {  9 1  }
    Vertex d  2   ind:  19  ars:  {  10 0  } {  10 1  }
    Vertex d  3   ind:  20  ars:  {  3 B  } {  4 B  } {  10 A  }
    Vertex d  4   ind:  21  ars:  {  9 A  } {  2 B  } {  9 B  } {  8 A  }
    Vertex d  4   ind:  22  ars:  {  0 B  } {  1 B  } {  6 A  } {  7 A  }
    */

    ASSERT_EQ(all.num(), 23);
    ASSERT_EQ(all.template num<0>(), 1);
    ASSERT_EQ(all.template num<1>(), 9);
    ASSERT_EQ(all.template num<2>(), 10);
    ASSERT_EQ(all.template num<3>(), 1);
    ASSERT_EQ(all.template num<4>(), 2);

    const auto all_c3_d3 = all.template for_compartment<3>(3);

    print<3>(all_c3_d3, "all_c3_d3 ");
    /* prints:
    all_c3_d3  Vertex d  3   ind:  20  ars:  {  3 B  } {  4 B  } {  10 A  }
    */

    ASSERT_EQ(all_c3_d3.num(), 1);
}


}  // namespace graph_mutator::tests::vertices
