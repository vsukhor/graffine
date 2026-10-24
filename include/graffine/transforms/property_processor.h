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
 * \file property_processor.h
 * \brief Concept of processors of non-topological properties.
 * \details Transformations accept a processor of non-topological properties
 * (positions, weights...) as their template parameter PP, and call its hooks
 * before and after they change the graph. PP is void for no processing.
 * The hooks are grouped by transformation family, in the members
 * vertexMerger, vertexSplit, edgeCreation, edgeDeletion, edgeMerger,
 * componentCreation, componentDeletion and pulling of the processor; each
 * transformation requires the hooks of its family, and those of the families
 * of the transformations it uses, as specified by the concept Processor of
 * each family.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_TRANSFORMS_PROPERTY_PROCESSOR_H
#define GRAFFINE_TRANSFORMS_PROPERTY_PROCESSOR_H

#include "graffine/transforms/component_creation/functor.h"
#include "graffine/transforms/component_deletion/functor.h"
#include "graffine/transforms/edge_creation/common.h"
#include "graffine/transforms/edge_deletion/common.h"
#include "graffine/transforms/edge_merger/functor.h"
#include "graffine/transforms/pulling/common.h"
#include "graffine/transforms/vertex_merger/common.h"
#include "graffine/transforms/vertex_split/common.h"

namespace graffine::transforms {

/**
 * Processor of non-topological properties usable by all transformations.
 * \details Either void, for no processing, or a type providing the hooks
 * of all the transformation families.
 * \tparam PP Property processor type.
 * \tparam G Graph type.
 */
template<typename PP,
         typename G>
concept PropertyProcessor =
    vertex_merger::Processor<PP, G> &&
    vertex_split::Processor<PP, G> &&
    edge_creation::Processor<PP, G> &&
    edge_deletion::Processor<PP, G> &&
    edge_merger::Processor<PP, G> &&
    component_creation::Processor<PP, G> &&
    component_deletion::Processor<PP, G> &&
    pulling::Processor<PP, G>;

}  // namespace graffine::transforms

#endif  // GRAFFINE_TRANSFORMS_PROPERTY_PROCESSOR_H
