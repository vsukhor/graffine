#include "graph_type.h"

#include "graffine/config_cmake.h"
#include "graffine/transforms/vertex_merger/core.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>

int main()
{
    graffine::msgr = new graffine::Msgr {&std::cout, nullptr, 6};

    std::cout << graffine::projectName << ' ' << graffine::projectVer << '\n';

    constexpr std::array len {4UL, 2UL};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    // PLY round trip, using the tinyply implementation from ply_io.cpp.
    const auto file = std::filesystem::temp_directory_path()
                    / "graffine_package_test.ply";
    const auto grNew = write_and_read(gr, file);
    std::filesystem::remove(file);

    std::cout << "PLY round trip: components " << gr.num_compts()
              << " -> " << grNew.num_compts()
              << ", chains " << gr.num_chains() << " -> " << grNew.num_chains()
              << ", edges " << gr.num_edges() << " -> " << grNew.num_edges()
              << '\n';

    const bool okIO = grNew.num_compts() == gr.num_compts() &&
                      grNew.num_chains() == gr.num_chains() &&
                      grNew.num_edges() == gr.num_edges() &&
                      grNew.num_vertices() == gr.num_vertices();

    // Topological transformation.
    graffine::transforms::vertex_merger::Core<G> core {gr, nullptr, "vm_core"};
    core.antiparallel(G::End::A, graffine::ChIdG{0}, graffine::ChIdG{1});

    const bool okMerge = gr.num_compts() == 1 &&
                         gr.num_chains() == 1 &&
                         gr.num_edges() == len[0] + len[1];

    const bool ok = okIO && okMerge;

    std::cout << (ok ? "package test passed" : "package test FAILED") << '\n';

    delete graffine::msgr;

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
