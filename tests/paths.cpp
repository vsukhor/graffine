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

#include "common.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/paths/over_endslots/generic.h"
#include "graffine/structure/paths/over_edges/generic.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_22.h"

#include "sample_graph.h"

#include <array>
#include <iostream>
#include <numeric>  // iota
#include <string>


namespace graffine::tests::paths {

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



namespace trs = graffine::transforms;

template<Degree D1,
         Degree D2,
         typename G> requires (isImplementedDegree<D1> &&
                               isImplementedDegree<D2>)
struct VertexMerger
    : public transforms::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : transforms::vertex_merger::From<D1, D2, G> {graph}
    {}
};


class PathTest
    : public Test {

    G gr;

protected:

    static constexpr auto undefined = graffine::undefined<size_t>;
};

// =============================================================================

TEST_F(PathTest, OverEndSlots)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests the shortest path between chain boundary edges"
        );

    const auto gr = create_sample_graph<G>();

//    for (const auto& c : gr.compts()) {
        const auto& c = gr.compt(CmpId{8});
        if constexpr (profuse)
            c.print();

        structure::paths::over_endslots::Generic<G::Compt> pp {c};
        for (const auto& w : c.ww) {
            for (const auto e : End::Ids) {

                const ESlot s {w, e};
                const ESlot t {19, End::B};
                const auto path = pp.find_shortest_path<true>(s, t);
                if constexpr (profuse) {
                    pp.print_distances("    From ", s.str_short(), " :: ");
                    pp.print_path(path, ": Shortest ");
                    jot("");
                }
            }
        }
        jot("");
//    }
}


TEST_F(PathTest, OverEdgeInds)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "the shortest path between arbirtrary edges ",
            "in a connected component"
        );

    const auto gr = create_sample_graph<G>();

//    for (const auto& c : gr.compts()) {
        const auto& c = gr.compt(CmpId{8});

        if constexpr (profuse)
            c.print();

        structure::paths::over_edges::Generic<G::Compt> pp {&c};
        for (EgIdC e1 {}; e1 < c.num_edges(); ++e1) {
            pp.compute_from_source(e1);

            if constexpr (profuse)
                pp.print_distances("    from ", e1, " :: ");

            for (EgIdC e2 {}; e2 < c.num_edges(); ++e2) {

                const auto path = pp.template find_shortest_path<false>(e1, e2);

                if constexpr (profuse) {
                    pp.print_path<false>(path, ": Shortest ");
                    jot("");
                }
            }
        }
        jot("");
//    }
}

/*
TEST_F(PathTest, OverEgIds)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "OverEgIds"
        );

    const auto gr = create_graph();

//    for (const auto& c : gr.compts()) {

        const auto& c = gr.ct[8];
        paths::Paths<EgId, G::Compt> pp {c};

        const auto ajl_old = gr.make_adjacency_list_old();

        for (EgId indc {}; indc<c.num_edges(); ++indc) {

            const auto s = indc;
            const auto t = 3;
            jot("component ", c.ind, ": source ", indc, ": ");
//           pp.template compute_from_source<true>(s);
            const auto path = pp.find_shortest_path(s, t);
            pp.print_distances("    from ", indc, " :: ");

            typename G::Base::pathT pr_old;
            std::vector<typename G::Base::weight_t> min_dist_old;

            gr.compute_paths(typename G::Base::vertex_t(c.gl[indc].i),
                             ajl_old,
                             min_dist_old,
                             pr_old);
            for (EgId tar {}; const auto& z: c.gl) {
                const auto prev = is_defined(pr_old[z.i])
                                ? gr.edge(pr_old[z.i]).indc
                                : pr_old[z.i];
                jot<false>(tar++, "=>[", prev, " ", min_dist_old[z.i], "] ");
            }
            jot("");

            const auto path_old_vec =
                gr.shortest_path(typename G::Base::vertex_t(c.gl[indc].i),
                                 typename G::Base::vertex_t(c.gl[t].i),
                                 ajl_old);
            const auto path_old_dq = pp.from_global_ind(path_old_vec);
            pp.print_path(path, "new");
            pp.print_path(path_old_dq, "old");
        }
        jot("");
//    }

}

*/
}  // namespace graffine::tests::paths
