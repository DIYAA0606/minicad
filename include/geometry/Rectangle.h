#pragma once

#include "geometry/Shape.h"

#include <vector>

namespace minicad {

/**
 * @brief Represents an oriented 2D rectangle with center, dimensions, and rotation angle.
 */
class Rectangle : public Shape {
public:
    /**
     * @brief Constructs an oriented rectangle with center, width, height, and rotation.
     * @throws std::invalid_argument if width <= 0 or height <= 0.
     */
    Rectangle(const Vector2D& center, double width, double height, double angle = 0.0);

    /**
     * @brief Constructs an axis-aligned rectangle from two opposing corner points.
     * @throws std::invalid_argument if minPoint.x >= maxPoint.x or minPoint.y >= maxPoint.y.
     */
    Rectangle(const Vector2D& minPoint, const Vector2D& maxPoint);

    Rectangle(const Rectangle& other)
        : Shape(), center_(other.center_), width_(other.width_), height_(other.height_), angle_(other.angle_) {}

    [[nodiscard]] std::unique_ptr<Shape> clone() const override;

    [[nodiscard]] double getArea() const override;
    [[nodiscard]] double getPerimeter() const override;
    [[nodiscard]] AABB getBoundingBox() const override;
    [[nodiscard]] Vector2D getCentroid() const override;
    [[nodiscard]] std::string getTypeName() const override { return "Rectangle"; }

    void translate(const Vector2D& offset) override;
    void rotate(double radians, const Vector2D& pivot = {0.0, 0.0}) override;
    void scale(double factor, const Vector2D& pivot = {0.0, 0.0}) override;

    void accept(ShapeVisitor& visitor) override;
    void accept(ConstShapeVisitor& visitor) const override;

    // --- Rectangle-Specific Methods & Accessors ---
    [[nodiscard]] const Vector2D& getCenter() const noexcept { return center_; }
    [[nodiscard]] double getWidth() const noexcept { return width_; }
    [[nodiscard]] double getHeight() const noexcept { return height_; }
    [[nodiscard]] double getAngle() const noexcept { return angle_; }

    void setCenter(const Vector2D& center) noexcept { center_ = center; }
    void setDimensions(double width, double height);
    void setAngle(double angle) noexcept { angle_ = angle; }

    /**
     * @brief Computes the 4 corners of the rectangle in counter-clockwise order:
     * [0] Bottom-Left, [1] Bottom-Right, [2] Top-Right, [3] Top-Left.
     */
    [[nodiscard]] std::vector<Vector2D> getVertices() const;

private:
    Vector2D center_{0.0, 0.0};
    double width_{1.0};
    double height_{1.0};
    double angle_{0.0}; // in radians
};

} // namespace minicad
