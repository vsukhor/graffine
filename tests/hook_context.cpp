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
#include "graffine/transforms/context.h"
#include "graffine/transforms/edge_creation/in_new_chain.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/edge_split/functor.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"
#include "graffine/transforms/vertex_split/to_22.h"

#include <array>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace graffine::tests::hook_context {

namespace trs = graffine::transforms;

using trs::Context;
using trs::Frame;
using TT = trs::TransformType;
using Vertex = G::Vertex;
using Path = trs::pulling::Path<G::Compt>;
using pulling::hooks::make_two_junctions;
using pulling::hooks::w1;
using pulling::hooks::w3;
using pulling::hooks::w4;

/// Calls of the hooks, as "family.hook", followed by " in " and the context
/// from its outermost transformation if the context is not empty.
using Log = std::vector<std::string>;

inline auto str(const Frame& f) -> std::string
{
    static constexpr std::array names {
        "ComponentCreation", "ComponentDeletion", "EdgeCreation",
        "EdgeDeletion", "EdgeMerger", "EdgeSplit", "VertexMerger",
        "VertexSplit", "Pulling"};

    std::string s {names[f.type]};
    if (is_defined(f.deg2))
        s += ' ' + trs::degrees_to_string(f.deg1, f.deg2);
    else if (is_defined(f.deg1))
        s += ' ' + std::to_string(f.deg1);

    return s;
}

inline auto str(const char* hook, const Context& ctx) -> std::string
{
    std::string s {hook};
    for (std::size_t i {}; i < ctx.depth(); ++i)
        s += (i ? " > " : " in ") + str(ctx[i]);

    return s;
}

/// Hooks of a family, logging their calls with their context.
struct Logging {
    Log* log {};
    const char* family {};

    void start(const Context& ctx) { put(".on_start", ctx); }
    void end(const Context& ctx) { put(".on_end", ctx); }

private:

    void put(const char* hook, const Context& ctx)
    {
        log->push_back(str((family + std::string {hook}).c_str(), ctx));
    }
};

/// Property processor whose hooks all take the context, and log their calls.
struct Recorder {

    struct : Logging {
        void on_start(const ESlot&, const ESlot&, const Context& c) { start(c); }
        void on_start(const BSlot&, const ESlot&, const Context& c) { start(c); }
        void on_start(const BSlot&, const BSlot&, const Context& c) { start(c); }
        void on_end(Vertex&, const Context& c) { end(c); }
    } vertexMerger;

    struct : Logging {
        void on_start(ChIdG, const Context& c) { start(c); }
        void on_start(const ESlot&, const Context& c) { start(c); }
        void on_start(const BSlot&, const Context& c) { start(c); }
        void on_start(const ESlot&, const ESlot&, const Context& c) { start(c); }
        void on_end(const std::array<Vertex*, 2>&, const Context& c) { end(c); }
    } vertexSplit;

    struct : Logging {
        void on_start(ChIdG, const Context& c) { start(c); }
        template<typename S> void on_start(const S&, const Context& c) { start(c); }
        void on_start(ChIdG, EgIdA, bool, const Context& c) { start(c); }
        template<bool isNewChain> void on_end(Edge&, const Context& c) { end(c); }
    } edgeCreation;

    struct : Logging {
        template<bool isChainDeleted> void on_start(Edge&, const Context& c) { start(c); }
        template<bool isChainDeleted> void on_end(Vertex&, const Context& c) { end(c); }
    } edgeDeletion;

    struct : Logging {
        void on_start(Edge&, Edge&, const Context& c) { start(c); }
        void on_end(Edge&, const Context& c) { end(c); }
    } edgeMerger;

    struct : Logging {
        void on_start(const Context& c) { start(c); }
        void on_end(const Context& c) { end(c); }
    } componentCreation, componentDeletion;

    struct : Logging {
        void on_start(Path&, const Context& c) { start(c); }
        void on_end(Path&, const Context& c) { end(c); }
    } pulling;

    Log log;

    Recorder()
    {
        vertexMerger.log = vertexSplit.log = edgeCreation.log =
        edgeDeletion.log = edgeMerger.log = componentCreation.log =
        componentDeletion.log = pulling.log = &log;
        vertexMerger.family = "vertexMerger";
        vertexSplit.family = "vertexSplit";
        edgeCreation.family = "edgeCreation";
        edgeDeletion.family = "edgeDeletion";
        edgeMerger.family = "edgeMerger";
        componentCreation.family = "componentCreation";
        componentDeletion.family = "componentDeletion";
        pulling.family = "pulling";
    }
};

