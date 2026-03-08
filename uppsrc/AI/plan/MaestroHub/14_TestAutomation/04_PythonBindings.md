# Task: Python Bindings (Ctrl/Automation)

# Status: DONE

# Description
Bind the UI Visitor capabilities and the Tree Walker to the `ByteVM` Python environment.

# Objectives
- [x] Create `uppsrc/Ctrl/Automation` package.
- [x] Bind `AutomationElement` class to Python:
    - `obj.label`, `obj.path`, `obj.value`, `obj.checked`, `obj.enabled`.
    - `obj.click()`: Triggers a click action on the element.
    - `obj.set(val)`: Writes a value to the element.
- [x] Bind the Tree Walker & Globals:
    - `find(path_or_label)`: Locates an element using `GuiAutomationVisitor`.
    - `dump_ui()`: Returns a full text dump of the accessible UI tree.
- [x] Bind Event helpers:
    - `wait_ready()`: Process pending GUI events.
    - `wait_time(seconds)`: Sleep while maintaining GUI responsiveness.

# Implementation Details
- `RegisterAutomationBindings(PyVM& vm)` provides the integration point.
- `AutomationElement` wraps the C++ `AutomationElement` structure.
- `find` and `dump_ui` utilize the newly implemented geometric Tree Walker.
