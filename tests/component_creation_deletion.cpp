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
#include "graffine/definitions.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "sample_graph.h"

#include <array>
#include <iostream>


namespace graffine::tests::component_creation_deletion {

namespace els = structure;
namespace trs = transforms;
using G = els::Graph<
          els::Component<
          els::Chain<
          els::Edge<
          els::Vertex>>>>;
using End = G::Chain::End;
constexpr auto eA = End::A;
constexpr auto eB = End::B;

/// Subclass to make protected members accessible for testing:
template<typename G>
struct CreateComponent
    : public trs::component_creation::Functor<G> {

    explicit CreateComponent(G& graph)
        : trs::component_creation::Functor<G> {graph}
    {}
};

template<typename G>
struct DeleteComponent
    : public trs::component_deletion::Functor<G> {

    explicit DeleteComponent(G& graph)
        : trs::component_deletion::Functor<G> {graph}
    {}
};


using CreateComponentTest = Test;
using DeleteComponentTest = Test;


// =============================================================================


/// Creation of single-edge and multi-edge components.
TEST_F(CreateComponentTest, Test1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("creation of single-edge and multi-edge components");

    constexpr std::array<size_t, 3> len {3, 1, 5};

    constexpr std::array<ChIdG, len.size()> w {0, 1, 2};

    G gr;

    CreateComponent<G> create_comp {gr};  // component creating functor

    for (std::size_t i {}, s {}; i<len.size(); ++i) {

        const auto c = create_comp(len[i]);

        s += len[i];

        ASSERT_EQ(gr.num_edges(), s);
        ASSERT_EQ(gr.num_chains(), i + 1);
        ASSERT_EQ(gr.num_compts(), i + 1);

        for (std::size_t j {}, t{}; j<=i; ++j) {
            const auto& m = gr.chain(w[j]);
            ASSERT_EQ(m.idw, w[j]);
            ASSERT_EQ(m.c, w[j]());
            ASSERT_EQ(m.length(), len[j]);
            ASSERT_EQ(m.g[0].ind, t);
            ASSERT_EQ(m.g.back().ind, t + len[j] - 1);
            ASSERT_EQ(m.ngs[eA].num(), 0);
            ASSERT_EQ(m.ngs[eB].num(), 0);
            t += len[j];
        }

        ASSERT_EQ(gr.num_vertices(Deg1), 2*gr.num_chains());
        ASSERT_EQ(gr.num_vertices(Deg2), s - gr.num_chains());
        ASSERT_EQ(gr.num_vertices(Deg3), 0);
        ASSERT_EQ(gr.num_vertices(Deg4), 0);
        ASSERT_EQ(gr.get_egl().size(), gr.num_edges());
        ASSERT_EQ(c[0], i);

        for (CmpId i {}; i<gr.num_compts(); ++i) {
            const auto& c = gr.compt(i);
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 1);
            ASSERT_EQ(c.num_edges(), gr.chain(ChIdG{i()}).length());
            c.template check<profuse>();
            ASSERT_EQ(c.ww[0], i());
            const auto& m = gr.chain(c.ww[0]);
            ASSERT_EQ(m.c, c.ind);
            ASSERT_EQ(m.idc, 0);
            for (std::size_t j {}; j<m.length(); ++j)
                ASSERT_EQ(m.g[j].c, c.ind);
            ASSERT_EQ(c.chis.cn_11()[0], c.ww[0]);
            ASSERT_EQ(c.chis.num({2, 2}), 0);
            ASSERT_EQ(c.chis.num({3, 3}), 0);
            ASSERT_EQ(c.chis.num({4, 4}), 0);
            ASSERT_EQ(c.chis.num({1, 3}), 0);
            ASSERT_EQ(c.chis.num({1, 4}), 0);
            ASSERT_EQ(c.chis.num({3, 4}), 0);
            c.template check<profuse>();
        }
    }
}

/// Deletion of graph connected components having various topologies and sizes.
TEST_F(DeleteComponentTest, Test1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Deletion of graph connected components having various topologies ",
            "and sizes"
        );
        //                             w =  0  1  2  3  4  5  6  7  8  9 10 11 12 13
//    constexpr std::array<size_t, 14> len {3, 1, 3, 5, 3, 5, 7, 6, 5, 4, 2, 2, 2, 4};
//    const std::array w = id_sequence<ChIdG, len.size()>();

    // Create initial graph (24 chains over 11 connected components).
    auto gr = create_sample_graph<G>();

    if constexpr (profuse)
        gr.print_components(tagBefore);
    /* Prints:
        0 0 (len 3) ** 0 >>> ( 0 1 2 )

        1 1 (len 1) ** 1 > ( 3 )

        2 2 (len 3) **{ 3 B }{ 14 A } 2 >>> ( 4 5 6 )
        2 3 (len 2) **{ 2 B }{ 14 A } 2 >> ( 7 8 )
        2 14 (len 3) { 2 B }{ 3 B }** 2 >>> ( 9 10 11 )

        3 13 (len 4) { 13 B }**{ 13 A } 3 >>>> ( 48 49 50 51 )

        4 4 (len 3) { 5 B }{ 15 A }** 4 >>> ( 12 13 14 )
        4 5 (len 1) **{ 4 A }{ 15 A } 4 > ( 15 )
        4 15 (len 4) { 4 A }{ 5 B }** 4 >>>> ( 16 17 18 19 )

        5 12 (len 2) { 12 B }**{ 12 A } 5 >> ( 46 47 )

        6 6 (len 2) { 6 B }{ 16 A }**{ 6 A }{ 16 A } 6 >> ( 20 21 )
        6 16 (len 5) { 6 A }{ 6 B }** 6 >>>>> ( 22 23 24 25 26 )

        7 7 (len 1) **{ 17 A }{ 17 B } 7 > ( 27 )
        7 17 (len 5) { 7 B }{ 17 B }**{ 7 B }{ 17 A } 7 >>>>> ( 28 29 30 31 32 )

        8 8 (len 2) **{ 19 A }{ 19 B }{ 18 A } 8 >> ( 33 34 )
        8 18 (len 1) { 19 A }{ 19 B }{ 8 B }** 8 > ( 37 )
        8 19 (len 2) { 8 B }{ 19 B }{ 18 A }**{ 19 A }{ 8 B }{ 18 A } 8 >> ( 35 36 )

        9 9 (len 1) **{ 21 A }{ 21 B }{ 20 A } 9 > ( 38 )
        9 20 (len 1) { 21 A }{ 21 B }{ 9 B }** 9 > ( 41 )
        9 21 (len 2) { 9 B }{ 21 B }{ 20 A }**{ 21 A }{ 9 B }{ 20 A } 9 >> ( 39 40 )

        10 10 (len 1) **{ 11 B }{ 22 A }{ 23 A } 10 > ( 42 )
        10 11 (len 1) **{ 10 B }{ 22 A }{ 23 A } 10 > ( 44 )
        10 22 (len 1) { 11 B }{ 10 B }{ 23 A }** 10 > ( 43 )
        10 23 (len 1) { 11 B }{ 22 A }{ 10 B }** 10 > ( 45 )
    */

    DeleteComponent del {gr};

    // note: on each deletion the numeration is updated to preserve continuity
    del(9);   // c9
    del(7);   // c7
    del(5);   // c5
    del(1);   // c1
    del(1);   // c10
    del(4);   // c4
    del(3);   // c3
    del(1);   // c6
    del(2);   // c2
    del(0);   // c0
    del(0);   // c8

    if constexpr (profuse)
        gr.print_components(tagAfter);
}


}  // namespace graffine::tests::component_creation_deletion
