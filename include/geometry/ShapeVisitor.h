#pragma once

namespace minicad {

// Forward declarations of concrete shapes in the hierarchy
class Circle;
class Rectangle;
class Polygon;

/**
 * @brief Abstract Visitor interface for non-mutating operations over shapes.
 *
 * Design Decisions & Architectural Rationale:
 * - Open/Closed Principle: In a CAD system, new operations are added frequently
 *   (e.g., SVG export, bounding volume calculation, tessellation, mass properties).
 *   Adding a virtual method to the base Shape interface for every new operation
 *   would require recompiling the entire shape hierarchy every time.
 * - Elimination of Antipatterns: The Visitor pattern eliminates the need for
 *   slow, brittle `dynamic_cast<T>` cascades or manual type enums.
 * - Double Dispatch: By pairing `Shape::accept(Visitor&)` with `Visitor::visit(ConcreteShape&)`,
 *   dispatch is resolved polymorphically in O(1) via the vtable.
 */
class ConstShapeVisitor {
public:
    virtual ~ConstShapeVisitor() = default;

    virtual void visit(const Circle& circle) = 0;
    virtual void visit(const Rectangle& rectangle) = 0;
    virtual void visit(const Polygon& polygon) = 0;
};

/**
 * @brief Abstract Visitor interface for mutating operations over shapes.
 */
class ShapeVisitor {
public:
    virtual ~ShapeVisitor() = default;

    virtual void visit(Circle& circle) = 0;
    virtual void visit(Rectangle& rectangle) = 0;
    virtual void visit(Polygon& polygon) = 0;
};

} // namespace minicad
