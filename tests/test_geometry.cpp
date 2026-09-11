#include "geometry/Vector2D.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <sstream>

using minicad::Vector2D;
using Catch::Matchers::WithinAbs;

TEST_CASE("Vector2D - Default and Component Construction", "[Vector2D]") {
    SECTION("Default constructor initializes to origin (0, 0)") {
        constexpr Vector2D v;
        STATIC_REQUIRE(v.x == 0.0);
        STATIC_REQUIRE(v.y == 0.0);
    }

    SECTION("Explicit components initialize correctly") {
        constexpr Vector2D v(3.5, -4.2);
        STATIC_REQUIRE(v.x == 3.5);
        STATIC_REQUIRE(v.y == -4.2);
    }
}

TEST_CASE("Vector2D - Basic Arithmetic Operations", "[Vector2D]") {
    const Vector2D a(2.0, 3.0);
    const Vector2D b(4.0, -1.0);

    SECTION("Vector addition") {
        const Vector2D sum = a + b;
        CHECK(sum.x == 6.0);
        CHECK(sum.y == 2.0);
    }

    SECTION("Vector subtraction") {
        const Vector2D diff = a - b;
        CHECK(diff.x == -2.0);
        CHECK(diff.y == 4.0);
    }

    SECTION("Unary negation") {
        const Vector2D neg = -a;
        CHECK(neg.x == -2.0);
        CHECK(neg.y == -3.0);
    }

    SECTION("Scalar multiplication is commutative") {
        const Vector2D rightMul = a * 2.5;
        const Vector2D leftMul = 2.5 * a;
        CHECK(rightMul.x == 5.0);
        CHECK(rightMul.y == 7.5);
        CHECK(leftMul == rightMul);
    }

    SECTION("Scalar division") {
        const Vector2D div = a / 2.0;
        CHECK(div.x == 1.0);
        CHECK(div.y == 1.5);
    }

    SECTION("Division by near-zero throws std::invalid_argument") {
        CHECK_THROWS_AS(a / 0.0, std::invalid_argument);
        CHECK_THROWS_AS(a / 1e-15, std::invalid_argument);
    }

    SECTION("Compound assignment operators") {
        Vector2D v(1.0, 2.0);
        v += Vector2D(3.0, 4.0);
        CHECK(v == Vector2D(4.0, 6.0));

        v -= Vector2D(1.0, 2.0);
        CHECK(v == Vector2D(3.0, 4.0));

        v *= 2.0;
        CHECK(v == Vector2D(6.0, 8.0));

        v /= 2.0;
        CHECK(v == Vector2D(3.0, 4.0));

        CHECK_THROWS_AS(v /= 0.0, std::invalid_argument);
    }
}

TEST_CASE("Vector2D - Dot and Cross Products", "[Vector2D]") {
    const Vector2D unitX(1.0, 0.0);
    const Vector2D unitY(0.0, 1.0);

    SECTION("Dot product properties") {
        // Orthogonal vectors have dot product == 0
        CHECK(unitX.dot(unitY) == 0.0);

        // Parallel vectors
        CHECK(unitX.dot(unitX) == 1.0);

        // General vectors
        const Vector2D u(2.0, 3.0);
        const Vector2D v(4.0, -1.0);
        // 2*4 + 3*(-1) = 8 - 3 = 5
        CHECK(u.dot(v) == 5.0);
        CHECK(u.dot(v) == v.dot(u)); // Commutative
    }

    SECTION("2D Cross (perp-dot) product properties") {
        // Basis vectors: unitX x unitY = 1.0 (standard counter-clockwise positive)
        CHECK(unitX.cross(unitY) == 1.0);
        // Anti-commutative: unitY x unitX = -1.0
        CHECK(unitY.cross(unitX) == -1.0);

        // Colinear vectors have cross product == 0
        CHECK(unitX.cross(unitX * 5.0) == 0.0);

        // General vectors: (2*(-1)) - (3*4) = -2 - 12 = -14
        const Vector2D u(2.0, 3.0);
        const Vector2D v(4.0, -1.0);
        CHECK(u.cross(v) == -14.0);
        CHECK(u.cross(v) == -v.cross(u));
    }
}

