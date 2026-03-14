# Current Task

Create the initial standalone `ScriptWebHost` package scaffold:

- standalone `SkylarkApp` executable
- localhost configuration
- command-line parsing for session bootstrap
- basic status and session routes

Next steps:

- define the ScriptIDE -> ScriptWebHost launch contract
- decide whether the VM runs in this process or over IPC
- add WebSocket session transport for live UI diffs
