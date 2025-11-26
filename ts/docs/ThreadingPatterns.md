/**
 * Threading and Async Patterns Documentation
 * 
 * This document describes the threading and async patterns implemented in the uppts library,
 * which provide U++-like functionality for concurrent programming in TypeScript/JavaScript.
 * 
 * The threading module includes classes and utilities for:
 * 1. Thread management and execution
 * 2. Synchronization primitives (Mutex, Semaphore, Event)
 * 3. Parallel processing (CoWork, ThreadPool)
 * 4. Async/await utilities that match U++ patterns
 * 5. Thread-safe container wrappers
 * 
 * Each class is fully documented with TypeDoc comments in the source code.
 */

// ================
// 1. THREAD CLASS
// ================

/**
 * The Thread class provides a U++-like interface for worker threads in Node.js.
 * It allows executing functions in separate threads to perform CPU-intensive tasks
 * without blocking the main thread.
 * 
 * Usage:
 * ```typescript
 * const thread = new Thread((data) => {
 *   // Perform computation on data
 *   return data * 2;
 * });
 * 
 * thread.Start(5);
 * const result = await thread.WaitForResult();
 * console.log(result.GetResult()); // 10 (in a real implementation)
 * ```
 * 
 * Key methods:
 * - Start(data): Starts the thread with the provided data
 * - WaitForResult(): Asynchronously waits for the result
 * - IsRunning(): Checks if the thread is still running
 * - Terminate(): Terminates the thread
 * - GetId(): Gets the thread ID
 * 
 * Static utilities:
 * - Sleep(milliseconds): Sleeps the current thread
 * - GetCurrentThreadId(): Gets the ID of the current thread
 */

// ================
// 2. SYNCHRONIZATION PRIMITIVES
// ================

/**
 * Mutex (Mutual Exclusion):
 * Ensures only one thread can access a critical section at a time.
 * 
 * Usage:
 * ```typescript
 * const mutex = new Mutex();
 * 
 * async function criticalSection() {
 *   const release = await mutex.Lock();
 *   try {
 *     // Critical section code
 *   } finally {
 *     release(); // Unlock
 *   }
 * }
 * ```
 * 
 * RecursiveMutex allows the same thread to lock the mutex multiple times.
 * 
 * Semaphore:
 * Controls access to a resource with a limited number of permits.
 * 
 * Usage:
 * ```typescript
 * const semaphore = new Semaphore(2); // Allow 2 threads at a time
 * 
 * async function accessResource() {
 *   await semaphore.Wait(); // Acquire a permit
 *   try {
 *     // Access the resource
 *   } finally {
 *     semaphore.Signal(); // Release the permit
 *   }
 * }
 * ```
 * 
 * SyncEvent:
 * A synchronization object that can be in either a signaled or non-signaled state.
 *
 * Usage:
 * ```typescript
 * const event = new SyncEvent(false); // Initially non-signaled
 *
 * // In one thread:
 * await event.WaitFor(); // Will block until event is signaled
 *
 * // In another thread:
 * event.Set(); // Signal the event, unblocking waiting threads
 * ```
 */

// ================
// 3. PARALLEL PROCESSING
// ================

/**
 * CoWork:
 * The CoWork class allows for parallel processing of work items.
 * It's similar to U++'s CoWork functionality.
 * 
 * Usage:
 * ```typescript
 * const coWork = new CoWork<number, number>(2); // Use 2 threads
 * coWork.Add(1, 2, 3, 4, 5);
 * 
 * const results = await coWork.Execute((n) => n * 2);
 * // Results: [2, 4, 6, 8, 10]
 * ```
 * 
 * ThreadPool:
 * Manages a pool of reusable threads to execute tasks efficiently.
 * 
 * Usage:
 * ```typescript
 * const pool = new ThreadPool(4); // Create a pool with 4 threads
 * 
 * // Submit tasks to the pool
 * const result1 = pool.Submit(() => {
 *   // Some work here
 *   return 'result1';
 * });
 * 
 * const result2 = pool.Submit(() => {
 *   // Some other work
 *   return 'result2';
 * });
 * 
 * // Wait for results
 * console.log(await result1); // 'result1'
 * console.log(await result2); // 'result2'
 * 
 * // Shutdown the pool when done
 * pool.Shutdown();
 * ```
 */

