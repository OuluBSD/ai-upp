/**
 * Semaphore class for synchronization between threads
 * A semaphore manages a set of permits, allowing a limited number of threads
 * to access a resource simultaneously
 * 
 * @example
 * ```typescript
 * const semaphore = new Semaphore(2); // Allow 2 threads at a time
 * 
 * async function accessResource() {
 *   await semaphore.Wait(); // Acquire a permit
 *   try {
 *     // Critical section code
 *   } finally {
 *     semaphore.Signal(); // Release a permit
 *   }
 * }
 * ```
 */
export class Semaphore {
    private availablePermits: number;
    private maxPermits: number;
    private waitingQueue: Array<() => void> = [];

    /**
     * Creates a new Semaphore with the specified number of permits
     * @param permits Number of permits (threads that can access the resource simultaneously)
     */
    constructor(permits: number) {
        this.availablePermits = permits;
        this.maxPermits = permits;
    }

    /**
     * Acquires a permit from the semaphore, blocking if necessary until one is available
     * @returns Promise that resolves when a permit is acquired
     */
    async Wait(): Promise<void> {
        return new Promise<void>((resolve) => {
            if (this.availablePermits > 0) {
                // If permits are available, acquire one directly
                this.availablePermits--;
                resolve();
            } else {
                // Otherwise, add to waiting queue
                this.waitingQueue.push(resolve);
            }
        });
    }

    /**
     * Releases a permit, returning it to the semaphore
     */
    Signal(): void {
        if (this.waitingQueue.length > 0) {
            // If threads are waiting, release the next one
            const next = this.waitingQueue.shift();
            if (next) {
                next();
            }
        } else {
            // Otherwise, increment available permits (up to max)
            if (this.availablePermits < this.maxPermits) {
                this.availablePermits++;
            }
        }
    }

    /**
     * Attempts to acquire a permit without blocking
     * @returns true if a permit was acquired, false otherwise
     */
    TryWait(): boolean {
        if (this.availablePermits > 0) {
            this.availablePermits--;
            return true;
        }
        return false;
    }

    /**
     * Gets the number of permits currently available
     * @returns Number of available permits
     */
    GetAvailablePermits(): number {
        return this.availablePermits;
    }

    /**
     * Gets the total number of permits in the semaphore
     * @returns Total number of permits
     */
    GetTotalPermits(): number {
        return this.maxPermits;
    }
}

/**
 * Event class for thread synchronization
 * An event is a synchronization object that can be in either a signaled or non-signaled state
 * Threads can wait for the event to become signaled
 * 
 * @example
 * ```typescript
 * const event = new Event(false); // Initially non-signaled
 * 
 * // In one thread:
 * await event.WaitFor(); // Will block until event is signaled
 * 
 * // In another thread:
 * event.Set(); // Signal the event, unblocking waiting threads
 * ```
 */
export class SyncEvent {
    private isSignaled: boolean;
    private waitingQueue: Array<() => void> = [];

    /**
     * Creates a new Event
     * @param initialState Initial state of the event (signaled or non-signaled)
     */
    constructor(initialState: boolean = false) {
        this.isSignaled = initialState;
    }

    /**
     * Waits for the event to become signaled
     * @returns Promise that resolves when the event is signaled
     */
    async WaitFor(): Promise<void> {
        return new Promise<void>((resolve) => {
            if (this.isSignaled) {
                // If already signaled, resolve immediately
                resolve();
            } else {
                // Otherwise, add to waiting queue
                this.waitingQueue.push(resolve);
            }
        });
    }

    /**
     * Sets the event to the signaled state
     * This will unblock all threads waiting on the event
     */
    Set(): void {
        this.isSignaled = true;
        
        // Release all waiting threads
        while (this.waitingQueue.length > 0) {
            const next = this.waitingQueue.shift();
            if (next) {
                next();
            }
        }
    }

    /**
     * Resets the event to the non-signaled state
     */
    Reset(): void {
        this.isSignaled = false;
    }

    /**
     * Pulse the event, temporarily setting it to signaled and then back to non-signaled
     * This releases exactly one thread waiting on the event
     */
    Pulse(): void {
        // If there's a waiting thread, release just that one
        if (this.waitingQueue.length > 0) {
            const next = this.waitingQueue.shift();
            if (next) {
                next();
            }
        }
        // Otherwise, the pulse has no effect since no threads are waiting
    }

    /**
     * Checks if the event is currently signaled
     * @returns true if the event is signaled, false otherwise
     */
    IsSignaled(): boolean {
        return this.isSignaled;
    }
}