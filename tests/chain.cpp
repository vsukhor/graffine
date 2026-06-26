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
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/vertex.h"

#include <memory>
#include <string>

namespace graffine::tests::chain {

class ChainTest
    : public Test {

protected:

    using Edge = elements::Edge<elements::Vertex>;
    using End = Edge::End;
    using Chain = elements::Chain<Edge>;
    using BulkSlot = Chain::BulkSlot;
    using EndSlot = Chain::EndSlot;
    using Neigs = Chain::Neigs;
    using Vertex = elements::Vertex;

    struct Config {

        static constexpr EgId chlen {4};
        static constexpr CmpId c {34};
        static constexpr ChId idw {7};
        static constexpr ChId idc {17};
        static constexpr EgId numEdges0 {3};
        static constexpr EgId ei0 = {8};
    };

    Config conf;

    ChainTest()
        : conf {}
    {}

};

// =============================================================================


/// Tests default constructor.
TEST_F(ChainTest, ConstructorDefault)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests default constructor");

    const Chain cn {};

    if constexpr (profuse)
        cn.print("new chain ");

    ASSERT_TRUE(cn.g.empty());
    ASSERT_EQ(cn.ngs[End::A]().size(), 0);
    ASSERT_EQ(cn.ngs[End::B]().size(), 0);

    ASSERT_EQ(cn.idw, ChId::undefined);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
}


/// Tests index-setting constructor of an empty chain.
TEST_F(ChainTest, ConstructorIndex)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests index-setting constructor of an empty chain");

    const Chain cn {Config::idw};

    if constexpr (profuse)
        cn.print("new chain ");

    ASSERT_TRUE(cn.g.empty());
    ASSERT_EQ(cn.ngs[End::A]().size(), 0);
    ASSERT_EQ(cn.ngs[End::B]().size(), 0);

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
}


/// Tests constructor of an empty chain setting scalar parameters.
TEST_F(ChainTest, ConstructorScalars)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests constructor of an empty chain setting scalar parameters");

    const Chain cn {Config::idw, Config::idc, Config::c};

    if constexpr (profuse)
        cn.print("new chain ");

    ASSERT_TRUE(cn.g.empty());
    ASSERT_EQ(cn.ngs[End::A]().size(), 0);
    ASSERT_EQ(cn.ngs[End::B]().size(), 0);

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.idc, Config::idc);
    ASSERT_EQ(cn.c, Config::c);
}


