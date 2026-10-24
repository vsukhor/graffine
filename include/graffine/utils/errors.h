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
 * \file errors.h
 * \brief Exception types reported by graffine and helpers for raising them.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_UTILS_ERRORS_H
#define GRAFFINE_UTILS_ERRORS_H

#include <expected>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>

namespace graffine {

/// Base class of all exceptions thrown by graffine.
struct error
    : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/**
 * Internal bookkeeping of a graph was found inconsistent.
 * \details Indicates a defect in graffine or a graph corrupted by direct
 * manipulation of its members. The graph is left in an unspecified state:
 * it can only be destroyed.
 */
struct invariant_error
    : public error
{
    using error::error;
};

/**
 * An operation was requested for graph elements not meeting its requirements.
 */
struct precondition_error
    : public error
{
    using error::error;
};

/// Reading or writing a graph file failed, or the file content is invalid.
struct io_error
    : public error
{
    using error::error;
};

/// Description of a failed consistency check.
struct Violation {
    std::string message;
};

/// Outcome of a consistency check: either success or the first violation.
using Validation = std::expected<void, Violation>;

namespace utils {

/**
 * Throws exception \p E with a message composed of \p args.
 * \details Out of line and cold, so that the checks calling it cost no more
 * than a predictable branch on the success path. The message is only
 * formatted when the check fails.
 * \tparam E Exception type to throw.
 * \param cond Text of the failed condition, or nullptr.
 * \param loc Location of the failed check.
 * \param args Parts of the message.
 */
template<typename E,
         typename... Args>
[[noreturn, gnu::cold, gnu::noinline]]
void raise(
    const char* cond,
    const std::source_location loc,
    Args... args
)
{
    std::ostringstream os;
    if constexpr (sizeof...(Args) > 0)
        (os << ... << args);
    if (cond)
        os << " [failed: " << cond << ']';
    os << " (" << loc.file_name() << ':' << loc.line() << ')';

    throw E(os.str());
}

}  // namespace utils
}  // namespace graffine

#endif  // GRAFFINE_UTILS_ERRORS_H
