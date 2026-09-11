#include "geometry/Polygon.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace minicad {

Polygon::Polygon(std::vector<Vector2D> vertices)
    : vertices_(std::move(vertices)) {
    if (vertices_.size() < 3) {
        throw std::invalid_argument("Polygon: must have at least 3 vertices");
    }
}

std::unique_ptr<Shape> Polygon::clone() const {
    return std::make_unique<Polygon>(*this);
}

double Polygon::getArea() const {
    // Gauss's area formula (Shoelace formula) using 2D cross products:
    // Signed Area = 0.5 * sum_{i=0}^{n-1} (v_i x v_{i+1})
    double signedArea = 0.0;
    const size_t n = vertices_.size();
    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        signedArea += vertices_[i].cross(vertices_[next]);
    }
    return 0.5 * std::abs(signedArea);
}

double Polygon::getPerimeter() const {
    double perim = 0.0;
    const size_t n = vertices_.size();
    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        perim += vertices_[i].distanceTo(vertices_[next]);
    }
    return perim;
}

AABB Polygon::getBoundingBox() const {
    AABB box;
    for (const auto& v : vertices_) {
        box.expandToInclude(v);
    }
    return box;
}

Vector2D Polygon::getCentroid() const {
    // Exact planar polygon area centroid (center of mass):
    // C_x = (1 / 6A) * sum (x_i + x_{i+1}) * (x_i * y_{i+1} - x_{i+1} * y_i)
    // C_y = (1 / 6A) * sum (y_i + y_{i+1}) * (x_i * y_{i+1} - x_{i+1} * y_i)
    const size_t n = vertices_.size();
    double signedAreaSum = 0.0;
    double cx = 0.0;
    double cy = 0.0;

    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        const double crossTerm = vertices_[i].cross(vertices_[next]);
        signedAreaSum += crossTerm;
        cx += (vertices_[i].x + vertices_[next].x) * crossTerm;
        cy += (vertices_[i].y + vertices_[next].y) * crossTerm;
    }

    const double signedArea = 0.5 * signedAreaSum;
    if (std::abs(signedArea) < 1e-12) {
        // Degenerate colinear polygon: fallback to arithmetic mean of vertices
        Vector2D mean{0.0, 0.0};
        for (const auto& v : vertices_) {
            mean += v;
        }
        return mean / static_cast<double>(n);
    }

    const double factor = 1.0 / (6.0 * signedArea);
    return Vector2D{cx * factor, cy * factor};
}

void Polygon::translate(const Vector2D& offset) {
    for (auto& v : vertices_) {
        v += offset;
    }
}

void Polygon::rotate(double radians, const Vector2D& pivot) {
    for (auto& v : vertices_) {
        v = v.rotated(radians, pivot);
    }
}

void Polygon::scale(double factor, const Vector2D& pivot) {
    if (factor <= 0.0) {
        throw std::invalid_argument("Polygon::scale: scale factor must be strictly positive");
    }
    for (auto& v : vertices_) {
        v = pivot + (v - pivot) * factor;
    }
}

bool Polygon::isConvex() const noexcept {
    const size_t n = vertices_.size();
    if (n < 3) return false;

    bool hasPositive = false;
    bool hasNegative = false;

    for (size_t i = 0; i < n; ++i) {
        const size_t prev = (i + n - 1) % n;
        const size_t next = (i + 1) % n;
        const Vector2D edge1 = vertices_[i] - vertices_[prev];
        const Vector2D edge2 = vertices_[next] - vertices_[i];
        const double cp = edge1.cross(edge2);

        if (cp > 1e-9) hasPositive = true;
        if (cp < -1e-9) hasNegative = true;

        if (hasPositive && hasNegative) {
            return false; // Sign change indicates a reflex/concave vertex
        }
    }
    return true;
}

bool Polygon::isCounterClockwise() const noexcept {
    double signedArea = 0.0;
    const size_t n = vertices_.size();
    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        signedArea += vertices_[i].cross(vertices_[next]);
    }
    return signedArea > 0.0;
}

void Polygon::ensureCounterClockwise() noexcept {
    if (!isCounterClockwise()) {
        std::reverse(vertices_.begin(), vertices_.end());
    }
}

std::vector<Vector2D> Polygon::getEdgeNormals() const {
    const size_t n = vertices_.size();
    std::vector<Vector2D> normals;
    normals.reserve(n);

    // Ensure counter-clockwise orientation for consistent outward normal direction
    const bool ccw = isCounterClockwise();

    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        const Vector2D edge = vertices_[next] - vertices_[i];
        // For CCW polygon, outward normal is (edge.y, -edge.x)
        // For CW polygon, outward normal is (-edge.y, edge.x)
        Vector2D normal = ccw ? Vector2D{edge.y, -edge.x} : Vector2D{-edge.y, edge.x};
        normals.push_back(normal.normalized());
    }
    return normals;
}

void Polygon::accept(ShapeVisitor& visitor) {
    visitor.visit(*this);
}

void Polygon::accept(ConstShapeVisitor& visitor) const {
    visitor.visit(*this);
}

} // namespace minicad
