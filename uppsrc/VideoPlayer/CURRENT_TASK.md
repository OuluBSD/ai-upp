# Current Task: Standalone Live VideoServer Viewer Window

Status: completed

## Completed
1. Created new package `uppsrc/VideoPlayer` (with `AGENTS.md`, `CURRENT_TASK.md`, `VideoPlayer.upp`, `VideoPlayer.h`, and `main.cpp`).
2. Implemented `VideoPlayerWindow` TopWindow class supporting borderless framing (`FrameLess()`), resizable window margins, and manual left-drag window movement.
3. Implemented aspect-ratio-preserving scaled image painting via `DrawImage` on the window layout.
4. Implemented a worker background thread connecting to a running `VideoServer` via `VsmVideoServerFrameSource` TCP polling loop.
5. Implemented thread-safe frame conversion (`VsmImageBufferToImage`) and event dispatching (`PostCallback`).
6. Implemented automatic connection retry and recovery.
7. Added a minimalist HUD overlay panel with connection host/port controls, stats display (FPS, Latency, Resolution, Status), and reconnect/close buttons.
8. Configured command-line parameter parsing for `--host` and `--port` parameters.
9. Added keyboard shortcut overrides (`Escape` and `q`/`Q` to quit) and context menu overrides (right click to exit).
10. Built and compiled the executable using the native MSVS compiler and verified successful linking under `bin/VideoPlayer.exe`.
11. Added support for direct local video file launching by automatically spawning an in-process libavcodec-based `VideoServer.exe` background process when a file path argument is supplied.
