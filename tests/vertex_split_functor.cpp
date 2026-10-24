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

#include "vertex_split_common.h"
#include "graffine/transforms/vertex_split/functor.h"

#include <array>
#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>


namespace graffine::tests::vertex_split {

using Vertex = G::Vertex;
using EgDescr = Vertex::EgDescr;
using Slots = std::vector<EgDescr>;
using SplitFunctor = trs::vertex_split::Functor<G>;
using SplitRes = SplitFunctor::Res;

// Graph builders ==============================================================

/// A single linear chain.
void make_line(G& gr, const std::size_t len)
{
    gr.add_single_chain_component(len);
}

/// A single disconnected cycle chain.
void make_cycle(G& gr, const std::size_t len)
{
    gr.add_single_chain_component(len);
    VertexMerger<1, 1, G> {gr}(ESlot{ChIdG{0}, End::A}, ESlot{ChIdG{0}, End::B});
}

/// Three linear chains joined at a degree 3 vertex: w0 at end A, w1 at end B
/// and w2 at end A.
void make_junction3(G& gr)
{
    gr.add_single_chain_component(3);
    gr.add_single_chain_component(4);
    VertexMerger<1, 2, G> {gr}(ESlot{ChIdG{0}, End::A}, BSlot{ChIdG{1}, EgIdA{2}});
}

/// Four linear chains w0 ... w3 joined at a degree 4 vertex.
void make_junction4(G& gr)
{
    gr.add_single_chain_component(4);
    gr.add_single_chain_component(4);
    VertexMerger<2, 2, G> {gr}(BSlot{ChIdG{0}, EgIdA{2}}, BSlot{ChIdG{1}, EgIdA{2}});
}

// Vertex and slot selection ===================================================

/// The first vertex of degree \p d found in the graph.
auto vertex_of_degree(G& gr, const Degree d) -> Vertex*
{
    for (ChIdG w {}; w < gr.num_chains(); ++w)
        for (auto& eg : gr.chain(w).g)
            for (const auto e : End::Ids)
                if (auto v = eg.vertex_at_outer(e); v && v->deg() == d)
                    return v;
    return nullptr;
}

/// The vertex at end B of edge \p a of chain \p w.
auto vertex_after_edge(G& gr, const ChIdG w, const EgIdA a) -> Vertex*
{
    return gr.chain(w).g[a].vertex_at_outer(End::B);
}

/**
 * Descriptors of the edges of chains \p ww incident to vertex \p v.
 * \note Edge descriptors encode edge ids which are unique across graphs,
 * so the slots are selected by chain to be comparable between graphs.
 */
auto slots_on_chains(const G& gr,
                     const Vertex& v,
                     const std::initializer_list<ChIdG> ww) -> Slots
{
    const auto& cmp = gr.compt(v.get_c());
    Slots ss;
    for (const auto& w : ww)
        for (const auto& egd : v.edges())
            if (cmp.end_slot(egd).w == w) {
                ss.push_back(egd);
                break;
            }
    return ss;
}

// Fixture =====================================================================

class VertexSplitFunctorTest
    : public VertexSplitTest {

protected:

    /**
     * Applies the degree-dispatching functor to a graph, and the
     * degree-specific functor to an identically built one, and checks that
     * both give the same result.
     * \param[out] grF Graph transformed by the dispatching functor.
     * \param[out] grD Graph transformed by the degree-specific functor.
     * \param[in] build Graph builder.
     * \param[in] pick Selects the split vertex and the slots in a graph.
     * \param[in] direct Applies the expected degree-specific functor.
     */
    template<typename Build,
             typename Pick,
             typename Direct>
    void split_and_compare(G& grF,
                           G& grD,
                           Build build,
                           Pick pick,
                           Direct direct)
    {
        build(grF);
        build(grD);
        ASSERT_EQ(Snapshot{grF}, Snapshot{grD});

        auto [vF, ssF] = pick(grF);
        auto [vD, ssD] = pick(grD);
        ASSERT_TRUE(vF);
        ASSERT_TRUE(vD);
        ASSERT_EQ(ssF.size(), ssD.size());

        SplitFunctor split {grF};
        const SplitRes resF = split(*vF, ssF);
        const SplitRes resD = direct(grD, *vD, ssD);

        for (std::size_t i {}; i < resF.size(); ++i) {
            ASSERT_EQ(resF[i] == nullptr, resD[i] == nullptr);
            if (resF[i]) {
                ASSERT_EQ(resF[i]->deg(), resD[i]->deg());
            }
        }

        ASSERT_EQ(Snapshot{grF}, Snapshot{grD});
    }
};

// Degree-specific functors applied directly.

constexpr auto to11 = [](G& gr, Vertex& v, const Slots&)
{
    return trs::vertex_split::To<Deg1, Deg1, G> {gr}(v);
};

constexpr auto to12 = [](G& gr, Vertex& v, const Slots& ss)
{
    return trs::vertex_split::To<Deg1, Deg2, G> {gr}(v, ss[0]);
};

constexpr auto to13 = [](G& gr, Vertex& v, const Slots& ss)
{
    return trs::vertex_split::To<Deg1, Deg3, G> {gr}(v, ss[0]);
};

constexpr auto to22 = [](G& gr, Vertex& v, const Slots& ss)
{
    return trs::vertex_split::To<Deg2, Deg2, G> {gr}(v, ss);
};

// =============================================================================


/// Tests dispatch of a degree 2 vertex inside a linear chain to 1+1 split.
TEST_F(VertexSplitFunctorTest, Deg2_LinearChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of a degree 2 vertex inside a ",
                          "linear chain to 1+1 split");

