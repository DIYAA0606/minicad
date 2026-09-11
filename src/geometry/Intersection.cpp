#include "geometry/Intersection.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace minicad {

double distanceSquaredPointToSegment(
    const Vector2D& point,
    const Vector2D& segA,
    const Vector2D& segB
) noexcept {
    const Vector2D ab = segB - segA;
    const double abLenSq = ab.lengthSquared();

    // Degenerate segment: point to point distance
    if (abLenSq < 1e-14) {
        return point.distanceSquaredTo(segA);
    }

    // Scalar projection of (point - segA) onto (segB - segA)
    const Vector2D ap = point - segA;
    const double t = std::clamp(ap.dot(ab) / abLenSq, 0.0, 1.0);

    // Clamped closest point on line segment
    const Vector2D closest = segA + ab * t;
    return point.distanceSquaredTo(closest);
}

// --- Point-in-Shape Algorithms ---

bool pointInCircle(const Circle& circle, const Vector2D& point) noexcept {
    const double r = circle.getRadius();
    return point.distanceSquaredTo(circle.getCenter()) <= (r * r + 1e-9);
}

bool pointInRectangle(const Rectangle& rectangle, const Vector2D& point) noexcept {
    // Transform point into rectangle's local unrotated coordinate space
    const Vector2D rel = point - rectangle.getCenter();
    const Vector2D localPt = rel.rotated(-rectangle.getAngle());

    const double halfW = rectangle.getWidth() * 0.5;
    const double halfH = rectangle.getHeight() * 0.5;

    return (std::abs(localPt.x) <= halfW + 1e-9) &&
           (std::abs(localPt.y) <= halfH + 1e-9);
}

bool pointInPolygon(const Polygon& polygon, const Vector2D& point) noexcept {
    const auto& vertices = polygon.getVertices();
    const size_t n = vertices.size();
    if (n < 3) return false;

    // Boundary check: if point lies on any edge segment, it is inside
    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        if (distanceSquaredPointToSegment(point, vertices[i], vertices[next]) <= 1e-12) {
            return true;
        }
    }

    // Ray-Casting algorithm (Jordan curve theorem / Even-Odd rule)
    // Cast a horizontal ray from point towards (+infinity, point.y)
    int crossCount = 0;
    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        const Vector2D& v1 = vertices[i];
        const Vector2D& v2 = vertices[next];

        // Check if the edge crosses the horizontal ray's Y level
        const bool crossesY = (v1.y > point.y) != (v2.y > point.y);
        if (crossesY) {
            // Compute X coordinate where the segment crosses Y = point.y
            const double xIntersection = v1.x + (point.y - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
            if (point.x < xIntersection) {
                ++crossCount;
            }
        }
    }

    return (crossCount % 2 != 0);
}

namespace {
class PointInShapeVisitor : public ConstShapeVisitor {
public:
    explicit PointInShapeVisitor(const Vector2D& pt) : point_(pt) {}

    void visit(const Circle& circle) override {
        result_ = pointInCircle(circle, point_);
    }

    void visit(const Rectangle& rectangle) override {
        result_ = pointInRectangle(rectangle, point_);
    }

    void visit(const Polygon& polygon) override {
        result_ = pointInPolygon(polygon, point_);
    }

    [[nodiscard]] bool result() const noexcept { return result_; }

private:
    Vector2D point_;
    bool result_{false};
};
} // anonymous namespace

bool pointInShape(const Shape& shape, const Vector2D& point) {
    PointInShapeVisitor visitor(point);
    shape.accept(visitor);
    return visitor.result();
}

// --- Separating Axis Theorem (SAT) Core Engine ---

void projectOntoAxis(
    const std::vector<Vector2D>& vertices,
    const Vector2D& axis,
    double& minProj,
    double& maxProj
) noexcept {
    minProj = std::numeric_limits<double>::infinity();
    maxProj = -std::numeric_limits<double>::infinity();

    for (const auto& v : vertices) {
        const double proj = v.dot(axis);
        if (proj < minProj) minProj = proj;
        if (proj > maxProj) maxProj = proj;
    }
}

