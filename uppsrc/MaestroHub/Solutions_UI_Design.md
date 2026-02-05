# Known-Fix Pattern Library (Solutions) UI Design (WinXP Style)

## 1. "Solutions Explorer" Window
- **Style**: Standard MDI child or modal dialog with a sidebar.
- **Left Pane (Category Tree)**: TreeCtrl showing solution categories (Build, Runtime, Logic, Style).
- **Right Pane (Pattern List)**: ArrayCtrl with columns: [Pattern ID, Name, Matches, Confidence].
- **Bottom Details**: Static text area showing the solution description and a code diff preview.

## 2. "Match Detected" Notification
- **Style**: Classic taskbar-like pop-up or a specific icon/color change in the Log Analyzer.
- **Action**: Clicking "View Fix" opens the Remediation Dialog.

## 3. "Remediation Wizard" (One-Click Fix)
- **Style**: Standard Windows Wizard (Step-by-step).
- **Step 1: Compare**: Side-by-side diff view using classic gray background for unchanged lines.
- **Step 2: Options**: Checkbox list for "Auto-commit after fix", "Run verification build", "Keep original as .bak".
- **Step 3: Execution**: Progress bar with a "Applying fix pattern..." label.
- **Buttons**: [ < Back ] [ Next > ] [ Finish ] [ Cancel ]

## 4. "Add New Solution" Dialog
- **Style**: Form-heavy dialog with GroupBoxes.
- **GroupBox "Match Condition"**: 
  - EditString for Regex pattern.
  - DropList for "Trigger Tool" (e.g. gcc, clang-tidy).
- **GroupBox "Action"**:
  - EditString for Transformation template.
  - Button "Test against current finding".
- **Buttons**: [OK] [Cancel]