    constexpr std::size_t len {4};

    G grF;
    const IntegrityCheck integrityCheckF {grF};
    G grD;
    const IntegrityCheck integrityCheckD {grD};

    split_and_compare(
        grF, grD,
        [](G& gr){ make_line(gr, len); },
        [](G& gr){ return std::pair{vertex_after_edge(gr, ChIdG{0}, EgIdA{1}),
                                    Slots{}}; },
        to11);
    if (HasFatalFailure()) return;

    ASSERT_EQ(grF.num_chains(), 2);
    ASSERT_EQ(grF.num_compts(), 2);
    ASSERT_EQ(grF.num_vertices(Deg1), 4);
    ASSERT_EQ(grF.num_vertices(Deg2), len - 2);
}


/// Tests dispatch of the boundary vertex of a disconnected cycle to 1+1 split.
TEST_F(VertexSplitFunctorTest, Deg2_CycleBoundary)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of the boundary vertex of a ",
                          "disconnected cycle to 1+1 split");

    constexpr std::size_t len {4};

    G grF;
    const IntegrityCheck integrityCheckF {grF};
    G grD;
    const IntegrityCheck integrityCheckD {grD};

    split_and_compare(
        grF, grD,
        [](G& gr){ make_cycle(gr, len); },
        [](G& gr){ return std::pair{gr.chain(ChIdG{0}).g[0].vertex_at_outer(End::A),
                                    Slots{}}; },
        to11);
    if (HasFatalFailure()) return;

    ASSERT_EQ(grF.num_chains(), 1);
    ASSERT_EQ(grF.num_compts(), 1);
    ASSERT_FALSE(grF.chain(ChIdG{0}).is_cycle());
    ASSERT_EQ(grF.num_vertices(Deg1), 2);
    ASSERT_EQ(grF.num_vertices(Deg2), len - 1);
}


/// Tests dispatch of an internal vertex of a disconnected cycle to 1+1 split.
TEST_F(VertexSplitFunctorTest, Deg2_CycleInternal)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of an internal vertex of a ",
                          "disconnected cycle to 1+1 split");

    constexpr std::size_t len {4};

    G grF;
    const IntegrityCheck integrityCheckF {grF};
    G grD;
    const IntegrityCheck integrityCheckD {grD};

    split_and_compare(
        grF, grD,
        [](G& gr){ make_cycle(gr, len); },
        [](G& gr){ return std::pair{vertex_after_edge(gr, ChIdG{0}, EgIdA{1}),
                                    Slots{}}; },
        to11);
    if (HasFatalFailure()) return;

    ASSERT_EQ(grF.num_chains(), 1);
    ASSERT_EQ(grF.num_compts(), 1);
    ASSERT_FALSE(grF.chain(ChIdG{0}).is_cycle());
    ASSERT_EQ(grF.num_vertices(Deg1), 2);
    ASSERT_EQ(grF.num_vertices(Deg2), len - 1);
}


