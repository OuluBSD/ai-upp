# Task: Internal Observability & System Health

# Status: DONE

# Description
Implement an internal "System Console" for the developer to monitor MaestroHub's internal operations, errors, and AI quota status.

# Objectives
- [x] Add an "Internal Console" tab to the bottom TabCtrl.
- [x] Implement a logging system with levels (INFO, WARN, ERROR) that feeds this console.
- [x] Add `ProgressIndicator` to the status bar for Quota visualization.
- [x] Implement "Quota Detection" logic: if AI error contains "quota" or "limit" (case-insensitive), set quota to 0%.

# UI Requirements
- [x] Console supports color-coded levels.
- [x] Error states in the UI are high-visibility (red status bar).
- [x] Status bar quota segment uses `ProgressIndicator`.