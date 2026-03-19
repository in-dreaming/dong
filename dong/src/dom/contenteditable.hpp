#pragma once

#include "dom/dom_node.hpp"
#include "selection.hpp"
#include <string>

namespace dong::dom {

// ContentEditable state management.
// Handles editability detection and basic text editing operations.
class ContentEditableState {
public:
    // Check if a node is content-editable (explicit attribute or inherited)
    static bool isContentEditable(const DOMNodePtr& node);

    // Find the nearest contenteditable ancestor (the editing host)
    static DOMNodePtr findEditableRoot(const DOMNodePtr& node);

    // Insert text at the current selection/caret position
    // Returns true if the insertion was performed
    static bool insertText(const DOMNodePtr& editable_root,
                           Selection& selection,
                           const std::string& text);

    // Delete backward (Backspace)
    static bool deleteBackward(const DOMNodePtr& editable_root,
                                Selection& selection);

    // Delete forward (Delete key)
    static bool deleteForward(const DOMNodePtr& editable_root,
                               Selection& selection);

    // Insert a paragraph break (Enter)
    static bool insertParagraph(const DOMNodePtr& editable_root,
                                 Selection& selection);

    // Move caret left (-1) or right (+1)
    static bool moveCaret(const DOMNodePtr& editable_root,
                          Selection& selection,
                          int direction);

    // Move caret to line start (to_end=false) or line end (to_end=true)
    static bool moveCaretToLineEdge(const DOMNodePtr& editable_root,
                                     Selection& selection,
                                     bool to_end);

    // Extend selection by one character in direction
    static bool extendSelection(const DOMNodePtr& editable_root,
                                 Selection& selection,
                                 int direction);

    // Select the word at the current caret position
    static bool selectWordAtCaret(const DOMNodePtr& editable_root,
                                   Selection& selection);

private:
    // Find next text node in document order within root
    static DOMNodePtr findNextTextNode(const DOMNodePtr& root,
                                        const DOMNodePtr& current);

    // Find previous text node in document order within root
    static DOMNodePtr findPrevTextNode(const DOMNodePtr& root,
                                        const DOMNodePtr& current);

    // Find first text node in subtree (depth-first)
    static DOMNodePtr findFirstTextNode(const DOMNodePtr& node);

    // Find last text node in subtree (depth-first, rightmost)
    static DOMNodePtr findLastTextNode(const DOMNodePtr& node);
};

} // namespace dong::dom
