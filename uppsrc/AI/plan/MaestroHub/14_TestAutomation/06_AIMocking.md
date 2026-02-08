# Task: AI Mocking Layer

# Status: DONE

# Description
Implement deterministic AI response mocking for test scripts.

# Objectives
- [x] Add `MockMaestroEngine` to `Maestro/Engine.h`.
- [x] Bind `mock_ai(regex, response)` to Python.
- [x] Allow matching prompts via regex for deterministic responses.
- [x] Verify that tests can run without real AI backend by using the mock engine.

# Implementation Details
- `MockMaestroEngine` provides a synchronous, deterministic response based on regex matching.
- Decoupled `Ctrl/Automation` from `Maestro` package using a callback mechanism for `mock_ai`.
- `TestCommand` automatically wires Python `mock_ai` to the `MockMaestroEngine`.