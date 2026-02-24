# Outlook Automation and the SetLayout Evolution
**Date Span:** 2011-05-01 to 2011-05-31

### Outlook Interactivity: MAPIEx
Launched the **MAPIEx** package for Outlook automation, enabling U++ applications to manage emails, folders, and address books directly. This was supported by a comprehensive demo and full MinGW compatibility.

### Modern UI Definitions: SetLayout_
Introduced the **SetLayout_** template functions, providing a type-safe, automatically generated alternative to the legacy `Layout` macros. This significantly modernized the way U++ developers interact with `.lay` files.

### Scripting and Ownership in Bazaar
Matured the **Py** (Python) integration with unidirectional callback exposure and `TopWindow` exports. The `Gen` package introduced the `Shared<T>` template for standardized reference-counted ownership.

### Core Architecture and Rainbow
`Xmlize` was enhanced to support custom tag names for container items, and `SqlCtrl` gained the `SqlNOption` control. Initial development of the **Rainbow** backend architecture began, signaling a long-term push toward unified GUI abstraction.
