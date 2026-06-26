/* =============================================================================

This file is part of graffine, a lightweight graph transformation library.

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
 * \file reporter.h
 * \brief Contains class responsible for logging summary in vertex splits.
 * \details The summary is printed out in standardized form before, during and
 * after the transform.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_SPLIT_REPORTER_H
#define GRAFFINE_TRANSFORMS_VERTEX_SPLIT_REPORTER_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/graph.h"

#include <algorithm>
#include <memory>
#include <ranges>
#include <string>
#include <vector>


namespace graffine::transforms::vertex_split {

/**
 * Structure responsible for outputting formatted summary of vertex split.
 * \details The summary is printed out in standardized form before, during and
 * after the transform.
 */
template<typename G>
struct Reporter
{
    using Graph = G;
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Vertex = Graph::Vertex;
    using ResT = Vertex*;
    using Res = std::array<ResT, 2>;

    const std::string longName {"Vertex split "};
    const std::string shortName {"vs"};

    constexpr explicit Reporter(
        const std::string& degs,
        Graph& gr
    );

    template<structure::descriptors::slot S=EndSlot>
    void before(
        const S& s,
        std::string&& tag={}
    );

    void before(
        std::vector<EndSlot>&& ss
    );

    void after(
        const Res& res,
        ChIdG w1,
        EgIdGs&& inds,
        bool isSelfCycled=false
    ) const;

    void after(
        const Res& res,
        EgIdGs&& inds1,
        EgIdGs&& inds2
    ) const;

    void intermediate(
        ChIdG w1,
        ChIdG w2
    ) const;

    constexpr void branch(int i,
                          std::string_view s) const;

    void print_res(const Res& res) const noexcept;

protected:

    std::string degs;
    std::string tag_before {};

    Graph& gr;  ///< Reference to the modified graph object.

    // References to some of graph class fields for convenience.
    Graph::Chains& cn; ///< Reference to the graph edge chains.

