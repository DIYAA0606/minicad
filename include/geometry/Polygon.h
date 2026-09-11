#pragma once

#include "geometry/Shape.h"

#include <vector>

namespace minicad {

/**
 * @brief Represents an arbitrary 2D polygon with at least 3 vertices.
 *
 * Supports both convex and concave simple polygons.
 * Calculations for area, centroid, and winding order are mathematically exact
 * and utilize Gauss's area formula (Shoelace theorem).
 */
class Polygon : public Shape {
public:
    /**
     * @brief Constructs a Polygon from a sequence of vertices.
     * @throws std::invalid_argument if vertices.size() < 3.
     */
    explicit Polygon(std::vector<Vector2D> vertices);

    Polygon(const Polygon& other)
        : Shape(), vertices_(other.vertices_) {}

    [[nodiscard]] std::unique_ptr<Shape> clone() const override;

    [[nodiscard]] double getArea() const override;
    [[nodiscard]] double getPerimeter() const override;
    [[nodiscard]] AABB getBoundingBox() const override;
    [[nodiscard]] Vector2D getCentroid() const override;
    [[nodiscard]] std::string getTypeName() const override { return "Polygon"; }

    void translate(const Vector2D& offset) override;
    void rotate(double radians, const Vector2D& pivot = {0.0, 0.0}) override;
    void scale(double factor, const Vector2D& pivot = {0.0, 0.0}) override;

    void accept(ShapeVisitor& visitor) override;
    void accept(ConstShapeVisitor& visitor) const override;

    // --- Polygon-Specific Geometric Analysis ---
    [[nodiscard]] const std::vector<Vector2D>& getVertices() const noexcept { return vertices_; }
    [[nodiscard]] size_t getVertexCount() const noexcept { return vertices_.size(); }

    /**
     * @brief Tests whether the polygon is strictly convex.
     * Evaluates whether consecutive edge cross-products maintain a uniform sign.
     */
    [[nodiscard]] bool isConvex() const noexcept;

    /**
     * @brief Returns true if vertices are arranged in counter-clockwise (CCW) order.
     * Determined by the sign of the Shoelace signed area.
     */
    [[nodiscard]] bool isCounterClockwise() const noexcept;

    /**
     * @brief Reverses vertices in-place if they are oriented clockwise.
     */
    void ensureCounterClockwise() noexcept;

    /**
     * @brief Returns outward-facing unit normals for all edges.
     * Assumes vertices are oriented counter-clockwise.
     */
    [[nodiscard]] std::vector<Vector2D> getEdgeNormals() const;

private:
    std::vector<Vector2D> vertices_;
};

} // namespace minicad
