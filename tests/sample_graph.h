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

#ifndef GRAFFINE_TESTS_SAMPLE_GRAPH_H
#define GRAFFINE_TESTS_SAMPLE_GRAPH_H

#include "common.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_22.h"


#include <array>
#include <iostream>
#include <string>


namespace graffine::tests {

namespace detail {

namespace trs = graffine::transforms;

template<Degree D1,
         Degree D2,
         typename G> requires (isImplementedDegree<D1> &&
                               isImplementedDegree<D2>)
struct VertexMerger
    : public graffine::transforms::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : transforms::vertex_merger::From<D1, D2, G> {graph}
    {}
};

    static constexpr std::array<ChIdG, 14>  w   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13};
    static constexpr std::array<size_t, 14> len {3, 1, 3, 5, 3, 5, 7, 6, 5, 4, 2, 2, 2, 4};

}  // namespace detail

template<typename G>
G create_sample_graph()
{
    using Chain = G::Chain;
    using End = Chain::End;
    using ESlot = Chain::EndSlot;
    using BSlot = Chain::BulkSlot;
    constexpr auto eA = End::A;
    constexpr auto eB = End::B;
    G gr;
    const std::array<ChIdG, 14>& w = detail::w;

    for (const auto& o : detail::len)
        gr.add_single_chain_component(o);

    // single-chain components:

    // c0 : w[0] : length 3
    // c1 : w[1] : length 1

    detail::VertexMerger<1, 2, G> merge12 {gr};

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

    detail::VertexMerger<2, 2, G> merge22 {gr};

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

   // components, containing unconnected cycle chains:

    detail::VertexMerger<1, 1, G> merge11 {gr};

    // c5 : w[12] : length (2) -> (2)
    merge11(ESlot{w[12], eA},
            ESlot{w[12], eB});

    // c3: w[13] : length (4) -> (4)
    merge11(ESlot{w[13], eA},
            ESlot{w[13], eB});

    return gr;
}

/*
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


}  // namespace graffine::tests

#endif  // GRAFFINE_TESTS_SAMPLE_GRAPH_H
