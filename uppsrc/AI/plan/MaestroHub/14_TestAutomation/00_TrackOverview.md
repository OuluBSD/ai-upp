# Track 14: Deep Automation & Accessibility

# Goal
Build a native accessibility and automation layer into the U++ Core, enabling Python scripts (`ByteVM`) to inspect, interact with, and verify the GUI state semantically, rather than just via coordinates.

# Tasks
1. [x] **Core Accessibility (CtrlCore)**:
    - Define `Visitor` class/struct.
    - Add `virtual void Access(Visitor&)` to `Ctrl` class.
    - Add `Event<Visitor&> WhenAccess` to `Ctrl` for dynamic binding.
2. [x] **Standard Controls (CtrlLib)**:
    - Implement `Access()` overrides for all major widgets: `Button`, `Label`, `EditField`, `TabCtrl`, `ArrayCtrl`, `TreeCtrl`, `Switch`, `Option`.
    - Ensure complex controls like `ArrayCtrl` expose their content (rows/columns) via `Visitor`.
3. [x] **Heuristic Tree Walker**:
    - Implement a traverser that visits the `Ctrl` tree.
    - Sort siblings based on visual geometry (Top-to-Bottom, Left-to-Right) to create a linear "Document Object Model" (DOM) for the scripts.
4. [x] **Python Bindings (Ctrl/Automation)**:
    - Create `Ctrl/Automation` package.
    - Bind `Visitor` and the Tree Walker to `ByteVM`.
    - Expose event waiting (`WaitReady`, `WaitTime`).
5. [x] **Maestro Integration**:
    - Integrate `ByteVM` into `Maestro`.
    - Implement the Test Runner CLI/GUI.
6. [x] **AI Mocking Layer**:
    - Implement deterministic AI response mocking.
7. [x] **GDB Crash Test**:
    - End-to-end verification script.