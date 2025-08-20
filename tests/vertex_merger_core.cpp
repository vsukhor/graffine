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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>

#include "common.h"
#include "graph-mutator/structure/basic/chain.h"
#include "graph-mutator/structure/basic/edge.h"
#include "graph-mutator/structure/basic/graph.h"
#include "graph-mutator/transforms/vertex_merger/core.h"


namespace graph_mutator::tests::vertex_merger_core {

class VertexMergerCoreTest
    : public Test {

protected:

    using Gr = structure::basic::Graph<structure::basic::Chain<structure::basic::Edge<maxDegree>>>;
    using Chain = Gr::Chain;
    using Edge = Chain::Edge;
    using Ends = Chain::Ends;  ///< Chain ends.
    using VMCore = vertex_merger::Core<Gr>;

    /// Subclass to make protected members accessible for testing:
    class Core
        : public VMCore {

    public:
        using VMCore::antiparallel;
        using VMCore::parallel;
        using VMCore::to_cycle;
        using VMCore::cn;

        explicit Core(Gr& gr) : VMCore {gr, "vm_core"} {}
    };
};

// =============================================================================


TEST_F(VertexMergerCoreTest, Constructor)
{
    ++testCount;

    Gr gr;
    Core ct {gr};

    ASSERT_TRUE(gr.glm.empty());
    ASSERT_TRUE(gr.gla.empty());
    ASSERT_TRUE(gr.cn.empty());
    EXPECT_EQ(gr.chain_num(), 0);
    EXPECT_EQ(gr.cmpt_num(), 0);
    EXPECT_EQ(gr.edgenum, 0);
    ASSERT_TRUE(gr.ct.empty());
    ASSERT_TRUE(gr.chis.cn11.empty());
    ASSERT_TRUE(gr.chis.cn22.empty());
    ASSERT_TRUE(gr.chis.cn33.empty());
    ASSERT_TRUE(gr.chis.cn13.empty());
}


TEST_F(VertexMergerCoreTest, FuseAntiparA)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests vertex merger antiparallel of two separate chains at ends A");

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    for (ChId w1=0; w1<len.size(); ++w1)
        for (ChId w2=0; w2<len.size(); ++w2)
            if (w1 != w2) {

                Gr gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgId ind {}; const auto& m: gr.cn)
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto g1 = gr.cn[w1].g;  // copy
                const auto g2 = gr.cn[w2].g;  // copy

                core.antiparallel(Ends::A, w1, w2);

                ASSERT_EQ(gr.edgenum, lensum);
                ASSERT_EQ(gr.chain_num(), len.size()-1);
                ASSERT_EQ(gr.cmpt_num(), len.size()-1);

                // 'res' is the merged chain:
                const auto res = w1 != len.size()-1 ? w1 : w2;

                const auto& r = gr.cn[res];

                for (ChId i=0; i<gr.chain_num(); ++i) {

                    const auto& m = gr.cn[i];

                    ASSERT_EQ(m.c, i);
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1] + len[w2]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the previously last chain.
                        ASSERT_EQ(m.length(), len[gr.chain_num()]);
                    else    // the rest is not affected
                        ASSERT_EQ(m.length(), len[i]);
                }

                for (szt c=0, i=len[w1]-1; c<g1.size(); ++c, --i)
                    ASSERT_EQ(r.g[i].ind, g1[c].ind);

                for (szt c=0, i=len[w1]; i<r.length(); ++c, ++i)
                    ASSERT_EQ(r.g[i].ind, g2[c].ind);

                for (CmpId i {}; i<gr.ct.size(); ++i) {
                    const auto& c = gr.ct[i];
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
                    ASSERT_EQ(c.gl.size(), c.num_edges());
                    EgId h {};
                    for (ChId k {}; const auto w: c.ww) {
                        const auto& m = gr.cn[w];
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgId j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.gl[h].w, m.g[j].w);
                            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                        }
                    }
                    ASSERT_EQ(c.chis.cn11, i);
                    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
                    ASSERT_EQ(c.chis.cn33.size(), 0);
                    ASSERT_EQ(c.chis.cn44.size(), 0);
                    ASSERT_EQ(c.chis.cn13.size(), 0);
                    ASSERT_EQ(c.chis.cn14.size(), 0);
                    ASSERT_EQ(c.chis.cn34.size(), 0);
                }
            }
}


