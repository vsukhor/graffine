// =============================================================================
//
// This example illustrates use of two types of elementary transformations:
// the creation of graph components and the merger of vertices.
// Here, we initiate a graph by inserting several unconnected components,
// and apply a single merger event of two vertices belonging to distinct
// components. Because the merged vertices are leaves, and the input subgraphs
// are single linear chains, the resulting component remains linear.
// Printing out the structure description is optional.
//
// =============================================================================

#include "graffine/definitions.h"
#include "graffine/structure/elements/chain.h"
#include "graffine/structure/elements/component.h"
#include "graffine/structure/elements/edge.h"
#include "graffine/structure/elements/graph.h"
#include "graffine/transforms/vertex_merger/core.h"

#include <array>
#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    namespace elements = graffine::structure;
    namespace trs = graffine::transforms;

    // Create logger for the command-line print-outs.
    graffine::msgr = new graffine::Msgr {&std::cout, nullptr, 6};

    using G = elements::Graph<
              elements::Component<
              elements::Chain<
              elements::Edge<elements::Vertex>>>>;
    using Chain = G::Chain;
    using End = Chain::End;  // Chain end descriptors.

    // Create graph object and populate it with four linear components

    // Initial component lengths in edges
    constexpr std::array len {4UL, 2UL, 1UL, 3UL};

    G gr;
    for (const auto u : len)
        gr.add_single_chain_component(u);

    gr.print_components("BEFORE");  // print out initial conformation

    // Create a 'vertex merger' transformation

    using Core = trs::vertex_merger::Core<G>;
    Core core {gr, nullptr, "vm_core"};

    // Pick up two chains for the merger. Their IDs:
    constexpr graffine::ChIdG w1 {0};
    constexpr graffine::ChIdG w2 {1};

    // Merge the chains on their tails;
    core.antiparallel(End::A, w1, w2);

    gr.print_components("AFTER");  // print out resulting structure

    delete graffine::msgr;
}
