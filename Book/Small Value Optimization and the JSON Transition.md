# Small Value Optimization and the JSON Transition
**Date Span:** 2012-03-01 to 2012-03-31

### SVO_VALUE and Native JSON
Reintegrated the **SVO_VALUE** optimization, allowing the `Value` class to store small primitives without heap allocations. This leap in core efficiency was immediately leveraged to complete the **Jsonize** engine, bringing JSON serialization to parity with the existing `Xmlize` system.

### Exclusive Access and Core Networking
Introduced **NOWRITESHARE** for `FileOut` and `FileStream`, enforcing exclusive file access across Win32 and POSIX. The **TcpSocket** class was formally promoted to the canonical `Core` package, cementing its status as a fundamental networking primitive.

### Precision UI and Snapping
Enhanced `RectTracker` with a specialized **'round' callback**, enabling pixel-perfect snapping and alignment during complex layout tasks. Refined the `DisplayPopup` synchronization to eliminate "stall" popups in high-frequency editing environments.

### Reliable Builds and Enterprise SQL
The **lpbuild2** tool reached stability with fixed parallel build support. The database layer was hardened with **ThreeState** mode for `SqlCtrl`, while the `BRC` resource compiler was refined to resolve PDB generation issues in professional Windows toolchains.
