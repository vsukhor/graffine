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
 * \file ply.h
 * \brief Machinery necessary for graph read/write to file using PLY format.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_IO_PLY_H
#define GRAFFINE_STRUCTURE_IO_PLY_H


#include "graffine/definitions.h"
#include "graffine/structure/modifiers/graph_factory.h"
#include "graffine/structure/descriptors/edge_in_vertex.h"
#include "tinyply.h"

#include "graffine/utils/io.h"

#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <filesystem>
#include <istream>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <type_traits>  // is_same_v, conditional_t

/// Format-specific input/output to files
namespace graffine::structure::io {

namespace ply = tinyply;

template<typename G>
struct Ply
{
    using Graph = G;
    using Vertex = Graph::Vertex;
    using Edge = Graph::Edge;
    using End = descriptors::End;
    using Reader = ply::FileIn;
    using Writer = ply::FileOut;
    using Type = ply::Type;

    using TypeVId  = ply::type<Type::UINT32>;
    using TypeBool = ply::type<Type::INT8>;
    using TypeOrnt = ply::type<Type::INT8>;
    static_assert(std::is_floating_point_v<typename Edge::weight_t>);
    using TypeWeight =
        std::conditional_t<std::is_same_v<typename Edge::weight_t, float>,
                           typename ply::type<Type::FLOAT32>,
                           typename ply::type<Type::FLOAT64>>;

    static constexpr const char* version {"11"};

    template<typename T>
    auto convert_from_data(const Reader::Data& dd) const -> std::vector<T>
    {
        std::vector<T> res(dd.count);
        std::memcpy(res.data(), dd.buffer.get(), dd.buffer.size_bytes());

        return res;
    };

    Ply() {}
    explicit Ply(const Graph* gr) {}

    template<bool beLoud = false>
    void write(const G& gr,
               const std::filesystem::path& file,
               const bool asBinary) const;

    template<bool beLoud = false>
    auto read(const std::filesystem::path& file,
              const bool preloadIntoMemory) -> Graph;

    void print_vector(const auto& con,
                      std::string_view s) const
    {
        jot<false>(s, " ");
        for (const auto& r : con)
            jot<false>(r, " ");
        jot("");
    };


protected:

    /// Registers an element and some of its properties for reading from file.
    auto register_for_import(
        const std::string& elementName,
        const std::vector<std::string>& properties,
        Reader& reader
    ) -> std::shared_ptr<Reader::Data>;

