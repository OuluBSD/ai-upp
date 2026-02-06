# Task: Playbook Manager Integration

# Status: DONE

# Description
Integrate Playbook management into the GUI, enabling users to browse, edit, and apply expert strategies (Playbooks) for common development tasks. This component serves as the "Instruction Manual" repository for AI operations.

# Objectives
- **Strategy Browser**: Create a list view of available Playbooks (system-wide and project-specific).
- **Metadata Editor**: Support editing Playbook intent, domain, and triggers.
- **Visual Logic Editor**: Integrate `StateEditor` / PUML components to visualize and edit the underlying state machines.
- **Application Logic**: Link Playbooks to the AI Assistant for "Guided Operations".

# Technical Tasks
- [x] Implement `PlaybookPane` class in `MaestroHub.h/cpp`.
- [x] Add `PlaybookManager` integration to load `.playbook` (JSON) files.
- [x] Add visual logic editor (StateEditor integration).
- [x] Create `PlaybookSelectDialog` for choosing strategies during work sessions.
- [x] Implement "Validation" pass to ensure PUML states match Playbook step definitions.

# UI Requirements (WinXP Aesthetic)
- **Left**: Playbook Tree (Category -> Strategy).
- **Center**: Tabbed View (General Info, Visual Logic, JSON Raw).
- **Toolbar**: "New Strategy", "Clone", "Validate", "Publish to Cloud".
- Use `LabelBox` for metadata groups (e.g., "AI Constraints", "Domain Expertise").
