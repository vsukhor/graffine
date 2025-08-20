/* =============================================================================

Copyright (c) 2021-2025 Valerii Sukhorukov <vsukhorukov@yahoo.com>
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
 * @file ends.h
 * @brief Implements edge ends.
 * @author Valerii Sukhorukov
 */

#ifndef GRAPH_MUTATOR_STRUCTURE_ENDS_H
#define GRAPH_MUTATOR_STRUCTURE_ENDS_H


#include "../definitions.h"


namespace graph_mutator::structure {

/**
 * @brief Represents the ends of an edge or those of a chain.
 */
struct Ends {

    using Id = szt;

    static constexpr szt num {2};  ///< An edge has two ends.

    static constexpr Id A {0};
    static constexpr Id B {1};
    static constexpr Id Undefined {undefined<Id>};
    static constexpr std::array Ids {A, B};

    static constexpr auto opp(const Id e) noexcept -> Id
    {
        return e == A ? B : A;
    }

    template<Id E> requires (E == A || E == B)
    static consteval auto opp() noexcept -> Id
    {
        if constexpr (E == A)
            return B;

        if constexpr (E == B)
            return A;

        return Undefined;
    }

    template<Id E> requires (E == A || E == B)
    static consteval auto str() noexcept -> const char
    {
        if constexpr (E == A)
            return 'A';

        if constexpr (E == B)
            return 'B';

        static_assert(false);
        return 'X';
    }

    static constexpr auto str(const Id e) noexcept -> std::string
    {
        return e == A ? "A" : e == B ? "B" : "Undefined";
    }
};


struct Bulks {

    using Id = EgId;

    static constexpr Id num {undefined<Id>};

    static constexpr Id Undefined {undefined<Id>};

    static constexpr auto opp(const Id) noexcept -> Id
    {
        return Undefined;
    }

    static constexpr auto str(const Id a) noexcept -> std::string
    {
        return std::to_string(a);
    }
};


template<typename T>
concept SlotCoord = std::is_same_v<T, Ends> ||
                    std::is_same_v<T, Bulks>;


template<Ends::Id E>
concept End = E == Ends::A || E == Ends::B;


}  // namespace graph_mutator::structure

#endif  // GRAPH_MUTATOR_STRUCTURE_ENDS_H
