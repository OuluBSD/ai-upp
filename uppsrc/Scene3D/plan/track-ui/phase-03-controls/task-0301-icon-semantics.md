# Task 0301 - Icon Semantics Mapping

## Status
- Done (2026-02-17)

## Goal
Replace placeholder ribbon icons with a proper semantic icon registry.

## Scope
- Map icon_semantics tokens to images.
- Provide fallback icons for missing semantics.
- Add style variants for small/large icons.

## Acceptance
- Ribbon icons match their semantics across all tabs.
- Missing icons degrade gracefully with a fallback.

## Implementation Notes
- Semantic icon lookup is mapped to available assets and now probes variant paths for better coverage.
- Unknown or missing semantic keys no longer hard-fail; they log and resolve to fallback icons.
- Ribbon controls required by semantics mapping were assigned stable layout IDs to keep lookups reliable.
