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
 * \file msgr.h
 * \brief Class for printing short messages to the console and a log file,
 * and the handler of failed assertions.
 * \author Valerii Sukhorukov
 */

#ifndef GRAFFINE_UTILS_MSGR_H
#define GRAFFINE_UTILS_MSGR_H

#include "graffine/utils/colorcodes.h"

#include <array>
#include <cstdlib>  // std::abort
#include <fstream>
#include <iostream>
#include <ostream>
#include <source_location>
#include <sstream>
#include <stdarg.h>
#include <string>
#include <type_traits>
#include <utility>   // forward
#include <vector>

/// General-purpose utilities.
namespace graffine::utils {


/** Reports a failed assertion to std::cerr and aborts the process.
 * \details Called by GRAFFINE_ASSERT and GRAFFINE_ASSERT_CALLING (see
 * misc.h) in DEBUG builds.
 * \param CND Text of the failed condition.
 * \param location Location of the failed assertion.
 * \param msg Parts of the explanation printed after the condition.
 */
template<typename ...Args>
[[noreturn, gnu::cold]]
void failure_message(
    const char* CND,
    const std::source_location location,
    Args... msg
)
{
    namespace cc = colorcodes;

    std::cerr << cc::BOLDRED << "Assertion ( "
              << cc::BOLDCYAN << CND << cc::BOLDRED
              << " ) failed!" << cc::RESET << std::endl
              << cc::BOLDRED << "Reason: " << cc::RESET;
    if constexpr (sizeof...(Args))
        (std::cerr << ... << msg);
    std::cerr << std::endl;

    std::cerr << cc::YELLOW << "    file:     " << cc::RESET
              << location.file_name() << '('
              << location.line() << ':'
              << location.column() << ")\n"
              << cc::YELLOW << "    function: " << cc::RESET
              << location.function_name() << std::endl;

    // A failed assertion reveals a defect: stop where it is detected.
    std::abort();
}

// /////////////////////////////////////////////////////////////////////////////

/** Formatted text output to the screen and to a log file.
 * \details Each message is written to both streams, if set; the copy
 * written to the log file is stripped of color codes. Real numbers are
 * printed in scientific notation.
 */
class Msgr {

public:

    using outstream = std::ostream;   ///< Type of the screen stream.
    using logstream = std::ofstream;  ///< Type of the log file stream.


    outstream* so {};  ///< Screen output stream; null for no screen output.
    logstream* sl {};  ///< Log file stream; null for no log file.

    /** Default constructor: no output streams, so nothing is printed.
     */
    Msgr() = default;

    /** Constructs the object given streams to the screen and to a file.
     * \param so Screen output stream, or nullptr.
     * \param sl Log file stream, or nullptr.
     * \param precision Number of digits of real numbers.
     */
    explicit Msgr(
        outstream* so,
        logstream* sl,
        int precision
    );

    /** Sets the streams to print real numbers in scientific notation with
     * \p precision digits.
     * \param precision Number of digits of real numbers.
     */
    void set_formats(int precision) noexcept;

    /** Prints a named std::array on one line.
     * \tparam V Data type of array elements.
     * \tparam N Number of array elements.
     * \param name Name/title.
     * \param v Array data.
     */
    template<typename V, auto N>
    void print_array(
        const std::string& name,
        const std::array<V,N>& v
    ) const noexcept;

    /** Prints a named std::vector on one line.
     * \tparam V Data type of vector elements.
     * \param name Name/title.
     * \param v Vector data.
     */
    template<typename V>
    void print_vector(
        const std::string& name,
        const std::vector<V>& v
    ) const noexcept;

    /** Prints the concatenation of values of various types, followed by
     * a space.
     * \details Based on a stackoverflow.com answer by geza:
     * https://stackoverflow.com/questions/51647834/printf-like-utility-in-c-without-format-specifier/51648068#51648068
     * \tparam endline Finish with a line end.
     * \param values Values to print.
     */
    template<bool endline=true, typename... T>
    void print(T... values) const;