// Hooks taking the context satisfy the concepts, as do those not taking it.
static_assert(trs::PropertyProcessor<Recorder, G>);
static_assert(trs::pulling::Processor<pulling::hooks::Recorder, G>);

/**
 * Property processor opting in for the context in the vertex split hooks
 * only, and counting the calls of the vertex merger hooks, which do not take
 * it. Its 2+2 split hook has both overloads: the one taking the context is
 * called.
 */
struct PartialRecorder
    : public pulling::hooks::Recorder {

    struct : Logging {
        void on_start(ChIdG, const Context& c) { start(c); }
        void on_start(const ESlot&, const Context& c) { start(c); }
        void on_start(const BSlot&, const Context& c) { start(c); }
        void on_start(const ESlot&, const ESlot&) { log->push_back("ignored"); }
        void on_start(const ESlot&, const ESlot&, const Context& c) { start(c); }
        void on_end(const std::array<Vertex*, 2>&, const Context& c) { end(c); }
    } vertexSplit;

    struct {
        int starts {};
        int ends {};
        void on_start(const ESlot&, const ESlot&) { ++starts; }
        void on_start(const BSlot&, const ESlot&) { ++starts; }
        void on_start(const BSlot&, const BSlot&) { ++starts; }
        void on_end(Vertex&) { ++ends; }
    } vertexMerger;

    Log log;

    explicit PartialRecorder(const G& gr)
        : pulling::hooks::Recorder {gr}
    {
        vertexSplit.log = &log;
        vertexSplit.family = "vertexSplit";
    }
};

static_assert(trs::vertex_split::Processor<PartialRecorder, G> &&
              trs::vertex_merger::Processor<PartialRecorder, G>);


class HookContextTest
    : public Test {

protected:

    static constexpr ChIdG w0 {0};

    std::shared_ptr<Recorder> pp {std::make_shared<Recorder>()};

    /// Expects the hooks to have been called as \p expected, and clears the log.
    void expect_log(const Log& expected)
    {
        EXPECT_EQ(pp->log, expected);
        pp->log.clear();
        EXPECT_TRUE(trs::current_context().empty());
    }

    /// Junction of degree 3 of chains w0 (3) at its end B, w1 (2) at its
    /// end B, and w2 (2) at its end A.
    static void make_junction3(G& gr)
    {
        gr.add_single_chain_component(5);
        gr.add_single_chain_component(2);
        trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot {ChIdG {1}, End::B},
                                                     BSlot {w0, 3});
    }
};

// =============================================================================


/// Context of the hooks of a transformation applied by the client.
TEST_F(HookContextTest, NotComposed)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that the hooks of a transformation applied by the client ",
            "see an empty context");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);

    trs::vertex_split::To<Deg1, Deg1, G, Recorder> {gr, pp}(BSlot {w0, 2});
    expect_log({"vertexSplit.on_start", "vertexSplit.on_end"});
}


/// 2+2 vertex split: splits to 1+3 and 1+2, then merger 1+1.
TEST_F(HookContextTest, VertexSplit22)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the 2+2 vertex split");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_junction3(gr);
    gr.add_single_chain_component(2);
    // degree 4 junction: w0 (3), w1 (2), w2 (2) and w3 (2) at its end B
    trs::vertex_merger::From<Deg1, Deg3, G> {gr}(ESlot {ChIdG {3}, End::B},
                                                 ESlot {ChIdG {1}, End::B});
    ASSERT_EQ(gr.ngs_at(ESlot {w0, End::B}).num(), 3);

    trs::vertex_split::To<Deg2, Deg2, G, Recorder> split {gr, pp};
    split(ESlot {w0, End::B}, ESlot {ChIdG {1}, End::B});

    expect_log({
        "vertexSplit.on_start",
        "vertexSplit.on_start in VertexSplit 2+2",
        "vertexSplit.on_end in VertexSplit 2+2",
        "vertexSplit.on_start in VertexSplit 2+2",
        "vertexMerger.on_start in VertexSplit 2+2 > VertexSplit 1+2",
        "vertexMerger.on_end in VertexSplit 2+2 > VertexSplit 1+2",
        "vertexSplit.on_end in VertexSplit 2+2",
        "vertexMerger.on_start in VertexSplit 2+2",
        "vertexMerger.on_end in VertexSplit 2+2",
        "vertexSplit.on_end",
    });
}


