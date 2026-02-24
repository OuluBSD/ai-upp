# NanoStrings and the Default GTK Backend (2013-12)
**Date Span:** 2013-12-01 to 2013-12-31

The final month of 2013 was a landmark period for U++ core performance and its primary Linux presence. The most significant architectural advancement was the introduction of **NanoStrings** (Short String Optimization). By enabling the `String` class to store small text buffers directly within the object's memory footprint—bypassing the heap entirely for common short identifiers and tokens—the framework achieved a massive leap in memory efficiency and allocation speed. This was accompanied by a major overhaul of the **Heap allocator**, featuring specialized enhancements for Win32 and improved multi-threaded safety, ensuring that U++ remained at the cutting edge of low-level C++ performance.

The **Rainbow** project reached a historic milestone as the **GTK backend became the default for Linux**. This shift reflected the maturity of the Gtk-based GUI abstraction, which now provided a more modern and stable experience than the legacy X11 path for most users. Simultaneously, the X11 backend was bolstered with **Xinerama support**, enabling proper multi-monitor awareness for traditional setups. This period also saw the development of **Tel++**, a new high-level terminal emulation primitive, and **WebWord**, a powerful demonstration of the **Skylark** framework's ability to host complex document editing within a web browser.

The core library's data handling and serialization reached new levels of completeness. The **Zlib stream operations** were refactored to support massive datasets larger than **2GB**, and the `GZDecompressFile` utility was expanded to support the `.gzip` extension. Every major container in the library—including the logarithmic-performance **InVector**, **InArray**, and the **SortedIndex/Map** family—was updated to natively support **Xmlize** and **Jsonize** methods, resulting in a perfectly consistent and object-oriented serialization API. The networking suite also entered a new frontier with the initial development of native **WebSockets** support, signaling the framework's readiness for real-time web communication.

TheIDE and its professional toolset were refined for maximum visual and functional polish. The environment received a comprehensive **icon overhaul**, introducing smoothed and modernized assets throughout the UI. Productivity was enhanced by the introduction of native **.qtf file support**, allowing rich text resources to be managed as first-class project assets. Debugging was also improved: the **Gdb_MI2** interface received critical stability fixes, and POSIX developers gained the ability to automatically launch a terminal window when debugging console applications. The month closed with the addition of **Dutch (nl-nl)** and **Brazilian Portuguese (pt-br)** translations, cementing U++'s global reach as it transitioned into 2014.

## References
- [1] fcc7acbcc — Core: NanoStrings (Short String Optimization) introduced (cxl, 2013-12-15)
- [2] ce8e52f4e — Rainbow: GTK backend becomes the default for Linux; Xinerama added (cxl, 2013-12-26)
- [3] bed93110d — Core: Heap enhancements and NanoStrings integration (cxl, 2013-12-17)
- [4] cc2668b9e — Core: Zlib stream operations refactored for >2GB support (cxl, 2013-12-17)
- [5] a90b02489 — Core: Container family gains native Xmlize/Jsonize methods (cxl, 2013-12-15)
- [6] 39082cb28 — Web: Initial development of native WebSockets support (cxl, 2013-12-28)
- [7] c50daeece — Rainbow: Developing Tel++ terminal emulation (cxl, 2013-12-01)
- [8] a7a8aaafc — Rainbow: WebWord (Skylark-based document editor) introduced (cxl, 2013-12-28)
- [9] 96f90df3a — ide: Native support for .qtf files as project assets (cxl, 2013-12-23)
- [10] 1ef8f5e93 — ide: Smoothed and modernized icon suite introduced (cxl, 2013-12-18)
- [11] 7bc553fee — ide: POSIX debugging launches console for console apps (cxl, 2013-12-19)
- [12] e3709c4d3 — Core: XmlParser with stream support; GZCompressFile (cxl, 2013-12-11)
- [13] 464d16e5c — Core: AsXML adds stream and file-based variants (cxl, 2013-12-12)
- [14] 359d993b7 — Core: Vector fixed for very large item counts in 64-bit (cxl, 2013-12-15)
- [15] 128c7c80a — Painter: Subpixel rendering over non-opaque backgrounds fixed (cxl, 2013-12-23)
- [16] 3aa4db90b — CtrlLib: FrameSplitter visual improvements; IML smoothing (cxl, 2013-12-09)
- [17] c5d1a7c51 — ide: Gdb_MI2 debugger stability fixes (cxl, 2013-12-26)
- [18] ee3d86703 — i18n: Dutch (nl-nl) translation added (cxl, 2012-12-09)
- [19] e90ce8af2 — i18n: Brazilian Portuguese (pt-br) translation updated (cxl, 2012-12-09)
- [20] ad52452d1 — ide: Insert file path feature added (cxl, 2013-12-19)
