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

#ifndef GRAFFINE_TESTS_PULLING_HOOKS_H
#define GRAFFINE_TESTS_PULLING_HOOKS_H

#include "pulling.h"
#include "graffine/transforms/property_processor.h"

#include <functional>
#include <memory>
#include <tuple>
#include <vector>

namespace graffine::tests::pulling::hooks {

using Vertex = G::Vertex;

/// Chain lengths and numbers of neighbours at both ends, sorted.
using Signature = std::vector<std::tuple<std::size_t, std::size_t, std::size_t>>;

/// Topology of component \p c, independent of the chain numbering.
inline auto signature(const G& gr, const CmpId c) -> Signature
{
    Signature s;
    for (const auto w : gr.compt(c).chids()) {
        const auto& m = gr.chain(w);
        s.emplace_back(m.length(), m.ngs[eA].num(), m.ngs[eB].num());
    }
    std::ranges::sort(s);

    return s;
}

/// What the pulling hooks see.
struct Snapshot {
    Signature sig;          ///< Topology of the path component.
    bool isContiguous {};   ///< Path::is_contiguous().
    bool isCmpValid {};     ///< The component passes its integrity check.
    std::size_t length {};  ///< Number of path edges.
    std::size_t numVertices {};  ///< Number of path vertices.
    Degree deg0 {};         ///< Degree of the driver vertex.
    Degree deg1 {};         ///< Degree of the path vertex next to the driver.
    bool endsAtSource {};   ///< The last path vertex is the source vertex.
    bool isSourceBulk {};   ///< The source is a degree 2 vertex.
    bool isSourceValid {};  ///< The source is inside its chain, or at an end.
};

inline auto snapshot(const G& gr, const Path& path) -> Snapshot
{
    const auto vv = path.vertices();

    return {signature(gr, path.cmp->ind),
            path.is_contiguous(),
            !path.cmp->template check<false>(),
            path.length(),
            vv.size(),
            vv.size() > 0 ? vv[0]->deg() : Degree {},
            vv.size() > 1 ? vv[1]->deg() : Degree {},
            !vv.empty() && (path.src().is_bulk() || End::is_valid(path.src().e)) &&
                *vv.back() == path.source_vertex(),
            path.src().is_bulk(),
            path.src().is_bulk()
                ? Path::is_bulk_position(gr.chain(path.src().w), path.src().pos)
                : End::is_valid(path.src().e)};
}

/**
 * Property processor recording what the pulling hooks see.
 * \details The hooks of the other families do nothing.
 */
struct Recorder {

    struct {
        void on_start(const ESlot&, const ESlot&) {}
        void on_start(const BSlot&, const ESlot&) {}
        void on_start(const BSlot&, const BSlot&) {}
        void on_end(Vertex&) {}
    } vertexMerger;

    struct {
        void on_start(ChIdG) {}
        void on_start(const ESlot&) {}
        void on_start(const BSlot&) {}
        void on_start(const ESlot&, const ESlot&) {}
        void on_end(const std::array<Vertex*, 2>&) {}
    } vertexSplit;

    struct {
        template<typename S> void on_start(const S&) {}
        void on_start(ChIdG, EgIdA, bool) {}
        template<bool isNewChain> void on_end(Edge&) {}
    } edgeCreation;

    struct {
        template<bool isChainDeleted> void on_start(Edge&) {}
        template<bool isChainDeleted> void on_end(Vertex&) {}
    } edgeDeletion;

    struct {
        void on_start() {}
        void on_end() {}
    } componentCreation, componentDeletion;

    struct {
        const G* gr {};
        int starts {};
        std::vector<Snapshot> ends;

        void on_start(Path&) { ++starts; }
        void on_end(Path& path) { ends.push_back(snapshot(*gr, path)); }
    } pulling;

