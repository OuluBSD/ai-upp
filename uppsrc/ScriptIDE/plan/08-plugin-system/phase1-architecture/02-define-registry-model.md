# Task: Define Registry and Manifest Model

## Goal
Establish how plugins are discovered, described, and registered within the system.

## Strategy
1. **Define Plugin Manifest**: A descriptor containing metadata (name, version, supported extensions, etc.).
2. **Define PluginRegistry**: A central service that tracks loaded plugins and their contributions to extension points.
3. **Registration Flow**:
   - Application scans a `plugins/` directory.
   - Plugins (likely static libraries or local packages for now) register themselves into the `PluginRegistry`.

## Implementation Details

### Plugin Manifest
```cpp
struct PluginManifest {
    String name;
    String version;
    Vector<String> file_extensions;
    bool has_dock_pane = false;
};
```

## Success Criteria
- [ ] `PluginRegistry` class interface defined.
- [ ] Manifest structure supports extension mapping.
- [ ] Loading order and dependency resolution (if any) considered.
