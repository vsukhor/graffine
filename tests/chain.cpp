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

#include <string>

#include "common.h"

#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"


namespace graph_mutator::tests::chain {

class ChainTest
    : public Test {

protected:

    using Edge = structure::basic::Edge<maxDegree>;
    using Ends = Edge::Ends;
    using Chain = structure::basic::Chain<Edge>;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using Neigs = Chain::Neigs;

    struct Config {

        static constexpr EgId chlen {4};
        static constexpr CmpId c {34};
        static constexpr ChId idw {7};
        static constexpr ChId idc {17};
        static constexpr EgId edgenum0 {3};
        static constexpr EgId ei0 = {8};
    };

    Config conf;

    ChainTest()
        : conf {}
    {}

};

// =============================================================================


/// Tests default constructor.
TEST_F(ChainTest, Constructor1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests default constructor");

    const Chain cn {};

    ASSERT_TRUE(cn.g.empty());
    ASSERT_EQ(cn.ngs[Ends::A]().size(), 0);
    ASSERT_EQ(cn.ngs[Ends::B]().size(), 0);

    ASSERT_EQ(cn.idw, undefined<ChId>);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
}


/// Tests index-setting constructor.
TEST_F(ChainTest, Constructor2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("index-setting constructor");

    const Chain cn {Config::idw};

    ASSERT_TRUE(cn.g.empty());
    ASSERT_EQ(cn.ngs[Ends::A]().size(), 0);
    ASSERT_EQ(cn.ngs[Ends::B]().size(), 0);

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
}


/// Tests index-setting constructor with edge creation.
TEST_F(ChainTest, Constructor3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("index-setting constructor with edge creation");

    const Chain cn {Config::chlen,
                 Config::idw,
                 conf.ei0};

    ASSERT_EQ(cn.g.size(), Config::chlen);
    for (EgId i {}; i<Config::chlen; ++i) {
        ASSERT_EQ(cn.g[i].ind, conf.ei0+i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, undefined<CmpId>);
        ASSERT_EQ(cn.g[i].w, Config::idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }

    ASSERT_EQ(cn.ngs[Ends::A]().size(), 0);
    ASSERT_EQ(cn.ngs[Ends::B]().size(), 0);

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
}


/// Tests insertion of an edge into a single-edge chain at the end A.
TEST_F(ChainTest, insertEdgeLen1A)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into a single-edge chain at the end A");

    constexpr EgId len {1};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {6};
    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a single-edge chain

    Chain cn {len, idw,  ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the left-most position in the chain;
    // the original edge is shifted forwards
    // p points to the new edge
    const auto p = cn.insert_edge(std::move(eg), 0);

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[0]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    ASSERT_EQ(cn.g[0].ind, eiE);
    ASSERT_EQ(cn.g[1].ind, ei);
    ASSERT_EQ(cn.g[0].indw, 0);
    ASSERT_EQ(cn.g[1].indw, 1);
    ASSERT_EQ(cn.g[0].w, cn.idw);
    ASSERT_EQ(cn.g[1].w, cn.idw);
    ASSERT_EQ(cn.g[0].c, undefined<CmpId>);
    ASSERT_EQ(cn.g[1].c, undefined<CmpId>);
}


/// Tests insertion of an edge into a single-edge chain at the end B.
TEST_F(ChainTest, insertEdgeLen1B)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into a single-edge chain at the end B");

    constexpr EgId len {1};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a single-edge chain

    Chain cn {len, idw, ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the right-most position in the chain;
    // the original edge is not affected
    // p points to the new edge
    const auto p = cn.insert_edge(std::move(eg), len);

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[len]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    ASSERT_EQ(cn.g[0].ind, ei);
    ASSERT_EQ(cn.g[1].ind, eiE);
    ASSERT_EQ(cn.g[0].indw, 0);
    ASSERT_EQ(cn.g[1].indw, 1);
    ASSERT_EQ(cn.g[0].indc, undefined<CmpId>);
    ASSERT_EQ(cn.g[1].indc, undefined<CmpId>);
    ASSERT_EQ(cn.g[0].w, cn.idw);
    ASSERT_EQ(cn.g[1].w, cn.idw);
    ASSERT_EQ(cn.g[0].c, undefined<CmpId>);
    ASSERT_EQ(cn.g[1].c, undefined<CmpId>);
}


/// Tests insertion of an edge into a multi-edge chain at the end A.
TEST_F(ChainTest, insertEdgeA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into a multi-edge chain at the end A");

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

    Chain cn {len, idw, ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the left-most position in the chain;
    // the original edge is shifted forwards
    // p points to the new edge
    constexpr EgId pos {};
    const auto p = cn.insert_edge(std::move(eg), pos);

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == pos ? eiE : ei + i - 1);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, undefined<CmpId>);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }
}


