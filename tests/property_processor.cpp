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
#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/edge_split/functor.h"
#include "graffine/transforms/property_processor.h"
#include "graffine/transforms/pulling/on_1.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_split/to_11.h"

#include <array>
#include <memory>

namespace graffine::tests::property_processor {

namespace trs = graffine::transforms;

using Vertex = G::Vertex;
using Path = trs::pulling::Path<G::Compt>;

/// Numbers of calls of the hooks of a transformation family.
struct Hooks {
    int starts {};
    int ends {};
};

/**
 * Property processor recording the calls of its hooks.
 * \details Implements exactly the hooks required by the concepts, so that
 * compiling the transformations with it checks both the concepts and the
 * calls of the hooks.
 */
struct Recorder {

    struct : Hooks {
        void on_start(const ESlot&, const ESlot&) { ++starts; }
        void on_start(const BSlot&, const ESlot&) { ++starts; }
        void on_start(const BSlot&, const BSlot&) { ++starts; }
        void on_end(Vertex&) { ++ends; }
    } vertexMerger;

    struct : Hooks {
        void on_start(ChIdG) { ++starts; }
        void on_start(const ESlot&) { ++starts; }
        void on_start(const BSlot&) { ++starts; }
        void on_start(const ESlot&, const ESlot&) { ++starts; }
        void on_end(const std::array<Vertex*, 2>&) { ++ends; }
    } vertexSplit;

    struct : Hooks {
        template<typename S> void on_start(const S&) { ++starts; }
        void on_start(ChIdG, EgIdA, bool) { ++starts; }
        template<bool isNewChain> void on_end(Edge&) { ++ends; }
    } edgeCreation;

    struct : Hooks {
        template<bool isChainDeleted> void on_start(Edge&) { ++starts; }
        template<bool isChainDeleted> void on_end(Vertex&) { ++ends; }
    } edgeDeletion;

    struct : Hooks {
        void on_start(Edge&, Edge&) { ++starts; }
        void on_end(Edge&) { ++ends; }
    } edgeMerger;

    struct : Hooks {
        void on_start() { ++starts; }
        void on_end() { ++ends; }
    } componentCreation, componentDeletion;

    struct : Hooks {
        void on_start(Path&) { ++starts; }
        void on_end(Path&) { ++ends; }
    } pulling;
};

static_assert(trs::PropertyProcessor<Recorder, G>);
static_assert(trs::PropertyProcessor<void, G>);

/**
 * Property processor observing the graph from the edge creation hooks.
 * \details A processor assigning properties to the new chain at on_start,
 * e.g. the positions of its vertices next to the source vertex, finds it
 * as the last chain of the graph and its edge as the last edge. Records what
 * on_start sees, and what the new edge is at on_end.
 */
struct NewChainObserver
    : public Recorder {

    struct {
        const G* gr {};
        ChIdG lastChain {ChIdG::undefined};  ///< Last chain seen at on_start.
        std::size_t numEdges {};             ///< Number of edges at on_start.
        bool isLastChainFree {};  ///< Last chain is single-edge and unconnected.
        ChIdG newChain {ChIdG::undefined};   ///< Host chain of the new edge.
        EgIdG newEdge {EgIdG::undefined};    ///< The new edge.

        template<typename S> void on_start(const S&) { observe(); }
        void on_start(ChIdG, EgIdA, bool) { observe(); }
        template<bool isNewChain> void on_end(Edge& eg)
        {
            newChain = eg.w;
            newEdge = eg.ind;
        }

        void observe()
        {
            lastChain = gr->ind_last_chain();
            numEdges = gr->num_edges();
            const auto& m = gr->chain(lastChain);
            isLastChainFree = m.length() == 1 &&
                              !m.is_connected_at(End::A) &&
                              !m.is_connected_at(End::B);
        }
    } edgeCreation;

    explicit NewChainObserver(const G& gr) { edgeCreation.gr = &gr; }
};

static_assert(trs::PropertyProcessor<NewChainObserver, G>);

/// A processor lacking the hooks of the vertex mergers.
struct WithoutVertexMerger {
    struct {
        void on_start() {}
        void on_end() {}
    } componentCreation;
};
static_assert(trs::component_creation::Processor<WithoutVertexMerger, G>);
static_assert(!trs::vertex_merger::Processor<WithoutVertexMerger, G>);
static_assert(!trs::PropertyProcessor<WithoutVertexMerger, G>);
static_assert(!trs::PropertyProcessor<int, G>);


class PropertyProcessorTest
    : public Test {

protected:

    static constexpr ChIdG w0 {0};
    static constexpr ChIdG w1 {1};

    std::shared_ptr<Recorder> pp {std::make_shared<Recorder>()};

    /// Expects \p h to have recorded \p n pairs of calls.
    static void expect_calls(const Hooks& h, const int n)
    {
        EXPECT_EQ(h.starts, n);
        EXPECT_EQ(h.ends, n);
    }
};

// =============================================================================


/// Hooks of the vertex mergers and splits.
TEST_F(PropertyProcessorTest, VertexMergerAndSplit)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the property processor hooks of vertex mergers and splits");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);
    gr.add_single_chain_component(4);

    trs::vertex_merger::From<Deg1, Deg1, G, Recorder> merge11 {gr, pp};
    const auto res = merge11(ESlot {w0, End::B}, ESlot {w1, End::A});
    expect_calls(pp->vertexMerger, 1);

    trs::vertex_split::To<Deg1, Deg1, G, Recorder> split11 {gr, pp};
    split11(*res[0]);
    expect_calls(pp->vertexSplit, 1);

    trs::vertex_merger::From<Deg1, Deg2, G, Recorder> merge12 {gr, pp};
    merge12(ESlot {gr.ind_last_chain(), End::A}, BSlot {w0, 1});
    expect_calls(pp->vertexMerger, 2);
}


