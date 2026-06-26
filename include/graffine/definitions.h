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
 * \file definitions.h
 * \brief project-scope typenames and definitions.
 * \details Contains various type definitions and utility functions used
 * throughout the project.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_DEFINITIONS_H
#define GRAFFINE_DEFINITIONS_H

#include "graffine/structure/descriptors/id.h"
#include "graffine/structure/descriptors/vertex_degrees.h"
#include "graffine/utils/colorcodes.h"
#include "graffine/utils/misc.h"
#include "graffine/utils/logger.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <istream>
#include <limits>
#include <ostream>
#include <ranges>
#include <streambuf>
#include <string>
#include <vector>

/// Project namespace
namespace graffine {
/// Contains definitions available to all graffine entities
inline namespace definitions {

using namespace std::string_literals;
using namespace graffine::utils::constants;
using namespace structure::descriptors::ids;
using namespace structure::descriptors::vertex_degrees;

namespace colorcodes = utils::colorcodes;

using Msgr = utils::Msgr;
using real = float;

/// Functors report in verbose mode.
inline constexpr bool verboseF {true};
/// Tests report in verbose mode.
inline constexpr bool verboseT {true};

/// Print detailed data on edges.
inline constexpr bool print_edges {true};

/// Print detailed data on vertices.
inline constexpr bool print_vertices {true};

/// Toggles integral testing on and off.
inline constexpr bool integral_testing {true};

/// Orientation of elements relative to ther orientation of their host.
enum Orientation {
    Backwards = -1,  ///< B -> A
    Forwards = 1,    ///< A -> B
    Undefined = 0
};

using itT = std::uint_fast64_t;  ///< Type for counting simulation iterations.

/// For use in if constexpr static_assert in case c++23 is not activated.
template <class...> constexpr std::false_type always_false{};

}  // namespace definitions
}  // namespace graffine


#endif  // GRAFFINE_DEFINITIONS_H
