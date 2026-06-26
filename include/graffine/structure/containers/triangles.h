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
 * \file triangles.h
 * \brief Implements container holding triangles.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H
#define GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H


#include <cmath>
#include <vector>

#include "graffine/definitions.h"


namespace graffine::structure {


struct Triangle
{
    static constexpr std::size_t numVertices {3};

    using VIds = std::array<VIdH, numVertices>;  // Indices of the three vertices

};

using TriVIds = Triangle::VIds;


//
// Data structure representing a collection of triangles.
//
struct Triangles
{
//    using Positions = std::vector<Triangle::VPos>;
    using VertIds = std::vector<Triangle::VIds>;

//    Positions verts;  ///< Positions of the three vertices.

    /// Indices in verts of the tree vertices: [number of triangles x 3]
    VertIds tris;

    constexpr std::size_t num() const
    {
//        assert(tris.size() == Triangle::numVertices * verts.size());

        return tris.size();
    }
/*
    constexpr
    BoundingBox bounding_box() const
    {
        BoundingBox b;

        for (const auto& u : verts)
            for (int j = 0; j < 3; ++j) {
                if (u[j] < b.min[j]) b.min[j] = u[j];  // min
                if (u[j] > b.max[j]) b.max[j] = u[j];  // max
            }

        return b;
    }


    constexpr
    Vec3r centroid() const
    {
        const auto bb = bounding_box();

        auto ctr = bb.min + (bb.max - bb.min) * 0.5f;

        jot("centroid tri: ", ctr[0], " ", ctr[1], " ", ctr[2]);

        return ctr;
    }


    void print(const std::string& s,
               const TrId ti) const
    {
        const auto& t = tris[ti];
        jot<false>(s, "[", ti, "] ");
        for (const auto vi : t)
            jot<false>(vi, "{", verts[vi][0], " ",
                                 verts[vi][1], " ",
                                 verts[vi][2], "} ");
        jot();
    }

    void print(const std::string& s1,
               const std::string& s2) const
    {
        jot(s1);
        for (TrId ti {}; ti <tris.size(); ++ti)
            print(s2, ti);
    }
*/
};

}  // namespace graffine::structure

#endif  // GRAFFINE_STRUCTURE_CONTAINERS_TRIANGLES_H