/// Hooks of the edge creations, deletions, splits and mergers.
TEST_F(PropertyProcessorTest, Edges)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the property processor hooks of edge transformations");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);

    trs::edge_creation::InExistingChain<G, Recorder> extend {gr, pp};
    const auto created = extend(ESlot {w0, End::A});
    expect_calls(pp->edgeCreation, 1);

    trs::edge_deletion::Functor<G, Recorder> remove {gr, pp};
    remove(created[0]->ind);
    expect_calls(pp->edgeDeletion, 1);

    trs::edge_split::Functor<G, Recorder> split {gr, pp};
    split(gr.chain(w0).g[1].ind);
    expect_calls(pp->edgeCreation, 2);

    trs::edge_merger::Functor<G, Recorder> merge {gr, pp};
    merge(gr.chain(w0).g[1].ind, gr.chain(w0).g[2].ind);
    expect_calls(pp->edgeMerger, 1);
    expect_calls(pp->edgeDeletion, 2);

    trs::edge_creation::InNewChain<Deg2, G, Recorder> branch {gr, pp};
    branch(BSlot {w0, 2});
    expect_calls(pp->edgeCreation, 3);
    expect_calls(pp->vertexMerger, 1);  // the new edge is merged to the chain
}


/// Hooks of the component creations and deletions, and of pulling.
TEST_F(PropertyProcessorTest, ComponentsAndPulling)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the property processor hooks of component transformations ",
            "and of pulling");

    G gr;
    const IntegrityCheck integrityCheck {gr};

    trs::component_creation::Functor<G, Recorder> create {gr, pp};
    create(5);
    create(2);
    expect_calls(pp->componentCreation, 2);

    trs::component_deletion::Functor<G, Recorder> remove {gr, pp};
    remove(CmpId {1});
    expect_calls(pp->componentDeletion, 1);

    Path path {&gr.compt(w0), Path::Driver {&gr.chain(w0).tail(), End::A},
               Path::Source {w0, End::B}};
    trs::pulling::On<Deg1, Orientation::Forwards, G, Recorder> pull {gr, pp};
    constexpr int steps {2};
    pull(path, steps);
    EXPECT_EQ(pp->pulling.starts, 1);      // once per pull
    EXPECT_EQ(pp->pulling.ends, steps);    // once per step
}

/**
 * The graph seen by the hooks of InNewChain.
 * \details Branching a new chain from a bulk vertex, from a junction and from
 * the boundary vertex of an unconnected cycle, by slot, by chain and by
 * vertex. In each case, on_start sees the new chain already added, unconnected,
 * as the last chain, and its edge as the last edge. The cycle is followed by
 * an unrelated line, which a hook would otherwise take for the new chain.
 */
TEST_F(PropertyProcessorTest, NewChainHooksSeeTheNewChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that the hooks of InNewChain see the new chain from on_start");

    static constexpr auto cycle = w0;
    static constexpr auto line = w1;

    /// Expects the hooks to have seen the new chain from on_start.
    const auto expect_seen = [](const NewChainObserver& obs,
                                const Edge* const eg)
    {
        const auto& h = obs.edgeCreation;
        ASSERT_TRUE(eg);
        EXPECT_EQ(h.newChain, eg->w);
        EXPECT_EQ(h.newEdge, eg->ind);
        EXPECT_EQ(h.lastChain, h.newChain);
        EXPECT_EQ(h.numEdges, h.newEdge + 1);
        EXPECT_TRUE(h.isLastChainFree);
    };

    /// Unconnected cycle of 3 edges, followed by a line of 2 edges.
    const auto make_graph = [](G& gr)
    {
        gr.add_single_chain_component(3);
        gr.add_single_chain_component(2);
        trs::vertex_merger::From<Deg1, Deg1, G> merge11 {gr};
        merge11(ESlot {cycle, End::A}, ESlot {cycle, End::B});
        ASSERT_TRUE(gr.chain(cycle).is_unconnected_cycle());
    };

    using Branch2 = trs::edge_creation::InNewChain<Deg2, G, NewChainObserver>;
    using Branch3 = trs::edge_creation::InNewChain<Deg3, G, NewChainObserver>;

    {   // bulk vertex, by slot
        G gr;
        const IntegrityCheck integrityCheck {gr};
        make_graph(gr);
        const auto obs = std::make_shared<NewChainObserver>(gr);
        Branch2 branch {gr, obs};
        expect_seen(*obs, branch(BSlot {line, 1})[0]);
    }
    {   // junction, by slot
        G gr;
        const IntegrityCheck integrityCheck {gr};
        make_graph(gr);
        trs::edge_creation::InNewChain<Deg2, G> {gr}(BSlot {line, 1});
        const auto obs = std::make_shared<NewChainObserver>(gr);
        Branch3 branch {gr, obs};
        expect_seen(*obs, branch(ESlot {line, End::B})[0]);
    }
    {   // cycle boundary vertex, by chain
        G gr;
        const IntegrityCheck integrityCheck {gr};
        make_graph(gr);
        const auto obs = std::make_shared<NewChainObserver>(gr);
        Branch2 branch {gr, obs};
        expect_seen(*obs, branch(cycle)[0]);
    }
    {   // cycle boundary vertex, by vertex
        G gr;
        const IntegrityCheck integrityCheck {gr};
        make_graph(gr);
        const auto obs = std::make_shared<NewChainObserver>(gr);
        Branch2 branch {gr, obs};
        auto& v = *gr.chain(cycle).vertex_at_end(End::A);
        expect_seen(*obs, branch(v)[0]);
    }
}

}  // namespace graffine::tests::property_processor
