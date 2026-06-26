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
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/end.h"
#include "graffine/structure/descriptors/slot.h"

namespace graffine::tests::edge {

namespace elements = structure;
class EdgeTest
    : public Test {

protected:

    using Edge = elements::Edge<elements::Vertex>;
    static constexpr auto eA = End::A;
    static constexpr auto eB = End::B;

    void print(const Vertex& v,
               const Edge& e,
               const char* s)
    {
        v.print(s);
        e.print(s);
    }
};

TEST_F(EdgeTest, Constructor)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge constructor");

    Edge e {3, 4, 5, 6, 7};

    ASSERT_EQ(3, e.ind);
    ASSERT_EQ(4, e.indc);
    ASSERT_EQ(5, e.indw);
    ASSERT_EQ(6, e.w);
    ASSERT_EQ(7, e.c);
    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}


TEST_F(EdgeTest, Orientation_PointsForwards_InnerEndId_Reverse)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge orientation and reversal functions");

    Edge e {3, 4, 5, 6, 7};

    // default is forwards
    ASSERT_EQ(e.orientation(), Orientation::Forwards);
    ASSERT_TRUE(e.points_forwards());
    ASSERT_EQ(e.inner_endId(eA), eA);
    ASSERT_EQ(e.inner_endId(eB), eB);

    e.reverse<true>();

    ASSERT_EQ(e.orientation(), Orientation::Backwards);
    ASSERT_FALSE(e.points_forwards());
    ASSERT_EQ(e.inner_endId(eA), eB);
    ASSERT_EQ(e.inner_endId(eB), eA);
}


TEST_F(EdgeTest, OppEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests access to edge opposite end");

    Edge e {3, 4, 5, 6, 7};

    ASSERT_EQ(e.opp_end(eA), eB);
    ASSERT_EQ(e.opp_end(eB), eA);
    ASSERT_EQ(e.opp_end(eA), eB);
    ASSERT_EQ(e.opp_end(eB), eA);
}


TEST_F(EdgeTest, SetCmp)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests setting edge data related to positioning in",
                          " the hosting compartment");

    Edge e {3, 4, 5, 6, 7};

    e.set_cmp(70, 40);

    ASSERT_EQ(3, e.ind);
    ASSERT_EQ(40, e.indc);
    ASSERT_EQ(5, e.indw);
    ASSERT_EQ(6, e.w);
    ASSERT_EQ(70, e.c);
}


TEST_F(EdgeTest, Connect_inner_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting via ",
                          "inner/inner end notations");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "bbb");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "ciA");

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eA);

    if constexpr (profuse)
        print(v, e, "diA");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.connect_to_inner(eB, v);

    if constexpr (profuse)
        print(v, e, "ciB");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_TRUE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eB);

    if constexpr (profuse)
        print(v, e, "dib");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}

TEST_F(EdgeTest, Connect_outer_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting via ",
                          "outer/outer end notations");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "bbb");

    e.connect_to_outer(eA, v);

    if constexpr (profuse)
        print(v, e, "coA");

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_outer(eA);

    if constexpr (profuse)
        print(v, e, "doA");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.connect_to_outer(eB, v);

    if constexpr (profuse)
        print(v, e, "coB");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_TRUE(e.is_connected_outer(eB));

    e.disconnect_end_outer(eB);

    if constexpr (profuse)
        print(v, e, "doB");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}

TEST_F(EdgeTest, Connect_outer_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting via a ",
                          "combination of inner/outer notations");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "bbb");

    e.connect_to_outer(eA, v);

    if constexpr (profuse)
        print(v, e, "coA");

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eA);

    if constexpr (profuse)
        print(v, e, "diA");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.connect_to_outer(eB, v);

    if constexpr (profuse)
        print(v, e, "coB");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_TRUE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eB);

    if constexpr (profuse)
        print(v, e, "diB");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}


TEST_F(EdgeTest, Connect_outer_reverse_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "outer/outer with reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_outer(eA, v);

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    if constexpr (profuse)
        print(v, e, "c");

    e.reverse<true>();

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    // reversed, but from outside, the connectivity did not changed:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_outer(eA);

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}


TEST_F(EdgeTest, Connect_outer_reverse_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "outer/inner with reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_outer(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.reverse<true>();

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    // reversed, but from outside, the connectivity did not changed:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eB);  // in inner coordinates, eA was not connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}

TEST_F(EdgeTest, Connect_inner_reverse_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "inner/inner with reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_TRUE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    // not reversed, so the same as inner:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.reverse<true>();

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_TRUE(e.is_connected_inner(eB));
    // reversed, but from outside, the connectivity did not changed:
    ASSERT_TRUE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));

    e.disconnect_end_inner(eB);  // in inner coordinates, eA was not connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_FALSE(e.is_connected_inner(eA));
    ASSERT_FALSE(e.is_connected_inner(eB));
    ASSERT_FALSE(e.is_connected_outer(eA));
    ASSERT_FALSE(e.is_connected_outer(eB));
}

