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
 * \file graph_factory.cpp
 * \brief Benchmarks of the graph assembly by GraphFactory.
 * \details The input consists of short free rings and linear chains, which
 * makes many chains and so exercises all lookups of the assembly. The
 * factory of graffine v1.87.0, kept in tests/graph_factory_reference.h, is
 * benchmarked on smaller inputs for comparison: its time grows at least
 * quadratically with the input size, that of the current one linearly.
 */

#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/modifiers/graph_factory.h"
#include "graph_factory_reference.h"

#include <benchmark/benchmark.h>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace {

namespace el = graffine::structure;

using G = el::Graph<el::Component<el::Chain<el::Edge<el::Vertex>>>>;
using Vertex = G::Vertex;
using Edge = G::Edge;
using End = G::End;
using graffine::EgIdG;
using graffine::Orientation;
using graffine::VIdH;

constexpr std::size_t chainLen {4};

/// Input of the factory.
struct Input {
    std::vector<std::unique_ptr<Vertex>> vv;
    std::vector<Edge> egs;
};

/// Free rings and linear chains of chainLen edges, alternating, with
/// about \p n edges in total.
auto generate(const std::size_t n) -> Input
{
    Input in;
    auto new_vertex = [&in]() {
        in.vv.push_back(std::make_unique<Vertex>(VIdH {in.vv.size()}, false));
        return in.vv.size() - 1;
    };
    auto link = [&in](const std::size_t u, const std::size_t v) {
        auto& eg = in.egs.emplace_back(EgIdG {in.egs.size()},
                                       Orientation::Forwards, Edge::weight_t {1});
        eg.connect_to_outer(End::A, *in.vv[u]);
        eg.connect_to_outer(End::B, *in.vv[v]);
    };

    in.egs.reserve(n);
    for (std::size_t k {}; in.egs.size() + chainLen <= n; ++k) {
        const auto first = new_vertex();
        auto prev = first;
        for (std::size_t i {1}; i < chainLen; ++i) {
            const auto x = new_vertex();
            link(prev, x);
            prev = x;
        }
        link(prev, k % 2 ? first : new_vertex());
    }
    return in;
}

/// Assembly of a graph by factory \p F from about N edges.
template<typename F>
void assemble(benchmark::State& state)
{
    const auto n = static_cast<std::size_t>(state.range(0));
    std::size_t numEdges {};
    for (auto _ : state) {
        state.PauseTiming();
        auto in = generate(n);
        numEdges = in.egs.size();
        state.ResumeTiming();

        auto gr = F {}.create(std::move(in.vv), std::move(in.egs));
        benchmark::DoNotOptimize(gr.num_edges());

        state.PauseTiming();
        {
            const auto discard = std::move(gr);
        }
        state.ResumeTiming();
    }
    state.counters["edges"] = static_cast<double>(numEdges);
    state.SetComplexityN(static_cast<benchmark::IterationCount>(numEdges));
}


/// Assembly by the current factory.
void BM_GraphFactory(benchmark::State& state)
{
    assemble<el::modifiers::GraphFactory<G>>(state);
}
BENCHMARK(BM_GraphFactory)->RangeMultiplier(4)->Range(2048, 2 << 20)
                          ->Unit(benchmark::kMillisecond)->Complexity();


/// Assembly by the factory of graffine v1.87.0.
void BM_GraphFactoryV1_87(benchmark::State& state)
{
    assemble<el::modifiers::reference::ReferenceGraphFactory<G>>(state);
}
BENCHMARK(BM_GraphFactoryV1_87)->RangeMultiplier(2)->Range(2048, 32768)
                               ->Unit(benchmark::kMillisecond)->Complexity();

}  // namespace
