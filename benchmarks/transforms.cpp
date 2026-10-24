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
 * \file transforms.cpp
 * \brief Benchmarks of graph construction, transformations and lookups.
 * \details The graphs consist of N separate linear chains of chainLen edges.
 * The benchmarks of single transformations run in a steady state: each
 * iteration applies a transformation and its inverse, so that the graph
 * keeps its size, and the time per iteration shows how the cost of a
 * transformation scales with the graph size.
 */

#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/transforms/edge_creation/in_existing_chain.h"
#include "graffine/transforms/edge_deletion/functor.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_split/to_11.h"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <iostream>
#include <vector>

namespace {

namespace el = graffine::structure;
namespace trs = graffine::transforms;

using G = el::Graph<el::Component<el::Chain<el::Edge<el::Vertex>>>>;
using End = G::End;
using ESlot = G::EndSlot;
using graffine::ChIdG;
using graffine::EgIdG;
using graffine::VIdH;

constexpr std::size_t chainLen {32};

/// Output message processor required by graffine.
struct Messenger {
    Messenger() { graffine::msgr = new graffine::Msgr {&std::cout, nullptr, 6}; }
    ~Messenger() { delete graffine::msgr; }
};
const Messenger messenger;

/// Graph of \p n separate linear chains.
void build(G& gr, const std::size_t n)
{
    for (std::size_t k {}; k < n; ++k)
        gr.add_single_chain_component(chainLen);
}

void set_counters(benchmark::State& state, const G& gr)
{
    state.counters["edges"] = static_cast<double>(gr.num_edges());
    state.SetComplexityN(static_cast<benchmark::IterationCount>(gr.num_edges()));
}


/// Building a graph of N chains.
void BM_BuildGraph(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    for (auto _ : state) {
        G gr;
        build(gr, n);
        benchmark::DoNotOptimize(gr.num_edges());
    }
    state.counters["edges"] = static_cast<double>(n * chainLen);
    state.SetComplexityN(static_cast<benchmark::IterationCount>(n * chainLen));
}
BENCHMARK(BM_BuildGraph)->RangeMultiplier(2)->Range(16, 256)
                        ->Unit(benchmark::kMillisecond)->Complexity();


/// Creation of an edge at a chain end, and its deletion.
void BM_EdgeCreationDeletion(benchmark::State& state)
{
    G gr;
    build(gr, static_cast<std::size_t>(state.range(0)));
    trs::edge_creation::InExistingChain<G> create {gr};
    trs::edge_deletion::Functor<G> remove {gr};

    for (auto _ : state) {
        const auto res = create(ESlot {ChIdG {0}, End::A});
        remove(res[0]->ind);
    }
    set_counters(state, gr);
}
BENCHMARK(BM_EdgeCreationDeletion)->RangeMultiplier(2)->Range(16, 256)
                                  ->Unit(benchmark::kMicrosecond)->Complexity();


/// Merger of the free ends of two chains, and the split of the merged vertex.
void BM_VertexMergerSplit(benchmark::State& state)
{
    G gr;
    build(gr, static_cast<std::size_t>(state.range(0)));
    trs::vertex_merger::From<graffine::Deg1, graffine::Deg1, G> merge {gr};
    trs::vertex_split::To<graffine::Deg1, graffine::Deg1, G> split {gr};

    for (auto _ : state) {
        const auto res = merge(ESlot {ChIdG {0}, End::B},
                               ESlot {gr.ind_last_chain(), End::A});
        split(*res[0]);
    }
    set_counters(state, gr);
}
BENCHMARK(BM_VertexMergerSplit)->RangeMultiplier(2)->Range(16, 256)
                               ->Unit(benchmark::kMicrosecond)->Complexity();


/// Lookup of vertices by their run-wide identifier.
void BM_VertexLookup(benchmark::State& state)
{
    G gr;
    build(gr, static_cast<std::size_t>(state.range(0)));

    std::vector<VIdH> ids;
    for (const auto& vd : gr.get_ved())
        ids.emplace_back(vd.idh);

    std::size_t k {};
    for (auto _ : state) {
        benchmark::DoNotOptimize(gr.vertex(ids[k]));
        k = k + 1 < ids.size() ? k + 1 : 0;
    }
    set_counters(state, gr);
}
BENCHMARK(BM_VertexLookup)->RangeMultiplier(2)->Range(16, 256)
                          ->Unit(benchmark::kNanosecond)->Complexity();

}  // namespace

BENCHMARK_MAIN();
