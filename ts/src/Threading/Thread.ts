/**
 * Thread class provides a U++-like interface for worker threads in Node.js
 * Implements threading functionality using Node.js worker_threads module
 *
 * @example
 * ```typescript
 * const thread = new Thread((data) => {
 *   console.log('Running in thread:', data);
 *   return data * 2;
 * });
 * 
 * thread.Start(5);
 * const result = await thread.WaitForResult();
 * console.log(result); // 10
 * ```
 */
// Note: Node.js worker_threads are not used in this simplified implementation
// due to complexity of serializing functions to worker threads
// For true parallelization of CPU-intensive tasks, use Node.js worker_threads directly

export class ThreadResult<T> {
    result: T | null = null;
    error: Error | null = null;

    constructor(result?: T | null, error?: Error | null) {
        this.result = result !== undefined ? result : null;
        this.error = error !== undefined ? error : null;
    }

    IsOk(): boolean {
        return this.error === null;
    }

    GetResult(): T {
        if (this.error) {
            throw this.error;
        }
        if (this.result === null) {
            throw new Error('No result available');
        }
        return this.result;
    }
}

export class Thread<T = any, R = any> {
    private task: (data: T) => R;
    private isRunning: boolean = false;
    private resultPromise: Promise<ThreadResult<R>> | null = null;
    private resolveFn: ((value: ThreadResult<R>) => void) | null = null;

    /**
     * Creates a new Thread instance
     * @param task The function to execute in a separate thread
     */
    constructor(task: (data: T) => R) {
        this.task = task;
    }

    /**
     * Starts the thread with the provided data
     * @param data Input data for the task
     * @returns This Thread instance for chaining
     */
    Start(data: T): Thread<T, R> {
        if (this.isRunning) {
            throw new Error('Thread is already running');
        }

        this.isRunning = true;

        // In JavaScript/Node.js, true thread parallelization with custom functions
        // is complex due to function serialization challenges.
        // For this implementation, we'll simulate thread behavior with async execution
        // while maintaining the U++ Thread API.

        this.resultPromise = new Promise<ThreadResult<R>>((resolve) => {
            this.resolveFn = resolve;

            // Execute the task asynchronously without blocking
            setImmediate(() => {
                try {
                    // Execute the task
                    const result = this.task(data);
                    const threadResult = new ThreadResult<R>(result, null);
                    resolve(threadResult);
                } catch (error) {
                    const threadResult = new ThreadResult<R>(null as R, error as Error);
                    resolve(threadResult);
                } finally {
                    this.isRunning = false;
                }
            });
        });

        return this;
    }

    /**
     * Waits for the thread to complete and returns the result
     * @returns Promise containing the ThreadResult
     */
    async WaitForResult(): Promise<ThreadResult<R>> {
        if (!this.resultPromise) {
            throw new Error('Thread has not been started');
        }

        return this.resultPromise;
    }

    /**
     * Checks if the thread is currently running
     * @returns Boolean indicating if the thread is running
     */
    IsRunning(): boolean {
        return this.isRunning;
    }

    /**
     * Terminates the thread if it's running
     */
    Terminate(): void {
        if (this.isRunning) {
            this.isRunning = false;

            if (this.resolveFn) {
                const threadResult = new ThreadResult<R>(null as R, new Error('Thread was terminated'));
                this.resolveFn(threadResult);
            }
        }
    }

    /**
     * Gets the ID of the thread
     * @returns Thread ID or null if not applicable
     */
    GetId(): number | null {
        // In this implementation, we don't have actual thread IDs
        // since we're simulating threading with async execution
        return null;
    }
}

/**
 * Static utility method to sleep the current thread
 * @param milliseconds Number of milliseconds to sleep
 */
export function Sleep(milliseconds: number): void {
    Atomics.wait(new Int32Array(new SharedArrayBuffer(4)), 0, 0, milliseconds);
}

/**
 * Gets the ID of the current thread
 * @returns Current thread ID
 */
export function GetCurrentThreadId(): number {
    // In JavaScript/Node.js, there's no true thread ID since JavaScript is single-threaded
    // in the main runtime. For this implementation, we return a fixed value.
    return 0; // Represent the main thread
}