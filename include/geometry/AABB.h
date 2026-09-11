#pragma once

#include "geometry/Vector2D.h"

#include <algorithm>
#include <limits>

namespace minicad {

/**
 * @brief Represents an Axis-Aligned Bounding Box (AABB) in 2D space.
 *
 * Design Decisions & Architectural Rationale:
 * - Broadphase Acceleration: In CAD, spatial indexing and collision detection
 *   are hierarchical. Calculating exact intersections (e.g. SAT, ray-casting)
 *   is O(V) or O(V1 * V2). An AABB overlap check is O(1) and eliminates
 *   up to 90%+ of non-intersecting candidate pairs prior to expensive narrowphase checks.
 * - Value Type: Consists of two Vector2D points (minPt and maxPt), 32 bytes total.
 */
struct AABB {
    Vector2D minPt;
    Vector2D maxPt;

    /// Default constructor: creates an inverted box that can be expanded by any point.
    constexpr AABB() noexcept
        : minPt(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()),
          maxPt(-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()) {}

    /// Explicit min/max constructor.
    constexpr AABB(const Vector2D& minPoint, const Vector2D& maxPoint) noexcept
        : minPt(minPoint), maxPt(maxPoint) {}

    [[nodiscard]] constexpr double width() const noexcept {
        return maxPt.x - minPt.x;
    }

    [[nodiscard]] constexpr double height() const noexcept {
        return maxPt.y - minPt.y;
    }

    [[nodiscard]] constexpr Vector2D center() const noexcept {
        return Vector2D{(minPt.x + maxPt.x) * 0.5, (minPt.y + maxPt.y) * 0.5};
    }

    [[nodiscard]] constexpr bool isValid() const noexcept {
        return minPt.x <= maxPt.x && minPt.y <= maxPt.y;
    }

    /**
     * @brief Tests if a point lies within the bounding box (inclusive of boundary).
     */
    [[nodiscard]] constexpr bool contains(const Vector2D& point) const noexcept {
        return point.x >= minPt.x && point.x <= maxPt.x &&
               point.y >= minPt.y && point.y <= maxPt.y;
    }

    /**
     * @brief Fast O(1) interval overlap test on both axes.
     * Two AABBs intersect if and only if their projections overlap on both X and Y.
     */
    [[nodiscard]] constexpr bool intersects(const AABB& other) const noexcept {
        return (minPt.x <= other.maxPt.x && maxPt.x >= other.minPt.x) &&
               (minPt.y <= other.maxPt.y && maxPt.y >= other.minPt.y);
    }

    /**
     * @brief Expands the bounding box to enclose the given point.
     */
    void expandToInclude(const Vector2D& point) noexcept {
        minPt.x = std::min(minPt.x, point.x);
        minPt.y = std::min(minPt.y, point.y);
        maxPt.x = std::max(maxPt.x, point.x);
        maxPt.y = std::max(maxPt.y, point.y);
    }

    /**
     * @brief Expands the bounding box to enclose another AABB (union operation).
     */
    void expandToInclude(const AABB& other) noexcept {
        minPt.x = std::min(minPt.x, other.minPt.x);
        minPt.y = std::min(minPt.y, other.minPt.y);
        maxPt.x = std::max(maxPt.x, other.maxPt.x);
        maxPt.y = std::max(maxPt.y, other.maxPt.y);
    }
};

} // namespace minicad
