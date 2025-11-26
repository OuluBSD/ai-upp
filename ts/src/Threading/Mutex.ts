/**
 * Mutex (Mutual Exclusion) class for synchronization between threads
 * This implementation uses async-mutex library to provide mutex functionality
 * in a U++-like API
 * 
 * @example
 * ```typescript
 * const mutex = new Mutex();
 * 
 * async function criticalSection() {
 *   const release = await mutex.Lock();
 *   try {
 *     // Critical section code here
 *     console.log('In critical section');
 *   } finally {
 *     release(); // Unlock
 *   }
 * }
 * ```
 */

// Note: For a more complete implementation, we would typically use a library like 'async-mutex'
// For this U++-like interface, we'll implement a simple mutex using Promise-based locking

export class Mutex {
    private _locked: boolean = false;
    private _waiting: Array<() => void> = [];

    /**
     * Acquires the mutex lock
     * If the mutex is already locked, the calling thread will block until the mutex is unlocked
     * @returns A function that must be called to unlock the mutex
     */
    async Lock(): Promise<() => void> {
        return new Promise<() => void>((resolve) => {
            const proceed = () => {
                this._locked = true;
                resolve(() => {
                    this._locked = false;

                    // If there are waiting threads, let the next one acquire the lock
                    if (this._waiting.length > 0) {
                        const next = this._waiting.shift();
                        if (next) {
                            next();
                        }
                    }
                });
            };

            if (!this._locked) {
                proceed();
            } else {
                this._waiting.push(proceed);
            }
        });
    }

    /**
     * Attempts to acquire the mutex lock without blocking
     * @returns A function that must be called to unlock the mutex if the lock was acquired,
     *          or null if the lock was not acquired
     */
    TryLock(): (() => void) | null {
        if (!this._locked) {
            this._locked = true;
            return () => this.Unlock();
        }
        return null;
    }

    /**
     * Unlocks the mutex
     * The mutex must be unlocked by the same thread that locked it
     */
    private Unlock(): void {
        if (this._waiting.length > 0) {
            // Execute the next waiting lock request
            const next = this._waiting.shift();
            if (next) {
                next();
            }
        } else {
            this._locked = false;
        }
    }

    /**
     * Checks if the mutex is currently locked
     * @returns Boolean indicating if the mutex is locked
     */
    IsLocked(): boolean {
        return this._locked;
    }
}

/**
 * RecursiveMutex class allows the same thread to lock the mutex multiple times
 * This is useful for recursive function calls that need to acquire the same lock
 */
export class RecursiveMutex {
    private _ownerThreadId: number | null = null;
    private _lockCount: number = 0;
    private _waiting: Array<{ resolve: (release: () => void) => void, threadId: number }> = [];

    /**
     * Acquires the recursive mutex lock
     * Same thread can acquire this lock multiple times
     * @returns A function that must be called to unlock the mutex
     */
    async Lock(): Promise<() => void> {
        const currentThreadId = GetCurrentThreadId(); // Assuming this function exists

        return new Promise<() => void>((resolve) => {
            const attemptAcquire = () => {
                // If no one owns the lock or if we already own it, acquire it
                if (this._ownerThreadId === null || this._ownerThreadId === currentThreadId) {
                    this._ownerThreadId = currentThreadId;
                    this._lockCount++;
                    resolve(() => this.Unlock());
                } else {
                    // Someone else owns the lock, wait for it
                    this._waiting.push({ resolve, threadId: currentThreadId });
                }
            };

            attemptAcquire();
        });
    }

    /**
     * Attempts to acquire the recursive mutex lock without blocking
     * @returns A function that must be called to unlock the mutex if the lock was acquired,
     *          or null if the lock was not acquired
     */
    TryLock(): (() => void) | null {
        const currentThreadId = GetCurrentThreadId(); // Assuming this function exists

        if (this._ownerThreadId === null || this._ownerThreadId === currentThreadId) {
            this._ownerThreadId = currentThreadId;
            this._lockCount++;
            return () => this.Unlock();
        }
        return null;
    }

    /**
     * Unlocks the recursive mutex
     * Must be called the same number of times as Lock() was called
     */
    private Unlock(): void {
        if (this._lockCount > 0) {
            this._lockCount--;
            
            // If lock count is 0, release the lock completely
            if (this._lockCount === 0) {
                this._ownerThreadId = null;
                
                // If there are waiting threads, let the next one acquire the lock
                if (this._waiting.length > 0) {
                    const next = this._waiting.shift();
                    if (next) {
                        // Need to call the next waiting thread's lock function
                        // This is slightly tricky - we'll just trigger the next attempt
                        const { resolve, threadId } = next;
                        this._ownerThreadId = threadId;
                        this._lockCount = 1;
                        resolve(() => this.Unlock());
                    }
                }
            }
        }
    }

    /**
     * Checks if the mutex is currently locked
     * @returns Boolean indicating if the mutex is locked
     */
    IsLocked(): boolean {
        return this._lockCount > 0;
    }
}

// Placeholder for GetCurrentThreadId - may be implemented in Thread.ts or elsewhere
function GetCurrentThreadId(): number {
    // In a real implementation, this would return the actual current thread ID
    // Since JavaScript is single-threaded in the main runtime, we'll return a fixed value
    // In a worker thread context, we'd return the appropriate worker thread ID
    return 0;
}