/*
/// Tests index-setting constructor with edge creation.
TEST_F(ChainTest, ConstructorIndexEdge)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests index-setting constructor with edge creation");

//    Edge eg0 {Config::ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
//        eg0.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("new chain ");
        print_vertices("new chain ");
    }

    ASSERT_EQ(cn.g.size(), Config::chlen);
    for (EgId i {}; i<Config::chlen; ++i) {
        ASSERT_EQ(cn.g[i].ind, conf.ei0+i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, CmpId::undefined);
        ASSERT_EQ(cn.g[i].w, Config::idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
    }

    ASSERT_EQ(cn.ngs[End::A]().size(), 0);
    ASSERT_EQ(cn.ngs[End::B]().size(), 0);

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
}


/// Tests insertion of an edge into a single-edge chain at the end A.
TEST_F(ChainTest, InsertEdgeLen1atEndA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests insertion of an edge into a single-edge chain at the end A"
        );

    constexpr EgId len {1};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {6};
    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a single-edge chain

//    Edge eg0 {ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    if constexpr (profuse)
//        eg0.print("eg0 ");

//    Chain cn {len, std::move(eg0)};

    auto cn = create_chain<true>(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    // create a stand-alone edge
    auto eg = create_edge(eiE);
//    Edge eg {eiE};

    if constexpr (profuse) {
        eg.print("eg ");
        print_vertices("eg ");
    }

    // move the edge to the left-most position in the chain;
    // the original edge is shifted forwards 'p' points to the new edge
    const auto p = cn.insert_edge(std::move(eg), 0);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[0]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    ASSERT_EQ(cn.g[0].ind, eiE);
    ASSERT_EQ(cn.g[1].ind, ei);
    ASSERT_EQ(cn.g[0].indw, 0);
    ASSERT_EQ(cn.g[1].indw, 1);
    ASSERT_EQ(cn.g[0].w, cn.idw);
    ASSERT_EQ(cn.g[1].w, cn.idw);
    ASSERT_EQ(cn.g[0].c, CmpId::undefined);
    ASSERT_EQ(cn.g[1].c, CmpId::undefined);
}


/// Tests insertion of an edge into a single-edge chain at the end B.
TEST_F(ChainTest, InsertEdgeLen1EndB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests insertion of an edge into a single-edge chain at the end B"
        );

    constexpr EgId len {1};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a single-edge chain

//    Edge eg0 {ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    if constexpr (profuse)
//        eg0.print("eg0 ");

//    Chain cn {len, std::move(eg0)};
    auto cn = create_chain<true>(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    // create a stand-alone edge to insert
//    Edge eg {eiE};
    auto eg = create_edge(eiE);

    if constexpr (profuse)
        eg.print("eg ");

    // move the edge to the right-most position in the chain;
    // the original edge is not affected
    // p points to the new edge
    const auto p = cn.insert_edge(std::move(eg), len);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[len]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    ASSERT_EQ(cn.g[0].ind, ei);
    ASSERT_EQ(cn.g[1].ind, eiE);
    ASSERT_EQ(cn.g[0].indw, 0);
    ASSERT_EQ(cn.g[1].indw, 1);
    ASSERT_EQ(cn.g[0].indc, CmpId::undefined);
    ASSERT_EQ(cn.g[1].indc, CmpId::undefined);
    ASSERT_EQ(cn.g[0].w, cn.idw);
    ASSERT_EQ(cn.g[1].w, cn.idw);
    ASSERT_EQ(cn.g[0].c, CmpId::undefined);
    ASSERT_EQ(cn.g[1].c, CmpId::undefined);
}


/// Tests insertion of an edge into a multi-edge chain at the end A.
TEST_F(ChainTest, InsertEdgeA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests insertion of an edge into a multi-edge chain at the end A"
        );

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

//    Edge eg0{ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    if constexpr (profuse)
//        eg0.print("eg0 ");

//    Chain cn {len, std::move(eg0)};
    auto cn = create_chain<true>(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    // create a stand-alone edge
//    Edge eg {eiE};
    auto eg = create_edge(eiE);

    if constexpr (profuse)
        eg.print("eg ");

    // move the edge to the left-most position in the chain;
    // the original edge is shifted forwards
    // p points to the new edge
    constexpr EgId pos {};
    const auto p = cn.insert_edge(std::move(eg), pos);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == pos ? eiE : ei + i - 1);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, CmpId::undefined);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
    }
}

/*
/// Tests insertion of an edge into a multi-edge chain at the end B.
TEST_F(ChainTest, insertEdgeB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests insertion of an edge into a multi-edge chain at the end B"
        );

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

//    Edge eg0{ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn {len, std::move(eg0)};
    auto cn = create_chain(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

        // create a stand-alone edge
//    Edge eg {eiE};
    auto eg = create_edge(eiE);

    if constexpr (profuse)
        eg.print("eg ");

        // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    constexpr EgId pos {len};
    const auto p = cn.insert_edge(std::move(eg), pos);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == len ? eiE : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, CmpId::undefined);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
    }
}


/// Tests insertion of an edge into an internal position in a multi-edge chain.
TEST_F(ChainTest, insertEdgeBulk)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests insertion of an edge into an internal ",
                          "position into a multi-edge chain");

    constexpr EgId len {5};

    // arbitrary values:
    // because no graph is available, no consistency is required
    constexpr ChId idw {7};

    constexpr EgId ei {5};
    constexpr EgId eiE {25};

    // create a multi-edge chain

//    Edge eg0{ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    if constexpr (profuse)
//        eg0.print("eg0 ");

//    Chain cn {len, std::move(eg0)};
    auto cn = create_chain(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    // create a stand-alone edge
//    Edge eg {eiE};
    auto eg = create_edge(eiE);

    if constexpr (profuse)
        eg.print("eg ");

    // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    constexpr auto pos {3};
    if constexpr (verboseT)
        jot("Inserting edge at pos a = ", pos);

    const auto p = cn.insert_edge(std::move(eg), pos);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(p, &cn.g[pos]);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == pos ? eiE
                                        : i > pos ? ei + i - 1
                                                  : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, CmpId::undefined);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
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

//    Edge eg0{ei, 0, idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif
//    if constexpr (profuse)
//        eg0.print("before ");

//    Chain cn {len, std::move(eg0)};
    auto cn = create_chain(idw, CmpId::undefined, len, ei);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

        // create a stand-alone edge
//    Edge eg {eiE};
    auto eg = create_edge(eiE);

    if constexpr (profuse)
        eg.print("eg ");

    // move the edge to the right-most position in the chain;
    // the original edges are not affected
    // p points to the new edge
    cn.append_edge(std::move(eg));

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.length(), len + 1);
    ASSERT_EQ(cn.idw, idw);
    ASSERT_EQ(cn.idc, CmpId::undefined);
    ASSERT_EQ(cn.c, CmpId::undefined);
    // insert_edge() does not update componenet-related internal edge data
    for (EgId i {}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, i == len ? eiE : ei + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].indc, CmpId::undefined);
        ASSERT_EQ(cn.g[i].w, cn.idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
    }
}
*/
/// Tests reverse_g(): reversing orientation of the edges in the chain.
TEST_F(ChainTest, reverse_g)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests reverse_g(): reversing orientation of the edges in the chain"
        );

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif
//    Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    cn.reverse_g();

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.idw, Config::idw);
    ASSERT_EQ(cn.c, CmpId::undefined);
    for (EgId i{}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].ind, conf.ei0 + Config::chlen - i - 1);
        ASSERT_EQ(cn.g[i].indc, EgId::undefined);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].w, Config::idw);
        ASSERT_EQ(cn.g[i].c, CmpId::undefined);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
    }
}