/// Tests insertion of an edge into a multi-edge chain at the end B.
TEST_F(ChainTest, insertEdgeB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into a multi-edge chain at the end B");

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

    Chain cn {len, idw, ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    constexpr EgId pos {len};
    const auto p = cn.insert_edge(std::move(eg), pos);

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == len ? eiE : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, undefined<CmpId>);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }
}


/// Tests insertion of an edge into an internal position in a multi-edge chain.
TEST_F(ChainTest, insertEdgeBulk)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into an internal position "s+
            "in a multi-edge chain");

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

    Chain cn {len, idw, ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    constexpr auto pos {3};
    const auto p = cn.insert_edge(std::move(eg), pos);

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == pos ? eiE
                                        : i > pos ? ei + i - 1
                                                  : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, undefined<CmpId>);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }
}


/// Tests appending an edge to a multi-edge chain.
TEST_F(ChainTest, appendEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests appending an edge to a multi-edge chain");

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

    Chain cn {len, idw, ei};

    // create a stand-alone edge
    Edge eg {eiE};

    // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    cn.append_edge(std::move(eg));

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, undefined<CmpId>);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == len ? eiE : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, undefined<CmpId>);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }
}


/// Tests reverse_g(): reversing orientation of the edges in the chain.
TEST_F(ChainTest, reverse_g)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests reverse_g(): reversing orientation of the edges in the chain");

    Chain cn {Config::chlen, Config::idw, conf.ei0};
    cn.reverse_g();

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.c, undefined<CmpId>);
    for (EgId i{}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, conf.ei0 + Config::chlen - i - 1);
        ASSERT_EQ(cn.g[i].indc, undefined<EgId>);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].w, Config::idw);
        ASSERT_EQ(cn.g[i].c, undefined<CmpId>);
    }
}


/// Tests set_g_cmp(): updating the component index of the chain edges.
TEST_F(ChainTest, set_g_cmp)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests set_g_cmp(): updating the component index of the chain edges");

    Chain cn {Config::chlen, Config::idw, conf.ei0};

    const auto newCl = Config::c + 100;
    const auto newindc = conf.ei0 + 100;

    const auto res = cn.set_g_cmp(newCl, newindc);

    for (EgId i{}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].indc, newindc + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].ind, conf.ei0 + i);
        ASSERT_EQ(cn.g[i].c, newCl);
        ASSERT_EQ(res, newindc + cn.length());
    }
}


/// Tests set_cmpt(): updating the component index.
TEST_F(ChainTest, set_cmpt)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests set_cmpt(): updating the component index");

    Chain cn {Config::chlen, Config::idw, conf.ei0};

    const auto newCl = Config::c + 100;
    const auto newIdc = Config::idc + 100;
    const auto newindc = conf.ei0 + 100;

    const auto res = cn.set_cmpt(newCl, newIdc, newindc);

    ASSERT_EQ(cn.c, newCl);
    ASSERT_EQ(cn.idc, newIdc);
    for (EgId i{}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].indc, newindc + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].ind, conf.ei0 + i);
        ASSERT_EQ(cn.g[i].c, newCl);
        ASSERT_EQ(cn.g[i].w, Config::idw);
    }
    ASSERT_EQ(res, cn.g.back().indc + 1);
}


/// Tests end2a(): convertion of the end index to position of the boundary edge.
TEST_F(ChainTest, End2A)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests end2a(): "s +
            "convertion of the end index to position of the boundary edge");

    const Chain cn {Config::chlen, Config::idw, conf.ei0};

    ASSERT_EQ(cn.end2a(Ends::A), zero<EgId>);
    ASSERT_EQ(cn.end2a(Ends::B), cn.length() - one<EgId>);
//    ASSERT_DEATH(cn.end2a(0), ::testing::Eq("Incorrect end index."));
}


/// Tests predicate returning true iff the chain has single unconnected end.
TEST_F(ChainTest, HasOneFreeEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests has_one_free_end(): predicate checking if "s +
            "the chain has single unconnected end");

    const Chain cn1 {Config::chlen, Config::idw, conf.ei0};

    ASSERT_FALSE(cn1.has_one_free_end());

    Chain cn2 {Config::chlen, Config::idw+1, conf.ei0};

    cn2.ngs[Ends::A].insert(EndSlot{});

    ASSERT_TRUE(cn2.has_one_free_end());
}


