# Small Value Optimization and the JSON Transition (2012-03)
**Date Span:** 2012-03-01 to 2012-03-31

March 2012 was a definitive month for U++ core efficiency and the maturation of its serialization suite. The groundbreaking effort in **SVO_VALUE** (Small Value Optimization) reached its first stable milestone. By enabling the `Value` class to store small primitives directly within its internal buffer—avoiding costly heap allocations for common integers, booleans, and dates—the framework achieved a new level of high-frequency data handling performance. This core refactor was immediately leveraged to provide native **Jsonize** support for the `Value` class, bringing JSON serialization to parity with the existing `Xmlize` system and allowing U++ applications to seamlessly interoperate with modern web data formats.

The core library's foundational utilities were also bolstered with a focus on system integrity and networking. The introduction of **NOWRITESHARE** for `FileOut` and `FileAppend` provided a cross-platform mechanism (utilizing `flock` on Linux and native Win32 locks) to enforce exclusive file access, significantly reducing race conditions in data-logging and configuration tasks. Parallel to this, the **TcpSocket** class was formally promoted from the `Web` package to `Core`, signaling its status as a fundamental building block for all U++ network-aware applications.

User interface interactivity reached a new level of visual polish. The `RectTracker` component was enhanced with a specialized **'round' callback**, allowing for pixel-perfect snapping and alignment during complex layout operations—a feature immediately showcased in a new reference example. The `DisplayPopup` system was also refined to avoid "stalled" popups in high-frequency environments like the TheIDE navigator, ensuring a cleaner and more responsive user experience.

TheIDE and its build infrastructure continued their climb toward enterprise reliability. The `lpbuild2` tool underwent a series of refinements, including critical fixes for parallel builds and maintainer configuration. The **BRC** (Binary Resource Compiler) was hardened across multiple toolchains, specifically resolving PDB generation issues under Visual C++ 2010. Architectural hardening also included the addition of explicit constructors for `AttrText` and the expansion of the `Ref` system to support automatic `ValueType<->Ref` conversions, simplifying the creation of type-safe, generic UI bindings.

## References
- [1] 71740dc8d — Core: SVO_VALUE (Small Value Optimization) reintegrated (cxl, 2012-01-30)
- [2] 3fb212fb7 — Core: Jsonize support for Value completed (cxl, 2012-03-03)
- [3] 6ade22475 — Core: FileOut NOWRITESHARE exclusive access enforced (cxl, 2012-03-02)
- [4] 7a3af4705 — Core: NOWRITESHARE implemented for Linux/BSD via flock (cxl, 2012-03-02)
- [5] 58312747a — Core: TcpSocket promoted to canonical Core package (cxl, 2012-03-30)
- [6] 9dd3a460c — CtrlCore: RectTracker 'round' callback introduced (cxl, 2012-03-12)
- [7] b4893a4f4 — reference: RectTracker rounding demonstration (cxl, 2012-03-12)
- [8] 6e958b4c4 — CtrlLib: Improved DisplayPopup syncing to avoid stalls (cxl, 2012-03-23)
- [9] a2a2d3067 — Core: Automatic ValueType<->Ref support added (cxl, 2012-03-06)
- [10] d92d97f96 — SqlCtrl: ThreeState mode support added (cxl, 2012-03-22)
- [11] 086bfac79 — lpbuild2: Fixed parallel build support (dolik, 2012-03-22)
- [12] bf5656650 — ide: MSC builder BRC compilation fixes (rylek, 2012-03-26)
- [13] 66bbc0573 — Bazaar: PolyXML adapted to new Xmlize changes (micio, 2012-03-02)
- [14] e1ca99081 — Web: HttpClient handling of '#' in URLs fixed (cxl, 2012-03-21)
- [15] d6b4c7623 — CtrlLib: Calendar respects first day of week setting (unodgs, 2012-03-11)
- [16] 667e5d97e — Draw: AttrText constructors made explicit (cxl, 2012-03-08)
- [17] 79d7d77e5 — Core: Value::GetCount fixed for non-array types (cxl, 2012-03-29)
- [18] 0d31756f0 — Functions4U: Geometry functions expanded (koldo, 2012-03-19)
- [19] f53cd2751 — SysInfo: GetProcessName fix for Linux (koldo, 2012-03-30)
- [20] 40d5d75d1 — Core: ValueUtil compatibility fixes for VC71 (rylek, 2012-03-17)
