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
#include "graffine/structure/analyzers/integral_tests.h"
#include "graffine/utils/errors.h"

#include <stdexcept>
#include <string>
#include <type_traits>

namespace graffine::tests::errors {

using vertex_merger::VertexMerger1;

static_assert(std::is_base_of_v<std::runtime_error, graffine::error>);
static_assert(std::is_base_of_v<graffine::error, graffine::invariant_error>);
static_assert(std::is_base_of_v<graffine::error, graffine::precondition_error>);
static_assert(std::is_base_of_v<graffine::error, graffine::io_error>);

class ErrorsTest
    : public Test {

protected:

    static constexpr ChIdG w0 {0};
    static constexpr ChIdG w1 {1};

    /// Chain w1 attached by end A to the bulk of chain w0, splitting it.
    static void make_junction(G& gr)
    {
        gr.add_single_chain_component(4);
        gr.add_single_chain_component(3);
        VertexMerger1<Deg2, G> {gr}(ESlot{w1, End::A}, BSlot{w0, 2});
    }

    /// Removes \p s from the neighbours of one of its neighbours only.
    static void break_symmetry_of_neighbours(G& gr, const ESlot& s)
    {
        const auto ne = *gr.chain(s.w).ngs[s.e]().begin();
        gr.chains()[ne.w].ngs[ne.e].erase(s);
    }
};

// =============================================================================


/// Validation of a consistent graph.
TEST_F(ErrorsTest, ValidateConsistentGraph)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests validation of a consistent graph");

    G gr;
    make_junction(gr);

    const structure::IntegralTests<G, profuse> tests {gr};
    EXPECT_TRUE(tests.validate().has_value());
    EXPECT_NO_THROW(tests());
}


/// Validation of a graph with inconsistent inter-chain connections.
TEST_F(ErrorsTest, ValidateInconsistentGraph)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that validation of an inconsistent graph reports the ",
            "violation, and that the throwing check raises invariant_error");

    G gr;
    make_junction(gr);
    break_symmetry_of_neighbours(gr, ESlot{w1, End::A});

    const structure::IntegralTests<G, profuse> tests {gr};

    const auto res = tests.validate();
    ASSERT_FALSE(res.has_value());
    EXPECT_NE(res.error().message.find("Consistency test"), std::string::npos)
        << res.error().message;

    EXPECT_THROW(tests(), graffine::invariant_error);
}


/// A mutation meeting inconsistent bookkeeping throws instead of exiting.
TEST_F(ErrorsTest, InvariantErrorFromMutation)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that removing a slot from inconsistent neighbour lists ",
            "throws invariant_error");

    G gr;
    make_junction(gr);
    const ESlot s {w1, End::A};
    break_symmetry_of_neighbours(gr, s);

    try {
        gr.remove_slot_from_neigs(s);
        ADD_FAILURE() << "no invariant_error";
    }
    catch (const graffine::invariant_error& e) {
        const std::string what {e.what()};
        EXPECT_NE(what.find("not found among"), std::string::npos) << what;
        EXPECT_NE(what.find("graph/neighbours.h:"), std::string::npos) << what;
    }
}


/// Lookups requested for unsuitable elements throw precondition_error.
TEST_F(ErrorsTest, PreconditionErrors)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that chain lookups requested for unsuitable elements throw ",
            "precondition_error");

    G gr;
    const IntegrityCheck integrityCheck {gr};
    gr.add_single_chain_component(3);
    const auto& m = gr.chain(w0);

    // Unconnected chain: no single connected end.
    EXPECT_THROW(m.get_single_connected_end(), graffine::precondition_error);
    EXPECT_THROW(m.get_single_3way_end(), graffine::precondition_error);

    // The middle edge is not at a chain end.
    try {
        [[maybe_unused]] const auto e = m.ind2end(m.g[1].ind);
        ADD_FAILURE() << "no precondition_error";
    }
    catch (const graffine::error& e) {  // catchable as the common base
        EXPECT_NE(dynamic_cast<const graffine::precondition_error*>(&e), nullptr);
        EXPECT_NE(std::string {e.what()}.find("is not an end edge"),
                  std::string::npos) << e.what();
    }

    // The graph is unchanged by the failed lookups.
}

}  // namespace graffine::tests::errors