/// Tests correct Neig assignment.
TEST_F(ChainTest, NeigIndexes)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests correct Neig assignment");

    Chain cn {Config::chlen, Config::idw, conf.ei0};

    for (const auto e : Ends::Ids)
        cn.ngs[e] = Neigs{EndSlot{0, Chain::opp_end(e)}};

    for (const auto e : Ends::Ids) {
        ASSERT_EQ(cn.ngs[e]().front().w, zero<EgId>);
        ASSERT_EQ(cn.ngs[e]().front().e, Chain::opp_end(e));
    }
}


/// Tests predicate checking if the chain is a disconnected cycle.
TEST_F(ChainTest, IsCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests is_disconnected_cycle(): checking if the chain is "s +
            "a disconnected cycle");

    const Chain cn {Config::chlen, Config::idw, conf.ei0};

    ASSERT_FALSE(cn.is_disconnected_cycle());

    Chain cn1 {Config::chlen, Config::idw+1, conf.ei0};

    cn1.ngs[Ends::A].insert(EndSlot{0, Ends::B});
    cn1.ngs[Ends::B].insert(EndSlot{0, Ends::A});

    ASSERT_TRUE(cn1.is_disconnected_cycle());
}


/// Tests num_vertices<D>(): counting of vertex numbers by degree.
TEST_F(ChainTest, NumVertices)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests num_vertices<D>(): counting of vertex numbers by degree");

    Chain cn {Config::chlen, Config::idw, conf.ei0};

    ASSERT_EQ(cn.num_vertices<0>(), 0);
    ASSERT_EQ(cn.num_vertices<1>(), 2);
    ASSERT_EQ(cn.num_vertices<2>(), Config::chlen - 1);
    ASSERT_EQ(cn.num_vertices<3>(), 0);
    ASSERT_EQ(cn.num_vertices<4>(), 0);

    Chain cn1 {Config::chlen, Config::idw+1, conf.ei0};

    cn1.ngs[Ends::A].insert(EndSlot{0, Ends::B});
    cn1.ngs[Ends::B].insert(EndSlot{0, Ends::A});

    ASSERT_EQ(cn1.num_vertices<0>(), 1);
    ASSERT_EQ(cn1.num_vertices<1>(), 0);
    ASSERT_EQ(cn1.num_vertices<2>(), Config::chlen - 1);
    ASSERT_EQ(cn1.num_vertices<3>(), 0);
    ASSERT_EQ(cn1.num_vertices<4>(), 0);
}

/// Tests egEnd_to_bulkslot():
/// convertion of an internal chain vertex descriptor into a bulk slot.
TEST_F(ChainTest, EgEnd_To_Bulkslot)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests egEnd_to_bulkslot():",
            " convertion of an internal chain vertex descriptor into a bulk slot"
        );


    constexpr EgId chlen {3};

    const Chain cn0 {chlen, Config::idw, conf.ei0};

    BulkSlot bs0 {cn0.idw, 0};
    BulkSlot bs1 {cn0.idw, 1};
    BulkSlot bs2 {cn0.idw, 2};
    BulkSlot bs3 {cn0.idw, 3};

    auto cn {cn0};

    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 2), bs3);

    cn.g[0].reverse();
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 2), bs3);

    cn = cn0;

    cn.g[1].reverse();
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 2), bs3);

    cn = cn0;

    cn.g[2].reverse();
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::A, 2), bs3);
    ASSERT_EQ(cn.egEnd_to_bulkslot(Ends::B, 2), bs2);
}


/// Tests internal_egEnd(): finding end of the edge connected to another edge.
TEST_F(ChainTest, Internal_egEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests internal_egEnd():",
            " finding end of the edge connected to another edge."
        );


    constexpr EgId chlen {3};

    Chain cn {chlen, Config::idw, conf.ei0};

    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::B);
    cn.g[0].reverse();
    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::B);
    cn.g[1].reverse();
    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::A);
    cn.g[2].reverse();
    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::A);
    cn.g[1].reverse();
    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::B);
    cn.g[0].reverse();
    ASSERT_EQ(cn.internal_egEnd(0, 1), Ends::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), Ends::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), Ends::B);
}

}  // namespace graph_mutator::tests::chain
