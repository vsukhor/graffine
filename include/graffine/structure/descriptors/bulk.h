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
 * \file bulk.h
 * \brief Location of a vertex inside a chain.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_BULK_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_BULK_H


#include "graffine/definitions.h"

#include <string>

/// Defines lightweight descriptors of the graph elements
namespace graffine::structure::descriptors {

/** Location of a vertex inside a chain, as opposed to at its ends (End).
 * \details The location is given by the position \a a along the chain:
 * the vertex joins edges a-1 and a. It mirrors the interface of End,
 * so that slots of both kinds share the same template (see _Slot).
 */
struct Bulk
{
    using Id = EgIdA;  ///< Position of the vertex along the chain.

    /// Number of locations: not fixed, as it depends on the chain length.
    static constexpr Id num {Id::undefined};
    static constexpr Id Undefined {Id::undefined};  ///< Not set.

    /// Inner vertices have no opposite: always returns \ref Undefined.
    static constexpr auto opp(const Id) noexcept -> Id
    {
        return Undefined;
    }

    /// The position \p a as a string.
    static auto str(const Id a) noexcept -> std::string
    {
        return std::to_string(a());
    }
};


}  // namespace graffine::structure::descriptors

#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_BULK_H
