# Skylark Web Framework and JSON-RPC (2012-07)
**Date Span:** 2012-07-01 to 2012-07-31

July 2012 was a month of massive architectural expansion for the U++ web ecosystem, marked by the formal introduction of the **Skylark** web framework. Promoting from the sandbox to the canonical `uppsrc` collection, Skylark provided U++ developers with a high-performance, template-driven environment for building modern web applications. The initial push featured a comprehensive tutorial suite—covering fundamental dispatch, Ajax integration, and session management—along with the introduction of specialized variable notations: prefixing session variables with '.' and cookie variables with '@'. This framework launch was immediately supported by the modernized `AddressBookWeb` example and native support for the `RPC_METHOD` keyword within Assist++, positioning U++ as a formidable choice for full-stack C++ web development.

The core library's connectivity suite reached a new level of versatility with the transformation of `XmlRpc` into the unified **Rpc** package. By adding native **JSON-RPC** support, U++ gained the ability to interoperate with modern web services using either XML or JSON payloads through a consistent, high-level API. This was supported by a new extraction/return notation and the introduction of `XmlizeByJsonize`, a cross-format bridge that further simplified the migration of persistent data. Core library performance was also a focus: the **INI parameter system** was refactored to use double-checked locking, eliminating mutex overhead for configuration access, and now supports `INI_DOUBLE`, `INI_INT64`, and specialized suffixes (K, M, G, T) for memory-sized values.

TheIDE and professional toolchains reached new heights of productivity and cross-platform resilience. The environment added native **MacOS X11 patches**, improving its behavior on Apple workstations, and the topic editor was upgraded to handle language-aware code insertion. Debugging was hardened with the addition of the `_DBG_Ungrab` mode for X11, resolving long-standing mouse-grabbing issues, and the PDB debugger received workarounds for high-speed alignment bugs in 64-bit MSC10 builds. The environment's intelligence was also bolstered: Layout files gained syntax highlighting, and Assist++ was refined to provide natural presentations for complex Skylark definitions.

Bazaar and community initiatives continued their steady growth. The **HelpViewer** was upgraded with standard browser-like "Back" and "Forward" navigation and the ability to programmatically follow links. **Scatter** and **ScatterDraw** reached new levels of professionalism with context menu support and improved invalid range handling. In the core library, `Value` comparison logic was hardened to resolve type compatibility instead of crashing, and `Thread::Priority` achieved full implementation for POSIX. The month closed with the addition of the **Euskara (Basque)** translation for the official website and a comprehensive new installation guide for Ubuntu users.

## References
- [1] 8e070bb69 — uppsrc: Skylark web framework moved from sandbox (cxl, 2012-07-05)
- [2] 569f2c310 — Skylark: Moved to Upp namespace (cxl, 2012-07-05)
- [3] e19309b88 — tutorial: Skylark tutorial suite introduced (cxl, 2012-07-06)
- [4] 13c80b9b8 — Core: XmlRpc renamed to Rpc; JSON-RPC support added (cxl, 2012-07-10)
- [5] 52c883a08 — Core: JSON-RPC implementation finished (cxl, 2012-07-10)
- [6] e3df5fdc3 — Core: INI system refactored with double-checked locking (cxl, 2012-07-24)
- [7] fd5a8200a — Core: INI_DOUBLE and ReloadIniFile introduced (cxl, 2012-07-10)
- [8] 9111d378e — Core: INI_INT64 adds support for K, M, G, T suffixes (cxl, 2012-07-13)
- [9] 84c601521 — ide: MacOS X11 patches and QtfReport numbering (cxl, 2012-10-03)
- [10] 7645c1b5d — CtrlCore: _DBG_Ungrab for X11 debugging sessions (cxl, 2012-05-17)
- [11] a9f7693f1 — Skylark: New prefix notation for session/cookie variables (cxl, 2012-07-10)
- [12] 6416ba324 — Core/Rpc: New extraction and return notation (cxl, 2012-07-10)
- [13] 81dc251c8 — Scatter: Added context menu support (koldo, 2011-10-06)
- [14] f2510b169 — Core: Value equality resolves type compatibility (cxl, 2012-05-03)
- [15] a745abee0 — Core: XmlizeByJsonize cross-format bridge (cxl, 2012-05-20)
- [16] b324b9e85 — theide: Skylark support and help group PDF export (cxl, 2012-07-14)
- [17] 800975614 — Bazaar: HelpViewer adds Back/Forward and FollowLink (micio, 2012-03-02)
- [18] 3c2194aad — Core: Thread::Priority implemented for POSIX (cxl, 2012-06-08)
- [19] df2e68ee4 — Core: NetNode adds more property getters (cxl, 2012-03-05)
- [20] 236efa6b1 — uppweb: Basque (Euskara) index translation added (koldo, 2012-01-10)
