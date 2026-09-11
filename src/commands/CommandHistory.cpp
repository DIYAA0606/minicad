#include "commands/CommandHistory.h"

#include <stdexcept>

namespace minicad {

CommandHistory::CommandHistory(size_t maxDepth)
    : maxDepth_(maxDepth) {}

void CommandHistory::executeCommand(std::unique_ptr<Command> command) {
    if (!command) {
        throw std::invalid_argument("CommandHistory::executeCommand: command must not be null");
    }

    command->execute();

    // Executing a new action prunes the redo timeline
    redoStack_.clear();

    // Check depth bound
    if (maxDepth_ > 0 && undoStack_.size() >= maxDepth_) {
        undoStack_.erase(undoStack_.begin());
    }

    undoStack_.push_back(std::move(command));
}

bool CommandHistory::undo() {
    if (undoStack_.empty()) {
        return false;
    }

    auto cmd = std::move(undoStack_.back());
    undoStack_.pop_back();

    cmd->undo();

    redoStack_.push_back(std::move(cmd));
    return true;
}

bool CommandHistory::redo() {
    if (redoStack_.empty()) {
        return false;
    }

    auto cmd = std::move(redoStack_.back());
    redoStack_.pop_back();

    cmd->execute();

    undoStack_.push_back(std::move(cmd));
    return true;
}

void CommandHistory::clear() noexcept {
    undoStack_.clear();
    redoStack_.clear();
}

} // namespace minicad
