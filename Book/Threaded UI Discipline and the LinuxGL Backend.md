# Threaded UI Discipline and the LinuxGL Backend
**Date Span:** 2013-07-01 to 2013-07-31

### Main-Thread-Only UI Rules
Enforced a major structural shift in `CtrlCore` by banning window creation and event loops in non-main threads. This rigorous multi-threading discipline eliminated vast categories of cross-platform UI race conditions and was supported by an updated `GuiLock` reference.

### Rainbow Project: LinuxGL
The **Rainbow** project reached a new peak with the initial release of the **LinuxGl** backend, providing a native OpenGL-accelerated GUI path for Linux. To support this, the core **Color** class was enhanced with alpha channel capabilities.

### SCGI and Web Hardening
Hardened the **Skylark** framework and the core HTTP parser against **SCGI header issues**, specifically for Apache and Nginx environments. Bazaar packages like **Protect** and **Updater** completed their migration away from the legacy web stack.

### Specialized UI and GIS Polish
Introduced the **EditNumber** control and improved `ArrayCtrl` support for hidden and reorganized columns. X11 background rendering was optimized, and `FileSel` received improved KDE support. The framework's accessibility was further expanded with Basque (Euskara) translations.
