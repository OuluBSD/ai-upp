# Grid Maturity and the Physics Plugin (2010-07)
**Date Span:** 2010-07-01 to 2010-07-31

July 2010 was defined by a surge in high-level control programmable and the introduction of real-time physics to the U++ ecosystem. The most significant UI advancement occurred within `GridCtrl`, which underwent a rapid transformation from a data display component into a highly interactive application backbone. New APIs like `SetValues`, `GetValues`, and `SetCtrl` allowed developers to programmatically manipulate grid content with ease, while the addition of `FindCol` (by string) and `CopyColumnNames` bridged the gap between raw data and UI metadata. These features were complemented by a critical shift from absolute to relative indexing in `ReadRow` and `ReadCol`, greatly simplifying logic in filtered or sorted views.

The framework's reach into advanced application domains was further signaled by the addition of the **Box2D** physics library as a core plugin. This provided U++ developers with a battle-tested 2D rigid body simulation engine, immediately supported by an initial example project. This move, combined with the ongoing maturation of the `ndisasm` (disassembler) plugin and the introduction of OLE `SAFEARRAY` conversion support, positioned U++ as a formidable choice for both low-level system tools and high-level simulations.

The database layer, `SqlExp`, reached a new level of robustness with the introduction of the `IsSame` operator for PostgreSQL and MySQL. This provided a "null-safe" equality test where `null == null` evaluates to true—a common pain point in SQL-to-C++ mapping. Oracle-style parity was also improved with the implementation of multi-column `SET` in `UPDATE` statements, allowing developers to write more concise and standard-compliant data manipulation logic. PostgreSQL's `Like` operator was also tuned to be case-sensitive by default, bringing it into alignment with the framework's other supported database backends.

Advanced UI containers like `TabBar` and `Docking` received exhaustive polish. `TabBar` gained alternative close button icons and a configurable `ContextMenu` property, while the `Docking` framework saw a major "SortTitles" refactor to improve the organization of undocked and auto-hidden windows. In the core library, `Time::Compare` was added for precise chronological ordering, and `XmlParser` was refined with better white-space preservation options. TheIDE also kept pace with the times, adding formal support for the Windows SDK 7.1 and improving the `PackageSelector` visuals.

## References
- [1] cfa55a193 — plugins: Added Box2D physics library (unodgs, 2010-07-30)
- [2] 0a4c6b19d — GridCtrl: SetValues, GetValues, and Xmlize support (unodgs, 2010-07-18)
- [3] 792cb5908 — GridCtrl: Added SetCtrl for embedded widgets (unodgs, 2010-07-19)
- [4] 658e53da0 — GridCtrl: FindCol by string, ReadCol, and CopyColumnNames (unodgs, 2010-07-18)
- [5] 907ca39c5 — SqlExp: IsSame (PGSQL, MySql) null-safe equality test (cxl, 2010-07-24)
- [6] 91b4a1838 — Sql: multicolumn SET in UPDATE (Oracle style) (rylek, 2010-07-27)
- [7] cd2243eee — PGSQL: default Like made case-sensitive for parity (cxl, 2010-07-21)
- [8] f5cc0f9e8 — TabBar: Alternative close icons and sorting fixes (mrjt, 2010-07-15)
- [9] b8c37c354 — Docking: SortTitles and static correctness (kohait, 2010-07-19)
- [10] e6448b355 — Ole: Conversion between ValueArray and OLE SAFEARRAY (rylek, 2010-07-11)
- [11] f64ec955b — XmlRpc: Added Proxy and ProxyAuth support (cxl, 2010-07-11)
- [12] 9c34953b4 — ide: Support for Windows SDK 7.1 (cxl, 2010-07-11)
- [13] 3ccf607d5 — Core: Time::Compare introduced (cxl, 2010-07-05)
- [14] 963439cbd — Xml: PreserveAllWhiteSpaces support (cxl, 2010-07-10)
- [15] 55ca60667 — ExpandFrame: Paint and style overhaul (mrjt, 2010-07-15)
- [16] ac7a7fc4b — plugin/ndisasm: Upgraded to latest version (cxl, 2010-07-12)
- [17] d3699d38a — Draw: CreateImage variant with RGBA data (cxl, 2010-07-29)
- [18] d7f521f91 — SysInfo: Improved GetMacAddress for Windows (koldo, 2010-07-29)
- [19] d44f875ae — Oracle: OCI8 charset fixes for UTF8 (rylek, 2010-07-12)
- [20] 69381f526 — GridCtrl: CancelDuplicate and duplication fixes (unodgs, 2010-07-29)
