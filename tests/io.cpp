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
#include "sample_graph.h"
#include "vertex_merger_common.h"

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/structure/io/ply.h"
#include "graffine/structure/analyzers/integral_tests.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>


namespace graffine::tests::io {

namespace elements = structure;
using G = elements::Graph<
          elements::Component<
          elements::Chain<
          elements::Edge<elements::Vertex>>>>;
using IOTest = Test;

using namespace elements::io;

/**
 * Topology of a graph independent of the order of chains and components.
 * \details Chain and component indexes are not preserved by write and read,
 * but the edge indexes and orientations are.
 */
struct Topology {

    /// Chain length and the numbers of chains connected at its ends.
    using ChainShape = std::array<std::size_t, 3>;

    std::size_t numCompts {};
    std::size_t numChains {};
    std::size_t numEdges {};
    std::array<std::size_t, 4> numVertices {};  ///< Per degree 1 to 4.
    std::vector<ChainShape> chains;             ///< Sorted.
    std::vector<Orientation> orientations;      ///< Per edge index.

    explicit Topology(const G& gr)
        : numCompts {gr.num_compts()}
        , numChains {gr.num_chains()}
        , numEdges {gr.num_edges()}
        , numVertices {gr.num_vertices(Deg1), gr.num_vertices(Deg2),
                       gr.num_vertices(Deg3), gr.num_vertices(Deg4)}
    {
        for (ChIdG w {}; w < gr.num_chains(); ++w) {
            const auto& m = gr.chain(w);
            const auto nA = m.ngs[End::A].num();
            const auto nB = m.ngs[End::B].num();
            chains.push_back({m.length(), std::min(nA, nB), std::max(nA, nB)});
        }
        std::ranges::sort(chains);

        for (std::size_t i {}; i < gr.num_edges(); ++i)
            orientations.push_back(gr.edge(i).orientation());
    }

    bool operator==(const Topology&) const = default;
};


/// Writes \p gr to \p file, reads it back and compares the topologies.
void expect_circular_write_read(
    const G& gr,
    const std::filesystem::path& file
)
{
    Ply<G> ply;
    for (const bool asBinary : {false, true}) {

        ply.template write<profuse>(gr, file, asBinary);

        for (const bool preload : {false, true}) {

            auto grNew = ply.template read<profuse>(file, preload);
            const IntegrityCheck integrityCheckNew {grNew};

            if constexpr (profuse)
                grNew.print_components("AFTER");

            EXPECT_TRUE(Topology {grNew} == Topology {gr})
                << (asBinary ? "binary" : "ascii")
                << (preload ? ", preloaded" : "");
        }
    }
}


/// Creation of single-edge and multi-edge components.
TEST_F(IOTest, PlyCircularWriteRead)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests circular write and read of a multicomponent graph using ",
            ".PLY file format"
        );

    // Create initial graph (24 chains over 11 connected components).
    auto gr = create_sample_graph<G>();
    const IntegrityCheck integrityCheck {gr};

    const auto workingDir = std::filesystem::current_path();

    if constexpr (profuse)
        gr.print_components(tagBefore);

    Ply<G> ply;
    ply.template write<profuse>(gr, workingDir/"testPlyCircularWriteRead", false);

    auto grNew = ply.template read<profuse>(workingDir/"testPlyCircularWriteRead", true);
    const IntegrityCheck integrityCheckNew {grNew};

    elements::IntegralTests<G, profuse> integral_test {grNew};
    integral_test();

    if constexpr (profuse)
        grNew.print_components(tagAfter);

    expect_circular_write_read(gr, workingDir/"testPlyCircularWriteRead");
}


