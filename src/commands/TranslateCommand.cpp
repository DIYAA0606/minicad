#include "commands/TranslateCommand.h"

namespace minicad {

TranslateCommand::TranslateCommand(Shape& target, const Vector2D& offset)
    : target_(target), offset_(offset) {}

void TranslateCommand::execute() {
    target_.translate(offset_);
}

void TranslateCommand::undo() {
    target_.translate(-offset_);
}

} // namespace minicad
