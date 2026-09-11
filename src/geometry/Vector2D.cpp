#include "geometry/Vector2D.h"

#include <cmath>
#include <iostream>
#include <stdexcept>

namespace minicad {

Vector2D Vector2D::operator/(double scalar) const {
    if (std::abs(scalar) < 1e-14) {
        throw std::invalid_argument("Vector2D::operator/: division by near-zero scalar");
    }
    const double inv = 1.0 / scalar;
    return Vector2D{x * inv, y * inv};
}

Vector2D& Vector2D::operator/=(double scalar) {
    if (std::abs(scalar) < 1e-14) {
        throw std::invalid_argument("Vector2D::operator/=: division by near-zero scalar");
    }
    const double inv = 1.0 / scalar;
    x *= inv;
    y *= inv;
    return *this;
}

double Vector2D::length() const noexcept {
    // std::hypot(x, y) prevents intermediate arithmetic overflow/underflow
    // (e.g. when x or y is near 1e160 or 1e-160) compared to sqrt(x^2 + y^2).
    return std::hypot(x, y);
}

Vector2D Vector2D::normalized() const {
    const double len = length();
    if (len < 1e-14) {
        throw std::runtime_error("Vector2D::normalized: cannot normalize degenerate zero-length vector");
    }
    const double invLen = 1.0 / len;
    return Vector2D{x * invLen, y * invLen};
}

double Vector2D::distanceTo(const Vector2D& other) const noexcept {
    return std::hypot(x - other.x, y - other.y);
}

Vector2D Vector2D::rotated(double radians) const noexcept {
    const double cosTheta = std::cos(radians);
    const double sinTheta = std::sin(radians);
    return Vector2D{
        x * cosTheta - y * sinTheta,
        x * sinTheta + y * cosTheta
    };
}

Vector2D Vector2D::rotated(double radians, const Vector2D& pivot) const noexcept {
    // Standard affine rotation around pivot: translate to origin, rotate, translate back
    return (*this - pivot).rotated(radians) + pivot;
}

double Vector2D::angle() const noexcept {
    return std::atan2(y, x);
}

bool Vector2D::equals(const Vector2D& other, double epsilon) const noexcept {
    return std::abs(x - other.x) <= epsilon && std::abs(y - other.y) <= epsilon;
}

std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
    os << "Vector2D(" << v.x << ", " << v.y << ")";
    return os;
}

} // namespace minicad