bool satOverlap(
    const std::vector<Vector2D>& vertsA,
    const std::vector<Vector2D>& vertsB,
    const std::vector<Vector2D>& axes
) noexcept {
    for (const auto& axis : axes) {
        // Degenerate axis guard
        if (axis.lengthSquared() < 1e-14) continue;

        double minA = 0.0;
        double maxA = 0.0;
        double minB = 0.0;
        double maxB = 0.0;

        projectOntoAxis(vertsA, axis, minA, maxA);
        projectOntoAxis(vertsB, axis, minB, maxB);

        // If projections do not overlap on this axis, a separating line exists
        if (maxA < minB - 1e-9 || maxB < minA - 1e-9) {
            return false;
        }
    }
    return true; // Overlaps on all tested axes
}

// --- Primitive Pairwise Intersections ---

bool intersectsCircleCircle(const Circle& c1, const Circle& c2) noexcept {
    const double radiusSum = c1.getRadius() + c2.getRadius();
    return c1.getCenter().distanceSquaredTo(c2.getCenter()) <= (radiusSum * radiusSum + 1e-9);
}

bool intersectsCircleRectangle(const Circle& c, const Rectangle& r) noexcept {
    // Condition 1: Center of circle is inside rectangle (containment)
    if (pointInRectangle(r, c.getCenter())) {
        return true;
    }

    // Condition 2: Minimum distance from circle center to any rectangle edge <= radius
    const auto vertices = r.getVertices();
    const double rSq = c.getRadius() * c.getRadius() + 1e-9;

    for (size_t i = 0; i < 4; ++i) {
        const size_t next = (i + 1) % 4;
        if (distanceSquaredPointToSegment(c.getCenter(), vertices[i], vertices[next]) <= rSq) {
            return true;
        }
    }

    return false;
}

bool intersectsCirclePolygon(const Circle& c, const Polygon& p) noexcept {
    // Condition 1: Center of circle is inside polygon (containment)
    if (pointInPolygon(p, c.getCenter())) {
        return true;
    }

    // Condition 2: Minimum distance from circle center to any polygon edge <= radius
    const auto& vertices = p.getVertices();
    const size_t n = vertices.size();
    const double rSq = c.getRadius() * c.getRadius() + 1e-9;

    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        if (distanceSquaredPointToSegment(c.getCenter(), vertices[i], vertices[next]) <= rSq) {
            return true;
        }
    }

    return false;
}

namespace {
// Helper to extract unique edge normals for SAT from a vertex list
std::vector<Vector2D> extractUniqueEdgeNormals(const std::vector<Vector2D>& vertices) {
    std::vector<Vector2D> normals;
    const size_t n = vertices.size();
    normals.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        const size_t next = (i + 1) % n;
        const Vector2D edge = vertices[next] - vertices[i];
        if (edge.lengthSquared() > 1e-14) {
            normals.push_back(edge.perpendicular().normalized());
        }
    }
    return normals;
}
} // anonymous namespace

bool intersectsRectangleRectangle(const Rectangle& r1, const Rectangle& r2) noexcept {
    const auto vertsA = r1.getVertices();
    const auto vertsB = r2.getVertices();

    // Candidate axes: edge normals of r1 and r2
    std::vector<Vector2D> axes = extractUniqueEdgeNormals(vertsA);
    const auto axesB = extractUniqueEdgeNormals(vertsB);
    axes.insert(axes.end(), axesB.begin(), axesB.end());

    return satOverlap(vertsA, vertsB, axes);
}

bool intersectsRectanglePolygon(const Rectangle& r, const Polygon& p) noexcept {
    const auto vertsA = r.getVertices();
    const auto& vertsB = p.getVertices();

    std::vector<Vector2D> axes = extractUniqueEdgeNormals(vertsA);
    const auto axesB = p.getEdgeNormals();
    axes.insert(axes.end(), axesB.begin(), axesB.end());

    return satOverlap(vertsA, vertsB, axes);
}

