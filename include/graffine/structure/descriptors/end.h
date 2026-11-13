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
 * \file end.h
 * \brief Ends of edges and chains.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_END_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_END_H


#include "graffine/definitions.h"

#include <array>
#include <string>
#include <type_traits>

namespace graffine::structure::descriptors {

/// Identifier of an end: A (tail) or B (head).
enum EndType
{
    A,         ///< Tail end.
    B,         ///< Head end.
    undefined  ///< Not set.
};

/** The two ends of an edge or of a chain.
 * \details Each edge and each chain has a tail end A and a head end B; an
 * edge or chain is directed from A to B. The struct groups the end
 * identifiers and the functions operating on them.
 */
struct End
{
    using Id = EndType;  ///< Identifier of an end.
    using value_type = std::underlying_type<EndType>;

    static constexpr std::size_t num {2};  ///< Number of ends.
    static constexpr EndType A {EndType::A};  ///< Tail end.
    static constexpr EndType B {EndType::B};  ///< Head end.
    static constexpr EndType undefined {EndType::undefined};  ///< Not set.
    static constexpr std::array Ids {A, B};  ///< Both ends, for iteration.

    /// The end opposite to \p e. (Returns A for an undefined \p e.)
    static constexpr auto opp(const Id e) noexcept -> Id
    {
        return e == A ? B : A;
    }

    /// The end opposite to \p E, or \c undefined if \p E is not valid.
    template<EndType E>
    static consteval auto opp() noexcept -> Id
    {
        if constexpr (E == A)
            return B;

        if constexpr (E == B)
            return A;

        return undefined;
    }

    /// Checks that \p e is not \c undefined.
    static constexpr bool is_defined(const Id e) noexcept
    {
        return e != undefined;
    }

    /// Checks that \p e is A or B.
    static constexpr bool is_valid(const Id e) noexcept
    {
        return e == A || e == B;
    }

    /// Checks that \p E is A or B.
    template<EndType E>
    static constexpr bool is_valid() noexcept
    {
        return E == A || E == B;
    }

    /// Name of end \p E as a character; does not compile for an invalid \p E.
    template<EndType E>
    static consteval auto str() noexcept ->  char
    {
        if constexpr (E == A)
            return 'A';

        if constexpr (E == B)
            return 'B';

        static_assert(false, "Undefined End::Id");
        return 'X';
    }

    /// Name of end \p e: "A", "B" or "Undefined".
    static constexpr auto str(const Id e) -> std::string
    {
        return e == A ? "A" : e == B ? "B" : "Undefined";
    }
};


/// A valid end identifier: A or B.
template<End::Id E>
concept Ends = E == End::A || E == End::B;


}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_END_H
