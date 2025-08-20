/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file log.h
 * @brief Contains class responsible for logging summary.
 * @details Implementation of the split operation depends on the vertex
 * degree. Degree-specific functors are specializations of the generic template
 * defined here. Besides they are the subclusses of the Core class holding the
 * degree-independent machinery.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_MERGER_LOG_H
#define GRAPH_MUTATOR_VERTEX_MERGER_LOG_H

#include <algorithm>
#include <ranges>
#include <string>

#include "../../definitions.h"
#include "../../structure/basic/chain.h"  // for slot


namespace graph_mutator::vertex_merger {

template<typename G>
struct Log {

    using Graph = G;
    using Chain = Graph::Chain;  ///< Chain type.
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;

    const std::string longName {"Vertex merger "};
    const std::string shortName {"vm"};

    explicit Log(
        const std::string& degs,
        Graph& gr,
        std::string&& ssA={},
        std::string&& ssB={}
    );

    template<structure::basic::slot S1=EndSlot,
             structure::basic::slot S2=EndSlot>
    void before(
        const S1& s1,
        const S2& s2,
        std::string&& tag={}
    );

    void after(
        ChId w1,
        ChIds&& ws={},
        const ChId mi=undefined<ChId>
    ) const;

    void branch(int i, const std::string& s) const;

private:

    std::string degs;
    std::string tag_ {};
    std::string ssA;
    std::string ssB;

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn; ///< Reference to the graph edge chains.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
Log<G>::
Log(
    const std::string& degs,
    Graph& gr,
    std::string&& ssA,
    std::string&& ssB
)
    : degs {degs}
    , ssA {ssA}
    , ssB {ssB}
    , gr {gr}
    , cn {gr.cn}
{}


template<typename G>
template<structure::basic::slot S1,
         structure::basic::slot S2>
void Log<G>::
before(
    const S1& s1,
    const S2& s2,
    std::string&& tag
)
{
    const auto [w1, ea1] = s1.we();
    const auto [w2, ea2] = s2.we();

    tag_ = std::move(tag);

    std::string str1 {" "};
    if constexpr (std::is_same_v<S1, BulkSlot>)
        if (is_defined(ea1))
            str1 += "at ";

    if (is_defined(ea1))
        str1 += s1.ea_str();

    std::string str2 {" "};
    if constexpr (std::is_same_v<S2, BulkSlot>)
        if (is_defined(ea2))
            str2 += "at ";

    if (is_defined(ea2))
        str2 += s2.ea_str();

    log_(
        colorcodes::GREEN, longName, degs, tag_, ":  ", colorcodes::RESET,
        ssA, colorcodes::BOLDYELLOW, w1, str1, colorcodes::RESET, " (length ", cn[w1].length(), ") with ",
        ssB, colorcodes::BOLDYELLOW, w2, str2, colorcodes::RESET, " (length ", cn[w2].length(), ")\n"
    );

    const auto str3 = "before "s + shortName + degs + tag_ + ":";
    cn[w1].print(str3);
    if (w2 != w1) {
        cn[w2].print(str3);
        if (degs == "13") {
            const Neigs ngs = cn[w2].ngs[ea2];  // segments connected to w2 at e2
            cn[ngs[0].w].print(str3);
            cn[ngs[1].w].print(str3);
        }
    }

    log_("");
}


template<typename G>
void Log<G>::
after(
    const ChId w1,
    ChIds&& ws,
    const ChId mi
) const
{
    auto str1 = shortName + degs + tag_ + " produces";
    const auto sz = str1.size();

    cn[w1].print(str1);

    if (ws.size() > 1) {
        std::sort(ws.begin(), ws.end());
        ws.erase(std::unique(ws.begin(), ws.end()), ws.end());
    }

    auto str2 = "and"s;
    if (sz > str2.size())
        str2.insert(str2.begin(), sz - str2.size(), ' ');

    for (const auto w : ws)
        if (w != w1)
            cn[w].print(str2);

    if (degs == "12" &&
        !cn[ws[0]].is_disconnected_cycle())  // because ws.size() == 1
        cn[mi].print(str2);

    log_('\n');
}

template<typename G>
void Log<G>::
branch(const int i, const std::string& s) const
{
    log_(colorcodes::BLUE, shortName + degs + tag_ + " case ",
         colorcodes::MAGENTA, i, colorcodes::RESET, ": ", s, '\n');
}

}  // namespace graph_mutator::vertex_merger

#endif  // GRAPH_MUTATOR_VERTEX_MERGER_LOG_H
