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
 * \brief Contains class responsible for formatted summary of vertex merger.
 * \details The summary is printed out in standardized form before, during and
 * after the transform.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_VERTEX_MERGER_REPORTER_H
#define GRAFFINE_TRANSFORMS_VERTEX_MERGER_REPORTER_H

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/vertex.h"
#include "graffine/structure/descriptors/slot.h"
#include "graffine/transforms/vertex_merger/common.h"  // UnconnectedCycleEnd

#include <algorithm>
#include <ranges>
#include <string>


namespace graffine::transforms::vertex_merger {

/**
 * Structure responsible for outputting formatted summary of vertex merger.
 * \details The summary is printed out in standardized form before, during and
 * after the transform.
 */
template<typename G>
struct Reporter
{
    using Graph = G;
    using Chain = Graph::Chain;  ///< Chain type.
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;
    using BulkSlot = Chain::BulkSlot;
    using Neigs = Chain::Neigs;
    using Vertex = Graph::Vertex;

    static constexpr std::string_view indent {"                        "};
    const std::string longName {"Vertex merger "};
    const std::string shortName {"vm"};

    explicit Reporter(
        Graph& gr,
        const std::string& degs={}
    );

    template<structure::descriptors::slot S1=EndSlot,
             structure::descriptors::slot S2=EndSlot>
    void before(
        const S1& s1,
        const S2& s2,
        std::string&& tag={},
        std::string&& ssA={},
        std::string&& ssB={},
        const std::string& dd={}
    );

    void after(
        Vertex* v,
        ChIdG w1,
        ChIdGs&& ws={},
        const ChIdG mi=ChIdG::undefined
    ) const;

    template<typename... Args>
    constexpr void branch(int i,
                          Args... args) const;

protected:

    std::string degs;
    std::string tag_ {};
    std::string taggedName {};

    Graph::Chains& cn; ///< Reference to the graph edge chains.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
Reporter<G>::
Reporter(
    Graph& gr,
    const std::string& degs
)
    : degs {degs}
    , cn {gr.chains()}
{}


template<typename G>
template<structure::descriptors::slot S1,
         structure::descriptors::slot S2>
void Reporter<G>::
before(
    const S1& s1,
    const S2& s2,
    std::string&& tag,
    std::string&& ssA,
    std::string&& ssB,
    const std::string& dd
)
{
    if (dd.length())
        degs = dd;

    const auto& [w1, ea1] = s1.we();
    const auto& [w2, ea2] = s2.we();

    bool isDefinedEA1 {};
    if constexpr (std::is_same_v<S1, BulkSlot>)
        isDefinedEA1 = ea1.is_defined();
    else
        isDefinedEA1 = End::is_defined(ea1);

    bool isDefinedEA2 {};
    if constexpr (std::is_same_v<S2, BulkSlot>)
        isDefinedEA2 = ea2.is_defined();
    else
        isDefinedEA2 = End::is_defined(ea2);

    const auto v1 = isDefinedEA1
                  ? cn[w1].template vertex_at_slot<S1>(s1)
                  : cn[w1].template vertex_at_slot<S1>(EndSlot{w1, UnconnectedCycleEnd});
    const auto v2 = isDefinedEA2
                  ? cn[w2].template vertex_at_slot<S2>(s2)
                  : cn[w2].template vertex_at_slot<S2>(EndSlot{w2, UnconnectedCycleEnd});

    tag_ = std::move(tag);
    taggedName = shortName + degs + tag_;

    std::string str1 {" "};
    if constexpr (std::is_same_v<S1, BulkSlot>)
        if (ea1.is_defined())
            str1 += "at ";

    if (isDefinedEA1)
        str1 += s1.ea_str();

    std::string str2 {" "};
    if constexpr (std::is_same_v<S2, BulkSlot>)
        if (isDefinedEA2)
            str2 += "at ";

    if (isDefinedEA2)
        str2 += s2.ea_str();

    jot(
        colorcodes::GREEN, longName, degs, tag_, ":  ", colorcodes::RESET,
        ssA, colorcodes::BOLDYELLOW, w1, str1, colorcodes::RESET, " (length ", cn[w1].length(), ") with ",
        ssB, colorcodes::BOLDYELLOW, w2, str2, colorcodes::RESET, " (length ", cn[w2].length(), ")\n"
    );
    v1->print(colored_as(colorcodes::MAGENTA, "Vertices "));
    v2->print(colored_as(colorcodes::MAGENTA, "     and "));
    jot("");

    const auto str3 = "before "s + taggedName + ":";
    cn[w1].print(str3);
    if (w2 != w1) {
        cn[w2].print(str3);
        if constexpr (std::is_same_v<S2, EndSlot>)
            if (degs == "13") {

                ASSERT(cn[w2].num_neigs(ea2) == 2, "num neigs is incorrect for 13");
                const auto ngs = cn[w2].ngs[ea2].as_vector();  // segments connected to w2 at e2
                for (const auto& ng : ngs)
                    cn[ng.w].print(str3);
            }
    }

    jot("");
}


template<typename G>
void Reporter<G>::
after(
    Vertex* v,
    const ChIdG w1,
    ChIdGs&& ws,
    const ChIdG mi
) const
{
    auto str0 = " produces"s;
    auto str1 = str0 + taggedName;
    const auto sz = str1.size();

    cn[w1].print(taggedName, str0);

    if (ws.size() > 1) {
        std::sort(ws.begin(), ws.end());
        ws.erase(std::unique(ws.begin(), ws.end()), ws.end());
    }

    auto str2 = "and"s;
    if (sz > str2.size())
        str2.insert(str2.begin(), sz - str2.size(), ' ');

    for (const auto& w : ws)
        if (w != w1)
            cn[w].print(str2);

    if (mi.is_defined() && degs == "12" &&
        !cn[ws[0]].is_unconnected_cycle())  // because ws.size() == 1
        cn[mi].print(str2);

    v->print(colored_as(colorcodes::MAGENTA, "Merged vertex: "));

    jot('\n');
}

template<typename G>
template<typename... Args>
constexpr
void Reporter<G>::
branch(const int i,
       Args... args) const
{
    jot(colorcodes::BLUE, shortName + degs + tag_ + " case ",
        colorcodes::MAGENTA, i, colorcodes::RESET, ": ", args..., '\n');
}


}  // namespace graffine::transforms::vertex_merger

#endif  // GRAFFINE_TRANSFORMS_VERTEX_MERGER_REPORTER_H
