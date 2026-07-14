AGENTS

Scope
- Applies to `uppsrc/CardBoardEditor` and its package files.

Purpose
- CardBoardEditor is a general card-game board editor. Poker provider boards are the first concrete target.
- The runtime renderer must stay standalone enough to replace a `.form` based poker table view.
- GUI editing is a wrapper around the shared document/model/renderer path; headless diagnostics must exercise the same data.

Conventions
- Keep `CardBoardEditor.h` as the only package umbrella header.
- Source files include `CardBoardEditor.h` first.
- Subheaders included from `CardBoardEditor.h` must not include other headers or open `namespace Upp`.
- Prefer relative geometry (`0.0..1.0`) for board elements unless a fixed pixel inset is explicitly required.
- Add CLI/stdout diagnostics for document loading, tree dumps, realized rects, and renderer smoke checks.
