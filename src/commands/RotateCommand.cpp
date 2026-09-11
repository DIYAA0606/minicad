#include "commands/RotateCommand.h"

namespace minicad {

RotateCommand::RotateCommand(Shape& target, double radians, const Vector2D& pivot)
    : target_(target), radians_(radians), pivot_(pivot) {}

void RotateCommand::execute() {
    target_.rotate(radians_, pivot_);
}

void RotateCommand::undo() {
    target_.rotate(-radians_, pivot_);
}

} // namespace minicad
