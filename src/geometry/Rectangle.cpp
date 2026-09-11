#include "geometry/Rectangle.h"

#include <cmath>
#include <stdexcept>

namespace minicad {

Rectangle::Rectangle(const Vector2D& center, double width, double height, double angle)
    : center_(center), width_(width), height_(height), angle_(angle) {
    if (width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("Rectangle: width and height must be strictly positive");
    }
}

Rectangle::Rectangle(const Vector2D& minPoint, const Vector2D& maxPoint)
    : center_{(minPoint.x + maxPoint.x) * 0.5, (minPoint.y + maxPoint.y) * 0.5},
      width_{maxPoint.x - minPoint.x},
      height_{maxPoint.y - minPoint.y},
      angle_{0.0} {
    if (width_ <= 0.0 || height_ <= 0.0) {
        throw std::invalid_argument("Rectangle: maxPoint must be strictly greater than minPoint");
    }
}

void Rectangle::setDimensions(double width, double height) {
    if (width <= 0.0 || height <= 0.0) {
        throw std::invalid_argument("Rectangle::setDimensions: width and height must be strictly positive");
    }
    width_ = width;
    height_ = height;
}

std::unique_ptr<Shape> Rectangle::clone() const {
    return std::make_unique<Rectangle>(*this);
}

double Rectangle::getArea() const {
    return width_ * height_;
}

double Rectangle::getPerimeter() const {
    return 2.0 * (width_ + height_);
}

std::vector<Vector2D> Rectangle::getVertices() const {
    const double halfW = width_ * 0.5;
    const double halfH = height_ * 0.5;

    const Vector2D localCorners[4] = {
        {-halfW, -halfH},
        { halfW, -halfH},
        { halfW,  halfH},
        {-halfW,  halfH}
    };

    std::vector<Vector2D> vertices;
    vertices.reserve(4);
    for (const auto& localPt : localCorners) {
        vertices.push_back(center_ + localPt.rotated(angle_));
    }
    return vertices;
}

AABB Rectangle::getBoundingBox() const {
    const auto vertices = getVertices();
    AABB box;
    for (const auto& v : vertices) {
        box.expandToInclude(v);
    }
    return box;
}

Vector2D Rectangle::getCentroid() const {
    return center_;
}

void Rectangle::translate(const Vector2D& offset) {
    center_ += offset;
}

void Rectangle::rotate(double radians, const Vector2D& pivot) {
    angle_ += radians;
    // Normalize angle to [0, 2*pi)
    const double twoPi = 2.0 * std::acos(-1.0);
    angle_ = std::fmod(angle_, twoPi);
    if (angle_ < 0.0) {
        angle_ += twoPi;
    }
    center_ = center_.rotated(radians, pivot);
}

void Rectangle::scale(double factor, const Vector2D& pivot) {
    if (factor <= 0.0) {
        throw std::invalid_argument("Rectangle::scale: factor must be strictly positive");
    }
    width_ *= factor;
    height_ *= factor;
    center_ = pivot + (center_ - pivot) * factor;
}

void Rectangle::accept(ShapeVisitor& visitor) {
    visitor.visit(*this);
}

void Rectangle::accept(ConstShapeVisitor& visitor) const {
    visitor.visit(*this);
}

} // namespace minicad
