#include "geometry/Circle.h"
#include "geometry/Intersection.h"
#include "geometry/Polygon.h"
#include "geometry/Rectangle.h"
#include "geometry/Vector2D.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <memory>
#include <vector>

using namespace minicad;
using Catch::Matchers::WithinAbs;

TEST_CASE("Projection - Point to Line Segment Distance", "[Intersection]") {
    const Vector2D segA(0.0, 0.0);
    const Vector2D segB(10.0, 0.0);

    SECTION("Point projects directly onto segment interior") {
        const Vector2D pt(5.0, 3.0);
        // Closest point is (5, 0), distance squared is 9
        CHECK(distanceSquaredPointToSegment(pt, segA, segB) == 9.0);
    }

    SECTION("Point projects before start endpoint A") {
        const Vector2D pt(-3.0, 4.0);
        // Closest point is segA (0, 0), distance squared is (-3)^2 + 4^2 = 25
        CHECK(distanceSquaredPointToSegment(pt, segA, segB) == 25.0);
    }

    SECTION("Point projects after end endpoint B") {
        const Vector2D pt(13.0, 4.0);
        // Closest point is segB (10, 0), distance squared is 3^2 + 4^2 = 25
        CHECK(distanceSquaredPointToSegment(pt, segA, segB) == 25.0);
    }

    SECTION("Point lies exactly on segment") {
        const Vector2D pt(7.5, 0.0);
        CHECK(distanceSquaredPointToSegment(pt, segA, segB) == 0.0);
    }
}

TEST_CASE("Point-in-Shape Containment Tests", "[Intersection]") {
    SECTION("Point in Circle") {
        const Circle circle(Vector2D(5.0, 5.0), 3.0);

        CHECK(pointInCircle(circle, Vector2D(5.0, 5.0))); // Center
        CHECK(pointInCircle(circle, Vector2D(6.0, 6.0))); // Interior
        CHECK(pointInCircle(circle, Vector2D(8.0, 5.0))); // Boundary (5+3, 5)
        CHECK_FALSE(pointInCircle(circle, Vector2D(9.0, 5.0))); // Outside
        CHECK_FALSE(pointInCircle(circle, Vector2D(0.0, 0.0))); // Far outside
    }

    SECTION("Point in Axis-Aligned and Rotated Rectangle") {
        const Rectangle rect(Vector2D(0.0, 0.0), 4.0, 2.0); // [-2, 2] x [-1, 1]

        CHECK(pointInRectangle(rect, Vector2D(0.0, 0.0)));
        CHECK(pointInRectangle(rect, Vector2D(1.5, 0.5)));
        CHECK(pointInRectangle(rect, Vector2D(2.0, 1.0))); // Corner boundary
        CHECK_FALSE(pointInRectangle(rect, Vector2D(2.1, 0.0)));
        CHECK_FALSE(pointInRectangle(rect, Vector2D(0.0, 1.1)));

        // 45-degree rotated 2x2 square: corners at (sqrt(2), 0), (0, sqrt(2)), (-sqrt(2), 0), (0, -sqrt(2))
        const double pi = std::acos(-1.0);
        const Rectangle rotRect(Vector2D(0.0, 0.0), 2.0, 2.0, pi / 4.0);

        CHECK(pointInRectangle(rotRect, Vector2D(0.0, 0.0)));
        CHECK(pointInRectangle(rotRect, Vector2D(1.0, 0.0))); // Inside diamond
        CHECK_FALSE(pointInRectangle(rotRect, Vector2D(1.0, 1.0))); // Outside diamond
    }

    SECTION("Point in Convex and Concave Polygons (Ray-Casting)") {
        // Right triangle: (0,0), (4,0), (0,3)
        const Polygon triangle({Vector2D(0.0, 0.0), Vector2D(4.0, 0.0), Vector2D(0.0, 3.0)});

        CHECK(pointInPolygon(triangle, Vector2D(1.0, 1.0))); // Interior
        CHECK(pointInPolygon(triangle, Vector2D(0.0, 0.0))); // Vertex boundary
        CHECK(pointInPolygon(triangle, Vector2D(2.0, 0.0))); // Edge boundary
        CHECK_FALSE(pointInPolygon(triangle, Vector2D(3.0, 3.0))); // Outside

        // Concave arrowhead polygon
        // (0,0), (4,2), (0,4), (1.5, 2)
        const Polygon arrow({
            Vector2D(0.0, 0.0),
            Vector2D(4.0, 2.0),
            Vector2D(0.0, 4.0),
            Vector2D(1.5, 2.0)
        });

        CHECK(pointInPolygon(arrow, Vector2D(3.0, 2.0))); // Inside forward tip
        CHECK_FALSE(pointInPolygon(arrow, Vector2D(0.5, 2.0))); // Inside concave notch (outside shape!)
    }

    SECTION("Polymorphic pointInShape dispatch") {
        std::unique_ptr<Shape> shape = std::make_unique<Circle>(Vector2D(2.0, 2.0), 1.0);
        CHECK(pointInShape(*shape, Vector2D(2.5, 2.0)));
        CHECK_FALSE(pointInShape(*shape, Vector2D(4.0, 2.0)));
    }
}

