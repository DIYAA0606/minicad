#pragma once

#include "commands/Command.h"
#include "geometry/Shape.h"
#include "geometry/Vector2D.h"

namespace minicad {

/**
 * @brief Reversible command that rotates a target Shape around a pivot point.
 */
class RotateCommand : public Command {
public:
    RotateCommand(Shape& target, double radians, const Vector2D& pivot = {0.0, 0.0});

    void execute() override;
    void undo() override;

    [[nodiscard]] std::string getName() const override { return "Rotate"; }

    [[nodiscard]] double getRadians() const noexcept { return radians_; }
    [[nodiscard]] const Vector2D& getPivot() const noexcept { return pivot_; }

private:
    Shape& target_;
    double radians_;
    Vector2D pivot_;
};

} // namespace minicad
