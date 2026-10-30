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

#include "vertex_merger_common.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/vertex.h"

#include <cstddef>

namespace graffine::tests::chain {

using vertex_merger::VertexMerger;
using vertex_merger::VertexMerger1;

class ChainTest
    : public Test {

protected:

    using Neigs = Chain::Neigs;

    struct Config {

        static constexpr std::size_t chlen {4};
        static constexpr CmpId c {34};
        static constexpr ChIdG idw {7};
        static constexpr ChIdC idc {17};
    };
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

    ASSERT_EQ(cn.idw, ChIdG::undefined);
    ASSERT_EQ(cn.idc, ChIdC::undefined);
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
    ASSERT_EQ(cn.idc, ChIdC::undefined);
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


/// Tests indexes of edges in a chain created as a graph component.
TEST_F(ChainTest, CreatedInGraph)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests indexes of edges in a chain created as a graph component");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);

    const auto& cn = gr.chain(ChIdG{});

    if constexpr (profuse)
        cn.print("new chain ");

    ASSERT_EQ(cn.length(), Config::chlen);
    for (EgIdA a {}; a<cn.length(); ++a) {
        const auto& eg = cn.g[a()];
        ASSERT_EQ(eg.ind, a());
        ASSERT_EQ(eg.indc, a());
        ASSERT_EQ(eg.indw, a);
        ASSERT_EQ(eg.w, cn.idw);
        ASSERT_EQ(eg.c, cn.c);
        ASSERT_TRUE(eg.points_forwards());
    }

    ASSERT_EQ(cn.ngs[End::A]().size(), 0);
    ASSERT_EQ(cn.ngs[End::B]().size(), 0);

    ASSERT_EQ(cn.idw, ChIdG{});
    ASSERT_EQ(cn.idc, ChIdC{});
    ASSERT_EQ(cn.c, CmpId{});
}


/// Tests reverse_g(): reversing orientation of the edges in the chain.
TEST_F(ChainTest, ReverseG)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests reverse_g(): reversing orientation of the edges in the chain"
        );

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);

    auto cn = gr.chain(ChIdG{});  // copy: leaves the graph intact

    if constexpr (profuse)
        cn.print("before ");

    cn.reverse_g();

    if constexpr (profuse)
        cn.print("after ");

    ASSERT_EQ(cn.idw, ChIdG{});
    ASSERT_EQ(cn.c, CmpId{});
    for (EgIdA a {}; a<cn.length(); ++a) {
        const auto& eg = cn.g[a()];
        ASSERT_EQ(eg.ind, Config::chlen - a() - 1);
        ASSERT_EQ(eg.indw, a);
        ASSERT_EQ(eg.w, cn.idw);
        ASSERT_FALSE(eg.points_forwards());
    }
}


/// Tests set_cmpt(): updating the component index.
TEST_F(ChainTest, SetCmpt)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests set_cmpt(): updating the component index");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);

    auto cn = gr.chain(ChIdG{});  // copy: leaves the graph intact

    cn.set_cmpt(Config::c, Config::idc);

    if constexpr (profuse)
        cn.print("after ");

    ASSERT_EQ(cn.c, Config::c);
    ASSERT_EQ(cn.idc, Config::idc);

    // The edges are not updated.
    for (EgIdA a {}; a<cn.length(); ++a) {
        const auto& eg = cn.g[a()];
        ASSERT_EQ(eg.c, CmpId{});
        ASSERT_EQ(eg.indc, a());
        ASSERT_EQ(eg.indw, a);
    }
}


/// Tests end2a(): convertion of the end index to position of the boundary edge.
TEST_F(ChainTest, End2A)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests end2a(): ",
            "convertion of the end index to position of the boundary edge");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);
    gr.add_single_chain_component(1);

    const auto& cn = gr.chain(ChIdG{0});
    ASSERT_EQ(cn.end2a(End::A), 0);
    ASSERT_EQ(cn.end2a(End::B), cn.length() - 1);

    const auto& cn1 = gr.chain(ChIdG{1});
    ASSERT_EQ(cn1.end2a(End::A), 0);
    ASSERT_EQ(cn1.end2a(End::B), 0);
}