    explicit Recorder(const G& gr) { pulling.gr = &gr; }
};

static_assert(trs::pulling::Processor<Recorder, G>);


/// Graph and path for a pull; the graph hosts the path, so is not movable.
struct Pullee {
    G gr;
    Path path;
};

/// Makes the graph and the path of a pull in \p s.
using Maker = std::function<void(Pullee& s)>;

/**
 * Linear chains: a leaf chain w1 and a junction J1, w3 between J1 and J2,
 * and w4 from J2 to its free end B, the source.
 * \details
 *     w0 (2) --J1-- w3 (3) --J2-- w4 (3)
 *     w1 (2) --'          w2 (2) --'
 */
inline void make_two_junctions(G& gr)
{
    gr.add_single_chain_component(8);  // w0
    gr.add_single_chain_component(2);  // w1
    gr.add_single_chain_component(2);  // w2

    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{ChIdG{1}, eB}, BSlot{ChIdG{0}, 2});  // w0 (2), w3 (6)
    merge12(ESlot{ChIdG{2}, eB}, BSlot{ChIdG{3}, 3});  // w3 (3), w4 (3)
}

/**
 * Linear chain w1 (2), free at end A, joined at its end B to both ends of
 * a cycle w0 of length \p len - 2. Edges 0 and 1 are w1, and edges 2, 3...
 * are w0 from its end A.
 */
inline void make_lollipop(G& gr, const std::size_t len)
{
    gr.add_single_chain_component(len);
    VertexMerger<1, 2, G> merge12 {gr};
    merge12(ESlot{ChIdG{0}, eB}, BSlot{ChIdG{0}, 2});
}

inline constexpr ChIdG w1 {1};
inline constexpr ChIdG w3 {3};
inline constexpr ChIdG w4 {4};


template<Degree D>
using Pull = trs::pulling::On<D, G, Recorder>;

/**
 * Pulls \p steps, and checks the hooks: on_start once, and on_end once per
 * step, each time seeing the topology that a pull of that many steps leaves,
 * over a contiguous path of the same number of vertices.
 */
template<Degree D>
void check_hooks(const Maker& make, const int steps)
{
    // Topology after each number of steps, from separate pulls.
    std::vector<Signature> after;
    for (int k {1}; k <= steps; ++k) {
        Pullee s;
        make(s);
        Pull<D> {s.gr, std::make_shared<Recorder>(s.gr)}(s.path, k);
        after.push_back(signature(s.gr, s.path.cmp->ind));
    }

    Pullee s;
    make(s);
    ASSERT_TRUE(s.path.is_contiguous());
    const auto length = s.path.length();
    const auto rec = std::make_shared<Recorder>(s.gr);
    Pull<D> pull {s.gr, rec};
    const auto before = signature(s.gr, s.path.cmp->ind);

    pull(s.path, steps);

    const auto& h = rec->pulling;
    EXPECT_EQ(h.starts, 1);
    ASSERT_EQ(h.ends.size(), static_cast<std::size_t>(steps));
    for (std::size_t k {}; k < h.ends.size(); ++k) {
        const auto& e = h.ends[k];
        EXPECT_EQ(e.sig, after[k]) << "on_end of step " << k + 1;
        EXPECT_NE(e.sig, before) << "on_end of step " << k + 1;
        EXPECT_TRUE(e.isContiguous) << "on_end of step " << k + 1;
        EXPECT_TRUE(e.isCmpValid) << "on_end of step " << k + 1;
        EXPECT_EQ(e.length, length) << "on_end of step " << k + 1;
        EXPECT_EQ(e.numVertices, length + 1) << "on_end of step " << k + 1;
        EXPECT_EQ(e.deg0, Deg1) << "on_end of step " << k + 1;
        EXPECT_TRUE(e.isSourceValid) << "on_end of step " << k + 1;
        EXPECT_TRUE(e.endsAtSource) << "on_end of step " << k + 1;
    }
    EXPECT_TRUE(s.path.is_contiguous());
    EXPECT_EQ(s.path.length(), length);
}

}  // namespace graffine::tests::pulling::hooks

#endif  // GRAFFINE_TESTS_PULLING_HOOKS_H
