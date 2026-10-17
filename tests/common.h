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

#ifndef GRAFFINE_TESTS_COMMON_H
#define GRAFFINE_TESTS_COMMON_H

#include "graffine/definitions.h"
#include "graffine/structure/analyzers/integral_tests.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/elements/vertex.h"

#include "gtest/gtest.h"

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace graffine::tests {

using namespace std::string_literals;

namespace elements = structure;

using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using Chain = G::Chain;
using Edge = Chain::Edge;
using End = Chain::End;
using ESlot = Chain::EndSlot;
using BSlot = Chain::BulkSlot;

inline constexpr auto profuse = verboseT && false;


/// Topology-level summary of a graph, used to compare graph states.
struct Snapshot {

    std::size_t numEdges {};
    std::size_t numChains {};
    std::size_t numCompts {};
    std::vector<std::size_t> lengths;               ///< Per chain.
    std::vector<std::array<std::size_t, 2>> ngs;    ///< Per chain end.
    std::array<std::size_t, 4> numVertices {};      ///< Per degree 1 to 4.

    explicit Snapshot(const G& gr)
        : numEdges {gr.num_edges()}
        , numChains {gr.num_chains()}
        , numCompts {gr.num_compts()}
        , numVertices {gr.num_vertices(Deg1), gr.num_vertices(Deg2),
                       gr.num_vertices(Deg3), gr.num_vertices(Deg4)}
    {
        for (ChIdG w {}; w < gr.num_chains(); ++w) {
            const auto& m = gr.chain(w);
            lengths.push_back(m.length());
            ngs.push_back({m.ngs[End::A].num(), m.ngs[End::B].num()});
        }
    }

    bool operator==(const Snapshot&) const = default;
};


/**
 * Runs the graph-wide integral tests on a graph when going out of scope.
 * \details Declared right after the graph in a test body, it is destroyed
 * just before the graph, so that the final graph state of every test gets
 * validated. The check is skipped if the test has already failed, or if
 * skip() was called by a test leaving the graph inconsistent on purpose.
 * \note The integral tests terminate the test executable on failure.
 * \tparam Gr Graph type.
 */
template<typename Gr>
struct IntegrityCheck {

    explicit IntegrityCheck(const Gr& gr) : gr {gr} {}

    IntegrityCheck(const IntegrityCheck&) = delete;
    IntegrityCheck& operator=(const IntegrityCheck&) = delete;

    ~IntegrityCheck()
    {
        if (!skipped && !testing::Test::HasFailure())
            structure::IntegralTests<Gr, profuse> {gr}();
    }

    /// Disables the check.
    void skip() noexcept { skipped = true; }

private:

    const Gr& gr;      ///< Graph to be checked.
    bool skipped {};   ///< Whether the check is disabled.
};

struct Test
    : public testing::Test {

    using Vertex = elements::Vertex;
    using Edge = elements::Edge<Vertex>;
    using End = Edge::End;
    using Chain = elements::Chain<Edge>;
    using Cmp = elements::Component<Chain>;
    using Chains = Cmp::Chains;


protected:

    static constexpr const char* tagBefore {"TEST_BEFORE"};
    static constexpr const char* tagAfter {"TEST_AFTER"};

    static unsigned int testCount;
    static constexpr unsigned char subtestCountIni {'A' - 1};

    void SetUp() override
    {
        msgr = new Msgr {&std::cout, nullptr, 6};
    }

    void TearDown() override
    {
        if (msgr)
            delete msgr;
    }

    template<typename...Args>
    void print_description(Args... args)
    {
        jot('\n',
            colorcodes::GREEN, testCount, " : ",
            colorcodes::YELLOW, args..., colorcodes::RESET,
            '\n');
    }

    template<typename...Args>
    void print_description_subtest(const char subind,
                                   Args... args)
    {
        jot('\n',
            colorcodes::GREEN, testCount, ".", subind, " : ",
            colorcodes::YELLOW, args..., colorcodes::RESET,
            '\n');
    }
};

inline unsigned int Test::testCount {};

}  // namespace graffine::tests

#endif  // GRAFFINE_TESTS_COMMON_H