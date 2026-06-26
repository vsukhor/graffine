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

#ifdef DEBUG
    #undef ASSERT
    #define ASSERT(EX, ...) \
        (void)((EX) || (utils::failure_message(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0))
#else
    #define ASSERT(EX, ...)
#endif

#ifdef DEBUG
    #undef ASSERT_CALLING
    #define ASSERT_CALLING(EX, FUN, ...) \
        (void)((EX) || (FUN, 0) || (utils::failure_message(#EX, std::source_location::current() __VA_OPT__(,) __VA_ARGS__), 0))
#else
    #define ASSERT_CALLING(EX, ...)
#endif

#undef ENSURE
#define ENSURE(EX, ...) \
    (void)((EX) || (utils::failure_message(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0))

#undef ABORT
/// \note Here, condition \p EX is only used for printing it as text.
#define ABORT(EX, ...) \
    (void)(utils::failure_message(#EX, std::source_location::current()  __VA_OPT__(,) __VA_ARGS__), 0)

/// General stuff.
namespace graffine::utils {

/// Sum at compile time.
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


template<auto E=EXIT_FAILURE>
void exit(const std::string& s)
{
    std::cerr << s << std::endl;
    std::exit(E);
};

/// Uncomplicated process termination with exception.
struct Exception
    : public std::exception
{
    /// Default constructor.
    Exception() = default;

    /**
     * Constructor for printing to a log record.
     * \details Outputs message \p msg to \a Msgr for standard
     * output (if \p msgr is nullptr).
     * \param msg Message to output.
     * \param msgr Output message processor.
     */
    explicit Exception(const std::string& msg,
                       Msgr* msgr)
    {
        if (msgr != nullptr)
            msgr->print(msg);
        else
            std::cerr << msg << std::endl;
    }
};

/// Find index of the last minimal vector element.
/// \return Index of the last minimal element of vector \b v
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

/// Remove from a vector all instances of an element a.
/// \tparam T Data type (must be EqualityComparable).
/// \param v The vector.
/// \param a The value to remove.
template<typename T>
void remove_vector_element( std::vector<T>& v,
                            const T& a )
{
    for (auto i=v.begin(); i!=v.end(); i++)
        if (a == *i) {
            v.erase(i);
            return;
        }
    ASSERT(false, "element not found");
}

// array from std::integer_sequence, based on
//https://stackoverflow.com/questions/41660062/how-to-construct-an-stdarray-with-index-sequence
template<typename T, std::size_t N, std::size_t... I>
constexpr auto create_array_impl(std::integer_sequence<T, I...>)
{
    return std::array<T, N>{{I...}};
}

template<typename T, std::size_t N>
constexpr auto integers_zero_based()
{
    return create_array_impl<T, N>(std::make_integer_sequence<T, N>{});
}


}  // namespace graffine::utils


#endif  // GRAFFINE_UTILS_MISC_H
