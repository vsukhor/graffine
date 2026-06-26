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

/**
 * Contains definitions and utilities for vertex degrees in the graph.
 * \details Defines the maximal degree, checks for implemented degrees,
 * and categorizes degrees into chain interior, leaf, junction, and end degrees.
 */
namespace graffine::structure::descriptors::vertex_degrees {

using Degree = unsigned int;

inline constexpr auto Deg0 = static_cast<Degree>(0);  ///< Degree 0.
inline constexpr auto Deg1 = static_cast<Degree>(1);  ///< Degree 1.
inline constexpr auto Deg2 = static_cast<Degree>(2);  ///< Degree 2.
inline constexpr auto Deg3 = static_cast<Degree>(3);  ///< Degree 3.
inline constexpr auto Deg4 = static_cast<Degree>(4);  ///< Degree 4.
inline constexpr auto DegUndef = utils::undefined<Degree>;  ///< Degree Undefined.

inline constexpr std::array degrees {
    Deg1,
    Deg2,
    Deg3,
    Deg4
};
inline constexpr auto numDegrees = degrees.size();

///< Maximal vertex degree.
inline constexpr auto maxDegree {degrees.back()};

/**
 * Checks if a degree is implemented.
 * \tparam D Degree to check.
 * \return True if implemented, false otherwise.
 * \details A degree is implemented if it is less than or equal to maxDegree.
 */
template <Degree D>
constexpr auto isImplementedDegree =
    std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();

constexpr bool is_implemented_degree(const Degree D)
{
    return std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();
}

template <Degree D>
concept ImplementedDegree =
    std::find(degrees.cbegin(), degrees.cend(), D) != degrees.cend();

// Leaf vertex degree.

inline constexpr auto leafDegree = Deg1;

template <Degree D>
inline constexpr auto isLeafDegree = D == leafDegree;

template<Degree D>
concept EndDegree = D != Deg2;

// Degree of a chain internal vertex.

inline constexpr auto bulkDegree = Deg2;

template<Degree D>
concept BulkDegree = D == Deg2;

template<Degree D>
constexpr auto isBulkDegree = D == Deg2;


constexpr bool is_bulk_degree(const Degree D)
{
    return D == Deg2;
}

// Junction degrees and types.

inline constexpr std::array junctionDegrees {Deg3, Deg4};

template <Degree D>
constexpr auto isJunctionDegree =
    std::find(junctionDegrees.cbegin(), junctionDegrees.cend(), D) !=
    junctionDegrees.cend();

/**
 * Checks if the vertices two degrees are compatible.
 * \tparam D1 First degree.
 * \tparam D2 Second degree.
 */
template <Degree D1,
          Degree D2>
constexpr auto areCompatibleDegrees =
    D1 + D2 <= maxDegree;


}  // namespace graffine::structure::descriptors::vertex_degrees


#endif  // GRAFFINE_STRUCTURE_DESCRIPTORS_VERTEX_DEGREES_H
