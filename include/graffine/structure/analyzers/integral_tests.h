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
 * \file integral_tests.h
 * \brief Contains class encapsulating intergal testing of the graph structure.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_STRUCTURE_ANALYZERS_INTEGRAL_TESTS_H
#define GRAFFINE_STRUCTURE_ANALYZERS_INTEGRAL_TESTS_H

#include "graffine/definitions.h"


namespace graffine::structure {

/**
 * Implements integral tests of the graph internal structure and its elements.
 * \tparam G Graph subjected to the tests.
 * \tparam beLoud Flag switching verbous reporting.
 */
template<typename G,
         bool beLoud = true>
struct IntegralTests
{
    using Graph = G;  ///< Graph class type.
    using Chain = Graph::Chain;
    using End = Chain::End;
    using EndSlot = Chain::EndSlot;

    constexpr explicit IntegralTests(const Graph& gr);

    /**
     * Runs consecutively all the specific tests implemented here.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void operator()(itT it = 0) const;

    /**
     * Tests component attributes and inter-component arrangement.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void components(itT it) const;

    void paths(itT it) const;

    /**
     * Tests correspondence of current numbers of vertices.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void vertex_numbers(itT it) const;

    /**
     * Tests cycles.
     * \details Ensures that no inappropriate cycles are present.
     */
     void loops() const;

    /**
     * Tests consistency of inter-chain connections within components.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void consistency(itT it) const;

    /**
     * Tests correctness of chain indexes.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void chain_id(itT it) const;

    /**
     * Tests edge attributes.
     * \details Ensures that edge global and intracomponent indexes, as well as
     * component indexes are within the limits of the current graph size.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void edges(const itT it) const;

    /**
     * Tests correctness of edge positioning within a host chain.
     * \param[in] it Index of the current iteration if used in a multi-step
     * sequence of transformations.
     */
    void chain_g(itT it) const;

private:

    const Graph& gr;  ///< Reference to the examined graph object.

    // Const references to some of graph class fields for convenience:

