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
 * @file definitions.h
 * @brief project-scope definitions.
 * @details Contains various type definitions and utility functions used
 * throughout the project.
 * @author Valerii Sukhorukov.
 */

#ifndef GRAPH_MUTATOR_DEFINITIONS_H
#define GRAPH_MUTATOR_DEFINITIONS_H

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <ranges>
#include <string>
#include <vector>

#include "utils/msgr.h"
#include "utils/random/with_boost.h"

#include "structure/basic/vertices/degrees.h"

namespace graph_mutator {
inline namespace definitions {

using namespace std::string_literals;

inline constexpr auto zip = std::views::zip;

using colorcodes = utils::Colorcodes;
using szt = std::size_t;

using namespace structure::basic::vertices::degrees;

template<typename T>
concept numerically_limited = std::numeric_limits<T>::is_specialized;

template<typename T>
concept arithmetic = std::integral<T> ||
                     std::floating_point<T>;

template<typename T> constexpr auto zero = static_cast<T>(0);
template<typename T> constexpr auto one = static_cast<T>(1);

template<numerically_limited T>
constexpr T huge {
        std::numeric_limits<T>::has_infinity
        ? std::numeric_limits<T>::infinity()
        : std::numeric_limits<T>::max()
    };

template<numerically_limited T>
constexpr T undefined {huge<T>};

template<numerically_limited T> constexpr
bool is_defined(const T a)
{
    return a != undefined<T>;
}
template<numerically_limited T> constexpr
bool is_undefined(const T a)
{
    return a == undefined<T>;
}
template<arithmetic T, T v>
consteval auto is_defined() -> bool
{
    return v != undefined<T>;
}

template<typename T> using vec2 = std::vector<std::vector<T>>;
template<typename T> using vec3 = std::vector<vec2<T>>;
template<typename T> using vup = std::vector<std::unique_ptr<T>>;
template<typename T, auto N> using vecarr = std::vector<std::array<T, N>>;

inline constexpr std::array<bool, 2> bools {false, true};
template<typename T> constexpr std::array<T, 2> zeroone {zero<T>, one<T>};
template<typename T> constexpr std::array<T, 2> zerohuge {zero<T>, huge<T>};
template<typename T> constexpr std::array<T, 2> onehuge {one<T>, huge<T>};


using real = float;

using Msgr = utils::Msgr;
using RandFactory = utils::random::Boost<real>;

/// Functors log in verbose mode.
inline constexpr bool verboseF {true};
/// Tests log in verbose mode.
inline constexpr bool verboseT {true};

/// Print detailed data on edges.
inline constexpr bool print_edges {true};

/// Toggles integral testing on and off.
inline constexpr bool integral_testing {true};

// Typenames for ids of structural elements and theirr containers.

using itT = std::uint_fast64_t;  ///< Type for counting simulation iterations.
using EgId = szt;  ///< Edge ids.
using ChId = szt;  ///< Chain ids.
using CmpId = szt;  ///< Component ids.

using ChIds = std::vector<ChId>;  ///< Container for chain ids.
using EgIds = std::vector<EgId>;  ///< Container for edge ids.

/// Orientation of edges relative orientation of the host chain.
enum Orientation {
    Backwards = -1,  ///< B -> A
    Forwards = 1     ///< A -> B
};


static thread_local Msgr* msgr {};  ///< Output message processor.


template<bool endline=true,
         typename ...Args>
static void log_(Args... args)
{
    if (msgr)
        msgr->print<endline>(args...);
    else
        std::cerr << "Error: msgr is NULL" << std::endl;
}


template<typename ...Args>
static void abort(Args... args)
{
    if (msgr)
        msgr->print(args...);
    else
        std::cerr << "Error: msgr is NULL" << std::endl;

    std::exit(EXIT_FAILURE);
}


template<typename ...Args>
static void abort_if(bool condition, Args... args)
{
    if (condition)
        abort(args...);
}


template<typename... T>
Msgr& operator<<(Msgr& msgr, T... values)
{
    msgr.print<false>(values...);

    return msgr;
}


// array from std::integer_sequence, based on
//https://stackoverflow.com/questions/41660062/how-to-construct-an-stdarray-with-index-sequence
template<typename T, std::size_t N, std::size_t... I>
consteval auto create_array_impl(std::integer_sequence<T, I...>)
{
    return std::array<T, N>{ {I...} };
}


template<typename T, std::size_t N>
consteval auto create_array()
{
    return create_array_impl<T, N>(std::make_integer_sequence<T, N>{});
}

}  // namespace definitions

}  // namespace graph_mutator


#endif  // GRAPH_MUTATOR_DEFINITIONS_H
