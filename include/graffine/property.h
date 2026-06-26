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
 * \file property.h
 * \brief Template for properties of the core elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_PROPERTY_H
#define GRAFFINE_PROPERTY_H

#include "graffine/definitions.h"


namespace graffine {

template<typename ValueT>
struct Property
{
    using value_t = ValueT;

    const char* nameLong;
    const char* nameShort;

private:  // Data

    ValueT value;

public:  // Functions

    constexpr Property(const char* nameL,
                       const char* nameS,
                       const ValueT value)
        : nameLong {nameL}
        , nameShort {nameS}
        , value {value}
    {}

    constexpr Property(const char* nameL,
                       const char* nameS)
        : nameLong {nameL}
        , nameShort {nameS}
    {}

    constexpr auto operator()() const noexcept -> const ValueT&
    {
        return value;
    }
    constexpr auto operator()() noexcept -> ValueT&
    {
        return value;
    }

    void print() const
    {
        if constexpr (std::is_same_v<value_t, bool>)
            jot<false>(nameShort,
                       colorcodes::BOLDBLUE, ' ', value ? "T " : "F ",
                       colorcodes::RESET);

        else if constexpr (std::is_arithmetic_v<value_t>)
            jot<false>(nameShort,
                       colorcodes::BOLDBLUE, ' ', value, ' ',
                       colorcodes::RESET);
    }
};


}  // namespace graffine

#endif  // GRAFFINE_PROPERTY_H
