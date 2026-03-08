# Headless Infrastructure and Parallel Builds
**Date Span:** 2010-04-01 to 2010-04-30

### lpbuild and Distribution Split
The `lpbuild` script was decoupled from the GUI and gained parallel build support. This enabled the Linux distribution to be split into three targeted packages: `upp`, `theide`, and `theide-nogtk`, optimizing for both desktop and server environments.

### XmlRpc and Concurrency
Resolved critical multi-threading issues in `XmlRpc` and introduced the `XmlRpcPerform` function for group-based execution. These stability fixes ensured reliable remote procedure calls under high concurrent load.

### Database Error Persistence
`SqlSession` was refactored to prioritize the recording of the first encountered error, simplifying the debugging of complex SQL transactions. `SqlArray` added the `WhenFilter` gate for real-time filtering of fetched datasets.

### UI Professionalism and Core Safety
`ArrayCtrl` received a `WhenScroll` event and better virtual selection handling. `MD5_CTX` was renamed to avoid OpenSSL conflicts, and the community site launched new French, Spanish, and Catalan versions along with an automated patch submission portal.