TEST_F(VertexMergerCoreTest, FuseAntiparB)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests vertex merger antiparallel of two separate chains at ends B");

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    for (ChId w1=0; w1<len.size(); ++w1)
        for (ChId w2=0; w2<len.size(); ++w2)
            if (w1 != w2) {

                Gr gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgId ind {}; const auto& m: gr.cn)
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto g1 = gr.cn[w1].g;  // copy
                const auto g2 = gr.cn[w2].g;  // copy

                core.antiparallel(Ends::B, w1, w2);

                ASSERT_EQ(gr.edgenum, lensum);
                ASSERT_EQ(gr.chain_num(), len.size()-1);
                ASSERT_EQ(gr.cmpt_num(), len.size()-1);

                // 'res' is the merged chain:
                const auto res = w1 != len.size()-1 ? w1 : w2;

                for (ChId i=0; i<gr.chain_num(); ++i) {

                    const auto& m = gr.cn[i];

                    ASSERT_EQ(m.c, i);
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1]+len[w2]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the previously last chain
                        ASSERT_EQ(m.length(), len[gr.chain_num()]);
                    else    // not affected
                        ASSERT_EQ(m.length(), len[i]);

                    for (const auto& gg : m.g)
                        ASSERT_EQ(gg.c, m.c);
                }

                int c {};
                for (EgId i=0; i<len[w1]; ++c, ++i)
                    ASSERT_EQ(gr.cn[res].g[i].ind, g1[c].ind);
                c = len[w2] - 1;
                for (EgId i=len[w1]; c>=0; --c, ++i)
                    ASSERT_EQ(gr.cn[res].g[i].ind, g2[c].ind);

                for (const auto e : Ends::Ids)
                    ASSERT_EQ(gr.cn[res].ngs[e].num(), 0);

                for (CmpId i {}; i<gr.ct.size(); ++i) {
                    const auto& c = gr.ct[i];
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
                    ASSERT_EQ(c.gl.size(), c.num_edges());
                    EgId h {};
                    for (ChId k {}; const auto w: c.ww) {
                        const auto& m = gr.cn[w];
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgId j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.gl[h].w, m.g[j].w);
                            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                        }
                    }
                    ASSERT_EQ(c.chis.cn11, i);
                    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
                    ASSERT_EQ(c.chis.cn33.size(), 0);
                    ASSERT_EQ(c.chis.cn44.size(), 0);
                    ASSERT_EQ(c.chis.cn13.size(), 0);
                    ASSERT_EQ(c.chis.cn14.size(), 0);
                    ASSERT_EQ(c.chis.cn34.size(), 0);
                }
            }
}


TEST_F(VertexMergerCoreTest, FuseParallel)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger parallel of two separate chains");

    constexpr std::array len {4UL, 2UL, 1UL, 3UL};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    for (ChId w1=0; w1<len.size(); ++w1)
        for (ChId w2=0; w2<len.size(); ++w2)
            if (w1 != w2) {

                Gr gr;
                Core core {gr};
                for (const auto u : len)
                    gr.add_single_chain_component(u);

                for (EgId ind {}; const auto& m: gr.cn)
                    for (const auto& g : m.g)
                        ASSERT_EQ(g.ind, ind++);

                const auto g1 = gr.cn[w1].g;  // copy
                const auto g2 = gr.cn[w2].g;  // copy

                core.parallel(w1, w2);

                ASSERT_EQ(gr.edgenum, lensum);
                ASSERT_EQ(gr.chain_num(), len.size()-1);
                ASSERT_EQ(gr.cmpt_num(), len.size()-1);

                // 'res' is the merged chain:
                const auto res = w1 != len.size()-1 ? w1 : w2;

                for (ChId i=0; i<gr.chain_num(); ++i) {

                    const auto& m = gr.cn[i];

                    ASSERT_EQ(m.c, i);
                    ASSERT_EQ(m.idw, i);

                    if (i == res)
                        ASSERT_EQ(m.length(), len[w1]+len[w2]);
                    else if (w1 != len.size()-1 && i == w2)
                        // w2 is occupied by the previously last chain
                        ASSERT_EQ(m.length(), len[gr.chain_num()]);
                    else    // not affected
                        ASSERT_EQ(m.length(), len[i]);

                    for (const auto& gg : m.g)
                        ASSERT_EQ(gg.c, m.c);
                }

                for (szt c=0, i=0; i<len[w2]; ++c, ++i)
                    ASSERT_EQ(gr.cn[res].g[i].ind, g2[c].ind);
                for (szt c=0, i=len[w2]; i<gr.cn[res].length(); ++c, ++i)
                    ASSERT_EQ(gr.cn[res].g[i].ind, g1[c].ind);

                for (const auto e : Ends::Ids)
                    ASSERT_EQ(gr.cn[res].ngs[e].num(), 0);

                for (CmpId i {}; i<gr.ct.size(); ++i) {
                    const auto& c = gr.ct[i];
                    ASSERT_EQ(c.ind, i);
                    ASSERT_EQ(c.num_chains(), 1);
                    ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
                    ASSERT_EQ(c.gl.size(), c.num_edges());
                    EgId h {};
                    for (ChId k {}; const auto w: c.ww) {
                        const auto& m = gr.cn[w];
                        ASSERT_EQ(m.c, c.ind);
                        ASSERT_EQ(m.idc, k++);
                        for (EgId j {}; j<m.length(); ++j, ++h) {
                            ASSERT_EQ(m.g[j].c, c.ind);
                            ASSERT_EQ(m.g[j].indc, h);
                            ASSERT_EQ(c.gl[h].w, m.g[j].w);
                            ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                            ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                        }
                    }
                    ASSERT_EQ(c.chis.cn11, i);
                    ASSERT_EQ(c.chis.cn22, undefined<ChId>);
                    ASSERT_EQ(c.chis.cn33.size(), 0);
                    ASSERT_EQ(c.chis.cn44.size(), 0);
                    ASSERT_EQ(c.chis.cn13.size(), 0);
                    ASSERT_EQ(c.chis.cn14.size(), 0);
                    ASSERT_EQ(c.chis.cn34.size(), 0);
                }
            }
}


