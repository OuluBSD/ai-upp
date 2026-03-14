# Task: Design Separate Skylark Web Host

## Goal

Design a separate `Skylark`-based local web-host program that serves a running `.gamestate` session to a browser, while ScriptIDE acts as launcher/controller rather than the server process.

## Background / Rationale

The browser host should not live inside ScriptIDE itself. The split should be:
- separate web-host executable owns HTTP/WebSocket serving
- ScriptIDE owns local developer UX, browser launching, and session control
- reusable runtime/protocol pieces stay outside GUI-specific code where possible

## Scope

- Separate program lifecycle for starting/stopping the server
- Static asset serving for the browser client
- Live session protocol between browser and host
- Browser launch/open workflow from ScriptIDE
- Use of `reference/Skylark*` and `uppsrc/Skylark` techniques

## Proposed Architecture

1. ScriptIDE launches a dedicated web-host executable with session arguments.
2. The web-host executable uses `SkylarkApp` patterns to bind a localhost-only server.
3. Opening or running a browser-capable `.gamestate` creates a session id.
4. Browser loads HTML/CSS/JS shell from the web-host.
5. Browser connects back via WebSocket.
6. Host VM sends form tree snapshots and UI diff events.
7. Browser sends user events back:
   - click
   - button
   - selection
   - optional drag later

## Transport Recommendation

- Use `Skylark` handlers/routes for HTTP assets and bootstrap endpoints
- Use `WebSocket` from `uppsrc/Core` for live UI synchronization where needed
- Bind to `127.0.0.1` only in phase 1
- Use `reference/Skylark*` and `reference/SkylarkUpload` as implementation references

## Required Program Contracts

- Add a browser-host launch capability to ScriptIDE/plugin execution flow
- Define a command-line or IPC contract from ScriptIDE to the web-host program
- Add a session service that maps document path -> running VM -> browser clients
- Decide whether the VM lives in the web-host process or in a controller process exposed to the host

## Risks

- If the VM remains inside ScriptIDE while the browser host is separate, IPC complexity rises
- If the VM moves into the web-host process, debugger integration with ScriptIDE becomes a new design task
- Hot reload expectations can become expensive unless session reset is explicit
- Multiple browser tabs against one mutable game session need defined ownership rules

## Acceptance Criteria

- [ ] Separate-program hosting model documented
- [ ] `Skylark` usage strategy documented
- [ ] Browser session lifecycle documented
- [ ] Event protocol categories documented
- [ ] Process/thread model documented for ScriptIDE vs web-host vs VM
