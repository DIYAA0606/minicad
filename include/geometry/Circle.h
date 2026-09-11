#pragma once

#include "geometry/Shape.h"

namespace minicad {

/**
 * @brief Represents a 2D circle defined by a center position and radius.
 */
class Circle : public Shape {
public:
    /**
     * @brief Constructs a Circle with the given center and radius.
     * @throws std::invalid_argument if radius <= 0.
     */
    Circle(const Vector2D& center, double radius);

    Circle(const Circle& other) : Shape(), center_(other.center_), radius_(other.radius_) {}

    [[nodiscard]] std::unique_ptr<Shape> clone() const override;

    [[nodiscard]] double getArea() const override;
    [[nodiscard]] double getPerimeter() const override;
    [[nodiscard]] AABB getBoundingBox() const override;
    [[nodiscard]] Vector2D getCentroid() const override;
    [[nodiscard]] std::string getTypeName() const override { return "Circle"; }

    void translate(const Vector2D& offset) override;
    void rotate(double radians, const Vector2D& pivot = {0.0, 0.0}) override;
    void scale(double factor, const Vector2D& pivot = {0.0, 0.0}) override;

    void accept(ShapeVisitor& visitor) override;
    void accept(ConstShapeVisitor& visitor) const override;

    // --- Circle-Specific Accessors ---
    [[nodiscard]] const Vector2D& getCenter() const noexcept { return center_; }
    [[nodiscard]] double getRadius() const noexcept { return radius_; }
    void setCenter(const Vector2D& center) noexcept { center_ = center; }
    void setRadius(double radius);

private:
    Vector2D center_{0.0, 0.0};
    double radius_{1.0};
};

} // namespace minicad
