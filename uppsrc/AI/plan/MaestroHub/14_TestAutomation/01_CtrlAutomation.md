# Task: Core Accessibility (CtrlCore)

# Status: DONE

# Description
Modify `uppsrc/CtrlCore` to introduce the fundamental accessibility structures using a Visitor pattern.

# Objectives
- [x] Define `struct Visitor`.
    - It serves as a base class for UI introspection and automation visitors.
- [x] Modify `Ctrl` class in `CtrlCore`:
    - Added `virtual bool Access(Visitor& v);` (Default implementation returns false).
    - Added `Event<Visitor&> WhenAccess;` member.
- [x] Ensure `Visitor` can capture callback hooks (e.g., via `AutomationVisitor` implementations).

# Technical Details
- `Visitor` is a pure U++ construct for safe UI traversal.
- `Access()` is safe to call for automation and accessibility purposes.
