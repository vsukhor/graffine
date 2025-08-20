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
 * @brief Contains class responsible for logging summary in vertex splits.
 * @details Implementation of the split operation depends on the vertex
 * degree. Degree-specific functors are specializations of the generic template
 * defined here. Besides they are the subclusses of the Core class holding the
 * degree-independent machinery.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_VERTEX_SPLIT_LOG_H
#define GRAPH_MUTATOR_VERTEX_SPLIT_LOG_H

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

#include "../../definitions.h"
#include "../../structure/basic/graph.h"


namespace graph_mutator::vertex_split {

template<typename G>
struct Log {

    using Graph = G;
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;

    const std::string longName {"Vertex split "};
    const std::string shortName {"vs"};

    constexpr explicit Log(
        const std::string& degs,
        Graph& gr
    );

    template<structure::basic::slot S=EndSlot>
    void before(
        const S& s,
        std::string&& tag={}
    );

    void before(
        std::vector<EndSlot>&& ss
    );

    void after(
        ChId w1,
        EgIds&& inds,
        bool isSelfCycled=false
    ) const;

    void after(
        EgIds&& inds1,
        EgIds&& inds2
    ) const;

    void intermediate(
        ChId w1,
        ChId w2
    ) const;

private:

    std::string degs;
    std::string tag_before {};

    Graph& gr;  ///< Reference to the graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn; ///< Reference to the graph edge chains.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
Log<G>::
Log(
    const std::string& degs,
    Graph& gr
)
    : degs {degs}
    , gr {gr}
    , cn {gr.cn}
{}

template<typename G>
void Log<G>::
before(std::vector<EndSlot>&& ss)
{
    const auto title = longName + degs + ":";
    std::string spaces {};
    spaces.insert(spaces.begin(), title.size(), ' ');

    std::sort(ss.begin(), ss.end());

    for (const auto& s :ss) {
        log_<false>(colorcodes::GREEN,
                          s == ss[0] ? title : spaces);
        const auto w = s.w;
        log_(colorcodes::BOLDYELLOW, s.str_short(),
                   colorcodes::RESET, " (length ", cn[w].length(), ") ");
    }
    log_("");

    std::string str {};
    const auto strB = "before "s + shortName + degs + ": disconnect ";
    auto strA = "and "s;
    strA.insert(strA.begin(), strB.size() - strA.size(), ' ');
    for (const auto& s :ss) {
        str = (s == ss[0] ? strB : strA) + s.ea_str();
        cn[s.w].print(str);
    }

    auto from = "from "s;
    if(str.size() > from.size())
        from.insert(from.begin(), strA.size() - from.size(), ' ');

    auto ws = gr.ngs_at(ss[0])() | std::ranges::to<std::vector>();
    std::sort(ws.begin(), ws.end());
    for (const auto& s :ss) {
        const auto r = std::find(ws.begin(), ws.end(), s);
        if (r != ws.end())
            ws.erase(r);
    }

    for (const auto u : ws) {
        str = (u == ws[0] ? from : strA) + u.ea_str();
        cn[u.w].print(str);
    }

    log_("");
}


template<typename G>
template<structure::basic::slot S>
void Log<G>::
before(
    const S& s,
    std::string&& tag
)
{
    const auto w = s.w;
    const std::string e = std::is_same_v<S, BulkSlot>
                        ? "at "s + s.ea_str()
                        :  s.e == Ends::Undefined
                        ? ""s
                        : s.ea_str();

    if (degs.find(tag) == std::string::npos)
        degs += tag;

    log_(
        colorcodes::GREEN, longName, degs, ": ",
        colorcodes::BOLDYELLOW, w, " ", e,
        colorcodes::RESET, " (length ", cn[w].length(), ") ",
        '\n'
    );

    const auto str = "before "s + shortName + degs + ": disconnect ";
    cn[w].print(str, e);

    if constexpr (std::is_same_v<S, EndSlot>)
        if (s.e != Ends::Undefined) {

            tag_before = "from "s;
            if(str.size() > tag_before.size())
                tag_before.insert(tag_before.begin(),
                                  str.size() - tag_before.size(),
                                  ' ');
            auto spaces = "and "s;
            spaces.insert(spaces.begin(), str.size() - spaces.size(), ' ');

            auto ws = gr.ngs_at(s)() | std::ranges::to<std::vector>();
            std::sort(ws.begin(), ws.end());

            for (const auto u : ws) {
                const auto ss = u == ws[0] ? tag_before : spaces;
                cn[u.w].print(ss, u.ea_str());
            }
        }

    log_("");
}


template<typename G>
void Log<G>::
after(
    const ChId w1,
    EgIds&& inds,
    const bool isSelfCycled
) const
{
    auto str1 = shortName + degs + " produces";
    const auto sz = std::max(str1.length(), tag_before.length() + 1);
    str1.insert(str1.begin(), sz - str1.length(), ' ');

    cn[w1].print(str1);

    if (isSelfCycled)
       log_(colorcodes::MAGENTA, "from a chain cycled into itself",
            colorcodes::RESET);
    else {
        ChIds w;
        for (const auto id : inds)
            if (gr.glm[id] != w1)
                w.push_back(gr.glm[id]);

        std::sort(w.begin(), w.end());
        w.erase(std::unique(w.begin(), w.end()), w.end());

        auto str2 = "and"s;
        str2.insert(str2.begin(), sz - str2.length(), ' ');

        for (const auto ww : w)
            cn[ww].print(str2);
    }

    log_('\n');
}

template<typename G>
void Log<G>::
after(
    EgIds&& inds1,
    EgIds&& inds2
) const
{
    auto str1 = shortName + degs + " produces";
    const auto sz = str1.length();

    std::string spaces(sz, ' ');

    auto str2 = "and"s;
    str2.insert(str2.begin(), sz - str2.length(), ' ');

    ChIds ww1;
    for (const auto i1 : inds1)
        ww1.push_back(gr.glm[i1]);
    std::sort(ww1.begin(), ww1.end());
    ww1.erase(std::unique(ww1.begin(), ww1.end()), ww1.end());

    ChIds ww2;
    for (const auto i2 : inds2)
        ww2.push_back(gr.glm[i2]);
    std::sort(ww2.begin(), ww2.end());
    ww2.erase(std::unique(ww2.begin(), ww2.end()), ww2.end());

    for (const auto w1 : ww1) {
        const auto r = std::find(ww2.begin(), ww2.end(), w1);
        if (r != ww2.end())
            ww2.erase(r);
    }

    for (const auto w1 : ww1)
        cn[w1].print(w1 == ww1[0] ? str1 : spaces);

    for (const auto w2 : ww2)
        cn[w2].print(w2 == ww2[0] ? str2 : spaces);

    log_('\n');
}


template<typename G>
void Log<G>::
intermediate(
    const ChId w1,
    const ChId w2
) const
{
    const auto str1 = shortName + degs + " transiently producing ";
    cn[w1].print(str1);

    auto str2 = "and "s;
    if(str1.size() > str2.size())
        str2.insert(str2.begin(), str1.size() - str2.size(), ' ');
    cn[w2].print(str2);

    log_('\n');
}

}  // namespace graph_mutator::vertex_split

#endif  // GRAPH_MUTATOR_VERTEX_SPLIT_LOG_H
