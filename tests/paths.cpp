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
#include <iostream>
#include <string>

#include "common.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/structure/basic/vertices/degrees.h"
#include "graph-mutator/structure/basic/paths/over_endslots/generic.h"
#include "graph-mutator/structure/basic/paths/over_edges/generic.h"
#include "graph-mutator/transforms/vertex_merger/from_11.h"
#include "graph-mutator/transforms/vertex_merger/from_12.h"
#include "graph-mutator/transforms/vertex_merger/from_22.h"


namespace graph_mutator::tests::paths {


using G = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using Ends = Chain::Ends;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;


template<Degree D1,
         Degree D2,
         typename G> requires (is_implemented_degree<D1> &&
                               is_implemented_degree<D2>)
struct VertexMerger
    : public vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : vertex_merger::From<D1, D2, G> {graph}
    {}
};


class PathTest
    : public Test {

protected:

    static constexpr auto undefined = graph_mutator::undefined<szt>;

    G create_graph() const;
};

// =============================================================================

inline
auto PathTest::
create_graph() const -> G
{
    //                             w =  0  1  2  3  4  5  6  7  8  9 10 11 12 13
    constexpr std::array<EgId, 14> len {3, 1, 3, 5, 3, 5, 7, 6, 5, 4, 2, 2, 2, 4};

    std::array<ChId, len.size()> w;
    std::iota(w.begin(), w.end(), 0);

    constexpr auto eA = Ends::A;
    constexpr auto eB = Ends::B;

    G gr;

    for (ChId ind {}; const auto o : len)
        gr.add_single_chain_component(o, ind++);

    // single-chain components:

    // c0 : w[0] : length 3
    // c1 : w[1] : length 1

    VertexMerger<1, 2, G> merge12 {gr};

    // components, each containing three linear chains, over a 3-way junction:

    // c2 : w[2], w[3], w[14] : length (3, 5) -> (3, 2, 3)
    merge12(ESlot{w[2], eB},
            BSlot{w[3], 2});     // adds w[14]

    // c4 : w[4], w[5], w[15] : length (3, 5) -> (3, 1, 4)
    merge12(ESlot{w[4], eA},
            BSlot{w[5], 1});     // adds w[15]

   // components, each containing a linear chain and a cycle, over a 3-way junction:

    // c6 : w[6], w[16] : length (7) -> (2, 5)
    merge12(ESlot{w[6], eA},
            BSlot{w[6], 2});     // adds w[16]

    // c7 : w[7], w[17] : length (6) -> (1, 5)
    merge12(ESlot{w[7], eB},
            BSlot{w[7], 1});     // adds w[17]

    VertexMerger<2, 2, G> merge22 {gr};

   // components, each containing two linear chains and a cycle, over a 4-way junction:

    // c8 : w[8], w[18], w[19] : length (5) -> (2, 1, 2)
    merge22(BSlot{w[8], 2},
            BSlot{w[8], 4});     // adds w[18], w[19]

    //  c9 : w[9], w[20], w[21] : length (4) -> (1, 1, 2)
    merge22(BSlot{w[9], 1},
            BSlot{w[9], 3});     // adds w[20], w[21]

   // component, containing four linear chains, over a 4-way junction:

    // c10 : w[10], w[11], w[22], w[23] : length (2, 2) -> (1, 1, 1, 1)
    merge22(BSlot{w[10], 1},
            BSlot{w[11], 1});     // adds w[22], w[23]

   // components, containing disconnected cycle chains:

    VertexMerger<1, 1, G> merge11 {gr};

    // c5 : w[12] : length (2) -> (2)
    merge11(ESlot{w[12], eA},
            ESlot{w[12], eB});

    // c3: w[13] : length (4) -> (4)
    merge11(ESlot{w[13], eA},
            ESlot{w[13], eB});

    //gr.print_components("        ");
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
       return gr;
}


TEST_F(PathTest, OverEndSlots)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the shortest path between chain boundary edges"
        );

    const auto gr = create_graph();

//    for (const auto& c: gr.ct) {
        const auto& c = gr.ct[8];
        c.print();
        structure::basic::paths::over_endslots::Generic<G::Cmpt> pp {c};
        for (const auto w: c.ww) {
            for (const auto e: Ends::Ids) {
                const ESlot s {w, e};
                const ESlot t {19, Ends::B};
                const auto path = pp.template find_shortest_path<true, true>(s, t);
                pp.print_distances("    From ", s.str_short(), " :: ");
                pp.print_path(path, ": Shortest ");
                log_("");
            }
        }
        log_("");
//    }
}


TEST_F(PathTest, OverEdgeInds)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "the shortest path between arbirtrary edges ",
            "in a disconnected component"
        );

    const auto gr = create_graph();

//    for (const auto& c: gr.ct) {
        const auto& c = gr.ct[8];
        c.print();
        structure::basic::paths::over_edges::Generic<G::Cmpt> pp {&c};
        for (EgId e1 {}; e1 < c.num_edges(); ++e1) {
            pp.compute_from_source(e1);
            pp.print_distances("    from ", e1, " :: ");
            for (EgId e2 {}; e2 < c.num_edges(); ++e2) {
                const auto path = pp.template find_shortest_path<false>(e1, e2);
                pp.print_path<false>(path, ": Shortest ");
                log_("");
            }
        }
        log_("");
//    }
}

/*
TEST_F(PathTest, OverEgIds)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "OverEgIds"
        );

    const auto gr = create_graph();

//    for (const auto& c: gr.ct) {

        const auto& c = gr.ct[8];
        paths::Paths<EgId, G::Cmpt> pp {c};

        const auto ajl_old = gr.make_adjacency_list_old();

        for (EgId indc {}; indc<c.num_edges(); ++indc) {

            const auto s = indc;
            const auto t = 3;
            log_("component ", c.ind, ": source ", indc, ": ");
//           pp.template compute_from_source<true>(s);
            const auto path = pp.find_shortest_path(s, t);
            pp.print_distances("    from ", indc, " :: ");

            typename G::Base::pathT pr_old;
            std::vector<typename G::Base::weight_t> min_dist_old;

            gr.compute_paths(typename G::Base::vertex_t(c.gl[indc].i), ajl_old, min_dist_old, pr_old);
            for (EgId tar {}; const auto& z: c.gl) {
                const auto prev = is_defined(pr_old[z.i]) ? gr.edge(pr_old[z.i]).indc : pr_old[z.i];
                log_<false>(tar++, "=>[", prev, " ", min_dist_old[z.i], "] ");
            }
            log_("");

            const auto path_old_vec =
                gr.shortest_path(typename G::Base::vertex_t(c.gl[indc].i),
                                 typename G::Base::vertex_t(c.gl[t].i),
                                 ajl_old);
            const auto path_old_dq = pp.from_global_ind(path_old_vec);
            pp.print_path(path, "new");
            pp.print_path(path_old_dq, "old");
        }
        log_("");
//    }

}

*/
}  // namespace graph_mutator::tests::paths
