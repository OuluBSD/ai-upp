# WinGL and the Digital Signing of PDF/A
**Date Span:** 2011-07-01 to 2011-07-31

### Rainbow Milestone: WinGL and LinuxFb
The **Rainbow** project achieved critical mass with the introduction of **WinGL**, an OpenGL-accelerated GUI backend for Windows. Simultaneously, the **LinuxFb** backend reached its first functional stage, enabling full U++ graphics, mouse, and keyboard support on raw Linux framebuffers.

### Secure Document Standards
Landed native support for **PDF/A** archiving and **digital signing** of PDF documents. Utilizing the new `wincert` plugin for certificate management, U++ applications can now produce legally verifiable and architecturally standardized document exports.

### Core and Scientific Expansion
Introduced a `Value`-compatible **Complex** number type and added `empty()` to all containers for STL parity. Landed 64-bit integer and LOB handle support for Oracle (OCI8) and added the **kissfft** (Fast Fourier Transform) plugin to the Bazaar.

### UI Interoperability and Polish
The `Painter` package was hardened with `Invert` support and `NaN` safety. `GridCtrl` was refactored for OpenGL compatibility, and `UWord` gained the ability to natively load and save `.rtf` files. TheIDE's "Insert Color" tool was updated to support QTF-specific color codes.
