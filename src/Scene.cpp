#include "Scene.h"
#include "geometry/Intersection.h"

#include <stdexcept>

namespace minicad {

size_t Scene::addShape(std::unique_ptr<Shape> shape) {
    if (!shape) {
        throw std::invalid_argument("Scene::addShape: cannot add null shape");
    }
    shapes_.push_back(std::move(shape));
    return shapes_.size() - 1;
}

std::unique_ptr<Shape> Scene::removeShape(size_t index) {
    if (index >= shapes_.size()) {
        throw std::out_of_range("Scene::removeShape: index out of range");
    }
    auto removed = std::move(shapes_[index]);
    shapes_.erase(shapes_.begin() + static_cast<std::ptrdiff_t>(index));
    return removed;
}

Shape* Scene::getShape(size_t index) {
    if (index >= shapes_.size()) {
        throw std::out_of_range("Scene::getShape: index out of range");
    }
    return shapes_[index].get();
}

const Shape* Scene::getShape(size_t index) const {
    if (index >= shapes_.size()) {
        throw std::out_of_range("Scene::getShape: index out of range");
    }
    return shapes_[index].get();
}

void Scene::clearShapes() noexcept {
    shapes_.clear();
}

AABB Scene::getBoundingBox() const {
    AABB cumulativeBox;
    for (const auto& shape : shapes_) {
        cumulativeBox.expandToInclude(shape->getBoundingBox());
    }
    return cumulativeBox;
}

std::vector<size_t> Scene::findShapesAtPoint(const Vector2D& point) const {
    std::vector<size_t> hitIndices;
    for (size_t i = 0; i < shapes_.size(); ++i) {
        if (pointInShape(*shapes_[i], point)) {
            hitIndices.push_back(i);
        }
    }
    return hitIndices;
}

std::vector<std::pair<size_t, size_t>> Scene::findIntersectingPairs() const {
    std::vector<std::pair<size_t, size_t>> pairs;
    const size_t n = shapes_.size();

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            // checkIntersection executes broadphase AABB check first,
            // then narrowphase symmetric double dispatch.
            if (checkIntersection(*shapes_[i], *shapes_[j])) {
                pairs.emplace_back(i, j);
            }
        }
    }
    return pairs;
}

void Scene::executeCommand(std::unique_ptr<Command> command) {
    history_.executeCommand(std::move(command));
}

bool Scene::undo() {
    return history_.undo();
}

bool Scene::redo() {
    return history_.redo();
}

} // namespace minicad
