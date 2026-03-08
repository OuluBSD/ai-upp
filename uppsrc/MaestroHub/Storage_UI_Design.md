# Storage Topology & Migration UI Design

## 1. Storage Topology Selector
- **Context**: Project Settings or `init` Wizard.
- **UI Element**: `DropList` or `Switch` control.
- **Options**:
  - **Standard**: `docs/maestro/` (Git-visible, human-readable).
  - **Unix-style**: `docs/maestro/` (Standard metadata).
  - **Distributed**: Adjacent to source files (e.g., `MegaFileUtil/Task1.md`).
- **Isolation control**: Checkbox for "Monorepo Isolation" + Path input field to define the boundary.

## 2. Mechanical Converter (The Layout Migrator)
- **Goal**: Safely move metadata from one topology to another without content loss.
- **View Architecture**:
  - **Source Map**: Tree showing current file locations.
  - **Target Preview**: Ghosted tree showing intended locations.
  - **Action Bar**: "Perform Mechanical Migration" + "Validate Integrity".
- **Progress Monitor**: Detailed log showing every file `mv` and reference update.

## 3. Storage Diagnostics
- **Status Indicator**: "Topology: [Distributed] - Integrity: [OK]".
- **Auto-Detect**: Button to scan for orphaned Maestro metadata outside the current topology.