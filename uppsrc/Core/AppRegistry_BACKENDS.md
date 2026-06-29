# AppRegistry Backend Policy

## 1. Current file-backed implementation summary

`AppRegistry` (v1, `uppsrc/Core/AppRegistry.h/.cpp`) is entirely file-backed on all
platforms. It stores:

- **`<ConfigDir>/registry.json`** — all scalar values, inline base64 blobs, external
  blob file paths, and JSON state strings.
- **`<StateDir>/blob/<key>.bin`** — external binary blobs (data exceeding the 64 KiB
  inline threshold).

The JSON file is written atomically (via `SaveFile`) and parsed with `ParseJSON`.
No locking is implemented; single-process access is assumed.

Platform paths:

| Platform | Config dir | State/blob dir |
|----------|-----------|---------------|
| Windows | `%APPDATA%\AiUpp\<App>\<Profile>` | `%LOCALAPPDATA%\AiUpp\<App>\<Profile>` |
| Linux/macOS | `$XDG_CONFIG_HOME/AiUpp/<App>/<Profile>` or `~/.config/AiUpp/...` | `$XDG_STATE_HOME/...` or `~/.local/state/AiUpp/...` |

---

## 2. Whether a native Windows Registry backend is worth implementing

**Recommendation: implement a native Windows Registry backend as an optional
secondary backend for scalar values only.** Rationale:

- Windows Registry survives user profile moves and is accessible from system tools.
- Registry writes are atomic at the key level; no JSON parse/write cycle needed.
- The native registry is already used by many Windows installers to record app
  version, installation path, and licensing state.

However, the native registry is **not** appropriate for all `AppRegistry` data.
Implementation should be additive (opt-in per value type), not a full replacement.

**Timeline recommendation:** file-backed is sufficient for the first year of use.
Add a native registry backend only when a concrete need arises (e.g., group policy
enforcement, MSIX/MSIX-packaged deployments, per-machine settings).

---

## 3. Values safe for the native Windows Registry backend

- Scalar values with small data: `String` (≤ 2 KB), `int`, `int64`, `bool`, `double`.
- Profile identity: vendor, app id, profile name, install path.
- Per-machine settings (via `HKLM`): license flags, installation mode.
- Per-user preferences (via `HKCU`): window theme, language, recent paths.

**Key format:** `HKCU\Software\AiUpp\<AppId>\<Profile>\<key>`

---

## 4. Values that must remain file-backed

The following must never move to the native Windows Registry:

| Data type | Reason |
|-----------|--------|
| **Large blobs** (> 64 KiB) | Registry has a per-value limit (~1 MB theoretical, ~64 KB practical). |
| **Dock layout binaries** | Binary `SerializeWindow` output; not human-readable; changes often. |
| **Cached images / thumbnails** | Can be megabytes; belong in `GetCacheDir()`. |
| **Large JSON states** | CEE variant lists, session timelines, detection results — can be hundreds of KB. |
| **External blob references** | File paths pointing to `<StateDir>/blob/` must remain in the JSON file so both sides stay in sync. |
| **Generated / derived data** | Anything that can be regenerated from source data. |

**Rule:** if the value exceeds 4 KB or is regenerable, it stays file-backed.

---

## 5. Proposed backend selection API

For a future v2 `AppRegistry`, introduce a per-key storage hint:

```cpp
enum StorageHint {
    STORE_DEFAULT,      // file-backed JSON (always supported)
    STORE_NATIVE_REG,   // native OS registry for scalars (Windows: HKCU, Unix: ignore)
    STORE_FILE_BLOB,    // external file in <StateDir>/blob/
};

void AppRegistry::Set(const String& key, const Value& value,
                      StorageHint hint = STORE_DEFAULT);
```

The hint is advisory — the implementation falls back to file-backed if the native
backend is unavailable (non-Windows, sandboxed, permission denied).

**Do not add this API in v1.** Add it only when a native backend is implemented.

---

## 6. Key versioning and migration strategy

**Recommended approach: schema version in `registry.json`.**

```json
{
  "_schema_version": 2,
  "sample.string": "hello",
  ...
}
```

Migration rules:

1. `AppRegistry::Load()` reads `_schema_version`. If absent, treats as version 1.
2. If `_schema_version < current`, call a registered migration callback before
   returning.