    void restore_consecutive_vids(
        std::vector<TypeVId>& vhs,   // ref
        std::vector<TypeVId>& vIdA,  // ref
        std::vector<TypeVId>& vIdB   // ref
    ) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<typename G>
template<bool beLoud>
void Ply<G>::
write(
    const G& gr,
    const std::filesystem::path& file,
    const bool asBinary
) const
{
    // Set vertex properties:

    const auto nVerts = gr.num_vertices();
    std::vector<TypeVId> vhs(nVerts);
    std::vector<TypeBool> isFixed(nVerts);

    for (std::size_t i {}; const auto& cmp : gr.compts())
        for (const auto& v : cmp.vv.get()) {
            vhs[i] = static_cast<uint32_t>(v->id());
            isFixed[i] = v->is_fixed();
            ++i;
        }

    // Set edge properties:

    const auto nEdges = gr.num_edges();
    std::vector<TypeVId> vIdA(nEdges);
    std::vector<TypeVId> vIdB(nEdges);
    std::vector<TypeOrnt> ornt(nEdges);
    std::vector<TypeWeight> egW(nEdges);

    for(std::size_t i {}; i<nEdges; ++i) {
        const auto& eg = gr.edge(i);
        vIdA[eg.ind()] = static_cast<TypeVId>(eg.vertex_at_outer(End::A)->id());
        vIdB[eg.ind()] = static_cast<TypeVId>(eg.vertex_at_outer(End::B)->id());
        ornt[eg.ind()] = static_cast<TypeOrnt>(eg.orientation());
        egW[eg.ind()] = static_cast<TypeWeight>(eg.weight());
    }

    // Restore consecutive numeration of vertex ids:

    restore_consecutive_vids(vhs, vIdA, vIdB);

    Writer writer;

    // Set data buffers of ply writer:

    writer.add_properties_to_element("vertex",
                                     {"id"},
                                     Type::UINT32,
                                     vhs.size(),
                                     reinterpret_cast<const uint8_t*>(vhs.data()),
                                     Type::INVALID,
                                     0);
    writer.add_properties_to_element("vertex",
                                     {"isFixed"},
                                     Type::INT8,
                                     isFixed.size(),
                                     reinterpret_cast<const uint8_t*>(isFixed.data()),
                                     Type::INVALID,
                                     0);
    writer.add_properties_to_element("edge",
                                     {"vertex_index_at_outer_end_A"},
                                     Type::UINT32,
                                     vIdA.size(),
                                     reinterpret_cast<const uint8_t*>(vIdA.data()),
                                     Type::INVALID,
                                     0);
    writer.add_properties_to_element("edge",
                                     {"vertex_index_at_outer_end_B"},
                                     Type::UINT32,
                                     vIdB.size(),
                                     reinterpret_cast<const uint8_t*>(vIdB.data()),
                                     Type::INVALID,
                                     0);
    writer.add_properties_to_element("edge",
                                     {"orientation"},
                                     Type::INT8,
                                     ornt.size(),
                                     reinterpret_cast<const uint8_t*>(ornt.data()),
                                     Type::INVALID,
                                     0);
    writer.add_properties_to_element("edge",
                                     {"weight"},
                                     std::is_same_v<TypeWeight, float>
                                        ? Type::FLOAT32
                                        : Type::FLOAT64,
                                     egW.size(),
                                     reinterpret_cast<const uint8_t*>(egW.data()),
                                     Type::INVALID,
                                     0);

    writer.add_comment("exporter version "s + version);

    // Write out to file:

    writer.template write<beLoud>(file, asBinary, msgr->so, msgr->sl);
}


template<typename G>
auto Ply<G>::
register_for_import(
    const std::string& elementName,
    const std::vector<std::string>& properties,
    Reader& reader
) -> std::shared_ptr<Reader::Data>
{
    std::shared_ptr<Reader::Data> dp;

    try {
        dp = reader.request_properties_from_element(elementName, properties);
    }
    catch (const std::exception& e) {
        jot("ply exception: ", e.what());
    }

    return dp;
}


template<typename G>
void Ply<G>::
restore_consecutive_vids(
    std::vector<TypeVId>& vhs,   // ref
    std::vector<TypeVId>& vIdA,  // ref
    std::vector<TypeVId>& vIdB   // ref
) const
{
    std::set<TypeVId> availableIds;
    for (TypeVId i {}; i < vhs.size(); ++i)
        if (std::find(vhs.begin(), vhs.end(), i) == vhs.end())
            availableIds.insert(i);

#if 0
    jot("Restoring consecutive indexing.");
    jot("vertex number: ", vhs.size());
    jot("Initial values:");
    print_vector(availableIds, "avabl");
    print_vector(vhs, "vhs");
    print_vector(vIdA, "vidA");
    print_vector(vIdB, "vidB");
#endif

    auto replace = [](const auto f, const auto t, auto& vec)
    {
        do {
            auto it = std::find(vec.begin(), vec.end(), f);
            const auto isFound = it != vec.end();
            if (isFound)
                *it = t;
            else break;
        } while (true);
    };

    while (availableIds.size()) {

        const auto ael = availableIds.begin();
        const auto idNew = *ael;
        availableIds.erase(ael);
        const auto mel = std::max_element(vhs.begin(), vhs.end());
        const auto idOld = *mel;
        const auto posMel = std::distance(vhs.begin(), mel);
        vhs[posMel] = idNew;
        replace(idOld, idNew, vIdA);
        replace(idOld, idNew, vIdB);
    }

#if 0
    jot("Remapped values:");
    print_vector(vhs, "vhs");
    print_vector(vIdA, "vidA");
    print_vector(vIdB, "vidB");
#endif
}


template<typename G>
template<bool beLoud>
auto Ply<G>::
read(
    const std::filesystem::path& file,
    const bool preloadIntoMemory
) -> Graph
{
    std::unique_ptr<std::istream> ifs;

    try {
        if (preloadIntoMemory) {

            std::optional<std::vector<uint8_t>> byte_buffer =
                utils::io::read_file_binary<beLoud>(file);
            if (byte_buffer)
                ifs.reset(new utils::io::BufferedStream(std::move(*byte_buffer)));
        }
        else
            ifs.reset(new std::ifstream(file, std::ios::binary));

        if (!ifs || ifs->fail())
            throw std::runtime_error("file_stream failed to open " + file.string());

        ply::FileIn reader;
        reader.parse_header(*ifs);
        if constexpr (beLoud)
            reader.report_structure();

        const auto vertexIds   = register_for_import("vertex", {"id"}, reader);
        const auto vertexFixed = register_for_import("vertex", {"isFixed"}, reader);
        const auto edgeVids    = register_for_import("edge",
                                                     {"vertex_index_at_outer_end_A",
                                                      "vertex_index_at_outer_end_B"},
                                                      reader);
        const auto edgeOrients = register_for_import("edge", {"orientation"}, reader);
        const auto edgeWeights = register_for_import("edge", {"weight"}, reader);

        reader.read(*ifs);  // raw data

        if (beLoud) {
            if (vertexIds) jot("Read ", vertexIds->count, " vertices");
            if (edgeVids)  jot("Read ", edgeVids->count,  " edges");
        }

        const auto globalVertexIds = convert_from_data<TypeVId>(*vertexIds);
        const auto globalVertexFs = convert_from_data<TypeBool>(*vertexFixed);
        const auto globalEdgeVids = convert_from_data<TypeVId[2]>(*edgeVids);
        const auto globalEdgeOrients = convert_from_data<TypeOrnt>(*edgeOrients);
        const auto globalEdgeWeights = convert_from_data<TypeWeight>(*edgeWeights);

        std::vector<TypeVId>globalEdgeVidsB;
        for (const auto& v : globalEdgeVids)
            globalEdgeVidsB.push_back(v[1]);

//        print_vector(globalEdgeVidsB, "globalEdgeVidsB");

        ASSERT(globalVertexIds.size() == globalVertexFs.size(),
               "difference in sizes of vertex parameter vectors");

        const auto numVertices = globalVertexIds.size();
        std::vector<std::unique_ptr<Vertex>> globalVertices;
        globalVertices.reserve(numVertices);
        for (std::size_t i {}; i < numVertices; ++i)
            globalVertices.push_back(std::make_unique<Vertex>(globalVertexIds[i],
                                                              globalVertexFs[i]));

//        for (const auto& v : globalVertices)
//            v->print("b_0");

        auto find_verex_by_id = [&globalVertices](const VIdH vid) -> std::size_t
        {
            for (std::size_t i {}; i < globalVertices.size(); ++i)
                if (globalVertices[i]->id == vid)
                    return i;

            return utils::undefined<size_t>;
        };

        ASSERT(globalEdgeVids.size() == globalEdgeOrients.size(),
               "difference in sizes of edge parameter vectors");

        const auto numEdges = globalEdgeVids.size();
        std::vector<Edge> globalEdges;
        globalEdges.reserve(numEdges);
        for (std::size_t i {}; i < numEdges; ++i) {
            globalEdges.emplace_back(
                i,
                static_cast<Orientation>(globalEdgeOrients[i]),
                static_cast<Edge::weight_t>(globalEdgeWeights[i])
            );
            for (const auto& e : End::Ids) {
                const auto j = find_verex_by_id(globalEdgeVids[i][e]);
                if (!utils::is_defined(j))
                    throw std::runtime_error(
                        "Cannot connect edge: vertex index not found: "s +
                        std::to_string(globalEdgeVids[i][e])
                    );
                globalEdges.back().connect_to_outer(e, *globalVertices[j]);
            }
        }

//        for (const auto& v : globalVertices)
//            v->print("b_1");
//        for (const auto& eg : globalEdges)
//            eg.print("r_1");

        modifiers::GraphFactory<Graph> factory;

        Graph gr = factory.template create<beLoud>(std::move(globalVertices),
                                                   std::move(globalEdges));
        return gr;
    }
    catch (const std::exception& e) {
        jot("Caught exception while reading ply: ", e.what());
    }

    return {};
}

}  // namespace graffine::structure::io

#endif  // GRAFFINE_STRUCTURE_IO_PLY_H
