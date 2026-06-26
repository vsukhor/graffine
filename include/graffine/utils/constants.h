/* =============================================================================

 Copyright (C) 2009-2025 Valerii Sukhorukov. All Rights Reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

================================================================================
*/

/**
* \file constants.h
* \brief A collection of useful definitions.
* \author Valerii Sukhorukov
*/

#ifndef GRAFFINE_UTILS_CONSTANTS_H
#define GRAFFINE_UTILS_CONSTANTS_H

#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

/// Library namespace.
namespace graffine::utils {

template<typename T>
concept arithmetic = std::integral<T> ||
                     std::floating_point<T>;


/// Make these definitions available to the whole library by defaule.
inline namespace constants {

template<typename T>
constexpr auto STR(T x)
{
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(x);
    else  //if constexpr (std::is_same_v<T, char*>)
        return std::string(x);
//    else
//        static_assert(false, "Unsupported type");
}

using ulong = unsigned long;
using uint = unsigned int;

// container typedefs ==========================================================

// std vector-based 2, 3, 4-dim containers
template<typename T> using vec2 = std::vector<std::vector<T>>;
template<typename T> using vec3 = std::vector<vec2<T>>;

// std vector and std array
template<typename T, auto N> using arrvec = std::array<std::vector<T>, N>;
template<typename T, auto N> using vecarr = std::vector<std::array<T, N>>;

// std vector of unique pointers
template<typename T> using vup = std::vector<std::unique_ptr<T>>;

// common constants ============================================================
template<std::floating_point T> inline constexpr auto
    pi = static_cast<T>(3.1415926535897932384626433832795L);
template<std::floating_point T> inline constexpr auto
    sqrtPI = static_cast<T>(1.7724538509055160272981674833411L);

template<std::floating_point T> inline constexpr T
    sqrt2PI {
        std::pow(2*pi<T>, T(0.5))
    };

template<std::floating_point T> inline constexpr auto
    sqrt2 = static_cast<T>(1.41421356237309504880168872420969807856967L);

template<std::floating_point T> inline constexpr auto
    e = static_cast<T>(2.7182818284590452353602874713527L);

// Templates for numeric limits. ===============================================

template<typename T>
concept numerically_limited = std::numeric_limits<T>::is_specialized;

template<numerically_limited T>
inline constexpr T EPS {std::numeric_limits<T>::epsilon()};

template<numerically_limited T>
inline constexpr T huge {
        std::numeric_limits<T>::has_infinity
        ? std::numeric_limits<T>::infinity()
        : std::numeric_limits<T>::max()
    };

template<numerically_limited T>
inline constexpr T undefined {huge<T>};

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

template<numerically_limited T> inline constexpr
T MAX {std::numeric_limits<T>::max()};

template<numerically_limited T> inline constexpr
T MIN {std::numeric_limits<T>::min()};

template<numerically_limited T> inline constexpr
T INF {std::numeric_limits<T>::infinity()};


}  // namespace constants

}  // namespace graffine::utils

#endif  // GRAFFINE_UTILS_CONSTANTS_H