/// Tests set_cmpt(): updating the component index.
TEST_F(ChainTest, set_cmpt)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests set_cmpt(): updating the component index");

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    const auto newCl = Config::c + 100;
    const auto newIdc = Config::idc + 100;
    const auto newindc = conf.ei0 + 100;

    const auto res = cn.set_cmpt(newCl, newIdc, newindc);

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    ASSERT_EQ(cn.c, newCl);
    ASSERT_EQ(cn.idc, newIdc);
    for (EgId i{}; i<cn.length(); ++i) {
        ASSERT_EQ(cn.g[i].indc, newindc + i);
        ASSERT_EQ(cn.g[i].indw, i);
        ASSERT_EQ(cn.g[i].ind, conf.ei0 + i);
        ASSERT_EQ(cn.g[i].c, newCl);
        ASSERT_EQ(cn.g[i].w, Config::idw);
#ifdef GRAFFINE_CONTEXT_SPATIAL
            if (i < cn.length() - 1) {
                const auto p = cn.g[i].pos();
                const auto q = cn.g[i+1].pos();
                ASSERT_DOUBLE_EQ(p[1][0], q[0][0]);
                ASSERT_DOUBLE_EQ(p[1][1], q[0][1]);
                ASSERT_DOUBLE_EQ(p[1][2], q[0][2]);
            }
#endif
    }
    ASSERT_EQ(res, cn.g.back().indc + 1);
}



/// Tests end2a(): convertion of the end index to position of the boundary edge.
TEST_F(ChainTest, End2A)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests end2a(): ",
            "convertion of the end index to position of the boundary edge");

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    const Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("the chain ");
        print_vertices("the chain ");
    }

    ASSERT_EQ(cn.end2a(End::A), 0);
    ASSERT_EQ(cn.end2a(End::B), cn.length() - 1);
//    ASSERT_DEATH(cn.end2a(0), ::testing::Eq("Incorrect end index."));
}


/// Tests predicate returning true iff the chain has single unconnected end.
TEST_F(ChainTest, HasOneFreeEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests has_single_leaf_vertex(): predicate checking if ",
            "the chain has single unconnected end");

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    const Chain cn1 {Config::chlen, std::move(eg0)};
    auto cn1 = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn1.print("1 before ");
        print_vertices("1 before ");
    }

    ASSERT_FALSE(cn1.has_single_leaf_vertex());

//    Edge eg1 {conf.ei0, 0, Config::idw+1};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg1.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn2 {Config::chlen, std::move(eg1)};
    auto cn2 = create_chain(Config::idw+1,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn2.print("2 before ");
        print_vertices("2 before ");
    }

    cn2.ngs[End::A].insert(EndSlot{});

    if constexpr (profuse) {
        cn1.print("after ");
        cn2.print("after ");
        print_vertices("after ");
    }

    ASSERT_TRUE(cn2.has_single_leaf_vertex());
}


/// Tests correct Neig assignment.
TEST_F(ChainTest, NeigIndexes)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests correct Neig assignment");

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    for (const auto e : End::Ids)
        cn.ngs[e] = Neigs{EndSlot{0, Chain::opp_end(e)}};

    if constexpr (profuse) {
        cn.print("after ");
        print_vertices("after ");
    }

    for (const auto e : End::Ids) {
        ASSERT_EQ(cn.ngs[e]().front().w, 0);
        ASSERT_EQ(cn.ngs[e]().front().e, Chain::opp_end(e));
    }
}


/// Tests predicate checking if the chain is an unconnected cycle.
TEST_F(ChainTest, IsCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests is_unconnected_cycle(): checking if the ",
                          "chain is an unconnected cycle");

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    const Chain cn0 {Config::chlen, std::move(eg0)};
    auto cn0 = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn0.print("0 before ");
        print_vertices("0 before ");
    }

    ASSERT_FALSE(cn0.is_unconnected_cycle());

    Edge eg1 {conf.ei0, 0, Config::idw+1};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg1.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn1 {Config::chlen, std::move(eg1)};
    auto cn1 = create_chain(Config::idw+1,
                            CmpId::undefined,
                            Config::chlen,
                            Config::ei0);

    cn1.ngs[End::A].insert(EndSlot{0, End::B});
    cn1.ngs[End::B].insert(EndSlot{0, End::A});

    if constexpr (profuse) {
        cn0.print("after ");
        cn1.print("after ");
        print_vertices("after ");
    }

    ASSERT_TRUE(cn1.is_unconnected_cycle());
}


