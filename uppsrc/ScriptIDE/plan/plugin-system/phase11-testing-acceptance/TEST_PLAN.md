# Testing & Acceptance Plan

## Unit Testing (Automated)
New `autotest` packages will be created for:
- **JSON Parsers**: Verify `.gamestate` and `.form` schemas are parsed into the correct `Value` structures.
- **Plugin Registry**: Verify `PluginManager` correctly tracks enabled/disabled states and extension points.
- **Hearts Logic**: Python unit tests for trick resolution and scoring.

## Integration Testing (Automated)
- **VM Bridge**: A test case that initializes `ByteVM`, injects a mock `BindingProvider`, and asserts that a Python script can call a C++ function and receive the correct return value.
- **Document Routing**: Assert that `PythonIDE::LoadFile` returns the correct `IDocumentHost` type for `.py`, `.gamestate`, and `.form`.

## UI Acceptance (Manual)
### Layout Editor
1. Open a new `.form` file.
2. Drag a `Zone` onto the canvas.
3. Change its ID in the property grid.
4. Save the file and verify the JSON output.

### Plugin Lifecycle
1. Open a `.gamestate` file.
2. Disable the Card Game Plugin in Preferences.
3. Verify the tab closes and `.gamestate` files now open as plain text.
4. Re-enable the plugin and verify the custom view returns.

## Gameplay Parity (Manual)
1. Launch Hearts via `game.gamestate`.
2. Verify the 2 of Clubs player starts.
3. Play through a full round.
4. Verify points are calculated correctly according to KDE Hearts rules.
5. Verify "Shooting the Moon" triggers the correct penalty for opponents.

## Performance Benchmarks
- Assert that card animations (72x96 PNGs) maintain 60 FPS on standard development hardware.
- Assert that ByteVM overhead for trick resolution is < 10ms.