/// Tests predicate returning true iff the chain has single unconnected end.
TEST_F(ChainTest, HasSingleLeafVertex)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests has_single_leaf_vertex(): predicate checking ",
            "if the chain has single unconnected end");

    const auto [w0, w1, w2] = id_sequence<ChIdG, 3>();

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);
    gr.add_single_chain_component(Config::chlen);

    ASSERT_FALSE(gr.chain(w0).has_single_leaf_vertex());
    ASSERT_FALSE(gr.chain(w1).has_single_leaf_vertex());

    // Attaches end A of w1 to the bulk of w0, splitting w0 into w0 and w2:
    VertexMerger1<Deg2, G> merge12 {gr};
    merge12(ESlot{w1, End::A}, BSlot{w0, 2});

    if constexpr (profuse)
        gr.print_chains("after ");

    ASSERT_EQ(gr.num_chains(), 3);
    for (const auto& w : {w0, w1, w2})
        ASSERT_TRUE(gr.chain(w).has_single_leaf_vertex());
}


/// Tests correct Neig assignment.
TEST_F(ChainTest, NeigIndexes)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests correct Neig assignment");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);

    auto cn = gr.chain(ChIdG{});  // copy: leaves the graph intact

    for (const auto e : End::Ids)
        cn.ngs[e] = Neigs{ESlot{cn.idw, Chain::opp_end(e)}};

    if constexpr (profuse)
        cn.print("after ");

    for (const auto e : End::Ids) {
        ASSERT_EQ(cn.ngs[e].num(), 1);
        ASSERT_EQ(cn.ngs[e]().begin()->w, cn.idw);
        ASSERT_EQ(cn.ngs[e]().begin()->e, Chain::opp_end(e));
    }
}


/// Tests predicates checking if the chain is a cycle.
TEST_F(ChainTest, IsCycle)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests is_unconnected_cycle() and is_cycle(): ",
                          "checking if the chain is a cycle");

    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(Config::chlen);
    gr.add_single_chain_component(Config::chlen);

    // Closes w1 into a cycle:
    VertexMerger<Deg1, Deg1, G> merge11 {gr};
    merge11(ESlot{w1, End::A}, ESlot{w1, End::B});

    if constexpr (profuse)
        gr.print_chains("after ");

    ASSERT_FALSE(gr.chain(w0).is_unconnected_cycle());
    ASSERT_FALSE(gr.chain(w0).is_cycle());
    ASSERT_TRUE(gr.chain(w1).is_unconnected_cycle());
    ASSERT_TRUE(gr.chain(w1).is_cycle());
}


