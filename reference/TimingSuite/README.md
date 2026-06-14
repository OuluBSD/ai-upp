# Timing Suite

This directory is a reference implementation and usage guide for the timing suite that lives across three layers:

- `uppsrc/Core/Timing.h` and `uppsrc/Core/Timing.cpp`
- `uppsrc/CtrlCore/Timing.h` and `uppsrc/CtrlCore/Timing.cpp`
- `uppsrc/CtrlLib/Timing.h` and `uppsrc/CtrlLib/Timing.cpp`

The sample in this directory is intentionally small. Its real purpose is to show how the stack is supposed to be used in a real application, not just to demonstrate that the code compiles.

For a new project that wants to adopt timing, the usual starting point is simple:

- add `TIMING` to the build config
- embed `TimingWidget` as a developer-only tab, dock page, or modal diagnostics window
- add a few well-placed `TimingScope` points in the code you suspect is slow
- use `TimingContextScope` to separate screens, modes, or pipeline phases
- keep callstack capture off until you need it

The sample uses both a visible GUI viewer and a headless reporting path so you can copy whichever part fits your project first.

## What This Suite Is For

The original `LTIMING` use case is still valid: place timing points in hot code paths and see where time goes when a GUI loop, paint pass, or headless pipeline feels slow.

This suite expands that idea into a reusable in-app timing system:

- `Core` owns the data model, recording, grouping, timeline retention, and optional callstack capture.
- `CtrlCore` exposes a low-level viewer that can inspect collected data in GUI code.
- `CtrlLib` adds a reusable frontend widget that applications can embed directly.

The goal is not to replace external profilers. The goal is to make "what is slow right now?" visible inside the running app, with as little extra scaffolding as possible.

## The Three Layers

### `uppsrc/Core/Timing.h`

This is the engine.

It defines the data model and the timing runtime:

- `TimingKey`
- `TimingCallstack`
- `TimingSample`
- `TimingRecord`
- `TimingStore`
- `TimingManager`
- `TimingScope`
- `TimingContextScope`

Important points:

- `TimingScope` is the RAII object you place around the code you want to measure.
- `TimingContextScope` lets you group otherwise identical code paths by higher-level state.
- `TimingManager::Global()` is the central coordinator for the current process.
- `TimingStore` retains aggregate records and, if requested, the full timeline history.
- Callstack capture is optional and can be disabled independently from timing collection.
- `flagTIMING_DEFAULT_OFF` lets the code be compiled in while starting disabled.

The data model is deliberately thread-aware:

- records are keyed by label, thread, context, and optional callstack
- timeline entries preserve sequence order
- the same label can appear many times in different contexts

### `uppsrc/CtrlCore/Timing.h`

This is the GUI-facing inspection layer.

`TimingView` is a lightweight control that renders the collected timing data and supports:

- summary mode
- timeline mode
- context filtering
- thread filtering
- row selection
- keyboard navigation

The key design point is separation:

- `Core` keeps the data
- `CtrlCore` visualizes the data
- `CtrlLib` turns it into something reusable for application code

`TimingView` is intentionally low level. It is good for embedding into tool windows, diagnostics panels, and developer-only views.

### `uppsrc/CtrlLib/Timing.h`

This is the user-facing widget layer.

`TimingWidget` wraps `TimingView` with controls for:

- toggling active collection
- toggling callstack capture
- toggling retained timeline mode
- filtering by context
- filtering by thread
- refreshing and clearing
- inspecting the selected row

If you want one class that an application can drop into a debug panel, this is the layer to use.

For a new project, this is usually the best first integration point:

- add it to an existing developer tab strip if your app already has one
- otherwise open it as a modal or modeless diagnostics window
- keep it hidden in normal release builds unless the project explicitly wants the feature always available
- wire it to a menu item, hotkey, or developer toolbar action so it is easy to open during investigation

The widget is most useful when it is close to the running app, because you can inspect timings in the same context that produced them.

## The Reference Sample

The sample app in `reference/TimingSuite/main.cpp` demonstrates two workflows:

### GUI workflow

- opens a `TopWindow`
- embeds `TimingWidget`
- shows retained timeline history
- lets you re-run the sample workload from inside the app

### Headless workflow

- runs the same instrumented workload
- prints the timing dump to the console
- is useful when you want timing collection but do not want a visible viewer

The sample records several distinct contexts:

- `main-menu`
- `gameplay`
- `highscore`
- `pipeline/<n>`

That matters because real applications often need to separate timings by state, not just by function name.

## Why Context Matters

The same function can be cheap in one state and slow in another.

Examples:

- a GUI update loop might behave differently in the main menu and in-game
- a paint path might depend on which screen is active
- a pipeline can run once per item and each item can have a different cost

So the suite supports a context string alongside the label.

If you are instrumenting a game, editor, or streaming pipeline, use contexts aggressively. They make the output much more useful than a single flat list of timings.

## Timeline Retention

There are two different ways to think about timing data:

### Repeated-process view

This is the classic profiling view:

- the same hot paths repeat
- you care about totals, averages, minimums, maximums, and counts
- the UI should show the current hotspots

### Retained timeline view

This is the more useful mode for long-running or pipeline-style workloads:

- every entry is preserved
- values are allowed to vary over time
- nothing overwrites the previous sample
- the UI can browse the whole execution history

This is especially important when the "thing being measured" is not one repeated loop, but a stream of changing items.

## Callstack Capture

Callstack capture is available, but it should be treated as an optional diagnostic tool, not as the default mode.

Recommendations:

- keep it off unless you are actively chasing a hard-to-explain hotspot
- enable it temporarily when label/context data is not enough
- disable it in performance-sensitive runs

The suite is designed so timing collection can stay on while callstack capture remains off.

That separation matters. A lot of applications want to pay the cost of recording timings, but not the cost of walking call stacks on every sample.

## Build Flags

The code is gated by `flagTIMING`.

Typical usage:

- add `TIMING` to the mainconfig or package flags
- compile the timing code in
- keep it off in normal builds unless you need it

`flagTIMING_DEFAULT_OFF` is for applications that want the feature compiled in but not actively collecting until the app turns it on.

That is a good fit for:

- performance-conscious tools
- applications that only want to enable timing from a debug panel
- builds where the instrumentation must exist, but should stay idle by default

## Macros

The existing timing/profiling macros already cover the common cases:

- `TIMING(x)` maps to `RTIMING(x)` in debug builds and to a no-op when timing is disabled
- `RTIMING(x)` is the runtime profiling scope from `Core/Profile.h`
- `DTIMING(x)` is the debug-only variant used when you want release builds to complain if profiling clutter remains
- `TIMESTOP(x)` and `RTIMESTOP(x)` are the matching timer-stop macros
- `ACTIVATE_TIMING()` / `DEACTIVATE_TIMING()` and `RACTIVATE_TIMING()` / `RDEACTIVATE_TIMING()` toggle the legacy timing inspector state

`LTIMING` is not a timing-suite-specific feature. It is a local alias pattern used throughout `uppsrc` to keep call sites readable while still mapping to the same underlying profiler primitive.

That means:

- yes, `LTIMING` can be reused
- it is best reused as a package-local or file-local alias
- it should stay a compatibility convenience, not a separate profiling system

Example patterns:

```cpp
#define LTIMING(x) // RTIMING(x)
```

or:

```cpp
#define LTIMING(x) // TIMING(x)
```

Which one you pick depends on whether you want to mirror the old runtime inspector style or the newer timing-suite style. The important part is consistency inside a package.

If you want cleaner source in a new project, define one local alias per package or subsystem and use it everywhere there. Avoid inventing many new timing macro names for the same thing.

## Automatic Scope Exit

There is no need to write a manual "end-timing-of-this-function" call.

The timing system is RAII-based:

- `TimingScope` starts timing in its constructor
- its destructor records the elapsed time automatically when the scope ends
- the same pattern is used by the existing `RTIMING(x)` / `TIMING(x)` macro family through `TimingInspector::Routine`

That means the clean usage pattern is simply:

```cpp
void Update()
{
    TimingScope timing("Update");
    // work here
}
```

or, in macro form:

```cpp
void Update()
{
    TIMING("Update");
    // work here
}
```

This is important for readability:

- no manual end marker is required
- early returns still record the time correctly
- exceptions still unwind through the destructor path

If a project still wants a local alias such as `LTIMING`, it should only wrap the start point. The destructor behavior is already automatic underneath.

### Before And After

Without RAII timing, people sometimes expect a paired end call:

```cpp
void Update()
{
    StartTiming("Update");
    // work here
    EndTiming("Update");
}
```