    /** Stream operator with the same functionality as print().
     * \note The output is not terminated by a newline.
     */
    template<typename... T>
    Msgr& operator<<(T... values);

    /** Packs \p values into an output stringstream.
     */
    template<typename... T>
    static auto oss(T... values) -> std::ostringstream;

private:

    /** Checks that \p S is one of the stream types supported.
     * \tparam S Stream type.
     */
    template<typename S>
    static constexpr auto is_valid_stream() noexcept;

    /** Removes the color codes from a string, before it is written to the
     * log file.
     */
    auto rm_colorcode(std::string&& v) const -> std::string;

    /** Prints a string to a stream \p io, then flushes it.
     * \tparam IO Stream type.
     * \param io Stream object.
     * \param v String to print.
     * \param endline Specifies if the line end should be added.
     */
    template<typename IO>
    void prn(
        IO* io,
        std::string&& v,
        bool endline
    ) const;
};


// IMPLEMENTATION ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline
Msgr::
Msgr(
    outstream* so,
    logstream* sl,
    int precision
)
    : so {so}
    , sl {sl}
{
    set_formats(precision);
}


inline
void Msgr::
set_formats(const int precision) noexcept
{
    if (so) {
        so->precision(precision);
        so->setf(std::ios::scientific);
    }
    if (sl) {
        sl->precision(precision);
        sl->setf(std::ios::scientific);
    }
}


template<typename S>
constexpr auto Msgr::
is_valid_stream() noexcept
{
    return std::is_same_v<S, outstream> ||
           std::is_same_v<S, logstream>;
}


inline
auto Msgr::
rm_colorcode(std::string&& s) const -> std::string
{
    for (const auto& p : colorcodes::sall) {
        auto n = p.length();
        for (auto i = s.find(p); i != std::string::npos; i = s.find(p))
            s.erase(i, n);
    }

    return s;
}


template<typename IO>
void Msgr::
prn(
    IO* io,
    std::string&& v,
    const bool endline
) const
{
    static_assert(is_valid_stream<IO>(),
                  "Stream type used in Msgr is not valid");

    if constexpr (std::is_same_v<IO, logstream>)
        v = rm_colorcode(std::forward<std::string>(v));

    *io << v << " ";
    if (endline)
        *io << std::endl;
    io->flush();
}


template<typename V, auto N>
void Msgr::
print_array(
    const std::string& name,
    const std::array<V,N>& v
) const noexcept
{
    print<false>(name+"[]:  ");
    for (const auto o : v)
        print<false>(o);
    print("");
}


template<typename V>
void Msgr::
print_vector(
    const std::string& name,
    const std::vector<V>& v
) const noexcept
{
    print<false>(name+"[]:  ");
    for (const auto o : v)
        print<false,V>(o);
    print<true>("");
}


template<typename... T>
auto Msgr::
oss(T... values) -> std::ostringstream  // static
{
    std::ostringstream s;

    if constexpr (sizeof...(T))
        (s << ... << values);
    s.flush();

    return s;
}


template<bool endline,
         typename... T>
void Msgr::
print(T... values) const
{
    const auto s = oss(values...);

	if (sl) prn(sl, s.str(), endline);
	if (so) prn(so, s.str(), endline);
}




template<typename... T>
Msgr& Msgr::
operator<<(T... values)
{
    print<false>(values...);

    return *this;
}


/// A type with a member function <tt>print(std::ostream&)</tt>.
template<typename T>
concept Printable = requires(std::ostream& os, const T& t)
{
    { t.print(os) };
};

/// Writes a Printable object to \p os by calling its print() member.
template<Printable T>
std::ostream& operator<<(std::ostream& os, const T& t)
{
    t.print(os);
    return os;
}

}  // namespace graffine::utils

#endif // GRAFFINE_UTILS_MSGR_H
