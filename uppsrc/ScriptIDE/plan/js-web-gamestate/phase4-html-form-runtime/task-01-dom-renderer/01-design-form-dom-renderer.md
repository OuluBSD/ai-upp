# Task: Design `.form` HTML/DOM Renderer

## Goal

Render the existing real U++ `.form` files as browser UI using HTML elements, CSS positioning, and JavaScript behavior, with no canvas dependency.

## Background / Rationale

The current game host uses semantic zones such as `HAND`, `TRICK`, `LABEL`, and `BUTTON`. The browser runtime must preserve those semantics while consuming the same `.form` source used by the desktop path.

## Scope

- Parse or serialize real `.form` XML into a browser-consumable model
- Map U++ form controls and properties into HTML elements
- Support positioning, anchors, labels, buttons, highlight state, and sprite overlays

## Non-goals

- Canvas rendering
- Full general-purpose U++ widget parity
- A visual designer implemented in the browser

## Proposed Rendering Model

1. Host loads the real `.form`.
2. Host converts relevant runtime geometry and metadata into a normalized JSON tree for the browser.
3. Browser renders:
   - container divs for layout regions
   - label elements for `LABEL`
   - button elements for `BUTTON`
   - absolutely positioned img/div nodes for card sprites
4. Browser applies UI diffs from the session socket instead of reloading the page.

## Important Constraint

Do not reintroduce the deprecated custom JSON `.form` format. The browser renderer must derive from the real U++ form model, even if it consumes a normalized transport representation at runtime.

## Open Design Points

- Whether geometry normalization lives in `ScriptCommon` or `ScriptIDE`
- How much anchor math is computed host-side versus browser-side
- Whether sprites are represented as children of zones or as a separate overlay layer

## Risks

- U++ form semantics may not map 1:1 onto CSS layout unless the phase-1 subset is kept intentionally small.
- If browser layout is computed differently from desktop layout, reference projects will drift.

## Acceptance Criteria

- [ ] Mapping from real `.form` properties to browser representation documented
- [ ] Supported control subset documented
- [ ] Zone/sprite layering model documented
- [ ] Desktop/browser parity risks documented