With this suite, that second line is not needed:

```cpp
void Update()
{
    TimingScope timing("Update");
    // work here
}
```

The same idea applies to the macro form:

```cpp
void Update()
{
    TIMING("Update");
    // work here
}
```

## Keeping Usage Clean

The suite works best when instrumentation stays readable and sparse.

Suggested habits:

- keep the alias local to the `.cpp` file or package where it is used
- use one naming convention consistently in a package
- keep labels short and stable
- place timing points around real hot paths, not around every helper
- prefer one obvious timer per code block
- use `TimingContextScope` for states, screens, or pipeline stages
- use `TimingWidget` to inspect data, not to replace disciplined instrumentation

The goal is to keep call sites obvious for the next person reading the code. If the timing code becomes noisy, the signal is worse even if the feature is technically present.

## Suggested Usage Pattern

In code, the usual pattern is:

```cpp
{
    TimingContextScope context("gameplay");
    TimingScope t("MySystem::Update");
    ...
}
```

The design intent is:

- place `TimingScope` only around code you actually want to measure
- use `TimingContextScope` to separate states
- keep labels stable and human-readable
- keep the instrumentation close to the code you are diagnosing
- if the project already uses `LTIMING`, keep it as a local alias rather than rewriting everything at once

Do not instrument everything by default. The suite becomes much more useful when the measured points are intentional and scarce.

## Starting A New Project

If you are adding the timing suite to a project that does not have any timing yet, start with this order:

1. Add the `TIMING` flag and rebuild.
2. Add one `TimingWidget` entry point to the application UI.
3. Make that entry point available in a developer tab, tool window, or modal diagnostics dialog.
4. Add 3 to 5 high-value `TimingScope` points around the slow path you are currently investigating.
5. Group the measurements with `TimingContextScope` if the same code can run in multiple screens or modes.
6. Keep callstack capture disabled until the plain label/context output is not enough.
7. Turn on retained timeline mode only when you need to study changing values over time.

This keeps the first integration small and low-risk.

## Practical Suggestions

- Use context names for major screens, phases, and pipeline stages.
- Keep callstack capture off until label/context data is not enough.
- Prefer short, stable labels over dynamic strings.
- For thread-heavy code, keep the thread filter visible in the widget and make thread IDs easy to compare.
- If a function is called in a loop, instrument the loop body, not just the outer container.
- If the code is pipeline-style, keep timeline retention enabled so no sample is lost.
- If you are investigating GUI stalls, instrument the refresh path, paint path, and any work queued from callbacks or timers.
- If the problem is headless, use the same engine and print the dump instead of forcing a visible UI.

## What To Watch Out For

- A single aggregate view can hide important differences between contexts.
- Timeline retention can grow quickly if you keep every sample forever.
- Callstack capture can be expensive enough to distort what you are trying to measure.
- Thread IDs are usually easier to inspect when displayed in hex.
- "Refresh" and "clear" are not the same thing. Clear should really clear the store, not just repaint the widget.

## How To Read The Sample

The sample is meant to show a workflow:

1. enable timing
2. record a few context-separated runs
3. keep timeline retention on
4. inspect the collected data in the GUI
5. optionally run in headless mode and print the dump

The sample itself is not the important part. The important part is the shape of the system:

- `Core` gathers the facts
- `CtrlCore` lets you inspect them in a window
- `CtrlLib` gives you a reusable control
- `reference/TimingSuite` shows how an app would actually use them

## Compatibility Notes

This suite is meant to coexist with the old `LTIMING` use case.

That means:

- existing `LTIMING` call sites should continue to compile
- the new timing stack should not force source changes in unrelated code
- the new suite should be opt-in

If you are extending the system, preserve that compatibility. It is more valuable than a cleaner-looking API that breaks existing instrumentation.

## If You Are Integrating This Into An App

Start with the smallest useful setup:

- turn on `TIMING`
- keep callstack capture off
- add a few high-value `TimingScope` points
- separate states with `TimingContextScope`
- embed `TimingWidget` as a new tab, a tool window, or a modal diagnostics window
- keep the entry point easy to open from a menu item or developer hotkey
- if the codebase already has `LTIMING`, reuse it as the local profiling alias instead of inventing a new one

Then expand only when you need more visibility.

The suite is most effective when it is used as a focused diagnostic tool, not as a permanent always-on subsystem.