/// 1+2 vertex split merging the two chains remaining at the junction.
TEST_F(HookContextTest, VertexSplit12)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the 1+2 vertex split");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_junction3(gr);

    trs::vertex_split::To<Deg1, Deg2, G, Recorder> {gr, pp}(
        ESlot {ChIdG {1}, End::B});

    expect_log({
        "vertexSplit.on_start",
        "vertexMerger.on_start in VertexSplit 1+2",
        "vertexMerger.on_end in VertexSplit 1+2",
        "vertexSplit.on_end",
    });
}


/// Deletion of a single-edge chain at a junction of degree 3: split, then
/// deletion of the component of the edge.
TEST_F(HookContextTest, EdgeDeletionDeletingHostChain3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the hook context of the edge deletion with its host chain, ",
            "at a junction of degree 3");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(5);
    gr.add_single_chain_component(1);
    trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot {ChIdG {1}, End::B},
                                                 BSlot {w0, 3});
    ASSERT_EQ(gr.chain(ChIdG {1}).length(), 1);
    const auto leaf = gr.chain(ChIdG {1}).g[0].ind;

    trs::edge_deletion::Functor<G, Recorder> {gr, pp}(leaf);

    expect_log({
        "edgeDeletion.on_start",
        "vertexSplit.on_start in EdgeDeletion 3",
        "vertexMerger.on_start in EdgeDeletion 3 > VertexSplit 1+2",
        "vertexMerger.on_end in EdgeDeletion 3 > VertexSplit 1+2",
        "vertexSplit.on_end in EdgeDeletion 3",
        "componentDeletion.on_start in EdgeDeletion 3",
        "componentDeletion.on_end in EdgeDeletion 3",
        "edgeDeletion.on_end",
    });
}


/// Deletion of a single-edge chain at a junction of degree 4.
TEST_F(HookContextTest, EdgeDeletionDeletingHostChain4)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the hook context of the edge deletion with its host chain, ",
            "at a junction of degree 4");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_junction3(gr);
    gr.add_single_chain_component(1);
    const auto wLeaf = gr.ind_last_chain();
    trs::vertex_merger::From<Deg1, Deg3, G> {gr}(ESlot {wLeaf, End::B},
                                                 ESlot {ChIdG {1}, End::B});
    const auto leaf = gr.chain(wLeaf).g[0].ind;

    trs::edge_deletion::Functor<G, Recorder> {gr, pp}(leaf);

    expect_log({
        "edgeDeletion.on_start",
        "vertexSplit.on_start in EdgeDeletion 4",
        "vertexSplit.on_end in EdgeDeletion 4",
        "componentDeletion.on_start in EdgeDeletion 4",
        "componentDeletion.on_end in EdgeDeletion 4",
        "edgeDeletion.on_end",
    });
}


/// Creation of an edge in a new chain, merged to the source vertex.
TEST_F(HookContextTest, EdgeCreationInNewChain)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the hook context of the edge creation in a new chain");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);

    trs::edge_creation::InNewChain<Deg2, G, Recorder> {gr, pp}(BSlot {w0, 2});
    expect_log({
        "edgeCreation.on_start",
        "vertexMerger.on_start in EdgeCreation 2",
        "vertexMerger.on_end in EdgeCreation 2",
        "edgeCreation.on_end",
    });

    trs::edge_creation::InNewChain<Deg3, G, Recorder> {gr, pp}(
        ESlot {w0, End::B});
    expect_log({
        "edgeCreation.on_start",
        "vertexMerger.on_start in EdgeCreation 3",
        "vertexMerger.on_end in EdgeCreation 3",
        "edgeCreation.on_end",
    });
}


/// Edge split: creation of an edge in the chain.
TEST_F(HookContextTest, EdgeSplit)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the edge split");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);

    trs::edge_split::Functor<G, Recorder> {gr, pp}(gr.chain(w0).g[1].ind);
    expect_log({
        "edgeCreation.on_start in EdgeSplit",
        "edgeCreation.on_end in EdgeSplit",
    });
}


