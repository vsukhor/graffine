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

#include "pulling_hooks.h"

namespace graffine::tests::pulling::hooks {

class PullingHooksTest
    : public Test {};

// =============================================================================


/// Degree 1 pull through two junctions; the last step dissolves the source.
TEST_F(PullingHooksTest, Deg1_TwoJunctions)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the pulling hooks of a degree 1 pull through two junctions");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w1);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {w4, eB}};
    };
    for (int n {1}; n <= 3; ++n)
        check_hooks<Deg1>(make, n);
}


/// Degree 2 pull through a junction; the last step dissolves the source.
TEST_F(PullingHooksTest, Deg2_Junction)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the pulling hooks of a degree 2 pull through a junction");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.g[1], eA},
                       Source {w4, eB}};
    };
    for (int n {1}; n <= 3; ++n)
        check_hooks<Deg2>(make, n);
}


/// Degree 2 pull over a single-edge path.
TEST_F(PullingHooksTest, Deg2_SingleEdgePath)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the pulling hooks of a degree 2 pull over a single edge");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w4);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eB), eA},
                       Source {w4, eB}};
    };

    Pullee s;
    make(s);
    ASSERT_EQ(s.path.length(), 1);
    const auto rec = std::make_shared<Recorder>(s.gr);
    Pull<Deg2> {s.gr, rec}(s.path, 2);

    // The edge is reversed: the topology does not change.
    const auto& h = rec->pulling;
    EXPECT_EQ(h.starts, 1);
    ASSERT_EQ(h.ends.size(), 2);
    for (const auto& e : h.ends) {
        EXPECT_TRUE(e.isContiguous);
        EXPECT_EQ(e.numVertices, 2);
    }
}


/// Degree 3 pull through a junction; the last step dissolves the source.
TEST_F(PullingHooksTest, Deg3_Junction)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the pulling hooks of a degree 3 pull through a junction");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {w4, eB}};
    };
    for (int n {1}; n <= 3; ++n)
        check_hooks<Deg3>(make, n);
}


/// Degree 3 pull with the source on the driver chain.
TEST_F(PullingHooksTest, Deg3_SameChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the pulling hooks of a degree 3 pull along a single chain");

    const Maker make = [](Pullee& s) {
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w4);
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {w4, eB}};
    };
    for (int n {1}; n <= 2; ++n)
        check_hooks<Deg3>(make, n);
}


/**
 * One step of a degree 2 and a degree 3 pull: the driver vertex becomes a
 * leaf, and the path vertex next to it the junction taking over the edges
 * formerly connected at the driver vertex.
 */
TEST_F(PullingHooksTest, FirstStepTopology)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the topology after one step of degree 2 and 3 pulls");

    /// Expects vertex \p v to be at one of the ends of edge \p i.
    const auto expect_at = [](const G& gr, const EgIdG i, const Vertex& v)
    {
        const auto& eg = gr.edge(i);
        EXPECT_TRUE(*eg.vertex_at_inner(eA) == v ||
                    *eg.vertex_at_inner(eB) == v) << "edge " << i;
    };

    {   // degree 2: driver between edges 0 (not in the path) and 1 of w3
        Pullee s;
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        const auto iQ = m.g[0].ind;
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.g[1], eA},
                       Source {w4, eB}};
        const auto iD = s.path.pthw[0];
        const auto i1 = s.path.pthw[1];

        Pull<Deg2> {s.gr, std::make_shared<Recorder>(s.gr)}(s.path, 1);

        const auto vv = s.path.vertices();
        ASSERT_EQ(vv.size(), s.path.length() + 1);
        EXPECT_EQ(vv[0]->deg(), Deg1);
        EXPECT_EQ(vv[1]->deg(), Deg3);
        expect_at(s.gr, iD, *vv[1]);
        expect_at(s.gr, i1, *vv[1]);
        expect_at(s.gr, iQ, *vv[1]);
    }
    {   // degree 3: driver at J1, joining w3 with w0 and w1
        Pullee s;
        make_two_junctions(s.gr);
        const auto& m = s.gr.chain(w3);
        const auto i0 = s.gr.chain(ChIdG{0}).end_edge(eB).ind;
        const auto i1 = s.gr.chain(w1).end_edge(eB).ind;
        s.path = Path {&s.gr.compt(CmpId{}), Driver {&m.end_edge(eA), eA},
                       Source {w4, eB}};
        const auto iD = s.path.pthw[0];
        const auto iN = s.path.pthw[1];

        Pull<Deg3> {s.gr, std::make_shared<Recorder>(s.gr)}(s.path, 1);

        const auto vv = s.path.vertices();
        ASSERT_EQ(vv.size(), s.path.length() + 1);
        EXPECT_EQ(vv[0]->deg(), Deg1);
        EXPECT_EQ(vv[1]->deg(), Deg4);
        expect_at(s.gr, iD, *vv[1]);
        expect_at(s.gr, iN, *vv[1]);
        expect_at(s.gr, i0, *vv[1]);
        expect_at(s.gr, i1, *vv[1]);
    }
}

}  // namespace graffine::tests::pulling::hooks
