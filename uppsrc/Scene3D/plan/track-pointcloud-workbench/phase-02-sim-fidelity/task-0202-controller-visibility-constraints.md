# Task 0202 - Controller Visibility Constraints

## Goal
Ensure controller localization only succeeds when controllers are visible in the fake HMD camera frustum.

## Scope
- "Simulate Controller Observations" must place controllers inside the fake camera frustum.
- "Run Controller Localization" must fail or return no result if controllers are outside the frustum.
- Model visibility should reflect controller model position inside the frustum.

## Implementation Notes
- Use the same frustum check as the observation generator.
- Log explicit "not visible" errors when constraints fail.

## Success Criteria
- Controllers outside frustum are not localized.
- Controllers inside frustum are localized reliably.

## Status
- Pending
