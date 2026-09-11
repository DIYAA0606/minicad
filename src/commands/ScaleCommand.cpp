#include "commands/ScaleCommand.h"

#include <stdexcept>

namespace minicad {

ScaleCommand::ScaleCommand(Shape& target, double factor, const Vector2D& pivot)
    : target_(target), factor_(factor), pivot_(pivot) {
    if (factor <= 0.0) {
        throw std::invalid_argument("ScaleCommand: factor must be strictly positive");
    }
}

void ScaleCommand::execute() {
    target_.scale(factor_, pivot_);
}

void ScaleCommand::undo() {
    // Invert the scaling factor: s * (1/s) = 1
    target_.scale(1.0 / factor_, pivot_);
}

} // namespace minicad
