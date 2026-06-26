/* =============================================================================

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

#ifndef GRAFFINE_TESTS_VERTEX_MERGER_COMMON_H
#define GRAFFINE_TESTS_VERTEX_MERGER_COMMON_H

#include "common.h"
//#include "graffine/properties/processors.h"
#include "graffine/transforms/vertex_merger/core.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"
//#include "graffine/transforms/vertex_merger/from_1N.h"
//#include "graffine/transforms/vertex_merger/from_2N.h"
#include "graffine/transforms/vertex_merger/functor.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <numeric>  // accumulate, iota
#include <string>


namespace graffine::tests::vertex_merger {

namespace trs = graffine::transforms;

/// Subclass to make protected members accessible for testing:
struct VertexMergerCore
    : public trs::vertex_merger::Core<G> {

    using Base = trs::vertex_merger::Core<G>;

    using Base::antiparallel;
    using Base::parallel;
    using Base::to_cycle;
    using Base::cn;

    explicit VertexMergerCore(G& gr)
        : Base {gr, nullptr, "vm_core"}
    {}
};


/// Subclass to make protected members accessible for testing:
template<Degree D1,
         Degree D2,
         typename G>
struct VertexMerger
    : public trs::vertex_merger::From<D1, D2, G> {

    explicit VertexMerger(G& graph)
        : trs::vertex_merger::From<D1, D2, G> {graph}
    {}
};

template<Degree D,
         typename G>
struct VertexMerger1
    : public trs::vertex_merger::From1<D, G> {

    explicit VertexMerger1(G& graph)
        : trs::vertex_merger::From1<D, G> {graph}
    {}
};


template<Degree D1,
         Degree D2,
         typename G>
struct VertexMergerFunctor
    : public trs::vertex_merger::Functor<G> {

    explicit VertexMergerFunctor(G& graph)
        : trs::vertex_merger::Functor<G> {graph}
    {}
};


class VertexMergerTest
    : public Test {

protected:

    constexpr ESlot sA(const ChIdG w)
    {
        return ESlot{w, End::A};
    }

    constexpr ESlot sB(const ChIdG w)
    {
        return ESlot{w, End::B};
    }
};


}  // namespace graffine::tests::vertex_merger

#endif  // GRAFFINE_TESTS_VERTEX_MERGER_COMMON_H
