# Media Foundations and UI Refinement (2010-12)
**Date Span:** 2010-12-01 to 2010-12-31

December 2010 was a landmark month for U++ multimedia and user interface ergonomics, closing the year with a surge of high-level features and deep performance tuning. The framework formally entered the audio-visual domain with the introduction of the **Media** package, providing native support for audio and video playback. This was complemented by the `MediaPlayer` control demo and a significant Bazaar contribution: the `plugin/portaudio` and `Sound` packages. Together, these additions provided U++ developers with a complete, cross-platform stack for building media-rich applications, from simple sound alerts to complex video players.

The standard UI toolkit received a major responsiveness boost through the "Lazy Icons" overhaul of `FileSel`. By implementing lazy and optimized file icon loading—particularly for the X11 backend—the file selection dialog became significantly more performant when navigating directories with thousands of items. This period also saw the maturation of the `TabBar` package, which received exhaustive painting fixes and improved drag-and-drop logic for its advanced "stacking" mode. Visual integrity was further refined with `LabelBox` now natively supporting a `Disabled` state, and `RichText` printing gaining automatic landscape mode detection.

The Bazaar flourished with a massive batch of specialized developer components. The "Controls4U" and "Functions4U" initiatives were joined by a suite of layout and property-editing tools, including `AutoScroller`, `CtrlFinder`, `CtrlProp`, and specialized editors for `Point`, `Rect`, and `LogPos`. The arrival of `PlotLib` and `PlotCtrl` provided new avenues for data visualization, while `SliderCtrlX` offered an extended alternative to the standard slider. These community-driven efforts were supported by core library expansions, including new `DayOfYear` and `StrToTime` utilities, and an extension of the `Callback` system to support even more argument combinations.

TheIDE and professional tooling also saw vital upgrades. The main package configuration editor was enhanced to show structured flags accepted by packages, providing much better visibility into complex build configurations. A new "Convert selection to ASCII" feature improved text processing workflows, and `umk` gained the `-d` option for better development-build control. The database layer was hardened with `SQLT_TIMESTAMP` support for Oracle and a new debug-mode check in `Sql::operator[]` that triggers a `NEVER` assertion if a column is missing, catching data-binding bugs at the earliest possible moment.

## References
- [1] c50fe093e — Media: Native Audio and Video playback introduced (koldo, 2010-12-23)
- [2] e27dfed07 — Bazaar: plugin/portaudio and Sound package initial release (dolik, 2010-12-25)
- [3] eb98dd584 — CtrlLib: FileSel lazy and optimized file icon loading (cxl, 2010-12-05)
- [4] b34849f17 — MediaPlayer: New control and demo launched (koldo, 2010-12-23)
- [5] 552d83c5b — TabBar: Exhaustive painting and stacking mode fixes (unodgs, 2010-12-29)
- [6] 775a56139 — Bazaar: AutoScroller, CtrlProp, and specialized property editors (kohait, 2010-12-28)
- [7] e8a8862d8 — Bazaar: PlotLib and PlotCtrl introduced (dolik, 2010-12-01)
- [8] d6ba2c4c9 — Ide: Convert selection to ASCII; structured flags in package editor (rylek, 2010-12-21)
- [9] aea71ca74 — CtrlCore: X11 TopWindow maximize/minimize/fullscreen fixes (cxl, 2010-12-27)
- [10] 2ea1f6fa8 — CtrlLib: LabelBox visually supports Disabled state (cxl, 2010-12-27)
- [11] e7b93ef0f — CtrlLib: Automatic landscape mode for RichText print (cxl, 2010-12-04)
- [12] 0248e1bb2 — Oracle: SQLT_TIMESTAMP support added (cxl, 2010-12-25)
- [13] d373b9d1c — Sql: operator[] asserts if column not found in debug (cxl, 2010-12-12)
- [14] 1a17744b5 — Core: DayOfYear and StrToTime utilities added (cxl, 2010-12-24)
- [15] d9238f9ca — Core: Callback system extended with more combinations (cxl, 2010-12-12)
- [16] c028f0eb6 — SliderCtrlX: New extended slider control (koldo, 2010-12-07)
- [17] c66c7a8d8 — theide: umk -d option introduced (cxl, 2010-12-16)
- [18] 006632bf8 — CtrlLib: TrayIcon notification length increased to 125 (cxl, 2010-12-26)
- [19] 1abacfff2 — CtrlLib: TreeCtrl Item::margin setter added (rylek, 2010-12-16)
- [20] cdfbc3eb7 — Draw: ImageOps hotspots; CtrlLib: Splitter Zoom fix (cxl, 2010-12-26)