/// Edge merger: deletion of one of the edges, preserving or deleting its
/// host chain.
TEST_F(HookContextTest, EdgeMerger)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the edge merger");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(4);

    trs::edge_merger::Functor<G, Recorder> merge {gr, pp};
    merge(gr.chain(w0).g[1].ind, gr.chain(w0).g[2].ind);
    expect_log({
        "edgeMerger.on_start",
        "edgeDeletion.on_start in EdgeMerger",
        "edgeDeletion.on_end in EdgeMerger",
        "edgeMerger.on_end",
    });

    // single-edge leaf chain w1 at a junction of degree 3 with w0 split in two
    gr.add_single_chain_component(1);
    trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot {ChIdG {1}, End::B},
                                                 BSlot {w0, 1});
    const auto& m = gr.chain(ChIdG {1});
    ASSERT_EQ(m.length(), 1);
    const auto ng = m.ngs[End::B].as_vector()[0];
    merge(m.g[0].ind, gr.chain(ng.w).end_edge(ng.e).ind);
    expect_log({
        "edgeMerger.on_start",
        "edgeDeletion.on_start in EdgeMerger",
        "vertexSplit.on_start in EdgeMerger > EdgeDeletion 3",
        "vertexMerger.on_start in EdgeMerger > EdgeDeletion 3 > VertexSplit 1+2",
        "vertexMerger.on_end in EdgeMerger > EdgeDeletion 3 > VertexSplit 1+2",
        "vertexSplit.on_end in EdgeMerger > EdgeDeletion 3",
        "componentDeletion.on_start in EdgeMerger > EdgeDeletion 3",
        "componentDeletion.on_end in EdgeMerger > EdgeDeletion 3",
        "edgeDeletion.on_end in EdgeMerger",
        "edgeMerger.on_end",
    });
}


/// Degree 1 pull whose last step dissolves the source chain.
TEST_F(HookContextTest, Pulling1)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the degree 1 pulling");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_two_junctions(gr);
    Path path {&gr.compt(CmpId {}), Path::Driver {&gr.chain(w1).end_edge(End::A), End::A},
               Path::Source {w4, End::B}};

    trs::pulling::On<Deg1, G, Recorder> {gr, pp}(path, 3);
    expect_log({
        "pulling.on_start",
        "pulling.on_end",
        "pulling.on_end",
        "edgeCreation.on_start in Pulling 1",
        "edgeCreation.on_end in Pulling 1",
        "edgeDeletion.on_start in Pulling 1",
        "vertexSplit.on_start in Pulling 1 > EdgeDeletion 3",
        "vertexMerger.on_start in Pulling 1 > EdgeDeletion 3 > VertexSplit 1+2",
        "vertexMerger.on_end in Pulling 1 > EdgeDeletion 3 > VertexSplit 1+2",
        "vertexSplit.on_end in Pulling 1 > EdgeDeletion 3",
        "componentDeletion.on_start in Pulling 1 > EdgeDeletion 3",
        "componentDeletion.on_end in Pulling 1 > EdgeDeletion 3",
        "edgeDeletion.on_end in Pulling 1",
        "pulling.on_end",
    });
}


/// Degree 2 pull: split 1+1 and merger 1+2 around the first step.
TEST_F(HookContextTest, Pulling2)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the degree 2 pulling");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_two_junctions(gr);
    Path path {&gr.compt(CmpId {}), Path::Driver {&gr.chain(w3).g[1], End::A},
               Path::Source {w4, End::B}};

    trs::pulling::On<Deg2, G, Recorder> {gr, pp}(path, 1);
    expect_log({
        "pulling.on_start",
        "vertexSplit.on_start in Pulling 2",
        "vertexSplit.on_end in Pulling 2",
        "vertexMerger.on_start in Pulling 2",
        "vertexMerger.on_end in Pulling 2",
        "pulling.on_end",
    });
}


