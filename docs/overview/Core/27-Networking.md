# Networking

## What this covers
This file documents the networking stack that is actually visible in Core: TCP sockets, DNS/address resolution, HTTP, WebSocket, MIME helpers, and optional SSL integration.

## Core socket layer
[`uppsrc/Core/Inet.h`](../../../uppsrc/Core/Inet.h) defines `Socket` as the central transport type and then aliases:

- `using TcpSocket = Socket;`

That alias is the clearest current status signal: the Core socket abstraction here is TCP-oriented.

The implementation in [`uppsrc/Core/Socket.cpp`](../../../uppsrc/Core/Socket.cpp) reinforces that:

- `getaddrinfo` is called with `SOCK_STREAM`
- protocol setup uses `IPPROTO_TCP`
- listen/accept/connect flow is stream-socket based

## What is present
Core networking currently includes:

- hostname resolution via `IpAddrInfo`
- TCP client/server sockets
- optional SSL startup on top of an existing socket
- `HttpRequest`, `HttpHeader`, `HttpCookie`, and URL parsing helpers
- `WebSocket` built on `TcpSocket`
- encoding helpers in [`uppsrc/Core/Inet.h`](../../../uppsrc/Core/Inet.h): URL, quoted-printable, Base64, MIME utilities, and `HMAC_SHA1`

`HttpRequest` in [`uppsrc/Core/Http.cpp`](../../../uppsrc/Core/Http.cpp) supports redirects, retries, proxies, digest auth, chunked transfer handling, and gzip content decoding.

`WebSocket` in [`uppsrc/Core/WebSocket.cpp`](../../../uppsrc/Core/WebSocket.cpp) implements HTTP upgrade plus frame send/receive on top of TCP, with optional TLS by starting SSL on the underlying socket.

## What is not present in Core
A code search across `uppsrc/Core/` does not show UDP socket classes, `SOCK_DGRAM` use, or ENet integration.

So the accurate distinction is:

- TCP sockets are implemented
- UDP sockets are not implemented in this Core package snapshot
- ENet is not implemented in this Core package snapshot

If those are mentioned as future work elsewhere, they are planned or external, not current Core behavior.

## SSL boundary
SSL is optional. `Socket::StartSSL()` reports an error if the SSL backend is not available:

- `"Missing SSL support (Core/SSL)"`

So networking support is split between Core's TCP/HTTP/WebSocket logic and the separate `Core/SSL` package that supplies the backend.

## Debug and trace behavior
HTTP tracing is runtime-configurable through `INI_BOOL(...)` flags in [`uppsrc/Core/Http.cpp`](../../../uppsrc/Core/Http.cpp).

WebSocket tracing exists too, but it uses a static runtime flag in `WebSocket::Trace(bool)`.

## Current vs legacy
The TCP/HTTP/WebSocket stack is current. The main limitation is scope: this package is stream-socket networking, not a general transport catalog.

## See also
- [06-Streams.md](06-Streams.md)
- [21-Compression.md](21-Compression.md)
- [29-Runtime-Linking.md](29-Runtime-Linking.md)
