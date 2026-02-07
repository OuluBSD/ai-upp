# Task: Visual State Editor

# Status: TODO

# Description
Implement real-time PUML-to-Graph visualization in the `StateEditor`.

# Objectives
- [ ] Implement a lightweight PUML parser (Regex-based) for state diagrams.
- [ ] Map PUML `state` definitions to `GraphLib::Node`.
- [ ] Map PUML `-->` transitions to `GraphLib::Edge`.
- [ ] Implement automatic layout or basic positioning for generated nodes.
- [ ] Update the `graph_view` whenever the `puml_editor` content changes (debounced).

# Requirements
- Support `state Name` and `state Name { ... }`.
- Support `[*] --> State` (initial) and `State --> [*]` (final).
- Support labels on transitions: `StateA --> StateB : Event`.
