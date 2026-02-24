# Single Instances and Secure Networking
**Date Span:** 2011-02-01 to 2011-02-28

### Single Instance Apps: Uniq
Introduced the **Uniq** package, providing a cross-platform mechanism to ensure only one instance of an application runs at a time. Utilizing overlapped I/O on Windows and specialized sockets on Linux, it enabled seamless argument passing between instances.

### Hardened Networking and SSL
The `Socket` class was refactored for per-instance error handling, replacing legacy global state. This significantly improved the reliability of the **HttpsClient**, which gained specialized error logic for SSL and better end-of-file detection.

### UI Interoperability and X11
`RichEdit` gained the ability to export single-object selections to the system clipboard in QTF and RTF formats. X11 support was bolstered with the "urgency hint" for demanding user attention in modern window managers.

### StaticPlugin and Bazaar Growth
Launched the **StaticPlugin** architecture in `Functions4U`, simplifying modular dependency management. This was used to refurbish the `OfficeAutomation` package. The community site was also updated with an extensive roadmap for the GSoC 2011 season.