/// Tests dispatch of a degree 3 vertex to 1+2 split, detaching each of the
/// three chains in turn.
TEST_F(VertexSplitFunctorTest, Deg3_Junction)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of a degree 3 vertex to 1+2 split, ",
                          "detaching each of the three chains in turn");

    for (const auto& w : {ChIdG{0}, ChIdG{1}, ChIdG{2}}) {

        G grF;
        const IntegrityCheck integrityCheckF {grF};
        G grD;
        const IntegrityCheck integrityCheckD {grD};

        split_and_compare(
            grF, grD,
            make_junction3,
            [w](G& gr){
                auto v = vertex_of_degree(gr, Deg3);
                return std::pair{v, slots_on_chains(gr, *v, {w})};
            },
            to12);
        if (HasFatalFailure()) return;

        ASSERT_EQ(grF.num_chains(), 2) << "detached chain " << w;
        ASSERT_EQ(grF.num_compts(), 2) << "detached chain " << w;
        ASSERT_EQ(grF.num_vertices(Deg1), 4) << "detached chain " << w;
        ASSERT_EQ(grF.num_vertices(Deg3), 0) << "detached chain " << w;
    }
}


/// Tests dispatch of a degree 4 vertex with a single slot to 1+3 split.
TEST_F(VertexSplitFunctorTest, Deg4_OneSlot)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of a degree 4 vertex with a single ",
                          "slot to 1+3 split");

    G grF;
    const IntegrityCheck integrityCheckF {grF};
    G grD;
    const IntegrityCheck integrityCheckD {grD};

    split_and_compare(
        grF, grD,
        make_junction4,
        [](G& gr){
            auto v = vertex_of_degree(gr, Deg4);
            return std::pair{v, slots_on_chains(gr, *v, {ChIdG{0}})};
        },
        to13);
    if (HasFatalFailure()) return;

    ASSERT_EQ(grF.num_compts(), 2);
    ASSERT_EQ(grF.num_vertices(Deg1), 5);
    ASSERT_EQ(grF.num_vertices(Deg3), 1);
    ASSERT_EQ(grF.num_vertices(Deg4), 0);
}


/// Tests dispatch of a degree 4 vertex with two slots to 2+2 split.
TEST_F(VertexSplitFunctorTest, Deg4_TwoSlots)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests dispatch of a degree 4 vertex with two ",
                          "slots to 2+2 split");

    G grF;
    const IntegrityCheck integrityCheckF {grF};
    G grD;
    const IntegrityCheck integrityCheckD {grD};

    split_and_compare(
        grF, grD,
        make_junction4,
        [](G& gr){
            auto v = vertex_of_degree(gr, Deg4);
            return std::pair{v, slots_on_chains(gr, *v, {ChIdG{0}, ChIdG{1}})};
        },
        to22);
    if (HasFatalFailure()) return;

    ASSERT_EQ(grF.num_chains(), 2);
    ASSERT_EQ(grF.num_vertices(Deg1), 4);
    ASSERT_EQ(grF.num_vertices(Deg3), 0);
    ASSERT_EQ(grF.num_vertices(Deg4), 0);
}


/// Tests that a degree 1 vertex, which cannot be split, is rejected.
TEST_F(VertexSplitFunctorTest, Deg1_Rejected)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests that a degree 1 vertex is rejected");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_line(gr, 3);
    auto& v = *gr.chain(ChIdG{0}).g[0].vertex_at_outer(End::A);
    ASSERT_EQ(v.deg(), Deg1);

    SplitFunctor split {gr};
    EXPECT_FALSE(split.is_applicable(v, Slots{}));

    const auto res = split.try_apply(v, Slots{});
    ASSERT_FALSE(res.has_value());
    EXPECT_NE(std::string {res.error().reason}.find("not implemented"),
              std::string::npos) << res.error().reason;

    EXPECT_THROW(split(v, Slots{}), graffine::precondition_error);
    EXPECT_EQ(gr.num_chains(), 1);  // unchanged
}


}  // namespace graffine::tests::vertex_split
