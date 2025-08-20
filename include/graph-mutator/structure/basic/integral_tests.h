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
 * @file integral_tester.h
 * @brief Contains class encapsulating intergal testing of the graph structure.
 * @author Valerii Sukhorukov.
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_INTEGRAL_TESTS_H
#define GRAPH_MUTATOR_STRUCTURE_INTEGRAL_TESTS_H

#include "../definitions.h"


namespace graph_mutator::structure {

/**
 * @brief Implements integral tests of the graph internal structure and elements.
 * @tparam G Graph subjected to the tests.
 */
template<typename G>
struct IntegralTests {

    using Graph = G;  ///< Graph class type.
    using Chain = Graph::Chain;
    using Ends = Chain::Ends;
    using EndSlot = Chain::EndSlot;

    constexpr explicit IntegralTests(const Graph& gr);

    /**
     * @brief Runs consecutively all the specific tests implemented here.
     * @param it Index of the current iteration.
     */
    void operator()(itT it) const;

    /**
     * @brief Tests component attributes and inter-component arrangement.
     * @param it Index of the current iteration.
     */
    void components(itT it) const;

    void paths(itT it) const;

    /**
     * @brief Tests correspondence of current numbers of vertices.
     */
    void vertex_numbers(itT it) const;

    /**
     * @brief Tests cycles.
     * @details Ensures that no inappropriate cycles are present.
     */
     void loops() const;

    /**
     * @brief Tests consistency of inter-chain connections within components.
     * @param it Index of the current iteration.
     */
    void consistency(itT it) const;

    /**
     * @brief Tests correctness of chain indexes.
     * @param it Index of the current iteration.
     */
    void chain_id(itT it) const;

    /**
     * @brief Tests edge attributes.
     * @details Ensures that edge global and intracomponent indexes, as well as
     * component indexes are within the limits of the current graph size.
     */
    void edges(const itT it) const;

    /**
     * @brief Tests correctness of edge positioning within a host chain.
     * @param it Index of the current iteration.
     */
    void chain_g(itT it) const;

private:

    const Graph& gr;  ///< Reference to the graph object.


    // Const references to some of graph class fields for convenience:

    const Graph::Chains& cn;  ///< Reference to the graph edge chains.
    const EgId& edgenum;  ///< Current number of edges.

    /// Mapping of indexes: edge to chain.
    const ChIds& glm;
     /// Mapping of edge indexes to positions within chains.
    const EgIds& gla;

