# Task: VideoRecord Plugin Scaffold

## Goal

Create the first reusable `ScriptIDE` plugin scaffold for video recording so that recording features can be implemented without adding direct package coupling from `uppsrc/ScriptIDE/` to a plugin package.

## Background / Rationale

The current plugin system already supports file handlers, dock panes, Python bindings, and execute providers. The VideoRecord work needs one more pass on the public GUI plugin API so external plugins can:

- register from their own `.icpp`
- add optional preference pages/categories
- listen to run-state changes
- react to recordable document hosts through capability interfaces
- provide viewer and/or editor hosts for file extensions without `ScriptIDE` including plugin headers

This task captures the infrastructure slice only. It does not implement actual frame capture or ffmpeg export yet.

## Scope

- Add GUI plugin interfaces for plugin-owned preference pages.
- Add GUI plugin interfaces for run-state listeners.
- Add capability interface(s) for recordable document hosts.
- Extend file handler APIs so a plugin can expose viewer and/or editor document hosts explicitly.
- Add a new `VideoRecord` plugin package registered through plugin macros only.
- Add a preferences page scaffold for video-recording settings.

## Non-goals

- Actual video encoding.
- Frame capture implementation.
- ffmpeg process execution.
- Toolbar/menu recording controls.

## Dependencies

- Existing ScriptIDE plugin manager and registry.
- Existing document-host abstraction.

## Concrete Investigation Steps

1. Define the missing GUI plugin interfaces in `PluginInterfacesGUI.h`.
2. Extend `PluginManager` to track preference providers and run-state listeners.
3. Wire plugin preference pages into `PreferencesWindow`.
4. Add explicit viewer/editor host support to `IFileTypeHandler`.
5. Add `IVideoRenderSource` capability to recordable document hosts.
6. Create `uppsrc/ScriptIDE/VideoRecord` as a plugin-only package.

## Affected Subsystems

- ScriptIDE plugin API
- ScriptIDE preferences window
- ScriptIDE plugin manager
- Card-game document host capability surface
- New VideoRecord plugin package

## Implementation Direction

Keep `ScriptIDE` depending only on forward-declared/public plugin interfaces. All concrete plugin code must remain inside the plugin package and register itself through a package-owned `.icpp`.

For preferences UI, start with a flat navigation entry that includes category information in the label. A richer grouped preferences UI can come later without changing plugin ownership boundaries.

## Risks

- Expanding the plugin API too narrowly could force another interface break when real recording is added.
- Expanding it too broadly could add dead surface area before the first real plugin feature is implemented.

## Acceptance Criteria

- [ ] `ScriptIDE` exposes public GUI plugin interfaces for preference pages and run-state listeners.
- [ ] File handlers support explicit viewer/editor host creation.
- [ ] A plugin-owned preferences page can appear in the Preferences window without `ScriptIDE` including plugin headers.
- [ ] A `.gamestate` document host can advertise recordability through a capability interface.
- [ ] `uppsrc/ScriptIDE/VideoRecord` builds and registers as a ScriptIDE plugin scaffold.