/// Degree 3 pull: split 1+2 and merger 2+2 around the first step.
TEST_F(HookContextTest, Pulling3)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the hook context of the degree 3 pulling");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_two_junctions(gr);
    Path path {&gr.compt(CmpId {}), Path::Driver {&gr.chain(w3).end_edge(End::A), End::A},
               Path::Source {w4, End::B}};

    trs::pulling::On<Deg3, G, Recorder> {gr, pp}(path, 1);
    expect_log({
        "pulling.on_start",
        "vertexSplit.on_start in Pulling 3",
        "vertexMerger.on_start in Pulling 3 > VertexSplit 1+2",
        "vertexMerger.on_end in Pulling 3 > VertexSplit 1+2",
        "vertexSplit.on_end in Pulling 3",
        "vertexMerger.on_start in Pulling 3",
        "vertexMerger.on_end in Pulling 3",
        "pulling.on_end",
    });
}


/// Hooks opting in one by one: those not taking the context are called
/// as before, and an overload taking it is preferred.
TEST_F(HookContextTest, PartialOptIn)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests a processor having only some hooks taking the context");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    make_junction3(gr);
    gr.add_single_chain_component(2);
    trs::vertex_merger::From<Deg1, Deg3, G> {gr}(ESlot {ChIdG {3}, End::B},
                                                 ESlot {ChIdG {1}, End::B});

    const auto partial = std::make_shared<PartialRecorder>(gr);
    trs::vertex_split::To<Deg2, Deg2, G, PartialRecorder> split {gr, partial};
    split(ESlot {w0, End::B}, ESlot {ChIdG {1}, End::B});

    EXPECT_EQ(partial->log, (Log {
        "vertexSplit.on_start",
        "vertexSplit.on_start in VertexSplit 2+2",
        "vertexSplit.on_end in VertexSplit 2+2",
        "vertexSplit.on_start in VertexSplit 2+2",
        "vertexSplit.on_end in VertexSplit 2+2",
        "vertexSplit.on_end",
    }));
    EXPECT_EQ(partial->vertexMerger.starts, 2);
    EXPECT_EQ(partial->vertexMerger.ends, 2);
}


/// The context is restored when an inner transformation throws.
TEST_F(HookContextTest, RestoredOnException)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that the context is restored when a hook throws");

    struct Throwing
        : public Recorder {

        struct {
            void on_start(const Context& c)
            {
                if (c.is_inside(Frame {TT::EdgeDeletion, Deg3}))
                    throw std::runtime_error {"inner hook"};
            }
            void on_end() {}
        } componentDeletion;
    };

    G gr;
    IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(5);
    gr.add_single_chain_component(1);
    trs::vertex_merger::From<Deg1, Deg2, G> {gr}(ESlot {ChIdG {1}, End::B},
                                                 BSlot {w0, 3});
    ASSERT_EQ(gr.chain(ChIdG {1}).length(), 1);
    const auto leaf = gr.chain(ChIdG {1}).g[0].ind;

    trs::edge_deletion::Functor<G, Throwing> remove {gr, std::make_shared<Throwing>()};
    EXPECT_THROW(remove(leaf), std::runtime_error);
    EXPECT_TRUE(trs::current_context().empty());
    integrityCheck.skip();  // the deletion was interrupted
}


/// Queries of the context.
TEST_F(HookContextTest, Queries)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests the queries of the hook context");

    EXPECT_TRUE(trs::current_context().empty());
    EXPECT_EQ(trs::current_context().depth(), 0);
    EXPECT_THROW(trs::current_context().innermost(), precondition_error);

    {
        const trs::Nesting outer {Frame {TT::Pulling, Deg1}};
        const trs::Nesting inner {Frame {TT::VertexSplit, Deg2, Deg2}};

        const auto& c = trs::current_context();
        EXPECT_EQ(c.depth(), 2);
        EXPECT_EQ(c.outermost(), (Frame {TT::Pulling, Deg1}));
        EXPECT_EQ(c.innermost(), (Frame {TT::VertexSplit, Deg2, Deg2}));
        EXPECT_TRUE(c.is_inside(TT::Pulling));
        EXPECT_TRUE(c.is_inside(Frame {TT::VertexSplit, Deg2, Deg2}));
        EXPECT_FALSE(c.is_inside(Frame {TT::VertexSplit, Deg1, Deg2}));
        EXPECT_FALSE(c.is_inside(TT::EdgeDeletion));
        std::size_t n {};
        for ([[maybe_unused]] const auto& f : c)
            ++n;
        EXPECT_EQ(n, 2);
    }
    EXPECT_TRUE(trs::current_context().empty());
}

}  // namespace graffine::tests::hook_context
