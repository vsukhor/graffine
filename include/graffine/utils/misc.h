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
* \file misc.h
* \brief A loose collection of functions of common use.
* \author Valerii Sukhorukov
*/

#ifndef GRAFFINE_UTILS_MISC_H
#define GRAFFINE_UTILS_MISC_H

#include "graffine/utils/constants.h"
#include "graffine/utils/errors.h"
#include "graffine/utils/msgr.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <numeric>
#include <source_location>
#include <string>
#include <vector>

// Assertion macros carry the GRAFFINE_ prefix so as not to collide with
// macros of the same name defined by client code or other libraries.
//
// GRAFFINE_ASSERT and GRAFFINE_ASSERT_CALLING are compiled in DEBUG builds
// only. They guard hot paths, and a failure aborts the process: it reveals
// a defect, and the abort stops in a debugger or leaves a core dump.
// GRAFFINE_ASSERT_CALLING evaluates the expression FUN before aborting,
// e.g. to print the state that made the assertion fail.
//
// The checks below are always compiled in, and throw:
// GRAFFINE_ENSURE  -- graffine::invariant_error, if EX is false;
// GRAFFINE_ABORT   -- graffine::invariant_error, unconditionally;
// GRAFFINE_REQUIRE -- graffine::precondition_error, if EX is false.
// The message arguments are only evaluated when the check fails.

#ifdef DEBUG
    #define GRAFFINE_ASSERT(EX, ...) \
        (void)((EX) || (::graffine::utils::failure_message(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0))
#else
    #define GRAFFINE_ASSERT(EX, ...)
#endif

#ifdef DEBUG
    #define GRAFFINE_ASSERT_CALLING(EX, FUN, ...) \
        (void)((EX) || (FUN, 0) || (::graffine::utils::failure_message(#EX, std::source_location::current() __VA_OPT__(,) __VA_ARGS__), 0))
#else
    #define GRAFFINE_ASSERT_CALLING(EX, ...)
#endif

#define GRAFFINE_ENSURE(EX, ...) \
    (void)((EX) || (::graffine::utils::raise<::graffine::invariant_error>(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0))

/// \note Here, condition \p EX is not evaluated: it only describes the
/// failure in the message.
#define GRAFFINE_ABORT(EX, ...) \
    ::graffine::utils::raise<::graffine::invariant_error>(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__)

#define GRAFFINE_REQUIRE(EX, ...) \
    (void)((EX) || (::graffine::utils::raise<::graffine::precondition_error>(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0))

/// General-purpose utilities.
namespace graffine::utils {

/** Binary operation adding the result of member function \p P of an
 * object to an accumulated value, e.g. for std::accumulate over a
 * container of \p Q objects.
 * \tparam T Type of the accumulated value and of the result of \p P.
 * \tparam Q Type of the objects.
 * \tparam P Pointer to a const member function of \p Q returning \p T.
 */
template<typename T,
         typename Q,
          T (Q::* P)() const>  // member function pointer parameter
struct Adder
{
    int operator()(const T& i, const Q& o) const
    {
        return (o.*P)() + i;
    }
};

/** Finds the index of the first smallest element of a vector.
 * \note \p v must not be empty.
 * \return Index of the first smallest element of vector \p v.
 */
template<typename T> constexpr
std::size_t index_min( const std::vector<T>& v ) noexcept
{
    std::size_t k {};
    auto m = v[0];
    for (std::size_t i=1; i<v.size(); i++)
        if (v[i] < m) {
            k = i;
            m = v[i];
        }
    return k;
}

/** Removes from a vector the first element equal to \p a.
 * \details The order of the remaining elements is preserved. In DEBUG
 * builds, the absence of such an element is an assertion failure; otherwise
 * the vector is then left unchanged.
 * \tparam T Data type (must be EqualityComparable).
 * \param v The vector.
 * \param a The value to remove.
 */
template<typename T>
void remove_vector_element( std::vector<T>& v,
                            const T& a )
{
    for (auto i=v.begin(); i!=v.end(); i++)
        if (a == *i) {
            v.erase(i);
            return;
        }
    GRAFFINE_ASSERT(false, "element not found");
}

/// Implementation of integers_zero_based(): builds an array from the
/// integers of a std::integer_sequence. Based on
/// https://stackoverflow.com/questions/41660062/how-to-construct-an-stdarray-with-index-sequence
template<typename T, std::size_t N, std::size_t... I>
constexpr auto create_array_impl(std::integer_sequence<T, I...>)
{
    return std::array<T, N>{{I...}};
}

/// Returns the compile-time array {0, 1, ..., N-1} of type \p T.
template<typename T, std::size_t N>
constexpr auto integers_zero_based()
{
    return create_array_impl<T, N>(std::make_integer_sequence<T, N>{});
}


}  // namespace graffine::utils


#endif  // GRAFFINE_UTILS_MISC_H
