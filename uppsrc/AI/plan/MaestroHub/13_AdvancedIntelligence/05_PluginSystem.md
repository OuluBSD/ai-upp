# Task: Plugin System (External Tools)
# Status: DONE

## Objective
Define and implement a plugin interface for `MaestroHub`, allowing external tools to register themselves and provide specialized functionality to the AI.

## Requirements
- Define `MaestroPlugin` interface.
- Implement a plugin loader that scans a `plugins/` directory.
- Allow plugins to register new tools in `MaestroToolRegistry`.
- Allow plugins to add new panes or menu items to the Cockpit.
- Support dynamic loading/unloading if possible.
