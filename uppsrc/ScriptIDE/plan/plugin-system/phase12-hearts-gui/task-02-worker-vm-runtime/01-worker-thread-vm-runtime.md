# Worker-Thread VM Runtime For Hearts GUI

## Goal

Move `.gamestate` execution for the Hearts GUI off the main GUI thread so that:

- long-running Python logic does not freeze `ScriptIDE`
- multiple `.gamestate` documents can later run in the same `ScriptIDE` instance
- GUI-facing `hearts_view` calls are marshalled safely to the UI thread
- autoplay, timers, and future AI/background work remain responsive

## Current Problem

`CardGameDocumentHost` currently calls `plugin->Execute(path)` and later `vm->Call(...)` directly on the GUI thread.

That causes three architectural failures:

1. A Python loop like `autoplay_loop()` blocks the event loop and starves repaint/input.
2. GUI state changes are mixed with VM execution, which makes future multi-VM hosting brittle.
3. The current workaround direction ("yield periodically") still keeps the wrong ownership model.

## Target Model

Each `.gamestate` document host owns:

- one worker thread responsible for all ByteVM calls for that document
- one VM task queue for GUI-to-VM work
- one UI command queue for VM-to-GUI updates
- one coalesced `PostCallback()` flush to apply queued UI commands on the main thread

This keeps VM execution serialized per document while keeping GUI updates batched and safe.

## Design

### 1. Per-document VM worker

`CardGameDocumentHost` should own:

- a worker thread
- a mutex-protected queue of pending VM tasks
- a shutdown flag / wakeup condition

Only the worker thread may call:

- `plugin->Execute(path)`
- `vm->Call(start/refresh_ui/on_click/on_button/...)`

### 2. Coalesced GUI flush

All `IHeartsView` methods should become enqueue-only when called from the VM worker:

- `SetCard`
- `MoveCardToZone`
- `ClearSprites`
- `SetLabel`
- `SetButton`
- `SetHighlight`
- `SetStatus`
- `Log`
- `SetTimeout`

They should append a command to a UI command queue and schedule at most one `PostCallback()` for draining.

The drain callback must:

- run on the GUI thread
- apply all queued commands in order
- update controls/sprites once per batch
- trigger one `Refresh()` / scene sync pass, not one per command

### 3. GUI-to-VM direction

User actions should stop calling Python directly from the GUI thread.

Replace direct calls in:

- `InvokePythonButton`
- `InvokePythonCard`
- initial `Load()` execution path
- timer callback path

with queueing into the VM worker.

### 4. Timer ownership

Timers should be created on the GUI thread, but timer expiration must enqueue VM work instead of executing Python inline on the UI thread.

This preserves CtrlCore threading rules while keeping ByteVM work off the GUI thread.

### 5. Multiple VM readiness

The worker/queue model should be document-local rather than global.

That avoids accidental cross-document coupling and keeps future multi-document `.gamestate` hosting straightforward.

## Implementation Steps

1. Add document-local worker-thread and queue primitives to `CardGameDocumentHost`.
2. Move initial `plugin->Execute(path)` into the worker thread.
3. Convert all VM entrypoints (`refresh_ui`, `on_click`, `on_button`, timer callbacks) to queued worker tasks.
4. Convert all `IHeartsView` methods to produce queued UI commands instead of direct control mutation.
5. Add a single coalesced UI drain callback that applies queued changes.
6. Remove direct GUI-thread `vm->Call(...)` usage from the Hearts runtime path.
7. Rework autoplay to use this new model without blocking the window.

## Acceptance Criteria

1. Opening `game.gamestate` does not block the `ScriptIDE` window during startup.
2. `--autoplay` visibly progresses while the window remains interactive.
3. Clicking cards/buttons keeps the GUI responsive even while AI turns are processed.
4. All GUI mutations still occur on the GUI thread only.
5. More than one `.gamestate` document can be opened without a shared global VM bottleneck.

## Risks

### Shared mutable state

The current host stores sprite/button/label state in mutable containers that were implicitly GUI-thread-owned.

After the refactor, these must be mutated only during the queued GUI drain or guarded carefully.

### Over-posting

If each `hearts_view` call posts separately, responsiveness will improve but overhead and ordering bugs will remain.

The whole point of this task is to batch them.

### Re-entrancy

`refresh_ui()` currently assumes a fairly direct synchronous flow. The queued model must preserve ordering without allowing overlapping VM calls for the same document.

## Notes

- Do not attempt to "fix" the current model by periodically yielding the GUI thread from Python. That is not the target architecture.
- `PostCallback()` usage should be centralized into a single queue-flush path for VM-to-GUI updates.
- Future generic plugin/runtime work can reuse this pattern beyond Hearts.
