# The Indexed Container Revolution and Filter Streams
**Date Span:** 2013-02-01 to 2013-02-28

### Launch of Indexed and Sorted Containers
Introduced the **InVector**, **InArray**, and the **SortedIndex/Map** family to the `Core` library. These high-performance structures provided logarithmic access and were supported by the new **InVectorCache**, along with exhaustive memory optimizations.

### Advanced Stream Filtering
Launched **FilterStream** and **FilterStreams**, enabling transparent, layered data processing for tasks like compression and encryption. The core `Stream` interface was simultaneously refactored to support **memory blocks larger than 2GB**.

### 64-bit Parity Milestone
Achieved a major leap in 64-bit reliability across the ecosystem. Specialized packages including `Controls4U`, `Functions4U`, `ScatterCtrl`, and `OfficeAutomation` were hardened for Win64 and POSIX-64.

### Pro Tooling and Web Stability
Skylark refined its variable management logic and error handling. The database layer was hardened against **ODBC Client 11.0** crashes, and the `RegExp` plugin gained native `Replace` methods. TheIDE received an optimized console and a dedicated icon for `.ddl` files.