/// Circular write and read of chains formed by edges of opposite orientations.
TEST_F(IOTest, PlyCircularWriteReadMixedOrientations)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests circular write and read using .PLY file format of chains ",
            "formed by edges of opposite orientations"
        );

    using vertex_merger::VertexMerger;
    using vertex_merger::VertexMerger1;
    using vertex_merger::VertexMergerCore;

    const auto file = std::filesystem::current_path()/"testPlyMixedOrientations";

    const auto [w0, w1] = id_sequence<ChIdG, 2>();

    {
        print_description_subtest('A', "Linear chain from merger at ends A");

        G gr;
        const IntegrityCheck integrityCheck {gr};
        gr.add_single_chain_component(4);
        gr.add_single_chain_component(2);

        VertexMergerCore {gr}.antiparallel(End::A, w0, w1);  // < < < < > >
        ASSERT_EQ(gr.num_chains(), 1);

        expect_circular_write_read(gr, file);
    }
    {
        print_description_subtest('B', "Linear chain from merger at ends B");

        G gr;
        const IntegrityCheck integrityCheck {gr};
        gr.add_single_chain_component(2);
        gr.add_single_chain_component(4);

        VertexMergerCore {gr}.antiparallel(End::B, w0, w1);  // > > < < < <
        ASSERT_EQ(gr.num_chains(), 1);

        expect_circular_write_read(gr, file);
    }
    {
        print_description_subtest('C', "Cycle closed from a mixed chain");

        G gr;
        const IntegrityCheck integrityCheck {gr};
        gr.add_single_chain_component(4);
        gr.add_single_chain_component(2);

        VertexMergerCore {gr}.antiparallel(End::A, w0, w1);
        VertexMerger<Deg1, Deg1, G> {gr}(ESlot{w0, End::A}, ESlot{w0, End::B});
        ASSERT_TRUE(gr.chain(w0).is_unconnected_cycle());

        expect_circular_write_read(gr, file);
    }
    {
        print_description_subtest('D', "Mixed chains at a 3-way junction");

        G gr;
        const IntegrityCheck integrityCheck {gr};
        gr.add_single_chain_component(4);
        gr.add_single_chain_component(2);

        VertexMergerCore {gr}.antiparallel(End::A, w0, w1);
        gr.add_single_chain_component(3);  // becomes w1
        VertexMerger1<Deg2, G> {gr}(ESlot{w1, End::A}, BSlot{w0, 3});
        ASSERT_EQ(gr.num_chains(), 3);
        ASSERT_EQ(gr.num_vertices(Deg3), 1);

        expect_circular_write_read(gr, file);
    }
}


/**
 * Rewrites a graph PLY file storing edge weights with type W instead of real.
 * \tparam W Floating-point type of edge weights in the destination file.
 * \param src Source file written by Ply<G> in the native precision.
 * \param dst Destination file.
 * \param asBinary Whether the destination file is binary.
 */
template<typename W>
void rewrite_with_weight_type(
    const std::filesystem::path& src,
    const std::filesystem::path& dst,
    const bool asBinary
)
{
    using Type = tinyply::Type;

    std::ifstream ifs {src, std::ios::binary};
    tinyply::FileIn reader;
    reader.parse_header(ifs);
    const auto vIds  = reader.request_properties_from_element("vertex", {"id"});
    const auto vFxd  = reader.request_properties_from_element("vertex", {"isFixed"});
    const auto eVidA = reader.request_properties_from_element(
                           "edge", {"vertex_index_at_outer_end_A"});
    const auto eVidB = reader.request_properties_from_element(
                           "edge", {"vertex_index_at_outer_end_B"});
    const auto eOrnt = reader.request_properties_from_element("edge", {"orientation"});
    const auto eW    = reader.request_properties_from_element("edge", {"weight"});
    reader.read(ifs);

    ASSERT_EQ(eW->buffer.size_bytes(), eW->count * sizeof(real));
    std::vector<W> weights(eW->count);
    for (std::size_t i {}; i < eW->count; ++i) {
        real w;
        std::memcpy(&w, eW->buffer.get() + i * sizeof(real), sizeof(real));
        weights[i] = static_cast<W>(w);
    }

    tinyply::FileOut writer;
    auto add = [&writer](const std::string& el, const std::string& prop,
                         const tinyply::FileIn::Data& d) {
        writer.add_properties_to_element(el, {prop}, d.t, d.count,
                                         d.buffer.get(), Type::INVALID, 0);
    };
    add("vertex", "id", *vIds);
    add("vertex", "isFixed", *vFxd);
    add("edge", "vertex_index_at_outer_end_A", *eVidA);
    add("edge", "vertex_index_at_outer_end_B", *eVidB);
    add("edge", "orientation", *eOrnt);
    writer.add_properties_to_element(
        "edge", {"weight"},
        std::is_same_v<W, float> ? Type::FLOAT32 : Type::FLOAT64,
        weights.size(), reinterpret_cast<const uint8_t*>(weights.data()),
        Type::INVALID, 0);

    std::ofstream ofs {dst, std::ios::binary};
    writer.write(ofs, asBinary);
}


