# AppRegistry — Core-only persistence example

## What it is

`AppRegistry` is a cross-platform key-value store and persistence helper that lives
in `uppsrc/Core`. It stores scalar values, JSON state, small inline blobs (base64),
and large blobs as external files. It requires only `Core` — no `CtrlLib`, no GUI.

## Why vendor `AiUpp`

All files and paths use `AiUpp` as the vendor directory name. Filesystem paths avoid
hyphens for portability. The visible name `AI-UPP` may appear in UI titles, but
registry keys and file paths always use `AiUpp`.

## Where files are stored

**Windows:**
```
%APPDATA%\AiUpp\<AppId>\<Profile>\registry.json       (config / registry)
%LOCALAPPDATA%\AiUpp\<AppId>\<Profile>\blob\<key>.bin (external blobs)
```

**Unix (XDG):**
```
~/.config/AiUpp/<AppId>/<Profile>/registry.json       (config / registry)
~/.local/state/AiUpp/<AppId>/<Profile>/blob/<key>.bin (external blobs)
~/.cache/AiUpp/<AppId>/<Profile>/                     (cache directory)
```

`GetConfigDir()`, `GetStateDir()`, and `GetCacheDir()` return the resolved paths.

## When to use inline base64 blobs

Use `BLOB_INLINE_BASE64` (or let `BLOB_AUTO` decide) for blobs under 64 KiB:
cursor state, small layout snapshots, compact settings that belong in the registry
file alongside other scalars.

## When to use external blob files

Use `BLOB_EXTERNAL_FILE` (or `BLOB_AUTO` for data over 64 KiB) for dock-window
layout binaries, cached images, pre-computed data, and anything that would bloat
`registry.json`.

## Core-only by design

This package intentionally has no window, no GUI, and no `CtrlLib` dependency.
It can be used from console tools, background services, and headless tests, as well
as from GUI windows that store their state through it.

## Building

```sh
umk reference/AppRegistry AppRegistry CLANG -br
./AppRegistry
```

Or with the local wrapper if the repository provides one.
