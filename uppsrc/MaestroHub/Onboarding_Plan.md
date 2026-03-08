# MaestroHub Onboarding & "Time to Value" Plan

## 1. Zero-State Experience
- **Initial State**: When no project is loaded, the `Fleet Dashboard` should show a "Get Started" card.
- **Guidance**: Buttons for "Open Existing Project" and "Initialize New Project" are prominently displayed in the center of the dashboard.
- **Empty State**: Functional hubs (Intelligence, Evidence) show helpful "No data to display. Please initialize your project first." placeholders.

## 2. The "First 5 Minutes" Journey
1. **Welcome**: `WelcomeDialog` appears on launch, explaining the Cockpit's mission.
2. **Setup**: User is guided to the `InitDialog`.
3. **Observation**: User is directed to the `Interactive Guide` (bottom tab) which explains the 3-pane layout.
4. **Action**: User is encouraged to run an "Inventory Scan" to see the `TU Browser` and `Repo View` populate.

## 3. Contextual Help Strategy
- **Hover Support**: Tooltips for all toolbar icons.
- **"What is this?"**: A small `?` button next to major LabelBox headers that opens a help popup.
- **Assistant Prompting**: If the user is idle in a "Zero State" for more than 30 seconds, the AI Assistant should offer a suggestion: "Would you like me to help you initialize this repository?"

## 4. UI Recommendations
- **Pulse Animation**: Use a subtle highlight on the `Select Project` button during the first run.
- **Progress Tracking**: The `Interactive Guide` tracks completion of onboarding steps (e.g., "Step 1: Project Init - [DONE]").
- **WinXP Aesthetic**: Maintain standard system dialogs for a familiar, "expert" feel that reduces anxiety for new users.