TEST_CASE("Circle-Circle Intersections", "[Intersection]") {
    const Circle c1(Vector2D(0.0, 0.0), 3.0);

    SECTION("Overlapping circles") {
        const Circle c2(Vector2D(4.0, 0.0), 2.0); // Centers dist = 4 <= 3+2=5
        CHECK(intersectsCircleCircle(c1, c2));
    }

    SECTION("Externally tangent circles") {
        const Circle c2(Vector2D(5.0, 0.0), 2.0); // Centers dist = 5 == 3+2
        CHECK(intersectsCircleCircle(c1, c2));
    }

    SECTION("Disjoint separated circles") {
        const Circle c2(Vector2D(6.0, 0.0), 2.0); // Centers dist = 6 > 5
        CHECK_FALSE(intersectsCircleCircle(c1, c2));
    }

    SECTION("Concentric circles (containment)") {
        const Circle c2(Vector2D(0.0, 0.0), 1.0);
        CHECK(intersectsCircleCircle(c1, c2));
    }
}

TEST_CASE("Circle-Rectangle Intersections", "[Intersection]") {
    const Rectangle rect(Vector2D(0.0, 0.0), 4.0, 4.0); // [-2, 2] x [-2, 2]

    SECTION("Circle overlaps rectangle edge") {
        const Circle c(Vector2D(2.5, 0.0), 1.0); // Center at (2.5, 0), penetrates right edge at x=2
        CHECK(intersectsCircleRectangle(c, rect));
    }

    SECTION("Circle touches corner") {
        // Corner at (2, 2). Circle at (3, 3) with radius sqrt(2) approx 1.4142
        const Circle c(Vector2D(3.0, 3.0), std::sqrt(2.0) + 0.01);
        CHECK(intersectsCircleRectangle(c, rect));
    }

    SECTION("Circle completely inside rectangle") {
        const Circle c(Vector2D(0.0, 0.0), 0.5);
        CHECK(intersectsCircleRectangle(c, rect));
    }

    SECTION("Rectangle completely inside circle") {
        const Circle c(Vector2D(0.0, 0.0), 10.0);
        CHECK(intersectsCircleRectangle(c, rect));
    }

    SECTION("Disjoint circle and rectangle") {
        const Circle c(Vector2D(5.0, 5.0), 1.0);
        CHECK_FALSE(intersectsCircleRectangle(c, rect));
    }
}

TEST_CASE("Circle-Polygon Intersections", "[Intersection]") {
    const Polygon triangle({Vector2D(0.0, 0.0), Vector2D(6.0, 0.0), Vector2D(0.0, 6.0)});

    SECTION("Circle overlapping hypotenuse") {
        const Circle c(Vector2D(4.0, 4.0), 1.5);
        CHECK(intersectsCirclePolygon(c, triangle));
    }

    SECTION("Circle completely inside polygon") {
        const Circle c(Vector2D(1.0, 1.0), 0.5);
        CHECK(intersectsCirclePolygon(c, triangle));
    }

    SECTION("Disjoint circle and polygon") {
        const Circle c(Vector2D(8.0, 8.0), 1.0);
        CHECK_FALSE(intersectsCirclePolygon(c, triangle));
    }
}

