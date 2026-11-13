/**
 * \file vertex_degrees.h
 * \brief Vertex degrees supported by graffine, and their classification.
 * \author Valerii Sukhorukov <vsukhorukov@yahoo.com>
 */

#ifndef GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_DEGREES_H
#define GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_DEGREES_H

#include <array>
#include <algorithm>  // for std::find
#include <cassert>
#include <string>
#include <type_traits>  // underlying_type
#include <utility>  // unreachable
#include <vector>

// does not require definitions.h

/** Definitions and utilities for vertex degrees.
 * \details The degree of a vertex is the number of edge ends meeting at it.
 * graffine supports degrees 1 to 4. They fall into three categories:
 * - degree 1: a \e leaf, i.e. a free end of the graph;
 * - degree 2: a \e bulk vertex, internal to a chain;
 * - degrees 3 and 4: a \e junction, where chain ends meet.
 *
 * Leaves and junctions lie at chain ends.
 */
namespace graffine::structure::descriptors::vertex_degrees {

using Degree = unsigned int;  ///< Type of vertex degrees.

inline constexpr auto Deg0 = static_cast<Degree>(0);  ///< Degree 0.
inline constexpr auto Deg1 = static_cast<Degree>(1);  ///< Degree 1.
inline constexpr auto Deg2 = static_cast<Degree>(2);  ///< Degree 2.
inline constexpr auto Deg3 = static_cast<Degree>(3);  ///< Degree 3.
inline constexpr auto Deg4 = static_cast<Degree>(4);  ///< Degree 4.
inline constexpr auto DegUndef = utils::undefined<Degree>;  ///< Degree Undefined.

/// The degrees supported, in increasing order.
inline constexpr std::array degrees {
    Deg1,
    Deg2,
    Deg3,
    Deg4
};
inline constexpr auto numDegrees = degrees.size();  ///< Number of supported degrees.

/// Maximal vertex degree.
inline constexpr auto maxDegree {degrees.back()};

/** True if degree \p D is supported, i.e. is one of \c degrees.
 * \tparam D Degree to check.
 */
template <Degree D>
constexpr auto isImplementedDegree =
    std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();

/// Checks if degree \p D is supported, i.e. is one of \c degrees.
constexpr bool is_implemented_degree(const Degree D)
{
    return std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();
}

/// A supported degree, i.e. one of \c degrees.
template <Degree D>
concept ImplementedDegree =
    std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();

// Leaf vertex degree.

inline constexpr auto leafDegree = Deg1;  ///< Degree of a leaf (free end).

/// True if \p D is the leaf degree.
template <Degree D>
inline constexpr auto isLeafDegree = D == leafDegree;

/// A degree of a vertex at a chain end, i.e. any but the bulk degree.
template<Degree D>
concept EndDegree = D != Deg2;

// Degree of a chain internal vertex.

inline constexpr auto bulkDegree = Deg2;  ///< Degree of a vertex inside a chain.

/// The bulk degree.
template<Degree D>
concept BulkDegree = D == Deg2;

/// True if \p D is the bulk degree.
template<Degree D>
constexpr auto isBulkDegree = D == Deg2;


/// Checks if \p D is the bulk degree.
constexpr bool is_bulk_degree(const Degree D)
{
    return D == Deg2;
}

// Junction degrees and types.

/// Degrees of junctions, i.e. of branching points joining chain ends.
inline constexpr std::array junctionDegrees {Deg3, Deg4};

/// True if \p D is a junction degree.
template <Degree D>
constexpr auto isJunctionDegree =
    std::find(junctionDegrees.cbegin(), junctionDegrees.cend(), D) !=
    junctionDegrees.cend();

/** True if vertices of degrees \p D1 and \p D2 can be merged, i.e. if the
 * degree D1 + D2 of the resulting vertex does not exceed \ref maxDegree.
 * \tparam D1 First degree.
 * \tparam D2 Second degree.
 */
template <Degree D1,
          Degree D2>
constexpr auto areCompatibleDegrees =
    D1 + D2 <= maxDegree;


}  // namespace graffine::structure::descriptors::vertex_degrees


#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_DEGREES_H
