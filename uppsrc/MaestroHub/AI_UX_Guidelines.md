# AI Interaction & Trust Guidelines

## Transparency
- **Verbosity**: Always show the *intent* of the current tool call (e.g., "Scanning for symbols...") rather than just raw logs.
- **Progress**: Use visual indicators for multi-step AI plans.

## Control
- **Pause/Resume**: User must be able to halt AI execution at any second.
- **Feedback**: Every AI proposal must have an "Edit/Correction" option before implementation.

## Trust
- **Validation**: Show build/test results immediately after AI edits code.
- **Safety**: Flag "destructive" tool calls (e.g., `rm -rf`, `git push`) with specific icons.
