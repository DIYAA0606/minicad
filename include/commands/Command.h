#pragma once

#include <string>

namespace minicad {

/**
 * @brief Abstract base class for the Command design pattern in MiniCAD.
 *
 * Design Decisions & Architectural Rationale:
 * - Decoupling Invoker from Receiver: Encapsulates all mutating actions
 *   (e.g., translation, rotation, scaling) into standalone objects.
 * - Symmetrical Invariance (Undo/Redo): Every command must provide both an
 *   `execute()` method and a strict mathematical `undo()` method that restores
 *   the system to its exact previous state.
 * - Virtual Destructor & Polymorphism: Managed via `std::unique_ptr<Command>`
 *   inside `CommandHistory` undo/redo stacks.
 */
class Command {
public:
    virtual ~Command() = default;

    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    Command(Command&&) noexcept = default;
    Command& operator=(Command&&) noexcept = default;

    /**
     * @brief Executes the command, modifying the receiver shape or scene.
     */
    virtual void execute() = 0;

    /**
     * @brief Inverts the command's effect, restoring prior geometric state.
     */
    virtual void undo() = 0;

    /**
     * @brief Descriptive label of the action for UI history / telemetry.
     */
    [[nodiscard]] virtual std::string getName() const = 0;

protected:
    Command() = default;
};

} // namespace minicad
