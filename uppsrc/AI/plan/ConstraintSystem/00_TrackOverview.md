# Track: Constraint System & Logic Integration

# Goal
Implement a robust, scientifically complete Logic Engine (`AI/Logic`) and verify it via CLI (`examples/TheoremProver`) and GUI (`examples/TheoremProverCtrl`). Only once this engine is mature will it be used to power the data-driven `.ugui` constraint system.

# Core Components
1.  **AI/Logic Package**: The core reusable logic engine, expanded with advanced theorem proving capabilities.
2.  **TheoremProverCtrl**: A GUI frontend for visualizing and debugging the logic engine.
3.  **UGUI Format**: YAML-based configuration for defining constraints.
4.  **Constraint Visitor**: Deep integration into `Ctrl::Access` for runtime validation.

# Strategy
- **Quality First**: The `AI/Logic` package must be feature-complete and robust before we attempt to use it for system constraints.
- **Visual Verification**: Use `TheoremProverCtrl` to interactively test logic scenarios.
- **Refactor -> Expand -> Integrate**: Strict sequence of operations.

# Phases
1.  **Refactoring**: Split `TheoremProver` into `AI/Logic` (library) and `examples/TheoremProver` (CLI).
2.  **Expansion**: Implement missing scientific features and improve the quality of `AI/Logic`.
3.  **GUI Tooling**: Create `examples/TheoremProverCtrl` to interface with the logic engine.
4.  **Logic Verification**: Comprehensive testing of the engine.
5.  **Formats & System**: Implement `.ugui` parser and logging.
6.  **Gui Integration**: Hook the robust engine into `Ctrl::Access`.