    const Graph::Components& ct;  ///< Chain indices segregated into components.
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G>
constexpr
IntegralTests<G>::
IntegralTests(const Graph& gr)
    : gr {gr}
    , cn {gr.cn}
    , edgenum {gr.edgenum}
    , glm {gr.glm}
    , gla {gr.gla}
    , ct {gr.ct}
{}


template<typename G>
void IntegralTests<G>::
operator()(const itT it) const
{
    consistency(it);
//    paths(it);
    components(it);
    loops();
    chain_id(it);
    edges(it);
    chain_g(it);
    vertex_numbers(it);
}

/*
template<typename G>
void IntegralTests<G>::
paths(const itT it) const
{
    if (!chain_num()) return;

    typename G::Base::pathT pr_old;
    std::vector<typename G::Base::weight_t> min_dist_old;

    const auto ajl_old = gr.make_adjacency_list_old();

    for (EgId j=0; j<edgenum; ++j) {
        const auto c1 = cn[glm[j]].c;
        gr.compute_paths(typename G::Base::vertex_t(j), ajl_old, min_dist_old, pr_old);

        for (const auto& m: cn) {
            const auto c2 = m.c;
            for (const auto& o : m.g)
                if (c2 == c1) {
                    if (min_dist_old[o.ind] == G::Base::EdgeT::maxWeight) {
                        gr.ct[c1].print("err_1");
                        gr.ct[c1].print_adjacency_list_edges("err_1: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " and ", o.ind,
                              " belong to the same cluster ", c1,
                              " but 'min_dist between them' == maxWeight");
                    }
                }
                else
                    if (min_dist_old[o.ind] != G::Base::EdgeT::maxWeight) {
                        gr.ct[c1].print("err_2");
                        gr.ct[c2].print("err_2");
                        gr.ct[c1].print_adjacency_list_edges("err_2: ");
                        gr.ct[c2].print_adjacency_list_edges("err_2: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " (cl ", c1, ") ", " and ", o.ind,
                              " (cl ", c2, ") have min_dist != maxWeight");
                    }
        }
    }
}
*/

template<typename G>
void IntegralTests<G>::
components(const itT it) const
{
    const auto nc = gr.chain_num();

    if (!nc) return;

    // min and max component indexes in chains

    std::vector<CmpId> cids(nc);
    for (ChId i=0; i<nc; ++i)
        cids[i] = cn[i].c;

    const auto maxv = *std::max_element(cids.begin(), cids.end());

    ENSURE(maxv == gr.ind_last_cmpt(),
           "at iteration ", it,
           ": max cn.c = ", maxv, " != number of components = ", gr.cmpt_num());

    const auto minv = *std::min_element(cids.begin(), cids.end());

    ENSURE(minv == 0, "at iteration ", it, " min cn.c ", minv, " is not 0");


//    using BaseEdgeType = utils::graph::EdgeType<real,
//                                                EgId,
//                                                utils::graph::EdgeMode::unweighted>;
//    using BaseGraph = utils::graph::Graph<BaseEdgeType>;
//    typename BaseGraph::pathT pr;
//    typename G::Cmpt::pathT pr;
//    std::vector<typename G::BaseEdgeType::vertex_t> pr;
//    std::vector<typename G::Edge::weight_t> min_dist;

//    auto egfun = [](const typename G::Base::EdgeT& eg)
//    {
//        return typename G::Base::EdgeT{eg.target};
//    };

//    const auto ajl =
//        gr.template adjacency_list_edges<typename G::Base::EdgeT>(egfun);

/*
    typename G::Base::pathT pr;
    std::vector<typename G::Base::weight_t> min_dist;
    const auto ajl = gr.make_adjacency_list_old();

    for (EgId j=0; j<edgenum; ++j) {
        const auto c1 = cn[glm[j]].c;
        gr.compute_paths(typename G::Base::vertex_t(j), ajl, min_dist, pr);
        for (const auto& m: cn) {
            const auto c2 = m.c;
            for (const auto& o : m.g)
                if (c2 == c1) {
                    if (min_dist[o.ind] == G::Base::EdgeT::maxWeight) {
                        gr.ct[c1].print("err_1");
                        gr.ct[c1].print_adjacency_list_edges("err_1: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " and ", o.ind,
                              " belong to the same cluster ", c1,
                              " but 'min_dist between them' == maxWeight");
                    }
                }
                else
                    if (min_dist[o.ind] != G::Base::EdgeT::maxWeight) {
                        gr.ct[c1].print("err_2");
                        gr.ct[c2].print("err_2");
                        gr.ct[c1].print_adjacency_list_edges("err_2: ");
                        gr.ct[c2].print_adjacency_list_edges("err_2: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " (cl ", c1, ") ", " and ", o.ind,
                              " (cl ", c2, ") have min_dist != maxWeight");
                    }

        }
    }
*/
    for (const auto& c : ct) {
        for (const auto w : c.ww) {

            ENSURE(cn[w].c == c.ind,
                   "at iter ", it, " in c.ww for w ", w, ", c.ind ", c.ind);

           for (const auto e: Ends::Ids)
                for (const auto& ng: cn[w].ngs[e]())
                    if (cn[ng.w].c != c.ind) {
                        cn[w].print(" wrong ng c ");
                        cn[ng.w].print(" wrong ng c ");
                        abort("at iteration ", it, " errorN in c.ww for w ", w,
                              ", c.ind ", c.ind, " in ng {", ng.w, " ",
                              Ends::str(ng.e), "} c is wrong: ", cn[ng.w].c);
                    }
        }
        for (ChId idc {}; idc<c.num_chains(); ++idc) {
            bool found {};
            for (const auto w : c.ww)
                if (cn[w].idc == idc) {
                    found = true; break;
                }
            c.ensure(
                found,
                "err_2",
                "at iter ", it, " idc ", idc, " not found, ind ", c.ind
            );
        }
    }

    for (ChId w=0; w<gr.chain_num(); ++w)
        for (const auto& o : cn[w].g) {
            ENSURE(o.c == cn[w].c,
                   "at iteration ", it, " in g.c != cn.c for cn: ", w);
        }

    vec2<CmpId> cc(gr.cmpt_num());
    for (CmpId ic=0; ic<gr.cmpt_num(); ++ic)
        cc[ic].resize(ct[ic].num_edges());

    for (const auto& m: cn)
        for (const auto& o : m.g)
            ++cc[o.c][o.indc];

    for (ChId w=0; w<gr.chain_num(); ++w)
        for (const auto& o : cn[w].g) {
            ENSURE(o.c == cn[w].c,
                   "at iteration ", it, " cn[w].g[i].c != cn[w].c for w: ", w);
        }

    for (CmpId ic=0; ic<gr.cmpt_num(); ++ic) {
        const auto c = cc[ic];

        ENSURE(std::accumulate(c.begin(), c.end(), EgId{}) == ct[ic].num_edges(),
               "at iteration ", it, " sum(c) != cmp num edges for ic: ", ic);

        ENSURE(*std::min_element(c.begin(), c.end()) == 1,
               "at iteration ", it, " minval(c) != 1 for ic: ", ic);

        ENSURE(*std::max_element(c.begin(), c.end()) == 1,
               "at iteration ", it, " maxval(c) != 1 for ic: ", ic);
    }

    for (auto& o: cc)
        o.clear();

    for (const auto& m: cn)
        for (const auto& o : m.g)
            cc[o.c].push_back(o.indc);

    for (CmpId ic=0; ic<gr.cmpt_num(); ++ic) {
        const auto c = cc[ic];
        const auto numeg = ct[ic].num_edges();

        ENSURE(c.size() == numeg,
               "at iteration ", it, " c.size() != cmp num edges for ic: ", ic);

        ENSURE(*std::min_element(c.begin(), c.end()) == 0,
               "at iteration ", it, " minval(c) != 0 for ic: ", ic);

        ENSURE(*std::max_element(c.begin(), c.end()) == ct[ic].num_edges()-1,
               "at iteration ", it,
               " maxval(c) != cmp num edges - 1", " for ic: ", ic);
    }

    for (const auto& cmp: ct) {

        cmp.ensure(
            cmp.gl.size() == cmp.num_edges(),
            "ERR",
            "at iteration ", it,
            " cmp.gl.size() ", cmp.gl.size(), " != cmp.num_edges()",
            " for ic: ", cmp.ind
        );

        for (szt i {}; const auto& o : cmp.gl) {

            const auto& eg = cn[o.w].g[o.a];

            cmp.ensure(
                eg.indc == i,
                "ERR",
                "at iteration ", it,
                " eg.indc ", eg.indc, " != i ", i,
                " for ic: ", cmp.ind, " w ", o.w, " a ", o.a
            );
            ++i;
            cmp.ensure(
                eg.ind == o.i,
                "ERR",
                "at iteration ", it,
                " eg.ind ", eg.ind, " != o.i ", o.i, " for ic: ", cmp.ind
            );
        }
    }
}


template<typename G>
void IntegralTests<G>::
vertex_numbers(itT it) const
{
    auto nvct = [&]<Degree D>() -> szt
    {
        szt n {};
        for (const auto& c: gr.ct)
            n += c.template num_vertices<D>();
        return n;
    };

    ENSURE(nvct.template operator()<0>() == gr.vertices.template num<0>(),
           "vertex_numbers by cmpts test faled for D = ", 0,"  -- ",
           "expected: ", gr.vertices.template num<0>(),
           ", actual: ", nvct.template operator()<0>());

    ENSURE(nvct.template operator()<1>() == gr.vertices.template num<1>(),
           "vertex_numbers by cmpts test faled for D = ", 1,"  -- ",
           "expected: ", gr.vertices.template num<1>(),
           ", actual: ", nvct.template operator()<1>());

    ENSURE(nvct.template operator()<2>() == gr.vertices.template num<2>(),
           "vertex_numbers by cmpts test faled for D = ", 2,"  -- ",
           "expected: ", gr.vertices.template num<2>(),
           ", actual: ", nvct.template operator()<2>());

    ENSURE(nvct.template operator()<3>() == gr.vertices.template num<3>(),
           "vertex_numbers by cmpts test faled for D = ", 3,"  -- ",
           "expected: ", gr.vertices.template num<3>(),
           ", actual: ", nvct.template operator()<3>());

    ENSURE(nvct.template operator()<4>() == gr.vertices.template num<4>(),
           "vertex_numbers by cmpts test faled for D = ", 4,"  -- ",
           "expected: ", gr.vertices.template num<4>(),
           ", actual: ", nvct.template operator()<4>());

    auto nvcn = [&]<Degree D>() noexcept -> szt
    {
        szt k {};
        for (const auto& m: cn)
            k += m.template num_vertices<D>();

        return D == Deg3 ? k/3 :
               D == Deg4 ? k/4 : k;
    };

    ENSURE(nvcn.template operator()<0>() == gr.vertices.template num<0>(),
           "vertex_numbers by chains test faled for D = ", 0,"  -- ",
            "expected: ", gr.vertices.template num<0>(),
            ", actual: ", nvcn.template operator()<0>());

    ENSURE(nvcn.template operator()<1>() == gr.vertices.template num<1>(),
           "vertex_numbers by chains test faled for D = ", 1,"  -- ",
           "expected: ", gr.vertices.template num<1>(),
           ", actual: ", nvcn.template operator()<1>());

    ENSURE(nvcn.template operator()<2>() == gr.vertices.template num<2>(),
           "vertex_numbers by chains test faled for D = ", 2,"  -- ",
            "expected: ", gr.vertices.template num<2>(),
            ", actual: ", nvcn.template operator()<2>());

    ENSURE(nvcn.template operator()<3>() == gr.vertices.template num<3>(),
           "vertex_numbers by chains test faled for D = ", 3,"  -- ",
           "expected: ", gr.vertices.template num<3>(),
           ", actual: ", nvcn.template operator()<3>());

    ENSURE(nvcn.template operator()<4>() == gr.vertices.template num<4>(),
           "vertex_numbers by chains test faled for D = ", 4,"  -- ",
           "expected: ", gr.vertices.template num<4>(),
            ", actual: ", nvcn.template operator()<4>());
}


template<typename G>
void IntegralTests<G>::
loops() const
{
    for (const auto& m: cn)
        m.ensure(
            !(m.is_cycle() &&
              m.length() < Chain::minCycleLength),
            " L ",
            "Forbidden loop found ","in chain ", m.idw, " printed above"
        );
}


template<typename G>
void IntegralTests<G>::
consistency(const itT it) const
{
//    print_chains( "ChC" );
    for (ChId i=0; i<gr.chain_num(); ++i) {
        for (const auto j: Ends::Ids) {
            for (const auto& s : cn[i].ngs[j]()) {

                const auto cnei = s.w;

                if (cnei >= gr.chain_num()) {
                    gr.print_chains("TEST FAILED ");
                    log_("");
                    abort("At iteration ", it, ": Consistency test failed ",
                          "for cn = ", i, ", cnei = ", cnei);
                }

                const auto ce = s.e;
                const auto& ss = cn[cnei].ngs[ce]();
                const EndSlot ij {i, j};
                auto itr = std::find_if(
                    std::begin(ss),
                    std::end(ss),
                    [&](const EndSlot& v){ return v == ij; }
                );
                if (ss.end() == itr) {
                    gr.print_chains("TEST FAILED ");
                    log_("");
                    abort("At iteration ", it, ": Consistency test ",
                          "failed for cn = ", i, ", end = ", j,
                          " cnei = ", cnei, " ce = ", ce);
                }
            }
            ENSURE(!cn[i].ngs[j].has_repeating_slots(),
                   "Consistency test failed: same neig and end",
                   " in chain ", i, ", end ", j, " at iteration ", it);
        }
    }

    for (EgId j=0; j<edgenum; ++j)
        ENSURE(cn[glm[j]].g[gla[j]].ind == j,
               "Error by checking indma at ind: ", j);
}


template<typename G>
void IntegralTests<G>::
chain_id(const itT it) const
{
    for (ChId i=0; i<gr.chain_num(); ++i)
        cn[i].ensure(cn[i].idw == i,
            "should have ind ",
            "Error 1: at iteration ", it," check.chain_id faied at ind ", i
        );
}


template<typename G>
void IntegralTests<G>::
edges(const itT it) const
{
    EgId egn {};
    for (const auto& m: cn) {
        egn += m.length();
        for (EgId a {}; const auto& o : m.g) {
            ENSURE(o.w == m.idw,
                   "Error 1: at iteration ", it,
                    " check.edges faied at w ", m.idw, " edge ind ", o.ind);
            if (o.indw != a++) {
                m.print("wrong indw ");
                abort("Error 2: at iteration ", it,
                      " check.edges faied at w ", m.idw, ": at a ", a-1,
                      "indw ", o.indw, " is incorrect; ", " edge ind ", o.ind);
            }

            o.check(edgenum, ct[m.c].num_edges(), m.length(),
                    gr.chain_num(), gr.cmpt_num());
        }
    }
    ASSERT(egn == edgenum, "incorrect total number of edges");
}


template<typename G>
void IntegralTests<G>::
chain_g(const itT it) const
{
    EgId egn {};
    for (ChId i=0; i<gr.chain_num(); ++i) {
        egn += cn[i].length();
        for (EgId r=0; r<cn[i].length(); ++r) {
            if (cn[i].g[r].ind >= edgenum)
                abort("Error 1: at iteration ", it,
                      " check.chain_g faied at ind ", cn[i].g[r].ind);
            for (ChId i1=i; i1<gr.chain_num(); ++i1)
                for (EgId r1=0; r1<cn[i1].length(); ++r1)
                    if (!(i == i1 && r == r1) &&
                        cn[i].g[r].ind == cn[i1].g[r1].ind)
                        abort("Error 4: at iteration ", it,
                              " check.chain_g faied at ind ", cn[i].g[r].ind);
        }
    }
    ENSURE(egn == edgenum,
           " Error 5: at iter ", it, " check.chain_g faied at egn ", egn);
}


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_INTEGRAL_TESTS_H
