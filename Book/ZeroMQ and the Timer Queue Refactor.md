# ZeroMQ and the Timer Queue Refactor
**Date Span:** 2011-03-01 to 2011-03-31

### Timer Queue and Event Stability
A major refactor of the core `Timer queue` in `CtrlCore` eliminated a significant class of recursion bugs by rescheduling repeated events *after* their callbacks execute. This fundamental change improved the stability of U++ applications under heavy load.

### High-Performance Messaging: ZeroMQ
Launched the **ZeroMQ (ZMQ)** library wrapper in the Bazaar, providing a next-generation asynchronous messaging stack. Support for request-reply and pub-sub patterns was immediately available for high-performance U++ services.

### Professional Tooling: PDF Export and SrcUpdater
TheIDE gained the ability to export entire topic groups to **PDF** and implemented unique output directories per assembly to prevent binary collisions. A new **SrcUpdater** system was introduced to automate environment synchronization.

### Bazaar Innovation: Python and LogCtrl
The Bazaar saw the first alpha signs of **Python** integration and the launch of **LogCtrl**, which allows redirecting all U++ internal logging to a visual UI control. **CtrlPos** was matured with full support for parent-child relationship editing via drag-and-drop.
