# The GDB Revolution and SQL Quoting
**Date Span:** 2012-02-01 to 2012-02-29

### Gdb_MI2 Debugger Interface
Launched a major overhaul of Linux-side debugging with the **Gdb_MI2** interface. It introduced a rich variable explorer, watches, and specialized decoders for core U++ types like `Array`, `Index`, and `Value`, bringing parity with the Win32 debugging experience.

### SQL Identifier Quoting
Introduced explicit **SqlId quoting** across the database layer, ensuring safe handling of reserved keywords and complex identifiers. Refined the `SqlExp` engine with new `JoinRef` heuristics and MS-SQL subselect optimizations.

### Core and Serialization Hardening
Added **STATIC_ASSERT** for compile-time validation and refactored `Xmlize` to use a reference-based `XmlIO&` API. The framework also gained the `ScanTime` utility and per-thread SQL session support for high-performance servers.

### Infrastructure and Bazaar
Introduced **UppBuilder** for automated Makefile generation and the **IPNServer** for PayPal payment notifications. TheIDE's export tool was upgraded to correctly handle `.brc` binary resources, and `umk` gained the `-k` directory preservation flag.