    const Graph::Components& ct;  ///< Connected components forming the graph.
    const Graph::Chains&     cn;  ///< Reference to the graph edge chains.
    /// Mapping of indexes: edge to chain and to positions within chains.
    const Graph::EdgeDescriptors& egl;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

template<typename G,
         bool beLoud>
constexpr
IntegralTests<G, beLoud>::
IntegralTests(const Graph& gr)
    : gr {gr}
    , ct {gr.compts()}
    , cn {gr.chains()}
    , egl {gr.get_egl()}
{}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
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
template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
paths(const itT it) const
{
    if (!num_chains()) return;

    typename G::Base::pathT pr_old;
    std::vector<typename G::Base::weight_t> min_dist_old;

    const auto ajl_old = gr.make_adjacency_list_old();

    for (std::size_t j=0; j<gr.num_edges(); ++j) {
        const auto c1 = cn[glm[j]].c;
        gr.compute_paths(typename G::Base::vertex_t(j), ajl_old, min_dist_old, pr_old);

        for (const auto& m : cn) {
            const auto c2 = m.c;
            for (const auto& o : m.g)
                if (c2 == c1) {
                    if (min_dist_old[o.ind] == G::Base::EdgeT::maxWeight) {
                        gr.compt(c1).print("err_1");
                        gr.compt(c1).print_adjacency_list_edges("err_1: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " and ", o.ind,
                              " belong to the same cluster ", c1,
                              " but 'min_dist between them' == maxWeight");
                    }
                }
                else
                    if (min_dist_old[o.ind] != G::Base::EdgeT::maxWeight) {
                        gr.compt(c1).print("err_2");
                        gr.compt(c2).print("err_2");
                        gr.compt(c1).print_adjacency_list_edges("err_2: ");
                        gr.compt(c2).print_adjacency_list_edges("err_2: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " (cl ", c1, ") ", " and ", o.ind,
                              " (cl ", c2, ") have min_dist != maxWeight");
                    }
        }
    }
}
*/

template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
components(const itT it) const
{
    const auto nc = gr.num_chains();

    if (!nc) return;

    // min and max component indexes in chains

    std::vector<CmpId> cids(nc);
    for (std::size_t i {}; i < nc; ++i)
        cids[i] = cn[i].c;

    const auto maxv = *std::max_element(cids.begin(), cids.end());

    ENSURE(maxv == gr.ind_last_cmpt(),
           "at iteration ", it,
           ": max cn.c = ", maxv, " != number of components = ", gr.num_compts());

    const auto minv = *std::min_element(cids.begin(), cids.end());

    ENSURE(minv == 0, "at iteration ", it, " min cn.c ", minv, " is not 0");


//    using BaseEdgeType = utils::graph::EdgeType<real,
//                                                EgId,
//                                                utils::graph::EdgeMode::unweighted>;
//    using BaseGraph = utils::graph::Graph<BaseEdgeType>;
//    typename BaseGraph::pathT pr;
//    typename G::Compt::pathT pr;
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

    for (std::size_t j=0; j<gr.num_edges(); ++j) {
        const auto c1 = cn[glm[j]].c;
        gr.compute_paths(typename G::Base::vertex_t(j), ajl, min_dist, pr);
        for (const auto& m : cn) {
            const auto c2 = m.c;
            for (const auto& o : m.g)
                if (c2 == c1) {
                    if (min_dist[o.ind] == G::Base::EdgeT::maxWeight) {
                        gr.compt(c1).print("err_1");
                        gr.compt(c1).print_adjacency_list_edges("err_1: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " and ", o.ind,
                              " belong to the same cluster ", c1,
                              " but 'min_dist between them' == maxWeight");
                    }
                }
                else
                    if (min_dist[o.ind] != G::Base::EdgeT::maxWeight) {
                        gr.compt(c1).print("err_2");
                        gr.compt(c2).print("err_2");
                        gr.compt(c1).print_adjacency_list_edges("err_2: ");
                        gr.compt(c2).print_adjacency_list_edges("err_2: ");
                        abort("ERROR check.components: at iteration ", it,
                              " edges ", j, " (cl ", c1, ") ", " and ", o.ind,
                              " (cl ", c2, ") have min_dist != maxWeight");
                    }

        }
    }
*/
    for (const auto& c : ct) {
        for (const auto& w : c.ww) {

            ENSURE(cn[w].c == c.ind,
                   "at iter ", it, " in c.ww for w ", w, ", c.ind ", c.ind);

           for (const auto e : End::Ids)
                for (const auto& ng : cn[w].ngs[e]())
                    if (cn[ng.w].c != c.ind) {
                        cn[w].print(" wrong ng c ");
                        cn[ng.w].print(" wrong ng c ");
                        abort("at iteration ", it, " errorN in c.ww for w ", w,
                              ", c.ind ", c.ind, " in ng {", ng.w, " ",
                              End::str(ng.e), "} c is wrong: ", cn[ng.w].c);
                    }
        }
        for (std::size_t idc {}; idc < c.num_chains(); ++idc) {
            bool found {};
            for (const auto& w : c.ww)
                if (cn[w].idc == idc) {
                    found = true;
                    break;
                }
            c.ensure(
                found,
                "err_2",
                "at iter ", it, " idc ", idc, " not found, ind ", c.ind
            );
        }

        c.template check<beLoud>();
    }

    for (std::size_t w {}; w<gr.num_chains(); ++w)
        for (const auto& o : cn[w].g) {
            ENSURE(o.c == cn[w].c,
                   "at iteration ", it, " in g.c != cn.c for cn: ", w);
        }

    vec2<size_t> cc(gr.num_compts());
    for (std::size_t ic {}; ic < gr.num_compts(); ++ic)
        cc[ic].resize(ct[ic].num_edges());

    for (const auto& m : cn)
        for (const auto& o : m.g)
            ++cc[o.c()][o.indc()];

    for (std::size_t w {}; w < gr.num_chains(); ++w)
        for (const auto& o : cn[w].g) {
            ENSURE(o.c == cn[w].c,
                   "at iteration ", it, " cn[w].g[i].c != cn[w].c for w: ", w);
        }

    for (std::size_t ic {}; ic < gr.num_compts(); ++ic) {
        const auto& c = cc[ic];

        ENSURE(std::accumulate(c.begin(), c.end(), 0U) == ct[ic].num_edges(),
               "at iteration ", it, " sum(c) != cmp num edges for ic: ", ic);

        ENSURE(*std::min_element(c.begin(), c.end()) == 1,
               "at iteration ", it, " minval(c) != 1 for ic: ", ic);

        ENSURE(*std::max_element(c.begin(), c.end()) == 1,
               "at iteration ", it, " maxval(c) != 1 for ic: ", ic);
    }

    for (auto& o : cc)
        o.clear();

    for (const auto& m : cn)
        for (const auto& o : m.g)
            cc[o.c()].push_back(o.indc());

    for (std::size_t ic {}; ic < gr.num_compts(); ++ic) {
        const auto c = cc[ic];
        const auto numeg = ct[ic].num_edges();

        ENSURE(c.size() == numeg,
               "at iteration ", it, " c.size() != cmp number of edges for ic: ", ic);

        ENSURE(*std::min_element(c.begin(), c.end()) == 0,
               "at iteration ", it, " minval(c) != 0 for ic: ", ic);

        ENSURE(*std::max_element(c.begin(), c.end()) == ct[ic].egId_last()(),
               "at iteration ", it,
               " maxval(c) != cmp number of edges - 1", " for ic: ", ic);
    }

    for (const auto& cmp : ct) {

        cmp.ensure(
            cmp.get_egl().size() == cmp.num_edges(),
            "ERR",
            "at iteration ", it,
            " cmp.egl.size() ", cmp.get_egl().size(), " != cmp.num_edges()",
            " for ic: ", cmp.ind
        );

        for (std::size_t i {}; const auto& o: cmp.get_egl()) {

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


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
vertex_numbers([[maybe_unused]] itT it) const
{
    auto nvct = [&]<Degree D>() -> std::size_t
    {
        std::size_t n {};
        for (const auto& c : gr.compts())
            n += c.num_vertices(D);
        return n;
    };

    ENSURE(nvct.template operator()<1>() == gr.num_vertices(Deg1),
           "vertex_numbers by cmpts test faled for D = ", 1,"  -- ",
           "expected: ", gr.num_vertices(Deg1),
           ", actual: ", nvct.template operator()<1>());

    ENSURE(nvct.template operator()<2>() == gr.num_vertices(Deg2),
           "vertex_numbers by cmpts test faled for D = ", 2,"  -- ",
           "expected: ", gr.num_vertices(Deg2),
           ", actual: ", nvct.template operator()<2>());

    ENSURE(nvct.template operator()<3>() == gr.num_vertices(Deg3),
           "vertex_numbers by cmpts test faled for D = ", 3,"  -- ",
           "expected: ", gr.num_vertices(Deg3),
           ", actual: ", nvct.template operator()<3>());

    ENSURE(nvct.template operator()<4>() == gr.num_vertices(Deg4),
           "vertex_numbers by cmpts test faled for D = ", 4,"  -- ",
           "expected: ", gr.num_vertices(Deg4),
           ", actual: ", nvct.template operator()<4>());

    auto nvcn = [&]<Degree D>() noexcept -> std::size_t
    {
        std::size_t k {};
        for (const auto& m : cn)
            k += m.num_vertices(D);

        return D == Deg3 ? k/3 :
               D == Deg4 ? k/4 : k;
    };

    ENSURE(nvcn.template operator()<1>() == gr.num_vertices(Deg1),
           "vertex_numbers by chains test faled for D = ", 1,"  -- ",
           "expected: ", gr.num_vertices(Deg1),
           ", actual: ", nvcn.template operator()<1>());

    ENSURE(nvcn.template operator()<2>() == gr.num_vertices(Deg2),
           "vertex_numbers by chains test faled for D = ", 2,"  -- ",
            "expected: ", gr.num_vertices(Deg2),
            ", actual: ", nvcn.template operator()<2>());

    ENSURE(nvcn.template operator()<3>() == gr.num_vertices(Deg3),
           "vertex_numbers by chains test faled for D = ", 3,"  -- ",
           "expected: ", gr.num_vertices(Deg3),
           ", actual: ", nvcn.template operator()<3>());

    ENSURE(nvcn.template operator()<4>() == gr.num_vertices(Deg4),
           "vertex_numbers by chains test faled for D = ", 4,"  -- ",
           "expected: ", gr.num_vertices(Deg4),
           ", actual: ", nvcn.template operator()<4>());
}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
loops() const
{
    for (const auto& m : cn)
        m.ensure(
            !(m.is_cycle() &&
              m.length() < Chain::minCycleLength),
            " L ",
            "Forbidden loop found ","in chain ", m.idw, " printed above"
        );
}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
consistency(const itT it) const
{
//    print_chains( "ChC" );
    for (ChIdG i=0; i<gr.num_chains(); ++i) {
        for (const auto j : End::Ids) {
            for (const auto& s : cn[i].ngs[j]()) {

                const auto cnei = s.w;

                if (cnei >= gr.num_chains()) {
                    gr.print_chains("TEST FAILED ");
                    jot("");
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
                    jot("");
                    abort("At iteration ", it, ": Consistency test ",
                          "failed for cn = ", i, ", end = ", j,
                          " cnei = ", cnei, " ce = ", ce);
                }
            }
        }
    }

    for (std::size_t j=0; j<gr.num_edges(); ++j)
        ENSURE(cn.edge(egl[j]).ind == j,
               "Error by checking indma at ind: ", j);
}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
chain_id(const itT it) const
{
    for (ChIdG i {}; i < gr.num_chains(); ++i)
        cn[i].ensure(
            cn[i].idw == i,
            "should have idw ", "Iteration ", it,
            " check.chain_id faied at i ", i, " != idw ", cn[i].idw
        );
}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
edges(const itT it) const
{
    std::size_t egn {};
    for (const auto& m : cn) {
        egn += m.length();
        for (EgIdA a {}; const auto& o : m.g) {
            ENSURE(o.w == m.idw,
                   "Error 1: at iteration ", it,
                    " check.edges faied at w ", m.idw, " edge ind ", o.ind);
            if (o.indw != a++) {
                m.print("wrong indw ");
                abort("Error 2: at iteration ", it,
                      " check.edges faied at w ", m.idw, ": at a ", a-1,
                      "indw ", o.indw, " is incorrect; ", " edge ind ", o.ind);
            }

            o.check(gr.num_edges(), ct[m.c].num_edges(), m.length(),
                    gr.num_chains(), gr.num_compts());
        }
    }
    ASSERT(egn == gr.num_edges(), "incorrect total number of edges");
}


template<typename G,
         bool beLoud>
void IntegralTests<G, beLoud>::
chain_g(const itT it) const
{
    std::size_t egn {};
    for (std::size_t i {}; i < gr.num_chains(); ++i) {
        egn += cn[i].length();
        for (std::size_t r {}; r < cn[i].length(); ++r) {
            if (cn[i].g[r].ind >= gr.num_edges())
                abort("Error 1: at iteration ", it,
                      " check.chain_g faied at ind ", cn[i].g[r].ind);
            for (ChIdG i1=i; i1<gr.num_chains(); ++i1)
                for (std::size_t r1 {}; r1 < cn[i1].length(); ++r1)
                    if (!(i == i1 && r == r1) &&
                        cn[i].g[r].ind == cn[i1].g[r1].ind)
                        abort("Error 4: at iteration ", it,
                              " check.chain_g faied at ind ", cn[i].g[r].ind);
        }
    }
    ENSURE(egn == gr.num_edges(),
           "Error 5: at iter ", it, " check.chain_g faied at egn ", egn);
}


}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_ANALYZERS_INTEGRAL_TESTS_H