TEST_F(VertexMergerCoreTest, FuseToLoop)
{
    ++testCount;

    if constexpr (verboseT)
        print_description("Tests vertex merger to cycle of separate chain");

    constexpr std::array<szt, 6> len {4, 3, 6, 5, 2, 7};
    constexpr auto lensum = std::accumulate(len.begin(), len.end(), 0);

    Gr gr;
    Core core {gr};

    const auto& mt = gr.cn;

    for (ChId w=0; w<len.size(); ++w)
        gr.add_single_chain_component(len[w]);

    for (EgId ind {}; const auto& m: mt)
        for (const auto& g : m.g)
            ASSERT_EQ(g.ind, ind++);

    for (ChId i=0; i<gr.chain_num(); ++i) {
        ASSERT_EQ(mt[i].idw, i);
        ASSERT_EQ(mt[i].c, i);
    }

    for (ChId w=0; w<len.size(); ++w) {

        const auto g = mt[w].g;  // copy

        core.to_cycle(w);

        ASSERT_EQ(gr.edgenum, lensum);
        ASSERT_EQ(gr.chain_num(), len.size());
        ASSERT_EQ(gr.cmpt_num(), len.size());

        for (ChId i=0; i<gr.chain_num(); ++i) {
            ASSERT_EQ(mt[i].length(), len[i]);
            ASSERT_EQ(mt[i].idw, i);
            ASSERT_EQ(mt[i].c, i);
        }
        for (EgId i=0; i<len[w]; ++i) {
            ASSERT_EQ(mt[w].g[i].ind, g[i].ind);
            ASSERT_EQ(mt[w].g[i].c, mt[w].c);
        }

        for (CmpId i {}; i<gr.ct.size(); ++i) {
            const auto& c = gr.ct[i];
            ASSERT_EQ(c.ind, i);
            ASSERT_EQ(c.num_chains(), 1);
            ASSERT_EQ(c.num_edges(), gr.cn[c.ww[0]].length());
            ASSERT_EQ(c.gl.size(), c.num_edges());
            EgId h {};
            for (ChId k {}; const auto w: c.ww) {
                const auto& m = gr.cn[w];
                ASSERT_EQ(m.c, c.ind);
                ASSERT_EQ(m.idc, k++);
                for (EgId j {}; j<m.length(); ++j, ++h) {
                    ASSERT_EQ(m.g[j].c, c.ind);
                    ASSERT_EQ(m.g[j].indc, h);
                    ASSERT_EQ(c.gl[h].w, m.g[j].w);
                    ASSERT_EQ(c.gl[h].a, m.g[j].indw);
                    ASSERT_EQ(c.gl[h].i, m.g[j].ind);
                }
            }
            if (i <= w) {
                ASSERT_EQ(c.chis.cn11, undefined<ChId>);
                ASSERT_EQ(c.chis.cn22, i);
            }
            else {
                ASSERT_EQ(c.chis.cn11, i);
                ASSERT_EQ(c.chis.cn22, undefined<ChId>);
            }
            ASSERT_EQ(c.chis.cn33.size(), 0);
            ASSERT_EQ(c.chis.cn44.size(), 0);
            ASSERT_EQ(c.chis.cn13.size(), 0);
            ASSERT_EQ(c.chis.cn14.size(), 0);
            ASSERT_EQ(c.chis.cn34.size(), 0);
        }

        ASSERT_EQ(mt[w].ngs[Ends::A].num(), 1);
        ASSERT_EQ(mt[w].ngs[Ends::A]().front().w, w);
        ASSERT_EQ(mt[w].ngs[Ends::A]().front().e, Ends::B);

        ASSERT_EQ(mt[w].ngs[Ends::B].num(), 1);
        ASSERT_EQ(mt[w].ngs[Ends::B]().front().w, w);
        ASSERT_EQ(mt[w].ngs[Ends::B]().front().e, Ends::A);

        ASSERT_EQ(gr.chis.cn11.size(), len.size() - w - 1);
        ASSERT_EQ(gr.chis.cn22.size(), w + 1);
        ASSERT_EQ(gr.chis.cn33.size(), 0);
        ASSERT_EQ(gr.chis.cn44.size(), 0);
        ASSERT_EQ(gr.chis.cn13.size(), 0);
        ASSERT_EQ(gr.chis.cn14.size(), 0);
        ASSERT_EQ(gr.chis.cn34.size(), 0);
    }
}


}  // namespace graph_mutator::tests::vertex_merger_core