TEST_CASE("Vector2D - Norm, Distance, and Normalization", "[Vector2D]") {
    const Vector2D v(3.0, 4.0);

    SECTION("Length and squared length") {
        CHECK(v.lengthSquared() == 25.0);
        CHECK(v.length() == 5.0);
    }

    SECTION("Normalization") {
        const Vector2D unit = v.normalized();
        CHECK_THAT(unit.x, WithinAbs(0.6, 1e-9));
        CHECK_THAT(unit.y, WithinAbs(0.8, 1e-9));
        CHECK_THAT(unit.length(), WithinAbs(1.0, 1e-9));
    }

    SECTION("Normalizing zero vector throws std::runtime_error") {
        const Vector2D zero(0.0, 0.0);
        CHECK_THROWS_AS(zero.normalized(), std::runtime_error);
    }

    SECTION("Perpendicular vector is orthogonal") {
        const Vector2D perp = v.perpendicular();
        // perp of (3, 4) is (-4, 3)
        CHECK(perp.x == -4.0);
        CHECK(perp.y == 3.0);
        CHECK(v.dot(perp) == 0.0);
    }

    SECTION("Distance between points") {
        const Vector2D p1(1.0, 2.0);
        const Vector2D p2(4.0, 6.0);
        // dx = 3, dy = 4 -> dist = 5
        CHECK(p1.distanceSquaredTo(p2) == 25.0);
        CHECK(p1.distanceTo(p2) == 5.0);
    }
}

TEST_CASE("Vector2D - Rotation and Angle", "[Vector2D]") {
    const double pi = std::acos(-1.0);
    const Vector2D unitX(1.0, 0.0);

    SECTION("Rotation around origin by 90 degrees counter-clockwise") {
        const Vector2D rotated90 = unitX.rotated(pi / 2.0);
        CHECK_THAT(rotated90.x, WithinAbs(0.0, 1e-9));
        CHECK_THAT(rotated90.y, WithinAbs(1.0, 1e-9));
    }

    SECTION("Rotation around origin by 180 degrees") {
        const Vector2D rotated180 = unitX.rotated(pi);
        CHECK_THAT(rotated180.x, WithinAbs(-1.0, 1e-9));
        CHECK_THAT(rotated180.y, WithinAbs(0.0, 1e-9));
    }

    SECTION("Rotation around pivot point") {
        // Rotate (3, 1) around pivot (1, 1) by 90 deg counter-clockwise
        // Relative: (2, 0) rotated 90 deg -> (0, 2)
        // Absolute: (1, 1) + (0, 2) = (1, 3)
        const Vector2D pt(3.0, 1.0);
        const Vector2D pivot(1.0, 1.0);
        const Vector2D rotatedPt = pt.rotated(pi / 2.0, pivot);
        CHECK_THAT(rotatedPt.x, WithinAbs(1.0, 1e-9));
        CHECK_THAT(rotatedPt.y, WithinAbs(3.0, 1e-9));
    }

    SECTION("Polar angle") {
        CHECK_THAT(unitX.angle(), WithinAbs(0.0, 1e-9));
        CHECK_THAT(Vector2D(0.0, 1.0).angle(), WithinAbs(pi / 2.0, 1e-9));
        CHECK_THAT(Vector2D(-1.0, 0.0).angle(), WithinAbs(pi, 1e-9));
        CHECK_THAT(Vector2D(0.0, -1.0).angle(), WithinAbs(-pi / 2.0, 1e-9));
    }
}

TEST_CASE("Vector2D - Equality and Stream Output", "[Vector2D]") {
    const Vector2D a(1.0000000001, 2.0000000001);
    const Vector2D b(1.0000000002, 2.0000000002);

    SECTION("Epsilon equality matches within tolerance") {
        CHECK(a.equals(b, 1e-8));
        CHECK_FALSE(a.equals(b, 1e-11));
    }

    SECTION("Stream insertion operator formats correctly") {
        const Vector2D v(12.5, -4.75);
        std::ostringstream oss;
        oss << v;
        CHECK(oss.str() == "Vector2D(12.5, -4.75)");
    }
}

#include "geometry/AABB.h"
#include "geometry/Circle.h"
#include "geometry/Rectangle.h"
#include "geometry/Polygon.h"
#include "geometry/ShapeVisitor.h"

using minicad::AABB;
using minicad::Circle;
using minicad::Rectangle;
using minicad::Polygon;
using minicad::Shape;

