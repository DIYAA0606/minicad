#pragma once

#include "commands/Command.h"

#include <memory>
#include <vector>

namespace minicad {

/**
 * @brief Manages the undo and redo history for reversible CAD operations.
 *
 * Architectural & Design Rationale:
 * 1. Stack-Based History: Uses two stacks (`undoStack_` and `redoStack_`)
 *    storing `std::unique_ptr<Command>` to represent the timeline of state mutations.
 * 2. Redo Invalidation: Executing any new command immediately discards the redo
 *    stack, pruning alternative branches to maintain a linear, consistent timeline.
 * 3. Memory Safety & Depth Limiting: Optional `maxHistoryDepth` bounds memory usage
 *    in long-running sessions by discarding the oldest commands when depth is exceeded.
 */
class CommandHistory {
public:
    explicit CommandHistory(size_t maxDepth = 100);

    // Non-copyable due to unique_ptr ownership
    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;

    CommandHistory(CommandHistory&&) noexcept = default;
    CommandHistory& operator=(CommandHistory&&) noexcept = default;

    /**
     * @brief Executes a command, registers it for undo, and clears the redo stack.
     */
    void executeCommand(std::unique_ptr<Command> command);

    /**
     * @brief Undoes the most recent command, moving it to the redo stack.
     * @return true if an action was undone, false if undo stack was empty.
     */
    bool undo();

    /**
     * @brief Redoes the most recently undone command, moving it back to undo stack.
     * @return true if an action was redone, false if redo stack was empty.
     */
    bool redo();

    [[nodiscard]] bool canUndo() const noexcept { return !undoStack_.empty(); }
    [[nodiscard]] bool canRedo() const noexcept { return !redoStack_.empty(); }

    [[nodiscard]] size_t undoCount() const noexcept { return undoStack_.size(); }
    [[nodiscard]] size_t redoCount() const noexcept { return redoStack_.size(); }

    void clear() noexcept;

    void setMaxHistoryDepth(size_t depth) noexcept { maxDepth_ = depth; }
    [[nodiscard]] size_t getMaxHistoryDepth() const noexcept { return maxDepth_; }

private:
    std::vector<std::unique_ptr<Command>> undoStack_;
    std::vector<std::unique_ptr<Command>> redoStack_;
    size_t maxDepth_{100};
};

} // namespace minicad
