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
* \brief General-purpose type aliases, mathematical constants and
* numeric limits.
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

/// General-purpose utilities.
namespace graffine::utils {

/// An integral or floating-point type.
template<typename T>
concept arithmetic = std::integral<T> ||
                     std::floating_point<T>;


/// Constants and aliases, made available to the whole library by
/// definitions.h.
inline namespace constants {

/// Converts \p x to std::string: a number by std::to_string, anything else
/// by the std::string constructor.
template<typename T>
constexpr auto STR(T x)
{
    if constexpr (std::is_arithmetic_v<T>)
        return std::to_string(x);
    else
        return std::string(x);
}

using ulong = unsigned long;  ///< Shorthand for unsigned long.
using uint = unsigned int;    ///< Shorthand for unsigned int.

// container typedefs ==========================================================

/// Two-dimensional std::vector.
template<typename T> using vec2 = std::vector<std::vector<T>>;
/// Three-dimensional std::vector.
template<typename T> using vec3 = std::vector<vec2<T>>;

/// Fixed-size std::array of \p N std::vectors.
template<typename T, auto N> using arrvec = std::array<std::vector<T>, N>;
/// std::vector of fixed-size std::arrays of length \p N.
template<typename T, auto N> using vecarr = std::vector<std::array<T, N>>;

/// std::vector of std::unique_ptr.
template<typename T> using vup = std::vector<std::unique_ptr<T>>;

// common constants ============================================================

/// The number π.
template<std::floating_point T> inline constexpr auto
    pi = static_cast<T>(3.1415926535897932384626433832795L);
/// Square root of π.
template<std::floating_point T> inline constexpr auto
    sqrtPI = static_cast<T>(1.7724538509055160272981674833411L);

/// Square root of 2π.
template<std::floating_point T> inline constexpr T
    sqrt2PI {
        std::pow(2*pi<T>, T(0.5))
    };

/// Square root of 2.
template<std::floating_point T> inline constexpr auto
    sqrt2 = static_cast<T>(1.41421356237309504880168872420969807856967L);

/// Euler's number e.
template<std::floating_point T> inline constexpr auto
    e = static_cast<T>(2.7182818284590452353602874713527L);

// Templates for numeric limits. ===============================================

/// A type for which std::numeric_limits is specialized.
template<typename T>
concept numerically_limited = std::numeric_limits<T>::is_specialized;

/// Machine epsilon of \p T: the difference between 1 and the next
/// representable value (0 for integral types).
template<numerically_limited T>
inline constexpr T EPS {std::numeric_limits<T>::epsilon()};

/// Largest value of \p T: infinity if \p T has one, otherwise its maximum.
template<numerically_limited T>
inline constexpr T huge {
        std::numeric_limits<T>::has_infinity
        ? std::numeric_limits<T>::infinity()
        : std::numeric_limits<T>::max()
    };

/// Sentinel marking a value of \p T as not set; equal to \ref huge.
template<numerically_limited T>
inline constexpr T undefined {huge<T>};

/// Checks that \p a is not the \ref undefined sentinel.
template<numerically_limited T> constexpr
bool is_defined(const T a)
{
    return a != undefined<T>;
}
/// Checks that \p a is the \ref undefined sentinel.
template<numerically_limited T> constexpr
bool is_undefined(const T a)
{
    return a == undefined<T>;
}

/// Maximum finite value of \p T.
template<numerically_limited T> inline constexpr
T MAX {std::numeric_limits<T>::max()};

/// Minimum value of \p T for integral types; for floating-point types,
/// the smallest \e positive normalized value (std::numeric_limits::min).
template<numerically_limited T> inline constexpr
T MIN {std::numeric_limits<T>::min()};

/// Positive infinity of \p T (0 for types without one).
template<numerically_limited T> inline constexpr
T INF {std::numeric_limits<T>::infinity()};


}  // namespace constants

}  // namespace graffine::utils

#endif  // GRAFFINE_UTILS_CONSTANTS_H
