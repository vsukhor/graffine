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

/**
 * \file pulling_identity.cu
 * \brief Check, under nvcc, that pulling keeps the vertex objects of its
 * path, for drivers of degrees 1 to 3, with a processor whose hooks take the
 * context and which has a planning hook.
 * \details Compiles the vertex splits and mergers keeping given vertex
 * objects (see Keep), which pulling uses. Returns 0 on success.
 */

#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/pulling/functor.h"
#include "graffine/transforms/vertex_merger/from_12.h"

#include <array>
#include <memory>
#include <vector>

namespace app {

namespace elements = graffine::structure;
namespace trs = graffine::transforms;

using G = elements::Graph<elements::Component<elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Vertex = G::Vertex;
using Edge = G::Chain::Edge;
using End = G::Chain::End;
using ESlot = G::Chain::EndSlot;
using BSlot = G::Chain::BulkSlot;
using Path = trs::pulling::Path<G::Compt>;
using Context = trs::Context;
using VId = decltype(std::declval<Vertex>().get_id());

auto ids(const Path& path) -> std::vector<VId>
{
    std::vector<VId> ii;
    for (const auto* v : path.vertices())
        ii.push_back(v->get_id());

    return ii;
}

struct Processor {

    struct {
        void on_start(const ESlot&, const ESlot&, const Context&) {}
        void on_start(const BSlot&, const ESlot&, const Context&) {}
        void on_start(const BSlot&, const BSlot&, const Context&) {}
        void on_end(Vertex&, const Context&) {}
    } vertexMerger;

    struct {
        void on_start(graffine::ChIdG, const Context&) {}
        void on_start(const ESlot&, const Context&) {}
        void on_start(const BSlot&, const Context&) {}
        void on_start(const ESlot&, const ESlot&, const Context&) {}
        void on_end(const std::array<Vertex*, 2>&, const Context&) {}
    } vertexSplit;

    struct {
        template<typename S> void on_start(const S&, const Context&) {}
        void on_start(graffine::ChIdG, graffine::EgIdA, bool, const Context&) {}
        template<bool isNewChain> void on_end(Edge&, const Context&) {}
    } edgeCreation;

    struct {
        template<bool isChainDeleted> void on_start(Edge&, const Context&) {}
        template<bool isChainDeleted> void on_end(Vertex&, const Context&) {}
    } edgeDeletion;

    struct {
        void on_start() {}
        void on_end() {}
    } componentCreation, componentDeletion;

    struct {
        std::vector<VId> start;
        bool kept {true};
        int plans {};

        void on_start(Path& path) { start = ids(path); }
        void on_end(Path& path) { kept = kept && ids(path) == start; }
        bool plan(const trs::pulling::Plan<G>&) { ++plans; return true; }
    } pulling;
};

/**
 * Pulls by \p steps, in a graph with a junction of 3, the path from the end
 * \p e of edge \p a of chain \p w to the free end B of the last chain.
 * \details The chains are w0, of 3 edges, w1, of 3, and w2, of 4, joined at
 * end B of w0 and w1 and end A of w2.
 */
auto pull(const graffine::ChIdG w,
          const std::size_t a,
          const End::Id e,
          const int steps) -> bool
{
    G gr;
    gr.add_single_chain_component(7);
    gr.add_single_chain_component(3);
    trs::vertex_merger::From<graffine::Deg1, graffine::Deg2, G> {gr}(
        ESlot {graffine::ChIdG {1}, End::B},
        BSlot {graffine::ChIdG {0}, graffine::EgIdA {3}});

    const auto& d = gr.chain(w).g[a];
    Path path {&gr.compt(d.c), Path::Driver {&d, d.inner_endId(e)},
               ESlot {gr.ind_last_chain(), End::B}};

    const auto pp = std::make_shared<Processor>();
    trs::pulling::Functor<G, Processor> f {gr, pp};
    if (!path.is_contiguous() || !f.is_applicable(path, steps))
        return false;
    f(path, steps);

    return pp->pulling.plans == 1 && pp->pulling.kept;
}

}  // namespace app

int main()
{
    // drivers of degree 1 (the free end of w0), 2 (inside w0) and 3 (the
    // junction, pulling along w2)
    constexpr graffine::ChIdG w0 {0};
    constexpr graffine::ChIdG w2 {2};
    const auto ok = app::pull(w0, 0, app::End::A, 3) &&
                    app::pull(w0, 1, app::End::A, 2) &&
                    app::pull(w2, 0, app::End::A, 2);

    return ok ? 0 : 1;
}
