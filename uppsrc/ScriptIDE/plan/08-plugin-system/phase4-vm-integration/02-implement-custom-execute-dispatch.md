# Task: Implement Custom Execute Dispatch

## Goal
Enable plugins to define what happens when "Execute" (F5) is pressed for their specific document types.

## Strategy
1. **Identify Current Dispatch**: Currently, `RunManager::Run` is called with the current editor content.
2. **Hook Execution**:
   - Before executing, check if the active document's extension has a registered `ICustomExecuteProvider`.
   - If so, delegate the execution to the provider.
   - Example: For `.gamestate`, the provider might find a related `.py` file and run that instead.

## Success Criteria
- [ ] Plugins can override the F5 behavior.
- [ ] System falls back to default behavior if no provider is found.
- [ ] Provider has access to the document path and content.
