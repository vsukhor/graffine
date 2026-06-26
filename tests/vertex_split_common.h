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

#ifndef GRAFFINE_TESTS_VERTEX_SPLIT_COMMON_H
#define GRAFFINE_TESTS_VERTEX_SPLIT_COMMON_H


#include "common.h"
#include "graffine/transforms/vertex_merger/from_11.h"
#include "graffine/transforms/vertex_merger/from_12.h"
#include "graffine/transforms/vertex_merger/from_13.h"
#include "graffine/transforms/vertex_merger/from_22.h"
#include "graffine/transforms/vertex_split/functor.h"
#include "graffine/transforms/vertex_split/to_11.h"
#include "graffine/transforms/vertex_split/to_12.h"
#include "graffine/transforms/vertex_split/to_13.h"
#include "graffine/transforms/vertex_split/to_22.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <numeric>  // accumulate
#include <string>

namespace graffine::tests::vertex_split {

namespace trs = graffine::transforms;

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


/// Subclass to make protected members accessible for testing:
/// Use Fusability rather than Divisibility to be able to produce
/// branched vertex merger constracts needed for testing 'Divide'.
template<Degree D1,
         Degree D2,
         typename G>
struct VertexSplit
    : public trs::vertex_split::To<D1, D2, G> {

    static_assert(areCompatibleDegrees<D1, D2>);

    explicit VertexSplit(G& graph)
        : trs::vertex_split::To<D1, D2, G> {graph}
    {}
};


class VertexSplitTest
    : public Test {

protected:

    static constexpr auto eA = End::A;
    static constexpr auto eB = End::B;
};

}  // namespace graffine::tests::vertex_split

#endif  // GRAFFINE_TESTS_VERTEX_SPLIT_COMMON_H
