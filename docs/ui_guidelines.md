# UI Guidelines

This document outlines the standard UI guidelines for the editor.

## Input Fields (TextBox, ComboBox)
- **Border styling:** Input fields must have `Transparent` borders by default to avoid visual clutter (no white or yellow borders).
- **Focus styling:** A blue border (`Brush_Accent`) should appear ONLY when the element is focused (`IsKeyboardFocusWithin="True"` or `IsFocused="True"`).
- **Validation:** When invalid, the border is set to `IndianRed` (or an equivalent error brush). When validation passes, the border must be cleared (`ClearValue`) to let the default style take over, rather than explicitly setting it to `White`.

## Dialog Windows
- **Border styling:** Tool windows and dialogs (like `NewProjectDialog`, `NewScriptDialog`) should have a 1-pixel yellow border (`Brush_Text_Primary`) to match the style of the main window.
