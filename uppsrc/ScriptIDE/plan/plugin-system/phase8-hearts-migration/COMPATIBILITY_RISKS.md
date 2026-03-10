# Migration & Compatibility Risk Assessment

## Risk Matrix

| Risk | Description | Mitigation Strategy |
| :--- | :--- | :--- |
| **Network Model** | KDE uses sockets/FDs for player communication. ScriptIDE is local. | Flatten logic into a single-process event loop. Replace sockets with direct method calls. |
| **Qt Event Loop** | KDE relies on Qt's signal/slot and timer system. | Use U++ `Callback`/`Event` and `PostCallback` for async UI updates. |
| **Rendering Perf** | Drawing 52+ high-res PNG cards via `ByteVM` bindings might be slow. | Optimize U++ `Draw` path. Use card caching and only redraw dirty regions. |
| **AI Blocking** | Complex AI logic might hang the IDE UI thread. | Split AI thinking into small chunks or run AI in a separate thread/coroutine pattern. |
| **Asset Scale** | 72x96 assets might be too small for modern HiDPI screens. | Implement DPI-aware scaling in the `.form` loader. |

## Features: Preserve vs. Adapt

### Preserve (Keep Exactly)
- Standard Hearts rules (KDE 2004 defaults).
- AI heuristic weights for card selection.
- Point distribution logic.

### Adapt (Rewrite for U++)
- UI Layout: From Qt `.ui`/code to `.form` JSON.
- Animation: From `QTimer` steps to U++ `SetTimeCallback`.
- Persistence: From KDE `KConfig` to U++ `Serialize` / `.bin`.

## Acceptance Criteria for Parity
- Round passing follows the correct rotation.
- Shooting the moon correctly penalizes opponents.
- AI never plays an illegal card (suit follow).
