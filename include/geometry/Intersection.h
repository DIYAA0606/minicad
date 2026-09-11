#pragma once

#include "geometry/Circle.h"
#include "geometry/Polygon.h"
#include "geometry/Rectangle.h"
#include "geometry/Shape.h"
#include "geometry/Vector2D.h"

#include <vector>

namespace minicad {

/**
 * @brief Point-to-line-segment projection and squared distance calculation.
 *
 * Given a line segment A -> B and a point P:
 * Computes closest point Q on segment [A, B] by clamping the scalar projection t:
 *   t = ((P - A) . (B - A)) / ||B - A||^2,  clamped to [0, 1].
 *   Q = A + t * (B - A).
 * Returns ||P - Q||^2.
 */
[[nodiscard]] double distanceSquaredPointToSegment(
    const Vector2D& point,
    const Vector2D& segA,
    const Vector2D& segB
) noexcept;

// --- Point-in-Shape Containment Algorithms ---

/**
 * @brief Tests if a point lies inside or on the boundary of a Circle.
 * Algorithmic complexity: O(1).
 */
[[nodiscard]] bool pointInCircle(const Circle& circle, const Vector2D& point) noexcept;

/**
 * @brief Tests if a point lies inside or on the boundary of an oriented Rectangle.
 * Transforms point into the rectangle's local coordinate frame.
 * Algorithmic complexity: O(1).
 */
[[nodiscard]] bool pointInRectangle(const Rectangle& rectangle, const Vector2D& point) noexcept;

/**
 * @brief Tests if a point lies inside or on the boundary of an arbitrary Polygon.
 * Implements the Ray-Casting algorithm (Jordan curve theorem / Even-Odd rule).
 * Works for both convex and concave simple polygons.
 * Algorithmic complexity: O(N) where N is the vertex count.
 */
[[nodiscard]] bool pointInPolygon(const Polygon& polygon, const Vector2D& point) noexcept;

/**
 * @brief Polymorphic point-in-shape query using Visitor double dispatch.
 */
[[nodiscard]] bool pointInShape(const Shape& shape, const Vector2D& point);

// --- Narrowphase Primitive Pairwise Intersections ---

[[nodiscard]] bool intersectsCircleCircle(const Circle& c1, const Circle& c2) noexcept;
[[nodiscard]] bool intersectsCircleRectangle(const Circle& c, const Rectangle& r) noexcept;
[[nodiscard]] bool intersectsCirclePolygon(const Circle& c, const Polygon& p) noexcept;
[[nodiscard]] bool intersectsRectangleRectangle(const Rectangle& r1, const Rectangle& r2) noexcept;
[[nodiscard]] bool intersectsRectanglePolygon(const Rectangle& r, const Polygon& p) noexcept;
[[nodiscard]] bool intersectsPolygonPolygon(const Polygon& p1, const Polygon& p2) noexcept;

// --- Separating Axis Theorem (SAT) Core Engine ---

/**
 * @brief Projects a collection of 2D vertices onto a 1D normal axis.
 * Returns the scalar interval [minProj, maxProj].
 */
void projectOntoAxis(
    const std::vector<Vector2D>& vertices,
    const Vector2D& axis,
    double& minProj,
    double& maxProj
) noexcept;

/**
 * @brief Evaluates whether two convex polygons overlap on all candidate separating axes.
 *
 * Separating Axis Theorem (SAT):
 * For any two convex shapes, if there exists a line (axis) onto which the projections
 * of the two shapes do not overlap, then the two shapes do not intersect.
 * If projections overlap on all unique face normals, the shapes are intersecting.
 *
 * Algorithmic complexity: O(V1 + V2) where V1, V2 are vertex counts.
 */
[[nodiscard]] bool satOverlap(
    const std::vector<Vector2D>& vertsA,
    const std::vector<Vector2D>& vertsB,
    const std::vector<Vector2D>& axes
) noexcept;

// --- Unified Polymorphic Shape-Shape Intersection Engine ---

/**
 * @brief Tests if any two polymorphic Shape objects intersect.
 *
 * Pipeline Architecture:
 * 1. Broadphase: Checks Axis-Aligned Bounding Box (AABB) overlap in O(1).
 *    If AABBs do not overlap, returns false immediately.
 * 2. Narrowphase: Resolves concrete shape types via symmetric double dispatch
 *    (Visitor pattern) and executes the exact mathematical intersection algorithm.
 */
[[nodiscard]] bool checkIntersection(const Shape& a, const Shape& b);

} // namespace minicad
