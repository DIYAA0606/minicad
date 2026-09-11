#pragma once

#include "commands/CommandHistory.h"
#include "geometry/AABB.h"
#include "geometry/Shape.h"
#include "geometry/Vector2D.h"

#include <memory>
#include <utility>
#include <vector>

namespace minicad {

/**
 * @brief Central document model holding shapes and managing CAD operations.
 *
 * Architecture & Design Rationale:
 * 1. Exclusive Ownership: Shapes are owned via `std::vector<std::unique_ptr<Shape>>`.
 *    Callers receive non-owning raw pointers (`Shape*` / `const Shape*`) for observation
 *    and mutation, following modern C++ pointer ownership guidelines.
 * 2. Spatial Indexing & Collision Broadphase: Provides scene-level queries including
 *    point picking (hit testing) and pairwise intersection queries.
 * 3. Integrated Command Pipeline: Integrates `CommandHistory` to manage document
 *    undo/redo workflows transparently.
 */
class Scene {
public:
    Scene() = default;

    // Non-copyable due to unique ownership
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    // --- Shape Lifecycle & Access ---

    /**
     * @brief Adds a shape to the scene, taking ownership.
     * @return The 0-based index of the newly added shape.
     */
    size_t addShape(std::unique_ptr<Shape> shape);

    /**
     * @brief Removes a shape by index, transferring ownership to the caller.
     * @throws std::out_of_range if index is invalid.
     */
    std::unique_ptr<Shape> removeShape(size_t index);

    /**
     * @brief Non-owning accessor to a shape by index.
     * @throws std::out_of_range if index is invalid.
     */
    [[nodiscard]] Shape* getShape(size_t index);
    [[nodiscard]] const Shape* getShape(size_t index) const;

    [[nodiscard]] size_t getShapeCount() const noexcept { return shapes_.size(); }
    [[nodiscard]] bool empty() const noexcept { return shapes_.empty(); }

    void clearShapes() noexcept;

    // --- Spatial Queries ---

    /**
     * @brief Computes the cumulative Axis-Aligned Bounding Box enclosing all shapes.
     * Returns an inverted/empty AABB if the scene is empty.
     */
    [[nodiscard]] AABB getBoundingBox() const;

    /**
     * @brief Finds the indices of all shapes containing the specified point.
     */
    [[nodiscard]] std::vector<size_t> findShapesAtPoint(const Vector2D& point) const;

    /**
     * @brief Identifies all intersecting pairs of shapes in the scene.
     * Evaluates broadphase AABB filtering followed by exact narrowphase tests.
     * @return List of unique index pairs (i, j) with i < j.
     */
    [[nodiscard]] std::vector<std::pair<size_t, size_t>> findIntersectingPairs() const;

    // --- Command & History Interface ---

    void executeCommand(std::unique_ptr<Command> command);
    bool undo();
    bool redo();

    [[nodiscard]] bool canUndo() const noexcept { return history_.canUndo(); }
    [[nodiscard]] bool canRedo() const noexcept { return history_.canRedo(); }

    [[nodiscard]] CommandHistory& getHistory() noexcept { return history_; }
    [[nodiscard]] const CommandHistory& getHistory() const noexcept { return history_; }

private:
    std::vector<std::unique_ptr<Shape>> shapes_;
    CommandHistory history_;
};

} // namespace minicad
