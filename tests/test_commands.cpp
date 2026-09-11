#include "Scene.h"
#include "commands/CommandHistory.h"
#include "commands/RotateCommand.h"
#include "commands/ScaleCommand.h"
#include "commands/TranslateCommand.h"
#include "geometry/Circle.h"
#include "geometry/Polygon.h"
#include "geometry/Rectangle.h"
#include "geometry/Vector2D.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <memory>

using namespace minicad;
using Catch::Matchers::WithinAbs;

TEST_CASE("TranslateCommand - Execution and Inversion", "[Command]") {
    Circle circle(Vector2D(10.0, 20.0), 5.0);
    TranslateCommand cmd(circle, Vector2D(5.0, -10.0));

    CHECK(cmd.getName() == "Translate");

    cmd.execute();
    CHECK(circle.getCenter() == Vector2D(15.0, 10.0));

    cmd.undo();
    CHECK(circle.getCenter() == Vector2D(10.0, 20.0));
}

TEST_CASE("RotateCommand - Execution and Inversion", "[Command]") {
    const double pi = std::acos(-1.0);
    Circle circle(Vector2D(5.0, 0.0), 2.0);
    RotateCommand cmd(circle, pi / 2.0, Vector2D(0.0, 0.0));

    CHECK(cmd.getName() == "Rotate");

    cmd.execute();
    CHECK_THAT(circle.getCenter().x, WithinAbs(0.0, 1e-9));
    CHECK_THAT(circle.getCenter().y, WithinAbs(5.0, 1e-9));

    cmd.undo();
    CHECK_THAT(circle.getCenter().x, WithinAbs(5.0, 1e-9));
    CHECK_THAT(circle.getCenter().y, WithinAbs(0.0, 1e-9));
}

TEST_CASE("ScaleCommand - Execution, Inversion, and Validation", "[Command]") {
    SECTION("Validation on invalid scale factor") {
        Circle circle(Vector2D(0.0, 0.0), 4.0);
        CHECK_THROWS_AS(ScaleCommand(circle, 0.0), std::invalid_argument);
        CHECK_THROWS_AS(ScaleCommand(circle, -1.5), std::invalid_argument);
    }

    SECTION("Scaling and inverse scaling") {
        Circle circle(Vector2D(2.0, 0.0), 3.0);
        ScaleCommand cmd(circle, 2.0, Vector2D(0.0, 0.0));

        cmd.execute();
        CHECK(circle.getRadius() == 6.0);
        CHECK(circle.getCenter() == Vector2D(4.0, 0.0));

        cmd.undo();
        CHECK(circle.getRadius() == 3.0);
        CHECK(circle.getCenter() == Vector2D(2.0, 0.0));
    }
}

TEST_CASE("CommandHistory - Multi-Step Undo, Redo, and Invalidation", "[CommandHistory]") {
    CommandHistory history;
    Circle circle(Vector2D(0.0, 0.0), 5.0);

    SECTION("Empty history query safety") {
        CHECK_FALSE(history.canUndo());
        CHECK_FALSE(history.canRedo());
        CHECK_FALSE(history.undo());
        CHECK_FALSE(history.redo());
        CHECK(history.undoCount() == 0);
        CHECK(history.redoCount() == 0);
    }

    SECTION("Multi-step command execution and sequential undo/redo") {
        // Step 1: Translate +10 on X
        history.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(10.0, 0.0)));
        CHECK(circle.getCenter() == Vector2D(10.0, 0.0));
        CHECK(history.canUndo());
        CHECK_FALSE(history.canRedo());
        CHECK(history.undoCount() == 1);

        // Step 2: Translate +20 on Y
        history.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(0.0, 20.0)));
        CHECK(circle.getCenter() == Vector2D(10.0, 20.0));
        CHECK(history.undoCount() == 2);

        // Undo Step 2
        REQUIRE(history.undo());
        CHECK(circle.getCenter() == Vector2D(10.0, 0.0));
        CHECK(history.canUndo());
        CHECK(history.canRedo());
        CHECK(history.undoCount() == 1);
        CHECK(history.redoCount() == 1);

        // Undo Step 1
        REQUIRE(history.undo());
        CHECK(circle.getCenter() == Vector2D(0.0, 0.0));
        CHECK_FALSE(history.canUndo());
        CHECK(history.canRedo());

        // Redo Step 1
        REQUIRE(history.redo());
        CHECK(circle.getCenter() == Vector2D(10.0, 0.0));

        // Redo Step 2
        REQUIRE(history.redo());
        CHECK(circle.getCenter() == Vector2D(10.0, 20.0));
    }

    SECTION("Redo stack is invalidated when a new command is executed") {
        history.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(10.0, 0.0)));
        history.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(0.0, 10.0)));

        history.undo(); // back to (10, 0), redoStack has 1 action
        CHECK(history.canRedo());

        // Execute a brand new command branch: Translate (0, -50)
        history.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(0.0, -50.0)));
        CHECK(circle.getCenter() == Vector2D(10.0, -50.0));
        // Redo stack must now be empty!
        CHECK_FALSE(history.canRedo());
        CHECK(history.redoCount() == 0);
    }

    SECTION("Bounded history depth bounds memory growth") {
        CommandHistory boundedHistory(3); // Max depth 3

        for (int i = 1; i <= 5; ++i) {
            boundedHistory.executeCommand(std::make_unique<TranslateCommand>(circle, Vector2D(1.0, 0.0)));
        }

        // Only the 3 most recent commands should remain
        CHECK(boundedHistory.undoCount() == 3);
    }
}

