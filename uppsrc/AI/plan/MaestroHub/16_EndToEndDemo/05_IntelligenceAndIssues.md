# Scenarios 21-40: Intelligence & Issue Management
# Status: TODO

## Objective
Verify the deep code intelligence features and the end-to-end issue lifecycle.

## Intelligence Hub Scenarios (21-30)
21. **TU Browser - Package Scan**: Load the project and verify 'TUBrowser' lists all `.upp` packages.
22. **Symbol Discovery**: Select a package and verify the 'Symbol List' populates with classes/functions.
23. **Synthesis Flow**: Select a missing symbol (e.g., a planned class) and click 'Synthesize'. Verify AI rationale appears.
24. **Dependency Map**: View the 'RichText' dependency graph and verify links are clickable.
25. **Search - Symbols**: Use the 'SymSearch' box to filter symbols by name.
26. **Cross-Reference**: Find a symbol and verify 'Usage' list shows correct line numbers.
27. **AI Chat Context**: Open Assistant and ask "What does class X do?". Verify the prompt includes the relevant TU context.
28. **Patch Application**: Request a change via AI, and click 'Apply Patch' in the diff pane.
29. **Code Intel UI Scaling**: Load a large package (e.g., `CtrlLib`) and verify search responsiveness.
30. **Symbol Pane Navigation**: Double-click a symbol and verify it opens the relevant file in the 'RepoView'.

## Issue Tracker Scenarios (31-40)
31. **Issue Creation**: Open 'IssueCreateDialog', fill mandatory fields, and verify it appears in the 'IssuesPane'.
32. **Triage Wizard**: Open 'TriageWizard', scan through 5 mock issues, and use 'Accept', 'Skip', and 'Ignore' buttons.
33. **Triage - Edit**: Modify an issue's severity during triage and verify persistence.
34. **Issue-Task Link**: Create a task file (Markdown) from an issue and verify the 'TaskPath' is linked in the UI.
35. **Bulk Operations**: Select 3 issues and use 'Bulk Status' to mark them as "Investigating".
36. **Filter - Severity**: Filter for "Critical" and verify count.
37. **Issue Search**: Verify real-time filtering of the issues `ArrayCtrl`.
38. **Issue Detail View**: Select an issue and verify the 'RichTextView' displays full description and AI suggestions.
39. **Resolve Flow**: Mark an issue as "Resolved" and verify it moves to the bottom of the list or disappears (based on filter).
40. **Manual Sync**: Edit an issue file externally and verify 'LoadData' picks up the change.

## Verification Method
- `ArrayCtrl` content inspection.
- Mock AI responses via `SafetyManager`.
- Check `docs/maestro/issues/` directory.