bool intersectsPolygonPolygon(const Polygon& p1, const Polygon& p2) noexcept {
    const auto& vertsA = p1.getVertices();
    const auto& vertsB = p2.getVertices();

    std::vector<Vector2D> axes = p1.getEdgeNormals();
    const auto axesB = p2.getEdgeNormals();
    axes.insert(axes.end(), axesB.begin(), axesB.end());

    return satOverlap(vertsA, vertsB, axes);
}

// --- Symmetric Double-Dispatch Intersection Engine ---

namespace {

// Secondary visitor: receives concrete Shape A and dispatches on concrete Shape B
class CircleIntersectionVisitor : public ConstShapeVisitor {
public:
    explicit CircleIntersectionVisitor(const Circle& circle) : circle_(circle) {}

    void visit(const Circle& c2) override {
        result_ = intersectsCircleCircle(circle_, c2);
    }
    void visit(const Rectangle& r) override {
        result_ = intersectsCircleRectangle(circle_, r);
    }
    void visit(const Polygon& p) override {
        result_ = intersectsCirclePolygon(circle_, p);
    }

    [[nodiscard]] bool result() const noexcept { return result_; }

private:
    const Circle& circle_;
    bool result_{false};
};

class RectangleIntersectionVisitor : public ConstShapeVisitor {
public:
    explicit RectangleIntersectionVisitor(const Rectangle& rect) : rect_(rect) {}

    void visit(const Circle& c) override {
        // Commutative: intersects(Rectangle, Circle) == intersects(Circle, Rectangle)
        result_ = intersectsCircleRectangle(c, rect_);
    }
    void visit(const Rectangle& r2) override {
        result_ = intersectsRectangleRectangle(rect_, r2);
    }
    void visit(const Polygon& p) override {
        result_ = intersectsRectanglePolygon(rect_, p);
    }

    [[nodiscard]] bool result() const noexcept { return result_; }

private:
    const Rectangle& rect_;
    bool result_{false};
};

class PolygonIntersectionVisitor : public ConstShapeVisitor {
public:
    explicit PolygonIntersectionVisitor(const Polygon& poly) : poly_(poly) {}

    void visit(const Circle& c) override {
        // Commutative: intersects(Polygon, Circle) == intersects(Circle, Polygon)
        result_ = intersectsCirclePolygon(c, poly_);
    }
    void visit(const Rectangle& r) override {
        // Commutative: intersects(Polygon, Rectangle) == intersects(Rectangle, Polygon)
        result_ = intersectsRectanglePolygon(r, poly_);
    }
    void visit(const Polygon& p2) override {
        result_ = intersectsPolygonPolygon(poly_, p2);
    }

    [[nodiscard]] bool result() const noexcept { return result_; }

private:
    const Polygon& poly_;
    bool result_{false};
};

// Primary visitor: visits Shape A and initiates dispatch on Shape B
class PrimaryIntersectionVisitor : public ConstShapeVisitor {
public:
    explicit PrimaryIntersectionVisitor(const Shape& shapeB) : shapeB_(shapeB) {}

    void visit(const Circle& circleA) override {
        CircleIntersectionVisitor secondary(circleA);
        shapeB_.accept(secondary);
        result_ = secondary.result();
    }

    void visit(const Rectangle& rectA) override {
        RectangleIntersectionVisitor secondary(rectA);
        shapeB_.accept(secondary);
        result_ = secondary.result();
    }

    void visit(const Polygon& polyA) override {
        PolygonIntersectionVisitor secondary(polyA);
        shapeB_.accept(secondary);
        result_ = secondary.result();
    }

    [[nodiscard]] bool result() const noexcept { return result_; }

private:
    const Shape& shapeB_;
    bool result_{false};
};

} // anonymous namespace

bool checkIntersection(const Shape& a, const Shape& b) {
    // Broadphase: O(1) Axis-Aligned Bounding Box overlap check
    if (!a.getBoundingBox().intersects(b.getBoundingBox())) {
        return false;
    }

    // Narrowphase: Symmetric double-dispatch via Visitor pattern
    PrimaryIntersectionVisitor primary(b);
    a.accept(primary);
    return primary.result();
}

} // namespace minicad
