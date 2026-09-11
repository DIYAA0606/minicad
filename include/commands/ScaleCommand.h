#pragma once

#include "commands/Command.h"
#include "geometry/Shape.h"
#include "geometry/Vector2D.h"

namespace minicad {

/**
 * @brief Reversible command that scales a target Shape relative to a pivot point.
 */
class ScaleCommand : public Command {
public:
    ScaleCommand(Shape& target, double factor, const Vector2D& pivot = {0.0, 0.0});

    void execute() override;
    void undo() override;

    [[nodiscard]] std::string getName() const override { return "Scale"; }

    [[nodiscard]] double getFactor() const noexcept { return factor_; }
    [[nodiscard]] const Vector2D& getPivot() const noexcept { return pivot_; }

private:
    Shape& target_;
    double factor_;
    Vector2D pivot_;
};

} // namespace minicad