3. Migrations are additive: rename keys, set defaults, delete obsolete entries.
4. `AppRegistry::Save()` always writes the current schema version.

**For v1:** no migration is needed (schema version 1, no `_schema_version` key).
Add the `_schema_version` key in v2 when the first migration is required.

**Key naming convention:**

- Use `category.subcategory.name` (dot-separated).
- Keep stable: once a key is shipped, renaming it requires a migration step.
- Prefix internal keys with `_` (e.g., `_schema_version`). Application keys must
  not start with `_`.

---

## 7. Corrupt registry JSON recovery

Current v1 behavior: if `ParseJSON` returns an error value, `Load()` logs a warning
and returns `false`. All values remain at their defaults.

**Recommended v2 recovery policy:**

1. On load error, rename the corrupt file to `registry.json.corrupt.<timestamp>`.
2. Create a fresh empty `registry.json`.
3. Log the corruption event with file path and timestamp.
4. Continue normally with defaults.
5. Do not silently delete the corrupt file — it may be useful for diagnosis.

**Implementation:** wrap `Load()` with a try-rename-recover helper. This prevents
a one-time JSON writer error from permanently breaking the registry.

---

## 8. Missing external blob recovery

If a `blobfile:` key points to a path that no longer exists:

1. Log the missing file with its key and expected path.
2. Remove the `blobfile:` entry from the in-memory map (to avoid repeated failures).
3. Return `false` from `LoadBlob()` — the caller must handle the absent data.
4. Do not crash or throw.

**Implementation:** add a `bool AppRegistry::VerifyBlobs()` method that checks all
`blobfile:` entries and logs/removes stale references. Call it optionally after `Load()`.

---

## 9. Profile rename / delete / list operations

**v1** has no profile management API. Add these in v2 or as a separate
`AppRegistryAdmin` class to avoid scope creep in the core class:

```cpp
// Future API:
Vector<String> AppRegistry::ListProfiles() const;
bool           AppRegistry::RenameProfile(const String& from, const String& to);
bool           AppRegistry::DeleteProfile(const String& name);
bool           AppRegistry::CloneProfile(const String& from, const String& to);
```

**Implementation:** operate on the filesystem directly (rename/copy the config and
state directories). Keep the current `Profile()` accessor for active-profile selection.

**Warning:** profile operations must not be called while any `AppRegistry` instance
with that profile is loaded. No lock-file mechanism exists in v1.

---

## 10. Test strategy for path resolution

Platform-specific path resolution must be tested on each target OS. Recommended
strategy:

| Test | Method |
|------|--------|
| Windows `%APPDATA%` | Set `APPDATA` env var in test, call `GetConfigDir()`, assert expected path. |
| Windows `%LOCALAPPDATA%` | Same for `LOCALAPPDATA`; assert state dir. |
| Windows missing env | Unset both; assert fallback to `GetAppDataFolder()`. |
| Linux `$XDG_CONFIG_HOME` | Set env var; assert config dir uses it. |
| Linux XDG missing | Unset env var; assert fallback to `~/.config/AiUpp/...`. |
| Linux `$XDG_STATE_HOME` | Set env var; assert state dir uses it. |
| Linux state missing | Assert fallback to `~/.local/state/AiUpp/...`. |
| All: dir creation | Call `Save()` with a fresh profile; assert directories are created. |
| All: round-trip | `Set` + `Save` + new instance + `Load` + `Get`; assert values match. |
| Corrupt JSON | Write an invalid JSON file; call `Load()`; assert returns `false`, does not crash. |
| Missing blob | Save blob ref, delete file, call `LoadBlob()`; assert returns `false`. |

Tests should use a temporary directory (via `TempFile` / `GetTempDirectory()`) as
the base to avoid polluting the real user config.

---

## Recommended follow-up tasks

1. **AppRegistry v1.1**: add `VerifyBlobs()` + corrupt-file recovery (rename-on-error).
2. **AppRegistry v2**: add `_schema_version` + migration callback API.
3. **AppRegistry native backend**: Win32 HKCU backend for scalar values, opt-in per key.
4. **AppRegistryAdmin**: profile list/rename/delete/clone operations.
5. **AppRegistry test suite**: Core-only unit tests covering all path resolution
   scenarios and round-trip persistence.
