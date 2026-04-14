# Specify Document Schema V1

## Purpose
Define the V1 on-disk/in-memory interchange schema for Node documents, including entity tables, style references, and metadata blocks.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Define schema sections for graph metadata, nodes, pins, edges, groups
- Define required vs optional fields and defaulting rules
- Define schema examples mapped from GraphLib tutorial graphs

## Out of Scope
- Implementing parser/writer code
- Command execution semantics
- Ctrl clipboard adapter format

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-model-document/01-domain-boundary-freeze/02-separate-persistent-vs-runtime-state.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/GraphNodeCtrl.cpp`
- `legacy reference only: ./tutorial/GraphLib2/main.cpp`

## Implementation Notes
- Document schema as deterministic and versioned from day one
- Avoid encoding transient UI state in core document schema
- Include explicit notes for unsupported legacy fields
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Schema document includes complete field definitions and examples
- [ ] At least one example covers pins and groups
- [ ] Schema excludes Ctrl-specific data

## Suggested Validation
- schema review
- golden schema fixture review