/// Tests num_vertices(): counting of vertex numbers by degree.
TEST_F(ChainTest, NumVertices)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests num_vertices(): counting of vertex numbers by degree"
        );

    const auto [w0, w1, w2, w3] = id_sequence<ChIdG, 4>();

    G gr;
    const IntegrityCheck integrityCheck {gr};
    for (int i {}; i<3; ++i)
        gr.add_single_chain_component(Config::chlen);

    // Linear chain.
    const auto& cn0 = gr.chain(w0);
    ASSERT_EQ(cn0.num_vertices(Deg1), 2);
    ASSERT_EQ(cn0.num_vertices(Deg2), Config::chlen - 1);
    ASSERT_EQ(cn0.num_vertices(Deg3), 0);
    ASSERT_EQ(cn0.num_vertices(Deg4), 0);

    // Unconnected cycle.
    VertexMerger<Deg1, Deg1, G> merge11 {gr};
    merge11(ESlot{w1, End::A}, ESlot{w1, End::B});

    const auto& cn1 = gr.chain(w1);
    ASSERT_EQ(cn1.num_vertices(Deg1), 0);
    ASSERT_EQ(cn1.num_vertices(Deg2), Config::chlen);
    ASSERT_EQ(cn1.num_vertices(Deg3), 0);
    ASSERT_EQ(cn1.num_vertices(Deg4), 0);

    // Chain attached by end A to a 3-way junction.
    VertexMerger1<Deg2, G> merge12 {gr};
    merge12(ESlot{w2, End::A}, BSlot{w0, 2});  // splits w0 into w0 and w3

    if constexpr (profuse)
        gr.print_chains("after ");

    for (const auto& w : {w0, w2, w3}) {
        const auto& cn = gr.chain(w);
        ASSERT_EQ(cn.num_vertices(Deg1), 1);
        ASSERT_EQ(cn.num_vertices(Deg2), cn.length() - 1);
        ASSERT_EQ(cn.num_vertices(Deg3), 1);
        ASSERT_EQ(cn.num_vertices(Deg4), 0);
    }
}


/// Tests egEnd_to_bulkslot():
/// convertion of an internal chain vertex descriptor into a slot.
TEST_F(ChainTest, EgEndToBulkslot)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests egEnd_to_bulkslot():",
            " convertion of an internal chain vertex descriptor into a slot"
        );

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);

    const auto& cn0 = gr.chain(ChIdG{});

    const BSlot bs0 {cn0.idw, 0};
    const BSlot bs1 {cn0.idw, 1};
    const BSlot bs2 {cn0.idw, 2};
    const BSlot bs3 {cn0.idw, 3};

    auto cn {cn0};  // copy: leaves the graph intact

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn.g[0].reverse<false>();

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn = cn0;
    cn.g[1].reverse<false>();

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs3);

    cn = cn0;
    cn.g[2].reverse<false>();

    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 0), bs0);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 0), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 1), bs1);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 1), bs2);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::A, 2), bs3);
    ASSERT_EQ(cn.egEnd_to_bulkslot(End::B, 2), bs2);
}


/// Tests internal_egEnd(): finding end of the edge connected to another edge.
TEST_F(ChainTest, InternalEgEnd)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests internal_egEnd():",
            " finding end of the edge connected to another edge."
        );

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);

    auto cn = gr.chain(ChIdG{});  // copy: leaves the graph intact

    // Finds edge end of the edge arg2 to which the edge at arg1 is connected
    // via a vertex internal to the chain.

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[0].reverse<false>();

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[1].reverse<false>();

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::A);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::A);

    cn.g[2].reverse<false>();

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::A);

    cn.g[1].reverse<false>();

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);

    cn.g[0].reverse<false>();

    ASSERT_EQ(cn.internal_egEnd(0, 1), End::A);
    ASSERT_EQ(cn.internal_egEnd(1, 0), End::B);
    ASSERT_EQ(cn.internal_egEnd(1, 2), End::B);
    ASSERT_EQ(cn.internal_egEnd(2, 1), End::B);
}


/// Tests selection of the slots by end in Neigs::incoming() and outgoing().
TEST_F(ChainTest, NeigsIncomingOutgoing)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests Neigs::incoming() and Neigs::outgoing()");

    using Neigs = structure::containers::Neigs<ESlot>;

    const Neigs n {ESlot{ChIdG{0}, End::A},
                   ESlot{ChIdG{1}, End::B},
                   ESlot{ChIdG{2}, End::A}};

    EXPECT_EQ(n.incoming(), (Neigs::Slots {ESlot{ChIdG{1}, End::B}}));
    EXPECT_EQ(n.outgoing(), (Neigs::Slots {ESlot{ChIdG{0}, End::A},
                                           ESlot{ChIdG{2}, End::A}}));
    EXPECT_TRUE(Neigs{}.incoming().empty());
}

}  // namespace graffine::tests::chain
