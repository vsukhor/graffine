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

#include <cmath>    // isfinite
#include <cstddef>  // byte
#include <cstdint>  // uint8_t, int8_t, uint16_t, int16_t, etc
#include <cstring>  // memcpy
#include <filesystem>
#include <fstream>
#include <istream>
#include <list>
#include <memory>
#include <new>      // bad_alloc
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>  // is_same_v, conditional_t
#include <unordered_map>
#include <vector>

/// Format-specific input/output to files
namespace graffine::structure::io {

namespace ply = tinyply;

/**
 * Reads and writes graphs as PLY files, in text or binary format.
 * \details A file holds two elements:
 * - \c vertex, with properties \c id and \c isFixed;
 * - \c edge, with properties \c vertex_index_at_outer_end_A and
 *   \c vertex_index_at_outer_end_B (vertex ids), \c orientation and
 *   \c weight.
 *
 * The vertex ids are written renumbered to 0, 1, ..., and reading
 * reassembles the chains and components with GraphFactory.
 * \tparam G Graph type.
 */
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

    /**
     * Copies property data read from file into a vector of T.
     * If the file stores the property with a type different from that of
     * T elements (e.g. edge weights written as float but read with real set
     * to double), the values are converted element by element.
     * \tparam T Arithmetic type or a fixed-size array of it.
     * \param dd Data buffer filled by the ply reader.
     */
    template<typename T>
    auto convert_from_data(const Reader::Data& dd) const -> std::vector<T>
    {
        using E = std::remove_all_extents_t<T>;
        static_assert(std::is_arithmetic_v<E>);
        constexpr auto n = sizeof(T) / sizeof(E);  // scalars per item
        constexpr auto ownType = static_cast<Type>(
            ply::impl::Index<E, ply::impl::typetup>::value);

        std::vector<T> res(dd.count);

        if (dd.t == ownType) {
            if (dd.buffer.size_bytes() != res.size() * sizeof(T))
                throw io_error(
                    "ply: unexpected size of property data buffer");
            std::memcpy(res.data(), dd.buffer.get(), dd.buffer.size_bytes());
            return res;
        }

        auto cast_from = [&]<typename U>() {
            if (dd.buffer.size_bytes() != dd.count * n * sizeof(U))
                throw io_error(
                    "ply: unexpected size of property data buffer");
            const auto* src = dd.buffer.get();
            auto* dst = reinterpret_cast<std::byte*>(res.data());
            for (std::size_t k {}; k < dd.count * n; ++k) {
                U u;
                std::memcpy(&u, src + k * sizeof(U), sizeof(U));
                const auto e = static_cast<E>(u);
                std::memcpy(dst + k * sizeof(E), &e, sizeof(E));
            }
        };

        switch (dd.t) {
            case Type::INT8:    cast_from.template operator()<ply::type<Type::INT8>>();    break;
            case Type::UINT8:   cast_from.template operator()<ply::type<Type::UINT8>>();   break;
            case Type::INT16:   cast_from.template operator()<ply::type<Type::INT16>>();   break;
            case Type::UINT16:  cast_from.template operator()<ply::type<Type::UINT16>>();  break;
            case Type::INT32:   cast_from.template operator()<ply::type<Type::INT32>>();   break;
            case Type::UINT32:  cast_from.template operator()<ply::type<Type::UINT32>>();  break;
            case Type::FLOAT32: cast_from.template operator()<ply::type<Type::FLOAT32>>(); break;
            case Type::FLOAT64: cast_from.template operator()<ply::type<Type::FLOAT64>>(); break;
            case Type::INVALID: throw io_error("ply: invalid property type");
        }

        return res;
    };

    Ply() {}
    explicit Ply(const Graph* gr) {}

    /**
     * Writes graph \p gr to \p file.
     * \throws io_error If the file cannot be written.
     */
    template<bool beLoud = false>
    void write(const G& gr,
               const std::filesystem::path& file,
               const bool asBinary) const;

    /**
     * Reads a graph from \p file.
     * \details The file content is validated before the graph is built.
     * \throws io_error If the file cannot be read, or does not describe
     * a valid graph.
     */
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

    /**
     * Renumbers the vertex ids \p vhs to 0, 1, ..., \p vhs.size() - 1, and
     * the vertex ids at the edge ends \p vIdA and \p vIdB accordingly.
     */
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

    std::ofstream ofs(file, asBinary ? std::ios::out | std::ios::binary
                                     : std::ios::out);
    if (ofs.fail())
        throw io_error("ply: failed to open " + file.string());

    if constexpr (beLoud)
        jot("writing to ", file.string());

