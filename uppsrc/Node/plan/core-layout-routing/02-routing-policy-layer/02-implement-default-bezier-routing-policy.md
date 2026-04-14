# Implement Default Bezier Routing Policy

## Purpose
Implement the default Core routing policy that reproduces and improves current GraphLib curved-edge behavior.

This task must keep domain logic, scene logic, and algorithmic ownership in Core and avoid any dependency on Ctrl-level event plumbing.

## Scope
- Implement anchor selection and control-point generation in policy module
- Emit arrow and label anchor metadata through contract outputs
- Add deterministic fallback behavior for degenerate geometries

## Out of Scope
- Orthogonal routing policy
- Ctrl paint playback
- Edge reroute interaction UI

## Package Ownership
- `Node/Core`

## Depends On
- `./uppsrc/Node/plan/core-layout-routing/02-routing-policy-layer/01-define-routing-policy-contract.md`

## Touches
- `./uppsrc/Node/Core/...`
- `./uppsrc/Node/plan/...`
- `legacy reference only: ./uppsrc/GraphLib/Renderer.cpp`

## Implementation Notes
- Use GraphLib DrawCurvedEdge logic as migration evidence while cleaning API boundaries
- Ensure policy returns geometry only and no paint calls
- Keep routing deterministic for golden tests
- Keep Core free of `Ctrl` subclasses, menu/focus policy, and OS clipboard integration.

## Acceptance Criteria
- [ ] Default routing policy matches expected curved-edge scenarios
- [ ] Degenerate cases are handled explicitly
- [ ] No Ctrl dependencies are introduced

## Suggested Validation
- golden path tests
- unit tests
- compile checks