TEST_CASE("Scene - Lifecycle, Spatial Queries, and Command Integration", "[Scene]") {
    Scene scene;

    SECTION("Shape addition, indexing, and bounds checking") {
        CHECK(scene.empty());
        CHECK(scene.getShapeCount() == 0);

        CHECK_THROWS_AS(scene.addShape(nullptr), std::invalid_argument);

        const size_t cIdx = scene.addShape(std::make_unique<Circle>(Vector2D(0.0, 0.0), 5.0));
        const size_t rIdx = scene.addShape(std::make_unique<Rectangle>(Vector2D(10.0, 10.0), 4.0, 4.0));

        CHECK(cIdx == 0);
        CHECK(rIdx == 1);
        CHECK(scene.getShapeCount() == 2);
        CHECK_FALSE(scene.empty());

        CHECK(scene.getShape(0)->getTypeName() == "Circle");
        CHECK(scene.getShape(1)->getTypeName() == "Rectangle");

        CHECK_THROWS_AS(scene.getShape(5), std::out_of_range);

        // Remove shape transfers ownership
        auto removed = scene.removeShape(0);
        REQUIRE(removed != nullptr);
        CHECK(removed->getTypeName() == "Circle");
        CHECK(scene.getShapeCount() == 1);
    }

    SECTION("Scene cumulative bounding box calculation") {
        scene.addShape(std::make_unique<Circle>(Vector2D(0.0, 0.0), 2.0)); // [-2, 2] x [-2, 2]
        scene.addShape(std::make_unique<Rectangle>(Vector2D(10.0, 10.0), 4.0, 4.0)); // [8, 12] x [8, 12]

        const AABB sceneBox = scene.getBoundingBox();
        CHECK(sceneBox.minPt == Vector2D(-2.0, -2.0));
        CHECK(sceneBox.maxPt == Vector2D(12.0, 12.0));
    }

    SECTION("Point picking (hit testing)") {
        scene.addShape(std::make_unique<Circle>(Vector2D(0.0, 0.0), 5.0)); // Index 0
        scene.addShape(std::make_unique<Circle>(Vector2D(2.0, 0.0), 2.0)); // Index 1 (overlaps index 0)
        scene.addShape(std::make_unique<Rectangle>(Vector2D(20.0, 20.0), 4.0, 4.0)); // Index 2

        // Point (2, 0) is inside both shape 0 and shape 1
        const auto hits = scene.findShapesAtPoint(Vector2D(2.0, 0.0));
        REQUIRE(hits.size() == 2);
        CHECK(hits[0] == 0);
        CHECK(hits[1] == 1);

        // Point (20, 20) is inside shape 2 only
        const auto hits2 = scene.findShapesAtPoint(Vector2D(20.0, 20.0));
        REQUIRE(hits2.size() == 1);
        CHECK(hits2[0] == 2);

        // Point outside all
        const auto misses = scene.findShapesAtPoint(Vector2D(100.0, 100.0));
        CHECK(misses.empty());
    }

    SECTION("Finding intersecting shape pairs in scene") {
        // Shape 0: Circle at origin, radius 3
        scene.addShape(std::make_unique<Circle>(Vector2D(0.0, 0.0), 3.0));
        // Shape 1: Rectangle centered at (2, 0), width 2, height 2 -> overlaps Shape 0
        scene.addShape(std::make_unique<Rectangle>(Vector2D(2.0, 0.0), 2.0, 2.0));
        // Shape 2: Distant circle at (50, 50), radius 2 -> disjoint from both
        scene.addShape(std::make_unique<Circle>(Vector2D(50.0, 50.0), 2.0));

        const auto pairs = scene.findIntersectingPairs();
        REQUIRE(pairs.size() == 1);
        CHECK(pairs[0] == std::make_pair(size_t(0), size_t(1)));
    }

    SECTION("Integrated scene-level undo/redo commands") {
        scene.addShape(std::make_unique<Circle>(Vector2D(0.0, 0.0), 5.0));
        Shape* shape = scene.getShape(0);

        scene.executeCommand(std::make_unique<TranslateCommand>(*shape, Vector2D(10.0, 20.0)));
        CHECK(shape->getCentroid() == Vector2D(10.0, 20.0));
        CHECK(scene.canUndo());

        REQUIRE(scene.undo());
        CHECK(shape->getCentroid() == Vector2D(0.0, 0.0));
        CHECK(scene.canRedo());

        REQUIRE(scene.redo());
        CHECK(shape->getCentroid() == Vector2D(10.0, 20.0));
    }
}
