AGENTS

Scope
- Applies to `upptst/WmrTest`.

Purpose
- WmrTest is the integration test app for SoftHMD camera + tracking + fusion.
- Planning artifacts for tracking work live under `upptst/WmrTest/plan/`.

Workflow
- Before changes, read `upptst/WmrTest/plan/cookie.txt` and relevant plan/task files.
- Update plan tasks as work progresses and record completions in `plan/cookie.txt`.
- Keep UI/debug toggles in WmrTest minimal and explicit; prefer menu-driven toggles.
- Interface naming convention: abstract interfaces use concrete names without a `Base` suffix; implementations are prefixed (e.g., `SoftHmdVisualTracker`).
- GeomEvent should be extended for VR/HMD/controller events; watch out for trivial/union constraints if required.
