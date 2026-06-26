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
 * \brief Implements edge or chain ends.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_END_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_END_H


#include "graffine/definitions.h"

#include <array>
#include <string>
#include <type_traits>

namespace graffine::structure::descriptors {

enum EndType
{
    A, B, undefined
};

/**
 * Represents the ends of an edge or those of a chain.
 */
struct End
{
    using Id = EndType;
    using value_type = std::underlying_type<EndType>;

    static constexpr std::size_t num {2};
    static constexpr EndType A {EndType::A};
    static constexpr EndType B {EndType::B};
    static constexpr EndType undefined {EndType::undefined};
    static constexpr std::array Ids {A, B};

    static constexpr auto opp(const Id e) noexcept -> Id
    {
        return e == A ? B : A;
    }

    template<EndType E>
    static consteval auto opp() noexcept -> Id
    {
        if constexpr (E == A)
            return B;

        if constexpr (E == B)
            return A;

        return undefined;
    }

    static constexpr bool is_defined(const Id e) noexcept
    {
        return e != undefined;
    }

    static constexpr bool is_valid(const Id e) noexcept
    {
        return e == A || e == B;
    }

    template<EndType E>
    static constexpr bool is_valid() noexcept
    {
        return E == A || E == B;
    }

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

    static constexpr auto str(const Id e) noexcept -> std::string
    {
        return e == A ? "A" : e == B ? "B" : "Undefined";
    }
};


template<End::Id E>
concept Ends = E == End::A || E == End::B;


}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_END_H
