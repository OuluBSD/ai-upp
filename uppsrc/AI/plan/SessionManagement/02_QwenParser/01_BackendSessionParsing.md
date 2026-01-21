# Task: Backend Session Parsing
# Status: DONE

## Objective
Implement specialized session parsers for all supported backends in `CliMaestroEngine::ListSessions`.

## Requirements
- Gemini: Scan `~/.gemini/projects/` or similar storage.
- Claude: Scan its specific local history.
- Codex: Scan project-local history.
- Extract session IDs and meaningful names (from first message).