// ================
// 4. ASYNC/AWAIT UTILITIES
// ================

/**
 * The library provides various async/await utilities with U++-like naming:
 * 
 * Async.Delay(ms): Delays execution for the specified milliseconds
 * Async.Parallel(...ops): Runs operations in parallel
 * Async.Race(...ops): Returns result of the first completing operation
 * Async.Retry(func, maxRetries, delay): Retries a function on failure
 * Async.Timeout(promise, timeoutMs): Times out a promise
 * 
 * Future<T>: Represents a value that will be available in the future
 * 
 * Additional helpers:
 * - AsyncCoWork<T, R>: Async version of CoWork
 * - AsyncParallel<T>: Runs multiple functions in parallel
 * - AsyncFirst<T>: Waits for the first function to complete
 * - AsyncFor, AsyncForEach, AsyncMap, AsyncFilter: Async versions of array operations
 */

// ================
// 5. THREAD-SAFE CONTAINERS
// ================

/**
 * The library provides thread-safe wrappers for common containers:
 * - SynchronizedVector<T>: Thread-safe Vector wrapper
 * - SynchronizedArray<T>: Thread-safe Array wrapper
 * - SynchronizedIndex<T>: Thread-safe Index wrapper
 * - SynchronizedMap<K, V>: Thread-safe Map wrapper
 * - Synchronized<T>: Generic wrapper for any object
 * 
 * All these wrappers use a mutex internally to ensure thread-safe access.
 * 
 * Usage:
 * ```typescript
 * const vector = new SynchronizedVector<number>();
 * 
 * await vector.Add(42);
 * const count = await vector.GetCount();
 * const value = await vector.At(0);
 * ```
 * 
 * These containers provide the same interface as their non-thread-safe counterparts
 * but with async methods to handle the potential blocking on locks.
 */

// ================
// BEST PRACTICES
// ================

/**
 * 1. Always release locks in a finally block to prevent deadlocks:
 * ```typescript
 * const release = await mutex.Lock();
 * try {
 *   // Critical section
 * } finally {
 *   release();
 * }
 * ```
 * 
 * 2. Use thread pools for managing multiple tasks efficiently:
 * - Avoid creating too many threads
 * - Use appropriate pool size based on CPU cores and task nature
 * 
 * 3. Be aware of the JavaScript single-threaded event loop:
 * - These threading utilities may not provide true parallelism
 * - For CPU-intensive tasks, consider using Node.js worker_threads directly
 * 
 * 4. Use async/await consistently to avoid blocking the event loop:
 * - Use AsyncFor instead of regular for loops for large iterations
 * - Yield control periodically in long-running operations
 * 
 * 5. Handle errors appropriately in concurrent code:
 * - Wrap async operations in try/catch blocks
 * - Use proper error handling when working with multiple threads
 */

// ================
// LIMITATIONS
// ================

/**
 * 1. JavaScript/Node.js limitations:
 * - JavaScript has a single-threaded event loop
 * - True parallelism is limited compared to native U++ threading
 * - Serialization of functions to worker threads is complex
 * 
 * 2. Implementation considerations:
 * - The Thread class implementation in this example uses eval for simplicity
 * - In production, you'd want to implement proper function serialization
 * - Performance characteristics may differ from native U++ implementations
 * 
 * 3. Testing concurrency:
 * - Testing concurrent code can be challenging
 * - Race conditions may be difficult to reproduce
 * - Use the provided test suite as a reference for testing patterns
 */

// ================
// MIGRATION FROM U++
// ================

/**
 * Developers migrating from U++ to uppts should note:
 * 
 * 1. Synchronization:
 * - U++ Mutex → uppts Mutex (with async/await)
 * - U++ Semaphore → uppts Semaphore (with async/await)
 * - U++ Event → uppts Event (with async/await)
 * 
 * 2. Threading:
 * - U++ Thread → uppts Thread (with async result handling)
 * - U++ CoWork → uppts CoWork (with async execution)
 * 
 * 3. Containers:
 * - U++ Vector → uppts Vector (same interface, async synchronized versions available)
 * - Similar for Array, Index, Map, etc.
 * 
 * 4. Async patterns:
 * - Use async/await for operations that would block in U++
 * - Handle Promises instead of synchronous operations
 */