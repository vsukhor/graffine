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
 * \brief Template for named properties of the graph elements.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_PROPERTY_H
#define GRAFFINE_PROPERTY_H

#include "graffine/definitions.h"


namespace graffine {

/**
 * \brief A value carried by a graph element, together with its names.
 * \details The names label the value in printouts; e.g. an edge holds
 * its weight as <tt>Property<real> weight {"weight", "wt", 1.0}</tt>.
 * The value is read and written through the call operator:
 * \c weight() returns a reference to it.
 * \tparam ValueT Type of the value.
 */
template<typename ValueT>
struct Property
{
    using value_t = ValueT;  ///< Type of the value.

    const char* nameLong;   ///< Full name of the property.
    const char* nameShort;  ///< Abbreviated name, used in printouts.

private:  // Data

    ValueT value;  ///< The value.

public:  // Functions

    /**
     * \brief Constructor.
     * \param nameL Full name of the property.
     * \param nameS Abbreviated name of the property.
     * \param value Initial value.
     */
    constexpr Property(const char* nameL,
                       const char* nameS,
                       const ValueT value)
        : nameLong {nameL}
        , nameShort {nameS}
        , value {value}
    {}

    /**
     * \brief Constructor leaving the value default-initialized.
     * \note For an arithmetic \p ValueT, the value is then indeterminate
     * until assigned.
     * \param nameL Full name of the property.
     * \param nameS Abbreviated name of the property.
     */
    constexpr Property(const char* nameL,
                       const char* nameS)
        : nameLong {nameL}
        , nameShort {nameS}
    {}

    /// Read-only access to the value.
    constexpr auto operator()() const noexcept -> const ValueT&
    {
        return value;
    }
    /// Read-write access to the value.
    constexpr auto operator()() noexcept -> ValueT&
    {
        return value;
    }

    /**
     * \brief Prints the short name and the value to the log, without a
     * line break.
     * \details Only boolean (printed as T or F) and arithmetic values are
     * printed; for other types the function prints nothing.
     */
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
