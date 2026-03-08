# The Multi-Threaded Draw Refactor and Native D&D
**Date Span:** 2012-11-01 to 2012-11-30

### Graphics Hardening: MT Draw Refactor
Embarked on a major architectural renovation of the `Draw` and `CtrlCore` subsystems, specifically targeting multi-threading (MT) race conditions. Introduced specialized ASSERTs to detect global variable widgets and missing `GuiLock` calls, significantly improving UI stability.

### Native Shell Integration
Landed full support for **native file lists in the clipboard and Drag & Drop** across Win32 and X11. This enabled U++ applications to seamlessly exchange file data with system explorers and other productivity software.

### Database and Serialization Cleanups
`SqlExp` gained the ability to perform `Insert` and `Update` operations using **ValueMap parameters**, streamlining bulk data mapping. `Xmlize` and `Jsonize` methods were moved directly into container classes for a cleaner, method-based API.

### Tooling and Bazaar Growth
TheIDE added a "Mimic case" find option and improved line-number accuracy for database schemas in Assist++. The **OCE** (OpenCascade) package was cleaned and synced with its upstream source, and **HelpViewer** added standard navigation controls.
