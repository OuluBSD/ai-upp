# Daemon

## What this covers
This file documents the daemon/service layer that exists in [`uppsrc/Core/Daemon.h`](../../../uppsrc/Core/Daemon.h) and [`uppsrc/Core/Daemon.cpp`](../../../uppsrc/Core/Daemon.cpp).

## What the code actually provides
The implementation is a small service host abstraction, not a full OS-daemon framework.

Main pieces:

- `DaemonBase` manages named services and a run loop
- `DaemonService` is the abstract per-service interface
- optional `Glib2Daemon` exists behind `flagGLIB2`
- `SerialServiceBase` and `SerialServiceServer` add a TCP-based request/response service layer

`DaemonBase` registers requested services by name, initializes them, then runs an update loop until stopped or timed out.

## Runtime model
`DaemonBase::Run()` is simple:

- set the running flag
- repeatedly call `Update()`
- sleep for 10 ms between iterations
- stop on shutdown, timeout, or explicit stop

This is cooperative service polling, not a supervisor, init-system adapter, or process manager.

## Platform behavior
POSIX and non-POSIX behavior are visibly different:

- on POSIX, `Init()` installs signal handlers for `SIGINT`, `SIGABRT`, `SIGTERM`, `SIGQUIT`, and `SIGHUP`
- on non-POSIX, `Init()` logs `"TODO signal handlers"`

So the signal/shutdown integration is materially more complete on POSIX than on Windows.

## Serial service layer
The `SerialServiceServer` portion uses `TcpSocket`, not local IPC primitives or UDP:

- `ListenTcp(uint16 port)`
- thread-per-client handling
- fixed, serialized, or stream-based handlers selected by message magic

That makes this subsystem a TCP service helper layered on top of Core sockets.

## Fork-specific status
This area appears fork-specific or at least non-central relative to the classic upstream Core runtime. It is in the package and functional, but it is not one of the foundational subsystems that most Core code depends on.

That is an inference from repository structure and usage emphasis, not a hard upstream-history claim.

## See also
- [23-CoWork.md](23-CoWork.md)
- [27-Networking.md](27-Networking.md)
