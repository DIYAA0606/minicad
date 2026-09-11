#pragma once

#include <cmath>
#include <iosfwd>

namespace minicad {

/**
 * @brief Represents a 2D Euclidean vector or geometric point in Cartesian space.
 *
 * Design Decisions & Architectural Rationale:
 * 1. Struct vs Class: Vector2D is an algebraic value type, not an encapsulated
 *    entity with complex internal invariants. Public `x` and `y` allow direct,
 *    ergonomic mathematical syntax (e.g., `v.x` instead of `v.getX()`), aggregate-like
 *    clarity, and standard-layout compliance.
 * 2. Value Semantics & Memory Layout: With exactly two 64-bit IEEE 754 doubles (16 bytes),
 *    Vector2D fits naturally into two CPU registers (or one 128-bit SIMD register). It is
 *    trivially copyable, standard layout, and has no heap overhead.
 * 3. Invariance & Const-Correctness: All non-modifying operations are marked `const noexcept`
 *    to enable aggressive compiler inlining and vectorization.
 */
struct Vector2D {
    double x{0.0};
    double y{0.0};

    /// Default constructor: initializes to the origin (0, 0).
    constexpr Vector2D() noexcept = default;

    /// Explicit component constructor.
    constexpr Vector2D(double xVal, double yVal) noexcept : x(xVal), y(yVal) {}

    // --- Vector Arithmetic Operators ---

    [[nodiscard]] constexpr Vector2D operator+(const Vector2D& other) const noexcept {
        return Vector2D{x + other.x, y + other.y};
    }

    [[nodiscard]] constexpr Vector2D operator-(const Vector2D& other) const noexcept {
        return Vector2D{x - other.x, y - other.y};
    }

    [[nodiscard]] constexpr Vector2D operator*(double scalar) const noexcept {
        return Vector2D{x * scalar, y * scalar};
    }

    [[nodiscard]] Vector2D operator/(double scalar) const;

    [[nodiscard]] constexpr Vector2D operator-() const noexcept {
        return Vector2D{-x, -y};
    }

    constexpr Vector2D& operator+=(const Vector2D& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2D& operator-=(const Vector2D& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2D& operator*=(double scalar) noexcept {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2D& operator/=(double scalar);

    // --- Geometric & Algebraic Operations ---

    /**
     * @brief Computes the Euclidean dot product: u . v = u_x * v_x + u_y * v_y.
     * Geometrically, u . v = |u| |v| cos(theta).
     */
    [[nodiscard]] constexpr double dot(const Vector2D& other) const noexcept {
        return x * other.x + y * other.y;
    }

    /**
     * @brief Computes the 2D cross product (perp-dot product): u x v = u_x * v_y - u_y * v_x.
     *
     * In 2D, the cross product represents the z-component of the 3D cross product of
     * vectors embedded in the XY plane.
     * Geometrical significance in CAD:
     * - Equal to the signed area of the parallelogram spanned by the two vectors.
     * - Sign indicates orientation: > 0 means `other` turns counter-clockwise from `this`,
     *   < 0 means clockwise, and 0 means colinear. Essential for winding order and SAT.
     */
    [[nodiscard]] constexpr double cross(const Vector2D& other) const noexcept {
        return x * other.y - y * other.x;
    }

    /**
     * @brief Computes the squared magnitude: ||v||^2 = x^2 + y^2.
     *
     * Rationale: std::sqrt is a relatively expensive floating-point instruction.
     * When comparing distances (e.g. nearest neighbor or bounding sphere intersection),
     * comparing squared distances avoids the square root computation entirely.
     */
    [[nodiscard]] constexpr double lengthSquared() const noexcept {
        return x * x + y * y;
    }

    /**
     * @brief Computes the Euclidean magnitude / norm: ||v|| = sqrt(x^2 + y^2).
     */
    [[nodiscard]] double length() const noexcept;

    /**
     * @brief Returns a unit vector pointing in the same direction.
     * @throws std::runtime_error if length is approximately 0 (degenerate vector).
     */
    [[nodiscard]] Vector2D normalized() const;

    /**
     * @brief Returns a perpendicular normal vector rotated 90 degrees counter-clockwise: (-y, x).
     *
     * Rationale: Used to generate edge normals during Separating Axis Theorem (SAT) tests.
     */
    [[nodiscard]] constexpr Vector2D perpendicular() const noexcept {
        return Vector2D{-y, x};
    }

    /**
     * @brief Euclidean distance to another point: ||this - other||.
     */
    [[nodiscard]] double distanceTo(const Vector2D& other) const noexcept;

    /**
     * @brief Squared Euclidean distance to another point. Avoids std::sqrt.
     */
    [[nodiscard]] constexpr double distanceSquaredTo(const Vector2D& other) const noexcept {
        const double dx = x - other.x;
        const double dy = y - other.y;
        return dx * dx + dy * dy;
    }

    /**
     * @brief Rotates the vector around the origin (0, 0) by an angle in radians.
     * Rotation formula: [x', y']^T = [cos -sin; sin cos] * [x, y]^T.
     */
    [[nodiscard]] Vector2D rotated(double radians) const noexcept;

    /**
     * @brief Rotates the point around a specified pivot center by an angle in radians.
     */
    [[nodiscard]] Vector2D rotated(double radians, const Vector2D& pivot) const noexcept;

    /**
     * @brief Polar angle of the vector in radians [-pi, pi], measured from +X axis.
     */
    [[nodiscard]] double angle() const noexcept;

    // --- Comparisons & Tolerances ---

    /**
     * @brief Floating-point approximate equality with an absolute tolerance epsilon.
     *
     * Rationale: In CAD engines, finite-precision floating point arithmetic produces
     * rounding errors (e.g. 0.1 + 0.2 != 0.3). Direct bitwise equality (`==`) fails
     * on mathematically identical coordinates computed via different paths.
     */
    [[nodiscard]] bool equals(const Vector2D& other, double epsilon = 1e-9) const noexcept;

    [[nodiscard]] bool operator==(const Vector2D& other) const noexcept {
        return equals(other);
    }

    [[nodiscard]] bool operator!=(const Vector2D& other) const noexcept {
        return !(*this == other);
    }
};

// Commutative scalar multiplication: 2.0 * v
[[nodiscard]] constexpr Vector2D operator*(double scalar, const Vector2D& v) noexcept {
    return v * scalar;
}

// Stream insertion operator for debugging and logging
std::ostream& operator<<(std::ostream& os, const Vector2D& v);

} // namespace minicad