TEST_F(EdgeTest, Connect_outer_vertex_at_reverse_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "outer/outer with outer reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_outer(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vertex_at_inner(eA), &v);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), &v);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_outer(eA);

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Connect_inner_vertex_at_reverse_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "innner/inner with inner reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vertex_at_inner(eA), &v);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), &v);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_inner(eB);  // in inner coordinates, eA was not connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Connect_inner_vertex_at_reverse_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "innner/outer with outer reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vertex_at_inner(eA), &v);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), &v);
    ASSERT_EQ(e.vertex_at_outer(eA), &v);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_outer(eA);  // in outer coordinates, eA is still connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Connect_outer_vid_at_reverse_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "outer/outer with outer reversal by vid");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_outer(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vid_at_inner(eA), v.id);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vid_at_inner(eB), v.id);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_outer(eA);

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Connect_inner_vid_at_reverse_disconnect_inner)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "inner/inner with reversal by vid");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};
    Vertex vundef {};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vid_at_inner(eA), v.id);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vid_at_inner(eB), v.id);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_inner(eB);  // in inner coordinates, eA was not connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Connect_inner_vid_at_reverse_disconnect_outer)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests edge connecting/disconnecting in combination ",
                          "inner/outer with reversal by vid");

    Edge e {3, 4, 5, 6, 7};

    Vertex v {e.c};
    Vertex vundef {};

    if constexpr (profuse)
        print(v, e, "b");

    e.connect_to_inner(eA, v);

    if constexpr (profuse)
        print(v, e, "c");

    ASSERT_EQ(e.vid_at_inner(eA), v.id);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse)
        print(v, e, "r");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vid_at_inner(eB), v.id);
    ASSERT_EQ(e.vid_at_outer(eA), v.id);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);

    e.disconnect_end_outer(eA);  // in outer coordinates, eA is still connected

    if constexpr (profuse)
        print(v, e, "a");

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}

TEST_F(EdgeTest, Vertex_edge_consistency_on_reverse)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests consistensy between edge and vertex ",
                          "desctiptors upon an edge reversal");

    Edge e {3, 4, 5, 6, 7};

    Vertex v1 {e.c};
    Vertex v2 {e.c};

    if constexpr (profuse) {
        print(v1, e, "b");
        print(v2, e, "b");
    }

    e.connect_to_inner(eA, v1);
    e.connect_to_inner(eB, v2);

    if constexpr (profuse) {
        print(v1, e, "c");
        print(v2, e, "c");
    }

    ASSERT_EQ(e.vertex_at_inner(eA), &v1);
    ASSERT_EQ(e.vertex_at_inner(eB), &v2);
    ASSERT_EQ(e.vertex_at_outer(eA), &v1);
    ASSERT_EQ(e.vertex_at_outer(eB), &v2);

    ASSERT_TRUE(e.vertex_at_inner(eA)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eA}));  // v1
    ASSERT_FALSE(e.vertex_at_inner(eA)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eB}));  // v1
    ASSERT_FALSE(e.vertex_at_inner(eB)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eA}));  // v2
    ASSERT_TRUE(e.vertex_at_inner(eB)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eB}));  // v2

    e.reverse<true>();  // reverses what is seen by inner but not by outer

    if constexpr (profuse) {
        print(v1, e, "r");
        print(v2, e, "r");
    }

    ASSERT_EQ(e.vertex_at_inner(eA), &v2);
    ASSERT_EQ(e.vertex_at_inner(eB), &v1);
    ASSERT_EQ(e.vertex_at_outer(eA), &v1);
    ASSERT_EQ(e.vertex_at_outer(eB), &v2);

    ASSERT_FALSE(e.vertex_at_inner(eB)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eA}));  // v1
    ASSERT_TRUE(e.vertex_at_inner(eB)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eB}));  // v1
    ASSERT_TRUE(e.vertex_at_inner(eA)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eA}));  // v2
    ASSERT_FALSE(e.vertex_at_inner(eA)->is_conected_to(Vertex::EgDescr{e.id, e.indc, eB}));  // v2

    e.disconnect_end_inner(eA);
    e.disconnect_end_inner(eB);

    if constexpr (profuse) {
        print(v1, e, "a");
        print(v2, e, "a");
    }

    ASSERT_EQ(e.vertex_at_inner(eA), nullptr);
    ASSERT_EQ(e.vertex_at_inner(eB), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eA), nullptr);
    ASSERT_EQ(e.vertex_at_outer(eB), nullptr);
}


}  // namespace graffine::tests::edge
