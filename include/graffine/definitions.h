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
 * \brief Project-wide type names and definitions.
 * \details Brings the element identifiers, the vertex degree tags and the
 * utility constants into the \c graffine namespace, and defines the
 * floating-point type and the edge orientation used throughout the library.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>.
 */

#ifndef GRAFFINE_DEFINITIONS_H
#define GRAFFINE_DEFINITIONS_H

#include "graffine/config_cmake.h"
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
#include <type_traits>
#include <vector>

/// Project namespace
namespace graffine {
/// Definitions visible throughout graffine (an inline namespace, so they
/// are accessible directly as members of \c graffine).
inline namespace definitions {

using namespace std::string_literals;
using namespace graffine::utils::constants;
using namespace structure::descriptors::ids;
using namespace structure::descriptors::vertex_degrees;

namespace colorcodes = utils::colorcodes;

using Msgr = utils::Msgr;  ///< Message logger, see utils/msgr.h.

/// Floating-point type of the non-topological quantities, such as edge
/// weights: \c double if CMake set GRAFFINE_REAL_IS_DOUBLE, else \c float.
using real = std::conditional_t<realIsDouble, double, float>;

// The reporting flags 'verboseF', 'printEdges' and 'printVertices' are set
// by CMake, see config_cmake.h.

/// Tests report in verbose mode.
inline constexpr bool verboseT {true};

/// Orientation of an element relative to the direction of its host,
/// e.g. of an edge relative to its chain, which runs from end A to end B.
enum Orientation {
    Backwards = -1,  ///< Points from the host's end B towards its end A.
    Forwards = 1,    ///< Points from the host's end A towards its end B.
    Undefined = 0    ///< Not yet set.
};

using itT = std::uint_fast64_t;  ///< Type for counting simulation iterations.

/// A false value depending on template parameters, for
/// <tt>static_assert(always_false<T>)</tt> in a discarded branch of
/// <tt>if constexpr</tt>, which a plain \c false would trigger
/// unconditionally before C++23.
template <class...> constexpr std::false_type always_false{};

}  // namespace definitions
}  // namespace graffine


#endif  // GRAFFINE_DEFINITIONS_H
