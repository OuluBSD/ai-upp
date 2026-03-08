# Task: Visual State Editor

# Status: DONE

# Description
Implement real-time PUML-to-Graph visualization in the `StateEditor`.

# Objectives
- [x] Implement a lightweight PUML parser (Regex-based) for state diagrams.
- [x] Map PUML `state` definitions to `GraphLib::Node`.
- [x] Map PUML `-->` transitions to `GraphLib::Edge`.
- [x] Implement automatic layout (grid-based fallback).
- [x] Update the `graph_view` whenever the `puml_editor` content changes (debounced).

# Requirements
- [x] Support `state Name` and `state Name { ... }`.
- [x] Support `[*] --> State` (initial) and `State --> [*]` (final).
- [x] Debounced preview update.