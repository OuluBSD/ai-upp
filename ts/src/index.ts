/**
 * uppts - U++ TypeScript Interface
 *
 * Main entry point for the library.
 * Exports all public APIs.
 */

// Core containers (Phase 2)
export { Vector } from './Core/Vector';
export { Array } from './Core/Array';
export { Index } from './Core/Index';
export { Map } from './Core/Map';
export { BiVector } from './Core/BiVector';

// String handling (Phase 3)
export { String } from './Core/String';
export { WString } from './Core/WString';
export { StringBuffer } from './Core/StringBuffer';

// Smart pointers (Phase 4)
export { One, Pick } from './Core/One';
export { Ptr, WeakPtr } from './Core/Ptr';

// Utilities (Phase 5)
export { Tuple } from './Core/Tuple';
export { Optional } from './Core/Optional';
export { Value, Variant } from './Core/Value';
export { Function } from './Core/Function';
export { Callback } from './Core/Callback';
export { Event } from './Core/Event';
export { Gate } from './Core/Gate';
export { Throttle, QueuedThrottle } from './Core/Throttle';
export * from './Core/Algorithms';

// I/O (Phase 6)
export { Stream } from './IO/Stream';
export { FileIn } from './IO/FileIn';
export { FileOut } from './IO/FileOut';
export { StringStream } from './IO/StringStream';
export { FileSystem, FindFile } from './IO/FileSystem';
export { Path } from './IO/Path';

// Threading (to be implemented in Phase 7)
// export { Thread } from './Threading/Thread';
// export { Mutex } from './Threading/Mutex';

// DateTime (to be implemented in Phase 8)
// export { Time } from './DateTime/Time';
// export { Date } from './DateTime/Date';

// Network (to be implemented in Phase 9)
// export { HttpRequest } from './Network/HttpRequest';
// export { TcpSocket } from './Network/TcpSocket';

// Placeholder export for Phase 1
export const VERSION = '0.1.0';
