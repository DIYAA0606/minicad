#pragma once

#include "commands/Command.h"
#include "geometry/Shape.h"
#include "geometry/Vector2D.h"

namespace minicad {

/**
 * @brief Reversible command that translates a target Shape by a vector offset.
 */
class TranslateCommand : public Command {
public:
    TranslateCommand(Shape& target, const Vector2D& offset);

    void execute() override;
    void undo() override;

    [[nodiscard]] std::string getName() const override { return "Translate"; }

    [[nodiscard]] const Vector2D& getOffset() const noexcept { return offset_; }

private:
    Shape& target_;
    Vector2D offset_;
};

} // namespace minicad
