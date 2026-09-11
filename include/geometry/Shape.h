#pragma once

#include "geometry/AABB.h"
#include "geometry/ShapeVisitor.h"
#include "geometry/Vector2D.h"

#include <memory>
#include <string>

namespace minicad {

/**
 * @brief Abstract base class representing a 2D geometric shape in MiniCAD.
 *
 * Architectural & C++ Design Rationale:
 * 1. Virtual Destructor:
 *    Ensures proper resource cleanup when deleting derived shapes through a base
 *    pointer (`std::unique_ptr<Shape>`). Omitting a virtual destructor leads to
 *    undefined behavior under the C++ standard.
 * 2. Non-Copyable by Direct Assignment (Preventing Object Slicing):
 *    Copy constructor and copy assignment are deleted. If value copying were permitted,
 *    assigning a derived instance (e.g. `Circle`) to a `Shape` value would slice away
 *    all derived state and reset the vptr to `Shape`.
 * 3. Prototype Pattern (Virtual `clone()`):
 *    To duplicate shapes without knowing their concrete runtime types, `clone()`
 *    returns an owning `std::unique_ptr<Shape>`. This provides safe, polymorphic deep copies.
 * 4. Exclusive Ownership Semantics (`std::unique_ptr<Shape>`):
 *    Scenes and commands own their shapes uniquely. `std::unique_ptr` avoids the
 *    atomic reference-counting overhead and potential cycle leaks of `std::shared_ptr`.
 * 5. Visitor Pattern Integration:
 *    `accept()` enables open-ended operations (e.g. SVG rendering, serialization)
 *    without modifying the shape hierarchy.
 */
class Shape {
public:
    virtual ~Shape() = default;

    // Prevent accidental slicing
    Shape(const Shape&) = delete;
    Shape& operator=(const Shape&) = delete;

    // Allow moves
    Shape(Shape&&) noexcept = default;
    Shape& operator=(Shape&&) noexcept = default;

    // --- Polymorphic Cloning (Prototype Pattern) ---
    [[nodiscard]] virtual std::unique_ptr<Shape> clone() const = 0;

    // --- Geometric Properties ---
    [[nodiscard]] virtual double getArea() const = 0;
    [[nodiscard]] virtual double getPerimeter() const = 0;
    [[nodiscard]] virtual AABB getBoundingBox() const = 0;
    [[nodiscard]] virtual Vector2D getCentroid() const = 0;
    [[nodiscard]] virtual std::string getTypeName() const = 0;

    // --- Affine Transformations ---
    virtual void translate(const Vector2D& offset) = 0;
    virtual void rotate(double radians, const Vector2D& pivot = {0.0, 0.0}) = 0;
    virtual void scale(double factor, const Vector2D& pivot = {0.0, 0.0}) = 0;

    // --- Visitor Pattern Dispatch ---
    virtual void accept(ShapeVisitor& visitor) = 0;
    virtual void accept(ConstShapeVisitor& visitor) const = 0;

protected:
    Shape() = default;
};

} // namespace minicad
