#include "geometry/Circle.h"

#include <cmath>
#include <stdexcept>

namespace minicad {

namespace {
constexpr double PI = 3.14159265358979323846;
}

Circle::Circle(const Vector2D& center, double radius)
    : center_(center), radius_(radius) {
    if (radius <= 0.0) {
        throw std::invalid_argument("Circle: radius must be strictly positive");
    }
}

void Circle::setRadius(double radius) {
    if (radius <= 0.0) {
        throw std::invalid_argument("Circle::setRadius: radius must be strictly positive");
    }
    radius_ = radius;
}

std::unique_ptr<Shape> Circle::clone() const {
    return std::make_unique<Circle>(*this);
}

double Circle::getArea() const {
    return PI * radius_ * radius_;
}

double Circle::getPerimeter() const {
    return 2.0 * PI * radius_;
}

AABB Circle::getBoundingBox() const {
    return AABB(
        Vector2D{center_.x - radius_, center_.y - radius_},
        Vector2D{center_.x + radius_, center_.y + radius_}
    );
}

Vector2D Circle::getCentroid() const {
    return center_;
}

void Circle::translate(const Vector2D& offset) {
    center_ += offset;
}

void Circle::rotate(double radians, const Vector2D& pivot) {
    // Circle is radially symmetric; rotating around pivot shifts its center
    center_ = center_.rotated(radians, pivot);
}

void Circle::scale(double factor, const Vector2D& pivot) {
    if (factor <= 0.0) {
        throw std::invalid_argument("Circle::scale: scale factor must be strictly positive");
    }
    radius_ *= factor;
    center_ = pivot + (center_ - pivot) * factor;
}

void Circle::accept(ShapeVisitor& visitor) {
    visitor.visit(*this);
}

void Circle::accept(ConstShapeVisitor& visitor) const {
    visitor.visit(*this);
}

} // namespace minicad