TEST_CASE("AABB - Construction and Spatial Queries", "[AABB]") {
    SECTION("Default constructor creates inverted bounds") {
        AABB box;
        CHECK_FALSE(box.isValid());
    }

    SECTION("Explicit construction and geometry calculations") {
        const AABB box(Vector2D(1.0, 2.0), Vector2D(5.0, 8.0));
        REQUIRE(box.isValid());
        CHECK(box.width() == 4.0);
        CHECK(box.height() == 6.0);
        CHECK(box.center() == Vector2D(3.0, 5.0));

        // Containment tests
        CHECK(box.contains(Vector2D(3.0, 5.0))); // Inside
        CHECK(box.contains(Vector2D(1.0, 2.0))); // Boundary min
        CHECK(box.contains(Vector2D(5.0, 8.0))); // Boundary max
        CHECK_FALSE(box.contains(Vector2D(0.5, 5.0))); // Outside left
        CHECK_FALSE(box.contains(Vector2D(3.0, 9.0))); // Outside top
    }

    SECTION("AABB intersection detection") {
        const AABB box1(Vector2D(0.0, 0.0), Vector2D(4.0, 4.0));
        const AABB box2(Vector2D(2.0, 2.0), Vector2D(6.0, 6.0));
        const AABB box3(Vector2D(5.0, 5.0), Vector2D(8.0, 8.0));
        const AABB boxTouching(Vector2D(4.0, 0.0), Vector2D(8.0, 4.0));

        CHECK(box1.intersects(box2));
        CHECK(box2.intersects(box1));
        CHECK_FALSE(box1.intersects(box3));
        CHECK(box1.intersects(boxTouching)); // Touching at boundary
    }

    SECTION("AABB expansion") {
        AABB box(Vector2D(2.0, 2.0), Vector2D(4.0, 4.0));
        box.expandToInclude(Vector2D(0.0, 5.0));
        CHECK(box.minPt == Vector2D(0.0, 2.0));
        CHECK(box.maxPt == Vector2D(4.0, 5.0));

        AABB other(Vector2D(-1.0, 1.0), Vector2D(2.0, 7.0));
        box.expandToInclude(other);
        CHECK(box.minPt == Vector2D(-1.0, 1.0));
        CHECK(box.maxPt == Vector2D(4.0, 7.0));
    }
}

TEST_CASE("Circle - Geometry, Transformations, and Clone", "[Circle]") {
    SECTION("Validation on invalid radius") {
        CHECK_THROWS_AS(Circle(Vector2D(0.0, 0.0), 0.0), std::invalid_argument);
        CHECK_THROWS_AS(Circle(Vector2D(0.0, 0.0), -2.5), std::invalid_argument);
    }

    SECTION("Area, Perimeter, Centroid, and Bounding Box") {
        const Circle c(Vector2D(5.0, 5.0), 3.0);
        const double pi = std::acos(-1.0);

        CHECK_THAT(c.getArea(), WithinAbs(pi * 9.0, 1e-9));
        CHECK_THAT(c.getPerimeter(), WithinAbs(2.0 * pi * 3.0, 1e-9));
        CHECK(c.getCentroid() == Vector2D(5.0, 5.0));

        const AABB aabb = c.getBoundingBox();
        CHECK(aabb.minPt == Vector2D(2.0, 2.0));
        CHECK(aabb.maxPt == Vector2D(8.0, 8.0));
    }

    SECTION("Transformations") {
        Circle c(Vector2D(2.0, 3.0), 4.0);

        // Translate
        c.translate(Vector2D(1.0, -1.0));
        CHECK(c.getCenter() == Vector2D(3.0, 2.0));

        // Scale relative to origin
        c.scale(2.0, Vector2D(0.0, 0.0));
        CHECK(c.getRadius() == 8.0);
        CHECK(c.getCenter() == Vector2D(6.0, 4.0));

        // Rotate around external pivot
        const double pi = std::acos(-1.0);
        Circle rotC(Vector2D(3.0, 0.0), 1.0);
        rotC.rotate(pi / 2.0, Vector2D(0.0, 0.0));
        CHECK_THAT(rotC.getCenter().x, WithinAbs(0.0, 1e-9));
        CHECK_THAT(rotC.getCenter().y, WithinAbs(3.0, 1e-9));
    }

    SECTION("Polymorphic clone creates independent deep copy") {
        Circle original(Vector2D(1.0, 2.0), 5.0);
        std::unique_ptr<Shape> clonedShape = original.clone();
        REQUIRE(clonedShape != nullptr);
        CHECK(clonedShape->getTypeName() == "Circle");
        CHECK(clonedShape->getCentroid() == Vector2D(1.0, 2.0));

        // Mutating the clone must not affect original
        clonedShape->translate(Vector2D(10.0, 10.0));
        CHECK(clonedShape->getCentroid() == Vector2D(11.0, 12.0));
        CHECK(original.getCentroid() == Vector2D(1.0, 2.0));
    }
}

