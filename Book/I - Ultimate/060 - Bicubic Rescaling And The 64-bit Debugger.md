# Bicubic Rescaling and the 64-bit Debugger (2013-04)
**Date Span:** 2013-04-01 to 2013-04-30

April 2013 was a month of significant graphical refinement and the beginning of a major push into modern Windows debugging. The most impactful visual advancement was the introduction of high-quality image processing primitives in the `Draw` package. The arrival of **RescaleBicubic** provided U++ applications with a superior alternative to standard linear rescaling, enabling sharper and more professional asset scaling. This was quickly expanded to support source rectangles and was integrated into **IconDes**, allowing icon designers to leverage bicubic filtering directly within TheIDE. The suite was further completed by the introduction of the **RescaleFilter** and **RescaleCached** systems, providing a flexible and performant way to manage high-quality image transformation across the framework.

TheIDE reached a significant milestone in professional Windows development with the start of the **native Win 64-bit debugger** implementation. While U++ had long supported 64-bit compilation, the ability to debug these processes natively within the environment represented a major leap in productivity for developers targeting modern server and workstation environments. The editor's intelligence was also bolstered by sophisticated **replace wildcard transformations**, allowing developers to change the case (UPPER, lower, Initcaps) of replaced text dynamically—a feature that significantly streamlined large-scale code refactoring tasks.

Core library and networking primitives reached new levels of versatility. **IpAddrInfo** was upgraded to support explicit IP protocol selection (v4, v6, or any), and `HttpRequest` was refined to handle complex redirection cases and alternate method names. The **Value** system gained the `IsSame` method for deep equality testing, and the **Eigen** scientific library was enhanced with native XML and JSON serialization support. STL compatibility was also a primary focus, with the core containers being updated to support `iterator_traits`, ensuring that U++ structures could be used more fluidly with standard algorithms.

User interface ergonomics continued their steady march toward platform parity and visual polish. `CtrlCore`'s default **MouseWheel implementation** was updated to automatically propagate events to parent controls, resulting in more intuitive scrolling behavior in complex nested layouts. The `GetScreenSize` function was deprecated in favor of the multi-monitor aware **GetVirtualScreenArea**, and the **EncodeRTF** engine was upgraded to support embedded images on POSIX platforms. Professional polish was rounded out by the addition of native resizing icons in `GridCtrl` and a native hand cursor for the `Calendar` control, ensuring that U++ applications maintained a cohesive, modern look and feel across all backends.

## References
- [1] c0075840b — Draw: RescaleBicubic introduced (cxl, 2013-04-09)
- [2] 3f8361dde — ide: Development of native Win 64-bit debugger started (cxl, 2013-04-22)
- [3] d48b9f24e — ide: Replace wildcard case transformations added (cxl, 2013-04-21)
- [4] 6e16a36ad — Core: IpAddrInfo adds protocol selection (v4, v6, any) (cxl, 2013-04-04)
- [5] 007ce0a09 — Core: Value::IsSame deep equality check introduced (cxl, 2013-04-05)
- [6] 8863931a2 — Eigen: Native Serialization (XML/Json) support added (koldo, 2013-04-05)
- [7] 33de2e920 — CtrlCore: MouseWheel events propagate to parents by default (cxl, 2013-04-06)
- [8] d05d4f02c — CtrlCore: GetScreenSize deprecated for GetVirtualScreenArea (cxl, 2013-04-06)
- [9] f14155682 — CtrlCore: EncodeRTF supports images in POSIX (cxl, 2013-04-07)
- [10] a73d9e4bc — ide: IconDes adds bicubic rescale option (cxl, 2013-04-11)
- [11] 564b18669 — Draw: RescaleFilter introduced (cxl, 2013-04-17)
- [12] 3d6cd603d — Draw: RescaleCached adds filter parameter (cxl, 2013-04-18)
- [13] bc2f28621 — Core: STL compatibility enhanced with iterator_traits (cxl, 2013-04-20)
- [14] 9a7ba33bd — GridCtrl: Native resizing icons added (unodgs, 2013-04-08)
- [15] c8368be75 — Calendar: Native hand cursor image added (unodgs, 2013-04-08)
- [16] e55b0b573 — Core: HttpRequest redirection fixes (cxl, 2013-04-11)
- [17] c027f94e3 — Core: Fixed GCC 4.7 issue with SortedVectorMap (cxl, 2013-04-03)
- [18] 047c3f550 — Sql: MassInsert maximum column count increased to 62 (cxl, 2013-04-04)
- [19] d38e505cd — CtrlLib: Splitter visual improvements (cxl, 2013-04-06)
- [20] 2d6d8a8d9 — PdfDraw: Fixed negative font height issue (cxl, 2013-04-02)
