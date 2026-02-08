# Track 14: Deep Automation & Accessibility

# Goal
Build a native accessibility and automation layer into the U++ Core, enabling Python scripts (`ByteVM`) to inspect, interact with, and verify the GUI state semantically, rather than just via coordinates.

# Tasks
1. [ ] **Core Accessibility (CtrlCore)**:
    - Define `AccObj` (Accessibility Object) class/struct.
    - Add `virtual void Access(AccObj&)` to `Ctrl` class.
    - Add `Event<AccObj&> WhenAccess` to `Ctrl` for dynamic binding.
2. [ ] **Standard Controls (CtrlLib)**:
    - Implement `Access()` overrides for all major widgets: `Button`, `Label`, `EditField`, `TabCtrl`, `ArrayCtrl`, `TreeCtrl`, `Switch`, `Option`.
    - Ensure complex controls like `ArrayCtrl` expose their content (rows/columns) via `AccObj`.
3. [ ] **Heuristic Tree Walker**:
    - Implement a traverser that visits the `Ctrl` tree.
    - Sort siblings based on visual geometry (Top-to-Bottom, Left-to-Right) to create a linear "Document Object Model" (DOM) for the scripts.
4. [ ] **Python Bindings (Ctrl/Automation)**:
    - Create `Ctrl/Automation` package.
    - Bind `AccObj` and the Tree Walker to `ByteVM`.
    - Expose event waiting (`WaitReady`, `WaitTime`).
5. [ ] **Maestro Integration**:
    - Integrate `ByteVM` into `Maestro`.
    - Implement the Test Runner CLI/GUI.
6. [ ] **AI Mocking Layer**:
    - Implement deterministic AI response mocking.
7. [ ] **GDB Crash Test**:
    - End-to-end verification script.