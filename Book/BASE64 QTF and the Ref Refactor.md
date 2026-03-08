# BASE64 QTF and the Ref Refactor
**Date Span:** 2013-11-01 to 2013-11-30

### Data Portability: BASE64 QTF
The **QTF (Quick Text Format)** engine was modernized to use **BASE64 encoding for embedded objects**, significantly improving the portability of documents containing images or binary data. This was supported by a major **Ref refactor**, providing unified reference handling for `ValueMap`, `ValueArray`, and `Complex` types.

### TheIDE Visual Data Tools
Introduced professional **JsonView and XmlView** interactive inspectors within TheIDE. **Assist++** was expanded to support `.iml` (image list) files, and the **Layout Designer** gained more flexible code generation options, including mandatory double-quoting for UI elements.

### Core Time and Configuration
The core library added **AlwaysTime** and **DayEnd** helpers for precise temporal boundaries. The **INI system** was enhanced with default content management, and **XML indentation** was refactored for better human readability.

### UI and PDF Optimization
`RichTextView` gained **triple-click selection**, and `RichEdit` added support for **Null cell backgrounds**. The **PdfDraw** engine implemented critical **image compression** for repeated assets, reducing the footprint of document-heavy exports.
