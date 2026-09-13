#include "Scene.h"
#include "SVGExporter.h"
#include "commands/RotateCommand.h"
#include "commands/ScaleCommand.h"
#include "commands/TranslateCommand.h"
#include "geometry/Circle.h"
#include "geometry/Polygon.h"
#include "geometry/Rectangle.h"
#include "geometry/Vector2D.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <unordered_set>

using namespace minicad;

int main() {
    std::cout << "========================================================\n";
    std::cout << "               MiniCAD Demonstration Scene              \n";
    std::cout << "  2D Geometry, Collision Detection & Command History   \n";
    std::cout << "========================================================\n\n";

    Scene scene;

    // 1. Construct Mechanical Bracket Assembly
    std::cout << "[Step 1] Assembling geometric CAD parts into Scene...\n";

    // Part 0: Base Mounting Plate (Rectangle)
    scene.addShape(
        std::make_unique<Rectangle>(Vector2D(250.0, 320.0), 360.0, 40.0)
    );

    // Part 1: Structural Gusset Bracket (Convex Polygon - Trapezoid)
    std::vector<Vector2D> gussetVertices = {
        Vector2D(140.0, 300.0), // Bottom-left
        Vector2D(360.0, 300.0), // Bottom-right
        Vector2D(310.0, 150.0), // Top-right
        Vector2D(190.0, 150.0)  // Top-left
    };
    scene.addShape(
        std::make_unique<Polygon>(std::move(gussetVertices))
    );

    // Part 2: Central Bearing Bore (Circle)
    scene.addShape(
        std::make_unique<Circle>(Vector2D(250.0, 200.0), 35.0)
    );

    // Part 3: Left Mounting Bolt Hole (Circle)
    scene.addShape(
        std::make_unique<Circle>(Vector2D(100.0, 320.0), 12.0)
    );

    // Part 4: Right Mounting Bolt Hole (Circle)
    scene.addShape(
        std::make_unique<Circle>(Vector2D(400.0, 320.0), 12.0)
    );

    // Part 5: Rotated Sensor Bracket (Oriented Rectangle at 30 degrees)
    const double angle30Deg = 30.0 * (std::acos(-1.0) / 180.0);
    const size_t sensorIdx = scene.addShape(
        std::make_unique<Rectangle>(Vector2D(360.0, 180.0), 90.0, 25.0, angle30Deg)
    );

    std::cout << "  Added " << scene.getShapeCount() << " parts successfully.\n\n";

    // 2. Inspect Shape Properties
    std::cout << "[Step 2] Querying geometric properties via polymorphic interface:\n";
    std::cout << std::left << std::setw(6)  << "ID"
              << std::setw(12) << "Type"
              << std::setw(14) << "Area"
              << std::setw(14) << "Perimeter"
              << "Centroid\n";
    std::cout << std::string(58, '-') << "\n";

    for (size_t i = 0; i < scene.getShapeCount(); ++i) {
        const Shape* s = scene.getShape(i);
        const Vector2D c = s->getCentroid();
        std::cout << std::left << std::setw(6)  << i
                  << std::setw(12) << s->getTypeName()
                  << std::setw(14) << std::fixed << std::setprecision(1) << s->getArea()
                  << std::setw(14) << s->getPerimeter()
                  << "(" << c.x << ", " << c.y << ")\n";
    }

    const AABB sceneBox = scene.getBoundingBox();
    std::cout << "\n  Scene Cumulative Bounding Box: ["
              << sceneBox.minPt.x << ", " << sceneBox.minPt.y << "] to ["
              << sceneBox.maxPt.x << ", " << sceneBox.maxPt.y << "]\n\n";

    // 3. Baseline Collision Check (structural contacts by design: bolt holes
    //    seated in the plate, gusset resting flush on the plate, etc.)
    //    These are NOT the collision we want to visualize -- they're
    //    intentional assembly contacts, so we capture them as a baseline
    //    to exclude from highlighting later.
    std::cout << "[Step 3] Running broadphase AABB + narrowphase SAT collision detection...\n";
    auto baselinePairs = scene.findIntersectingPairs();
    std::cout << "  Baseline structural contacts (by design): " << baselinePairs.size() << "\n";
    for (const auto& [i, j] : baselinePairs) {
        std::cout << "    Contact: Part #" << i << " (" << scene.getShape(i)->getTypeName()
                  << ") <---> Part #" << j << " (" << scene.getShape(j)->getTypeName() << ")\n";
    }
    std::cout << "\n";

    // Store baseline as an ordered set of pairs for fast membership testing
    std::set<std::pair<size_t, size_t>> baselineSet(baselinePairs.begin(), baselinePairs.end());

    // 4. Command Pattern Demonstration: Move Sensor into Deep Collision
    std::cout << "[Step 4] Executing Command: Translate sensor bracket by (-50, 20)...\n";
    Shape* sensorShape = scene.getShape(sensorIdx);
    scene.executeCommand(std::make_unique<TranslateCommand>(*sensorShape, Vector2D(-50.0, 20.0)));

    auto afterPairs = scene.findIntersectingPairs();

    // Only treat a pair as a "new" collision if it wasn't already touching
    // in the baseline (pre-translate) state. This isolates the collision
    // actually introduced by the command from pre-existing structural
    // contacts (bolt holes in the plate, gusset resting on the plate, etc.)
    std::unordered_set<size_t> collidingShapeIndices;
    std::vector<std::pair<size_t, size_t>> newCollisions;
    for (const auto& pair : afterPairs) {
        if (baselineSet.find(pair) == baselineSet.end()) {
            newCollisions.push_back(pair);
            collidingShapeIndices.insert(pair.first);
            collidingShapeIndices.insert(pair.second);
        }
    }

    std::cout << "  Total intersecting pairs after translation: " << afterPairs.size() << "\n";
    std::cout << "  Newly introduced collisions (excluding baseline contacts): " << newCollisions.size() << "\n";
    for (const auto& [i, j] : newCollisions) {
        std::cout << "    NEW Collision: Part #" << i << " (" << scene.getShape(i)->getTypeName()
                  << ") <---> Part #" << j << " (" << scene.getShape(j)->getTypeName() << ")\n";
    }
    std::cout << "\n";

    // 5. Test Undo/Redo Workflow
    std::cout << "[Step 5] Testing Undo / Redo history:\n";
    std::cout << "  Calling scene.undo()...\n";
    scene.undo();
    auto undoCollisions = scene.findIntersectingPairs();
    std::cout << "  Collision count after undo: " << undoCollisions.size()
              << " (should match baseline: " << baselinePairs.size() << ")\n";

    std::cout << "  Calling scene.redo()...\n";
    scene.redo();
    auto redoCollisions = scene.findIntersectingPairs();
    std::cout << "  Collision count after redo: " << redoCollisions.size() << " (Re-applied translation!)\n\n";

    // 6. Export to SVG with Collision Highlighting
    std::cout << "[Step 6] Serializing Scene to SVG via Visitor Pattern...\n";
    const std::string outputPath = "mini_cad_output.svg";

    SVGStyleOptions options;
    options.canvasWidth = 900;
    options.canvasHeight = 650;
    options.drawBoundingBoxes = true;
    options.padding = 40.0;

    SVGExporter exporter(options);
    exporter.exportToFile(scene, outputPath, collidingShapeIndices);

    std::cout << "  Successfully exported SVG visualization to: " << outputPath << "\n";
    std::cout << "  - Parts newly colliding due to the translate are highlighted in red alert styling.\n";
    std::cout << "  - Pre-existing structural contacts (bolt holes, gusset-plate seating) render in standard CAD blue.\n";
    std::cout << "  - Dashed gray boxes represent broadphase Axis-Aligned Bounding Boxes (AABBs) for every part.\n\n";

    std::cout << "MiniCAD Demo completed successfully.\n";
    return 0;
}