TEST_CASE("Rectangle - Vertices, Rotation, and Clone", "[Rectangle]") {
    SECTION("Validation on invalid dimensions") {
        CHECK_THROWS_AS(Rectangle(Vector2D(0.0, 0.0), 0.0, 5.0), std::invalid_argument);
        CHECK_THROWS_AS(Rectangle(Vector2D(0.0, 0.0), 5.0, -1.0), std::invalid_argument);
        CHECK_THROWS_AS(Rectangle(Vector2D(5.0, 5.0), Vector2D(2.0, 2.0)), std::invalid_argument);
    }

    SECTION("Axis-aligned rectangle geometry") {
        const Rectangle rect(Vector2D(0.0, 0.0), Vector2D(4.0, 6.0));
        CHECK(rect.getWidth() == 4.0);
        CHECK(rect.getHeight() == 6.0);
        CHECK(rect.getArea() == 24.0);
        CHECK(rect.getPerimeter() == 20.0);
        CHECK(rect.getCentroid() == Vector2D(2.0, 3.0));

        const auto vertices = rect.getVertices();
        REQUIRE(vertices.size() == 4);
        CHECK(vertices[0] == Vector2D(0.0, 0.0)); // Bottom-Left
        CHECK(vertices[1] == Vector2D(4.0, 0.0)); // Bottom-Right
        CHECK(vertices[2] == Vector2D(4.0, 6.0)); // Top-Right
        CHECK(vertices[3] == Vector2D(0.0, 6.0)); // Top-Left
    }

    SECTION("Rotated rectangle bounding box calculation") {
        // 2x2 square rotated by 45 degrees (pi / 4)
        const double pi = std::acos(-1.0);
        const Rectangle rotatedSquare(Vector2D(0.0, 0.0), 2.0, 2.0, pi / 4.0);
        const AABB aabb = rotatedSquare.getBoundingBox();

        // Diagonal of 2x2 square is 2*sqrt(2) approx 2.828427, so bounding half-span is sqrt(2)
        const double sqrt2 = std::sqrt(2.0);
        CHECK_THAT(aabb.minPt.x, WithinAbs(-sqrt2, 1e-9));
        CHECK_THAT(aabb.minPt.y, WithinAbs(-sqrt2, 1e-9));
        CHECK_THAT(aabb.maxPt.x, WithinAbs(sqrt2, 1e-9));
        CHECK_THAT(aabb.maxPt.y, WithinAbs(sqrt2, 1e-9));
    }

    SECTION("Rectangle clone independence") {
        Rectangle rect(Vector2D(1.0, 1.0), 2.0, 4.0);
        auto cloned = rect.clone();
        cloned->translate(Vector2D(5.0, 5.0));
        CHECK(cloned->getCentroid() == Vector2D(6.0, 6.0));
        CHECK(rect.getCentroid() == Vector2D(1.0, 1.0));
    }
}