/// Tests num_vertices<D>(): counting of vertex numbers by degree.
TEST_F(ChainTest, NumVertices)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests num_vertices<D>(): counting of vertex numbers by degree"
        );

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn {Config::chlen, std::move(eg0)};
    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }

    ASSERT_EQ(cn.num_vertices<1>(), 2);
    ASSERT_EQ(cn.num_vertices<2>(), Config::chlen - 1);
    ASSERT_EQ(cn.num_vertices<3>(), 0);
    ASSERT_EQ(cn.num_vertices<4>(), 0);

    if constexpr (profuse)
        cn.print("after ");

//    Edge eg1 {conf.ei0, 0, Config::idw+1};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg1.set_pos({{{0.}, {1.}}});
#endif

//    Chain cn1 {Config::chlen, std::move(eg1)};
    auto cn1 = create_chain(Config::idw+1,
                           CmpId::undefined,
                           Config::chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn1.print("1 before ");
        print_vertices("1 before ");
    }

    cn1.ngs[End::A].insert(EndSlot{0, End::B});
    cn1.ngs[End::B].insert(EndSlot{0, End::A});

    if constexpr (profuse) {
        cn1.print("1 after ");
        print_vertices("1 after ");
    }

    ASSERT_EQ(cn1.num_vertices<1>(), 0);
    ASSERT_EQ(cn1.num_vertices<2>(), Config::chlen);
    ASSERT_EQ(cn1.num_vertices<3>(), 0);
    ASSERT_EQ(cn1.num_vertices<4>(), 0);
}


/// Tests egEnd_to_bulkslot():
/// convertion of an internal chain vertex descriptor into a slot.
TEST_F(ChainTest, EgEnd_To_Bulkslot)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests egEnd_to_bulkslot():",
            " convertion of an internal chain vertex descriptor into a slot"
        );

    constexpr EgId chlen {3};

//    Edge eg0 {conf.ei0, 0, Config::idw};

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

//    const Chain cn0 {chlen, std::move(eg0)};
    auto cn0 = create_chain(Config::idw,
                           CmpId::undefined,
                           chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn0.print("cn0 before ");
        print_vertices("cn0 before ");
    }

    BulkSlot bs0 {cn0.idw, 0};
    BulkSlot bs1 {cn0.idw, 1};
    BulkSlot bs2 {cn0.idw, 2};
    BulkSlot bs3 {cn0.idw, 3};

    auto cn {cn0};

    if constexpr (profuse) {
        cn.print("cn before ");
        print_vertices("cn before ");
    }

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn.g[0].reverse();

    if constexpr (profuse) {
        cn.print("g0r ");
        print_vertices("g0r ");
    }

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn = cn0;

    cn.g[1].reverse();

    if constexpr (profuse) {
        cn.print("g1r ");
        print_vertices("g1r ");
    }

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn = cn0;

    cn.g[2].reverse();

    if constexpr (profuse) {
        cn.print("g2r ");
        print_vertices("g2r ");
    }

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs3);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs2);
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

#ifdef GRAFFINE_CONTEXT_SPATIAL
        // The 1st edge added to a chain must have position set explicitly.
        eg0.set_pos({{{0.}, {1.}}});
#endif

    auto cn = create_chain(Config::idw,
                           CmpId::undefined,
                           chlen,
                           Config::ei0);

    if constexpr (profuse) {
        cn.print("before ");
        print_vertices("before ");
    }
    // Finds edge end of the edge arg2 to which the edge at arg1 is connected
    // via a vertex internal to the chain.

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[0].reverse();

    if constexpr (profuse) {
        cn.print("g0r ");
        print_vertices("g0r ");
    }

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[1].reverse();

    if constexpr (profuse) {
        cn.print("g1r ");
        print_vertices("g1r ");
    }

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::A);

    cn.g[2].reverse();

    if constexpr (profuse) {
        cn.print("g2r ");
        print_vertices("g2r ");
    }

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::A);

    cn.g[1].reverse();

    if constexpr (profuse) {
        cn.print("g1r ");
        print_vertices("g1r ");
    }

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[0].reverse();

    if constexpr (profuse) {
        cn.print("g0r ");
        print_vertices("g0r ");
    }

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);
}
*/
}  // namespace graffine::tests::chain