    template<structure::descriptors::slot S>
    void vertex_before(const S& s) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
Reporter<G>::
Reporter(
    const std::string& degs,
    Graph& gr
)
    : degs {degs}
    , gr {gr}
    , cn {gr.chains()}
{}

template<typename G>
void Reporter<G>::
before(std::vector<EndSlot>&& ss)
{
    const auto title = longName + degs + ":";
    std::string spaces {};
    spaces.insert(spaces.begin(), title.size(), ' ');

    std::sort(ss.begin(), ss.end());

    for (const auto& s : ss) {
        jot<false>(colorcodes::GREEN,
                          s == ss[0] ? title : spaces);
        const auto w = s.w;
        jot(colorcodes::BOLDYELLOW, s.str_short(),
                   colorcodes::RESET, " (length ", cn[w].length(), ") ");
    }
    jot("");

    vertex_before(ss[0]);

    std::string str {};
    const auto strB = "before "s + shortName + degs + ": disconnect ";
    auto strA = "and "s;
    strA.insert(strA.begin(), strB.size() - strA.size(), ' ');
    for (const auto& s : ss) {
        str = (s == ss[0] ? strB : strA) + s.ea_str();
        cn[s.w].print(str);
    }

    auto from = "from "s;
    if(str.size() > from.size())
        from.insert(from.begin(), strA.size() - from.size(), ' ');

    auto ws = gr.ngs_at(ss[0]).as_vector(); //() | std::ranges::to<std::vector>();
    std::sort(ws.begin(), ws.end());
    for (const auto& s : ss) {
        const auto r = std::find(ws.begin(), ws.end(), s);
        if (r != ws.end())
            ws.erase(r);
    }

    for (const auto& u : ws) {
        str = (u == ws[0] ? from : strA) + u.ea_str();
        cn[u.w].print(str);
    }

    jot("");
}


template<typename G>
template<structure::descriptors::slot S>
void Reporter<G>::
before(
    const S& s,
    std::string&& tag
)
{
    const auto w = s.w;
    const std::string e = std::is_same_v<S, BulkSlot>
                        ? "at "s + s.ea_str()
                        :  s.e == End::undefined
                        ? ""s
                        : s.ea_str();

    if (degs.find(tag) == std::string::npos)
        degs += tag;

    jot(
        colorcodes::GREEN, longName, degs, ": ",
        colorcodes::BOLDYELLOW, w, " ", e,
        colorcodes::RESET, " (length ", cn[w].length(), ") ",
        '\n'
    );

    vertex_before(s);

    const auto str = "before "s + shortName + degs + ": disconnect ";
    cn[w].print(str, e);

    if constexpr (std::is_same_v<S, EndSlot>)
        if (s.e != End::undefined) {

            tag_before = "from "s;
            if(str.size() > tag_before.size())
                tag_before.insert(tag_before.begin(),
                                  str.size() - tag_before.size(),
                                  ' ');
            auto spaces = "and "s;
            spaces.insert(spaces.begin(), str.size() - spaces.size(), ' ');

            auto ws = gr.ngs_at(s).as_vector();//() | std::ranges::to<std::vector>();
            std::sort(ws.begin(), ws.end());

            for (const auto& u : ws) {
                const auto ss = u == ws[0] ? tag_before : spaces;
                cn[u.w].print(ss, u.ea_str());
            }
        }

    jot("");
}

template<typename G>
template<structure::descriptors::slot S>
void Reporter<G>::
vertex_before(const S& s) const
{
    auto v = s.e == End::undefined

        // unconnected chain boundary vertex
        ? cn[s.w].vertex_at_end(End::A)

        // chain internal or regular boundary vertex
        : cn[s.w].template vertex_at_slot<S>(s);

    v->print(colored_as(colorcodes::MAGENTA, "At vertex: "));
}

template<typename G>
void Reporter<G>::
after(
    const Res& res,
    const ChIdG w1,
    EgIdGs&& inds,
    const bool isSelfCycled
) const
{
    auto str1 = shortName + degs + " produces";
    const auto sz = std::max(str1.length(), tag_before.length() + 1);
    str1.insert(str1.begin(), sz - str1.length(), ' ');

    cn[w1].print(str1);

    if (isSelfCycled)
       jot(colorcodes::MAGENTA, "from a chain cycled into itself",
            colorcodes::RESET);
    else {

        ChIdGs ww;
        for (const auto& id : inds)
            if (auto w = gr.chid(id);
                w != w1)
                ww.push_back(w);

        std::sort(ww.begin(), ww.end());
        ww.erase(std::unique(ww.begin(), ww.end()), ww.end());

        auto str2 = "and"s;
        str2.insert(str2.begin(), sz - str2.length(), ' ');

        for (const auto& w : ww)
            cn[w].print(str2);
    }

    print_res(res);

    jot('\n');
}

template<typename G>
void Reporter<G>::
after(
    const Res& res,
    EgIdGs&& inds1,
    EgIdGs&& inds2
) const
{
    auto str1 = shortName + degs + " produces";
    const auto sz = str1.length();

    std::string spaces(sz, ' ');

    auto str2 = "and"s;
    str2.insert(str2.begin(), sz - str2.length(), ' ');

    ChIdGs ww1;
    for (const auto& i1 : inds1)
        ww1.push_back(gr.chid(i1));
    std::sort(ww1.begin(), ww1.end());
    ww1.erase(std::unique(ww1.begin(), ww1.end()), ww1.end());

    ChIdGs ww2;
    for (const auto& i2 : inds2)
        ww2.push_back(gr.chid(i2));
    std::sort(ww2.begin(), ww2.end());
    ww2.erase(std::unique(ww2.begin(), ww2.end()), ww2.end());

    for (const auto& w1 : ww1) {
        const auto r = std::find(ww2.begin(), ww2.end(), w1);
        if (r != ww2.end())
            ww2.erase(r);
    }

    for (const auto& w1 : ww1)
        cn[w1].print(w1 == ww1[0] ? str1 : spaces);

    for (const auto& w2 : ww2)
        cn[w2].print(w2 == ww2[0] ? str2 : spaces);

    print_res(res);

    jot('\n');
}


template<typename G>
void Reporter<G>::
intermediate(
    const ChIdG w1,
    const ChIdG w2
) const
{
    const auto str1 = shortName + degs + " transiently producing ";
    cn[w1].print(str1);

    auto str2 = "and "s;
    if(str1.size() > str2.size())
        str2.insert(str2.begin(), str1.size() - str2.size(), ' ');
    cn[w2].print(str2);

    jot('\n');
}

template<typename G>
void Reporter<G>::
print_res(const Res& res) const noexcept
{
    res[0]->print(colored_as(colorcodes::MAGENTA, "Vertex: "));
    res[1]->print(colored_as(colorcodes::MAGENTA, "   and: "));
}


template<typename G>
constexpr
void Reporter<G>::
branch(const int i,
       const std::string_view s) const
{
    jot(colorcodes::BLUE, shortName + degs + " case ",
        colorcodes::MAGENTA, i, colorcodes::RESET, ": ", s, '\n');
}

}  // namespace graffine::transforms::vertex_split

#endif  // GRAFFINE_TRANSFORMS_VERTEX_SPLIT_REPORTER_H