TEST_CASE("Polygon - Shoelace Area, Centroid, and Convexity", "[Polygon]") {
    SECTION("Validation requires at least 3 vertices") {
        CHECK_THROWS_AS(Polygon({Vector2D(0, 0), Vector2D(1, 1)}), std::invalid_argument);
    }

    SECTION("Right triangle area and centroid") {
        // Vertices: (0, 0), (4, 0), (0, 3) in CCW order
        const Polygon triangle({Vector2D(0.0, 0.0), Vector2D(4.0, 0.0), Vector2D(0.0, 3.0)});

        CHECK_THAT(triangle.getArea(), WithinAbs(6.0, 1e-9));
        CHECK_THAT(triangle.getPerimeter(), WithinAbs(12.0, 1e-9)); // 4 + 5 + 3 = 12

        // Centroid of a triangle is the arithmetic mean of its vertices: (4/3, 1)
        const Vector2D centroid = triangle.getCentroid();
        CHECK_THAT(centroid.x, WithinAbs(4.0 / 3.0, 1e-9));
        CHECK_THAT(centroid.y, WithinAbs(1.0, 1e-9));
        CHECK(triangle.isConvex());
        CHECK(triangle.isCounterClockwise());
    }

    SECTION("Convexity detection: convex polygon vs concave arrow") {
        // Convex regular square
        const Polygon convexSquare({
            Vector2D(0.0, 0.0), Vector2D(2.0, 0.0),
            Vector2D(2.0, 2.0), Vector2D(0.0, 2.0)
        });
        CHECK(convexSquare.isConvex());

        // Concave arrowhead: (0,0), (2,1), (0,2), (0.5, 1) indented vertex
        const Polygon concaveArrow({
            Vector2D(0.0, 0.0), Vector2D(2.0, 1.0),
            Vector2D(0.0, 2.0), Vector2D(0.5, 1.0)
        });
        CHECK_FALSE(concaveArrow.isConvex());
    }

    SECTION("Winding order correction and edge normal generation") {
        // Clockwise triangle: (0, 0), (0, 3), (4, 0)
        Polygon cwTriangle({Vector2D(0.0, 0.0), Vector2D(0.0, 3.0), Vector2D(4.0, 0.0)});
        CHECK_FALSE(cwTriangle.isCounterClockwise());

        cwTriangle.ensureCounterClockwise();
        CHECK(cwTriangle.isCounterClockwise());

        const auto normals = cwTriangle.getEdgeNormals();
        REQUIRE(normals.size() == 3);
        // Each normal must be a unit vector
        for (const auto& n : normals) {
            CHECK_THAT(n.length(), WithinAbs(1.0, 1e-9));
        }
    }

    SECTION("Polygon transformations and clone") {
        Polygon poly({Vector2D(0.0, 0.0), Vector2D(2.0, 0.0), Vector2D(1.0, 2.0)});
        poly.translate(Vector2D(3.0, 4.0));
        CHECK(poly.getVertices()[0] == Vector2D(3.0, 4.0));

        const Vector2D originalCentroid = poly.getCentroid();
        auto cloned = poly.clone();
        REQUIRE(cloned != nullptr);
        CHECK(cloned->getCentroid() == originalCentroid);

        // Scale clone relative to origin (0, 0)
        cloned->scale(2.0, Vector2D(0.0, 0.0));
        CHECK(cloned->getCentroid() == originalCentroid * 2.0);
        // Original polygon must remain unchanged
        CHECK(poly.getCentroid() == originalCentroid);
    }
}

namespace {
class ShapeStatsVisitor : public minicad::ConstShapeVisitor {
public:
    int circleCount{0};
    int rectangleCount{0};
    int polygonCount{0};
    double totalArea{0.0};

    void visit(const Circle& circle) override {
        ++circleCount;
        totalArea += circle.getArea();
    }

    void visit(const Rectangle& rectangle) override {
        ++rectangleCount;
        totalArea += rectangle.getArea();
    }

    void visit(const Polygon& polygon) override {
        ++polygonCount;
        totalArea += polygon.getArea();
    }
};
} // anonymous namespace

TEST_CASE("Visitor Pattern - Polymorphic Double Dispatch", "[Visitor]") {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(Vector2D(0.0, 0.0), 2.0));
    shapes.push_back(std::make_unique<Rectangle>(Vector2D(0.0, 0.0), 3.0, 4.0));
    shapes.push_back(std::make_unique<Polygon>(std::vector<Vector2D>{
        Vector2D(0.0, 0.0), Vector2D(4.0, 0.0), Vector2D(0.0, 3.0)
    }));

    ShapeStatsVisitor visitor;
    for (const auto& shape : shapes) {
        shape->accept(visitor);
    }

    CHECK(visitor.circleCount == 1);
    CHECK(visitor.rectangleCount == 1);
    CHECK(visitor.polygonCount == 1);

    const double pi = std::acos(-1.0);
    const double expectedTotalArea = (pi * 4.0) + (12.0) + (6.0);
    CHECK_THAT(visitor.totalArea, WithinAbs(expectedTotalArea, 1e-9));
}