/// Reading of a file storing edge weights in the precision other than real.
TEST_F(IOTest, PlyReadWeightsOfOtherPrecision)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests reading of a .PLY file having edge weights stored ",
            "in the floating-point precision other than that of 'real'"
        );

    using Other = std::conditional_t<std::is_same_v<real, float>, double, float>;
    const std::string otherName {std::is_same_v<Other, float> ? "float" : "double"};

    for (const bool asBinary : {false, true}) {

        auto gr = create_sample_graph<G>();

        // Distinct weights exactly representable in both float and double.
        for (std::size_t i {}; i < gr.num_edges(); ++i)
            gr.edge(i).weight() = static_cast<real>(0.25 * (i + 1));

        const auto workingDir = std::filesystem::current_path();
        const auto fileNative = workingDir/"testPlyNativePrecision";
        const auto fileOther = workingDir/"testPlyOtherPrecision";

        Ply<G> ply;
        ply.template write<profuse>(gr, fileNative, asBinary);
        rewrite_with_weight_type<Other>(fileNative, fileOther, asBinary);

        // Make sure the file really stores weights in the other precision.
        std::ifstream hs {fileOther, std::ios::binary};
        std::string header;
        for (std::string line; std::getline(hs, line) && line != "end_header"; )
            header += line + '\n';
        ASSERT_NE(header.find("property " + otherName + " weight"), std::string::npos)
            << header;

        for (const bool preload : {false, true}) {
            auto grNew = ply.template read<profuse>(fileOther, preload);
            const IntegrityCheck integrityCheckNew {grNew};

            ASSERT_EQ(grNew.num_edges(), gr.num_edges());
            for (std::size_t i {}; i < gr.num_edges(); ++i)
                EXPECT_EQ(grNew.edge(i).weight(), gr.edge(i).weight())
                    << "edge " << i << (asBinary ? " binary" : " ascii");
        }
    }
}


/// Reading of a file lacking a property required by the importer.
TEST_F(IOTest, PlyReadMissingProperty)
{
    ++testCount;

    if constexpr (verboseT)
        print_description(
            "Tests that reading of a .PLY file lacking a required property ",
            "returns an empty graph"
        );

    using Type = tinyply::Type;

    const std::vector<uint32_t> vIds {0, 1};
    const std::vector<int8_t> vFxd {0, 0};
    const std::vector<uint32_t> eVidA {0};
    const std::vector<uint32_t> eVidB {1};
    const std::vector<int8_t> eOrnt {1};

    // Same as written by Ply<G>, but without edge weights.
    tinyply::FileOut writer;
    auto add = [&writer](const std::string& el, const std::string& prop,
                         const Type t, const auto& v) {
        writer.add_properties_to_element(
            el, {prop}, t, v.size(),
            reinterpret_cast<const uint8_t*>(v.data()), Type::INVALID, 0);
    };
    add("vertex", "id", Type::UINT32, vIds);
    add("vertex", "isFixed", Type::INT8, vFxd);
    add("edge", "vertex_index_at_outer_end_A", Type::UINT32, eVidA);
    add("edge", "vertex_index_at_outer_end_B", Type::UINT32, eVidB);
    add("edge", "orientation", Type::INT8, eOrnt);

    const auto file = std::filesystem::current_path()/"testPlyMissingProperty";
    {
        std::ofstream ofs {file, std::ios::binary};
        writer.write(ofs, false);
    }

    Ply<G> ply;
    for (const bool preload : {false, true}) {
        const auto grNew = ply.template read<profuse>(file, preload);
        EXPECT_EQ(grNew.num_vertices(), 0);
        EXPECT_EQ(grNew.num_edges(), 0);
    }
}

}  // namespace graffine::tests::io