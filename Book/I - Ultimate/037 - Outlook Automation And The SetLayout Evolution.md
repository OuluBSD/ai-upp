# Outlook Automation and the SetLayout Evolution (2011-05)
**Date Span:** 2011-05-01 to 2011-05-31

May 2011 was a month of significant high-level automation and deep structural refinement for U++ user interface definitions. The most prominent addition was the **MAPIEx** package, providing a robust cross-platform interface for **Outlook automation**. This enabled U++ applications to interact with Microsoft Outlook's folders, emails, and address books directly, complete with MinGW compatibility and a comprehensive demo. This push into office interactivity was supported by the maturity of the **StaticPlugin** system and updates to `SysInfo`, which gained hibernate and suspend capabilities within its shutdown logic.

The framework's user interface definition underwent a fundamental evolutionary step with the introduction of the **SetLayout_** template functions. Automatically generated from `.lay` files, these type-safe functions provided a more modern and predictable alternative to the legacy `Ctrl::Layout` macros, immediately showcased in a new `SetLayout` reference example. The core `CtrlCore` library also saw critical fixes for X11 focus management during background builds and a resolution for the "black corners" effect in certain window managers. `RichTextView` reached a new level of interactivity with improved table selections and better mouse-over feedback for links.

The Bazaar saw an intensive expansion of the **Py** (Python) integration. The `BoostPyTest` sandbox reached "80% completion" for `TopWindow` and `CtrlCore` exports, enabling U++ callbacks to be exposed unidirectionally to hosted Python scripts. This allowed for complex scripted UI logic and live evaluation within U++ applications. Simultaneously, the `Gen` package introduced the `Shared<T>` template—a reference-counted container similar to `boost::shared_ptr` for developers seeking non-standard ownership models—and `ValueCtrl` added support for `ValueMap` editing.

Core architectural improvements were abundant. The `Xmlize` engine was enhanced to allow custom tag names for container items, providing finer control over persistent data formats. `SqlCtrl` introduced `SqlNOption`, a variant of the standard option control that returns `Null` instead of "0" when false, simplifying database-backed forms. `ArrayMap` was extended with a new `Insert` overload, and the `CalcContext` in the expression evaluator gained the ability to return raw formula strings. The development of **Rainbow**, a new unified GUI backend architecture, also began to appear in the logs, signaling the framework's long-term interest in further platform abstraction.

## References
- [1] 5ba939fd2 — MAPIEx: Outlook automation package introduced (koldo, 2011-05-01)
- [2] c78825527 — CtrlCore: New SetLayout_ template functions from .lay (cxl, 2011-05-24)
- [3] 847790ece — reference: SetLayout example (cxl, 2011-05-24)
- [4] 86508d182 — Bazaar: BoostPyTest: TopWindow export and ValueMap support (kohait, 2011-05-16)
- [5] 332c085ad — Bazaar: BoostPyTest: Hosted Python script invocation (kohait, 2011-05-17)
- [6] a466eb619 — Bazaar: Shared<T> reference-counted container (kohait, 2011-05-20)
- [7] 6f4b84b81 — Core: Xmlize defines tag names for container items (cxl, 2011-05-24)
- [8] 9e0debc32 — SqlCtrl: SqlNOption added (cxl, 2011-05-26)
- [9] b29a45641 — CtrlCore: X11 focus fix during theide building (cxl, 2011-05-16)
- [10] 44e91dcec — CtrlLib: Improved selection in RichTextView tables (cxl, 2011-05-17)
- [11] 358d9a755 — SysInfo: Hibernate and suspend added to Shutdown() (koldo, 2011-05-21)
- [12] 7569b0088 — Functions4U: Tokenize() function added (koldo, 2011-05-21)
- [13] a71f2a8bd — CtrlLib: EditField text highlighting support (cxl, 2011-05-05)
- [14] 0cc4a5f10 — Sql: Overwrite last error out of transactions (rylek, 2011-05-27)
- [15] 42fbb8e06 — MySQL: SQLDEFAULT support added (cxl, 2011-05-04)
- [16] b3e59fb66 — theide: Improved source updater with batch actions (dolik, 2011-05-17)
- [17] f3575fc0e — theide: fixed menu focus loss when building (cxl, 2011-05-21)
- [18] e092b4c2f — Draw: Fixed CJK glyph issues (cxl, 2011-05-13)
- [19] b6e804f59 — TCore: CalcContext returns raw formula strings (rylek, 2011-05-30)
- [20] 42f88f553 — CtrlCore: Initial signs of Rainbow backend development (cxl, 2011-05-29)
