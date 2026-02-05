# MaestroHub User Flows

## Flow 1: The "Bug Fix" Cycle (Critical)
1. **Trigger**: Build error or Log Finding.
2. **Action**: User clicks "Create Issue" in Log Analyzer.
3. **Action**: User opens "Maintenance -> Triage Wizard".
4. **AI Role**: AI suggests severity and fix path.
5. **Action**: User clicks "Accept & Start Work".
6. **Execution**: "Work" tab becomes active; real-time breadcrumbs stream.
7. **Verify**: AI finishes; user clicks "Verify" (runs build/test).

## Flow 2: Project Onboarding
1. **Trigger**: New directory opened.
2. **AI Role**: "Project not initialized" banner appears.
3. **Action**: User clicks "Initialize".
4. **Setup**: Wizard asks for project name and model.
5. **Ready**: "Technology" tab populates with Repo structure.
