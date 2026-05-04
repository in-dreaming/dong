#pragma once

#include "dom/dom_node.hpp"
#include "selection.hpp"
#include <string>

namespace dong::dom {

// Supported editing commands for document.execCommand()
enum class EditingCommand {
    BOLD,
    ITALIC,
    UNDERLINE,
    INSERT_TEXT,
    DELETE,
    SELECT_ALL,
    UNDO,
    REDO,
    UNKNOWN,
};

// Parse command name to enum
EditingCommand parseEditingCommand(const std::string& command);

// Check if a command is supported
bool queryCommandSupported(const std::string& command);

// Execute an editing command
// Returns true if the command was executed
bool execCommand(const DOMNodePtr& editable_root,
                 Selection& selection,
                 const std::string& command,
                 const std::string& value = "");

} // namespace dong::dom