TEST_CASE("Separating Axis Theorem (SAT) - Rectangle & Polygon Intersections", "[Intersection]") {
    SECTION("Rectangle-Rectangle axis-aligned overlap and separation") {
        const Rectangle r1(Vector2D(0.0, 0.0), 4.0, 4.0); // [-2, 2] x [-2, 2]
        const Rectangle r2(Vector2D(3.0, 0.0), 4.0, 4.0); // [1, 5] x [-2, 2] -> overlap in [1, 2]
        const Rectangle r3(Vector2D(5.0, 0.0), 1.0, 1.0); // Disjoint

        CHECK(intersectsRectangleRectangle(r1, r2));
        CHECK(intersectsRectangleRectangle(r2, r1));
        CHECK_FALSE(intersectsRectangleRectangle(r1, r3));
    }

    SECTION("Rectangle-Rectangle rotated 45 degrees") {
        const double pi = std::acos(-1.0);
        const Rectangle r1(Vector2D(0.0, 0.0), 4.0, 4.0, 0.0);
        const Rectangle rRot(Vector2D(2.5, 0.0), 2.0, 2.0, pi / 4.0); // Diamond intersecting right edge
        const Rectangle rRotFar(Vector2D(6.0, 0.0), 2.0, 2.0, pi / 4.0); // Separated diamond

        CHECK(intersectsRectangleRectangle(r1, rRot));
        CHECK(intersectsRectangleRectangle(rRot, r1));
        CHECK_FALSE(intersectsRectangleRectangle(r1, rRotFar));
    }

    SECTION("Rectangle-Polygon intersection via SAT") {
        const Rectangle rect(Vector2D(0.0, 0.0), 4.0, 4.0);
        const Polygon triangle({Vector2D(1.0, 1.0), Vector2D(5.0, 1.0), Vector2D(1.0, 5.0)});
        const Polygon farPoly({Vector2D(10.0, 10.0), Vector2D(12.0, 10.0), Vector2D(11.0, 12.0)});

        CHECK(intersectsRectanglePolygon(rect, triangle));
        CHECK_FALSE(intersectsRectanglePolygon(rect, farPoly));
    }

    SECTION("Polygon-Polygon intersection via SAT") {
        const Polygon poly1({
            Vector2D(0.0, 0.0), Vector2D(4.0, 0.0),
            Vector2D(4.0, 4.0), Vector2D(0.0, 4.0)
        });
        const Polygon poly2({
            Vector2D(2.0, 2.0), Vector2D(6.0, 2.0),
            Vector2D(6.0, 6.0), Vector2D(2.0, 6.0)
        });
        const Polygon polyDisjoint({
            Vector2D(10.0, 10.0), Vector2D(14.0, 10.0),
            Vector2D(14.0, 14.0), Vector2D(10.0, 14.0)
        });

        CHECK(intersectsPolygonPolygon(poly1, poly2));
        CHECK(intersectsPolygonPolygon(poly2, poly1));
        CHECK_FALSE(intersectsPolygonPolygon(poly1, polyDisjoint));
    }
}

TEST_CASE("Unified Polymorphic checkIntersection & Double Dispatch", "[Intersection]") {
    std::unique_ptr<Shape> circle = std::make_unique<Circle>(Vector2D(0.0, 0.0), 2.0);
    std::unique_ptr<Shape> rect = std::make_unique<Rectangle>(Vector2D(1.5, 0.0), 2.0, 2.0);
    std::unique_ptr<Shape> poly = std::make_unique<Polygon>(std::vector<Vector2D>{
        Vector2D(1.0, 1.0), Vector2D(4.0, 1.0), Vector2D(1.0, 4.0)
    });
    std::unique_ptr<Shape> farCircle = std::make_unique<Circle>(Vector2D(100.0, 100.0), 1.0);

    SECTION("Symmetric double dispatch across all shape combinations") {
        // Circle <-> Rectangle
        CHECK(checkIntersection(*circle, *rect));
        CHECK(checkIntersection(*rect, *circle));

        // Circle <-> Polygon
        CHECK(checkIntersection(*circle, *poly));
        CHECK(checkIntersection(*poly, *circle));

        // Rectangle <-> Polygon
        CHECK(checkIntersection(*rect, *poly));
        CHECK(checkIntersection(*poly, *rect));

        // Broadphase early rejection with distant shape
        CHECK_FALSE(checkIntersection(*circle, *farCircle));
        CHECK_FALSE(checkIntersection(*rect, *farCircle));
        CHECK_FALSE(checkIntersection(*poly, *farCircle));
    }
}
