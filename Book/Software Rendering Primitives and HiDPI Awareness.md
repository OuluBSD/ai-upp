# Software Rendering Primitives and HiDPI Awareness
**Date Span:** 2013-05-01 to 2013-05-31

### Software Rasterization: SDraw and DDA
Launched **SDraw** and the **DDARasterizer**, bringing high-quality, anti-aliased software rendering to the core library. This included sophisticated handling for fat-line joins and polylines, immediately integrated into the **IconDes** editor with a new **AutoAliasing** function.

### Initial HiDPI Support
Took the first major steps toward modern display density with targeted **HiDPI fixes for Win32**. This effort ensured that U++ applications maintained visual sharpness and correct scaling on emerging high-resolution monitors.

### Core Diagnostics and Tooling
Introduced **GCC stack trace** support for Linux and implemented the `TryLoadFromXML` helper. TheIDE's **AutoSetup** was updated to support **Visual C++ 2012**, and the Layout Designer added a "Duplicate layout" feature.

### Enterprise Connectivity and Math
The **MySQL** driver gained native library initialization, and **ScatterCtrl** achieved full GTK backend compatibility. The core scientific suite was further solidified by the finalization of the **Eigen** matrix library promotion.
