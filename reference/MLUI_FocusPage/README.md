# MLUI_FocusPage

Reference package for the proposed MLUI FocusPage model.

This package is intentionally design-first:
- It demonstrates the API shape and data flow direction.
- It is not intended to be production-complete yet.

## Why FocusPage

Raw MLUI snapshots can contain hundreds of nodes. FocusPage is a compact, task-oriented view that behaves like one HTML page on a larger website:
- clear context
- small relevant dataset
- explicit actionable commands

## Included Focus Pages

1. `issue_board`
- Target use: navigate and triage issues quickly.
- Core context: workspace, sprint, selected issue.
- Actions: select/status/filter.

2. `issue_editor`
- Target use: inspect and edit selected issue fields.
- Core context: key/title/status/assignee/severity/flags.
- Actions: assign/severity/repro/crash/notes.

## API Shape Demonstrated

Registration (global metadata):
- `INITBLOCK { MLUI::RegisterFocusPage(...) }`

Control binding by reference (window constructor):
- `MLUI::GetFocusPage(\"issue_board\").Add(issue_list);`

Dynamic page output (automation path):
- `bool Access(Visitor& v) override { ... MLUI::EmitFocusPages(av); }`
- Runtime values/controls/actions are added with macro helpers:
  - `MLUI_USE_VAR(page, var, "desc")`
  - `MLUI_USE_CTRL(page, ctrl, "desc")`
  - `MLUI_USE_STATE(page, key, value, "desc")`
  - `MLUI_USE_ACTION(page, id, enabled, "desc")`

## MCP Usage Intent

Intended command flow for agents:
1. `mlui.focus.list`
2. `mlui.focus.get issue_board`
3. `mlui.focus.action issue_board.select_issue {"key":"OVR-130"}`
4. `mlui.focus.get issue_editor`
5. `mlui.focus.action issue_editor.set_severity {"value":4}`

The key point is usability:
- page-sized information first
- raw control tree only as fallback
