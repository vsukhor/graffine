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
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/elements/vertex.h"

#include "gtest/gtest.h"

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