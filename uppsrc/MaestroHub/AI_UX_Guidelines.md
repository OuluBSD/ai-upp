# MaestroHub AI Interaction & Trust Guidelines

## 1. Observability (The "Glass Box" Principle)
- **Visual Feedback**: Every AI tool call MUST be reflected in the `AI Trace` tab.
- **Activity Indicators**: When the AI is "Thinking" (waiting for LLM response), the Status Bar should show a pulsed progress indicator.
- **Context Stack**: The `Maestro Assistant` must display its current "Focus" (Track/Phase/Task) so the user knows exactly what context the AI is using.

## 2. Transparency & Intervention
- **Destructive Actions**: Any action that modifies user code (via `replace` or `write_file`) MUST be accompanied by a Diff view in the `Conversion Factory` or `Work Dashboard`.
- **The "Pause" Hook**: Before executing a sequence of more than 3 tool calls, the AI should provide a summary of intent and await a "Proceed" signal if the task is marked as "Sensitive".
- **Rationale Disclosure**: The `AI Rationale` tab is mandatory. It must explain *why* a transformation was chosen (e.g., "Refactored to BiVectorRecycler to reduce heap fragmentation").

## 3. Handling the "Stuck" State
- **Loop Detection**: If the AI executes the same tool with the same arguments twice, it must abort and ask the user for clarification.
- **The Panic Button**: The `Execution Console` (Debug Workspace) allows the user to "Stop" the active orchestration process immediately.
- **Feedback Channel**: If the AI's proposal is wrong, the "Reject / Feedback" button in the `Work Dashboard` allows the user to provide corrective input which is prioritized in the next reasoning cycle.

## 4. Visual Language for Trust
- **Color Coding**: 
  - `Blue`: Informational / Observation.
  - `Green`: Successful transformation / Verified evidence.
  - `Amber`: Manual intervention required / Potential risk.
  - `Red`: Error / Failed verification / High-risk conflict.
- **Icons**: Use the standard `Network` icon to represent AI connectivity and the `Key` icon for authenticated secure operations.