    try {
        writer.write(ofs, asBinary);
        ofs.flush();
    }
    catch (const std::bad_alloc&) {
        throw;
    }
    catch (const std::exception& e) {
        throw io_error("ply: failed to write " + file.string() + ": " + e.what());
    }
    if (!ofs)
        throw io_error("ply: failed to write " + file.string());
}


template<typename G>
auto Ply<G>::
register_for_import(
    const std::string& elementName,
    const std::vector<std::string>& properties,
    Reader& reader
) -> std::shared_ptr<Reader::Data>
{
    // The reader throws if the element or any of the properties are absent.
    try {
        return reader.request_properties_from_element(elementName, properties);
    }
    catch (const std::bad_alloc&) {
        throw;
    }
    catch (const std::exception& e) {
        std::string names;
        for (const auto& p : properties)
            names += (names.empty() ? "" : ", ") + p;
        throw io_error("ply: missing element '" + elementName +
                       "' or its properties {" + names + "}: " + e.what());
    }
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
            throw io_error("ply: failed to open " + file.string());

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

        // Validate the file content: the graph factory relies on it.

        auto invalid = [&file](const auto&... msg) -> io_error
        {
            std::ostringstream os;
            os << "ply: invalid graph in " << file.string() << ": ";
            (os << ... << msg);
            return io_error(os.str());
        };

        const auto numVertices = globalVertexIds.size();
        const auto numEdges = globalEdgeVids.size();

        if (globalVertexFs.size() != numVertices)
            throw invalid("vertex properties differ in size");
        if (globalEdgeOrients.size() != numEdges ||
            globalEdgeWeights.size() != numEdges)
            throw invalid("edge properties differ in size");

        std::unordered_map<TypeVId, std::size_t> vertexIndex;  // by vertex id
        vertexIndex.reserve(numVertices);
        for (std::size_t i {}; i < numVertices; ++i)
            if (!vertexIndex.emplace(globalVertexIds[i], i).second)
                throw invalid("duplicate vertex id ", globalVertexIds[i]);

        std::vector<std::size_t> degrees(numVertices);
        for (std::size_t i {}; i < numEdges; ++i) {
            for (const auto e : End::Ids) {
                const auto it = vertexIndex.find(globalEdgeVids[i][e]);
                if (it == vertexIndex.end())
                    throw invalid("edge ", i, " refers to missing vertex ",
                                  globalEdgeVids[i][e]);
                ++degrees[it->second];
            }
            if (globalEdgeVids[i][End::A] == globalEdgeVids[i][End::B])
                throw invalid("edge ", i, " connects vertex ",
                              globalEdgeVids[i][End::A], " to itself");
            if (globalEdgeOrients[i] != Orientation::Forwards &&
                globalEdgeOrients[i] != Orientation::Backwards)
                throw invalid("edge ", i, " has orientation ",
                              static_cast<int>(globalEdgeOrients[i]));
            const auto w = static_cast<Edge::weight_t>(globalEdgeWeights[i]);
            if (!std::isfinite(w) || !(w <= Edge::maxWeight))
                throw invalid("edge ", i, " has weight ", w);
        }
        for (std::size_t i {}; i < numVertices; ++i)
            if (degrees[i] < Deg1 || degrees[i] > maxDegree)
                throw invalid("vertex ", globalVertexIds[i], " has degree ",
                              degrees[i]);

        // Build the graph.

        std::vector<std::unique_ptr<Vertex>> globalVertices;
        globalVertices.reserve(numVertices);
        for (std::size_t i {}; i < numVertices; ++i)
            globalVertices.push_back(std::make_unique<Vertex>(globalVertexIds[i],
                                                              globalVertexFs[i]));

        std::vector<Edge> globalEdges;
        globalEdges.reserve(numEdges);
        for (std::size_t i {}; i < numEdges; ++i) {
            globalEdges.emplace_back(
                i,
                static_cast<Orientation>(globalEdgeOrients[i]),
                static_cast<Edge::weight_t>(globalEdgeWeights[i])
            );
            for (const auto& e : End::Ids) {
                const auto j = vertexIndex.at(globalEdgeVids[i][e]);
                globalEdges.back().connect_to_outer(e, *globalVertices[j]);
            }
        }

        modifiers::GraphFactory<Graph> factory;

        Graph gr = factory.template create<beLoud>(std::move(globalVertices),
                                                   std::move(globalEdges));
        return gr;
    }
    catch (const io_error&) {
        throw;
    }
    catch (const std::bad_alloc&) {
        throw;
    }
    catch (const std::exception& e) {
        // Parsing errors, and graph checks failing on the file content.
        throw io_error("ply: cannot read " + file.string() + ": " + e.what());
    }
}

}  // namespace graffine::structure::io

#endif  // GRAFFINE_STRUCTURE_IO_PLY_H
