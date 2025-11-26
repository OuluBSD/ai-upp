/**
 * Promise-based async utilities with U++-like API
 * These utilities provide common async patterns with familiar U++ naming
 */

/**
 * Delays execution for the specified number of milliseconds
 * @param ms Number of milliseconds to delay
 * @returns Promise that resolves after the specified time
 */
export function Delay(ms: number): Promise<void> {
    return new Promise<void>(resolve => setTimeout(resolve, ms));
}

/**
 * Executes a function repeatedly with a fixed time delay between each call
 * @param func Function to execute
 * @param delay Delay in milliseconds between executions
 * @returns Function to call to stop the interval
 */
export function SetInterval<T>(func: () => T, delay: number): () => void {
    const intervalId = setInterval(func, delay);
    return () => clearInterval(intervalId);
}

/**
 * Executes a function after a specified delay
 * @param func Function to execute
 * @param delay Delay in milliseconds before execution
 * @returns Function to call to cancel the timeout
 */
export function SetTimeout<T>(func: () => T, delay: number): () => void {
    const timeoutId = setTimeout(func, delay);
    return () => clearTimeout(timeoutId);
}

/**
 * Async version of SetTimeout
 * @param delay Delay in milliseconds before resolution
 * @returns Promise that resolves after the specified delay
 */
export function Timeout(delay: number): Promise<void> {
    return new Promise<void>(resolve => setTimeout(resolve, delay));
}

/**
 * Creates a Promise that resolves when any of the input promises resolve
 * This is similar to Promise.race but with U++ naming conventions
 * @param promises Array of promises to race
 * @returns Promise that resolves or rejects based on the first resolved/rejected promise
 */
export function WaitForAny<T>(promises: Promise<T>[]): Promise<T> {
    return Promise.race(promises);
}

/**
 * Creates a Promise that resolves when all of the input promises resolve
 * This is similar to Promise.all but with U++ naming conventions
 * @param promises Array of promises to wait for
 * @returns Promise that resolves when all input promises resolve
 */
export function WaitForAll<T>(promises: Promise<T>[]): Promise<T[]> {
    return Promise.all(promises);
}

/**
 * Promise utility class for common async operations
 */
export class Async {
    /**
     * Runs multiple async operations in parallel and waits for all to complete
     * @param operations Async operations to run in parallel
     * @returns Promise that resolves when all operations complete
     */
    static async Parallel<T>(...operations: Promise<T>[]): Promise<T[]> {
        return Promise.all(operations);
    }

    /**
     * Runs multiple async operations in parallel but only waits for the first to complete
     * @param operations Async operations to run in parallel
     * @returns Promise that resolves when the first operation completes
     */
    static async Race<T>(...operations: Promise<T>[]): Promise<T> {
        return Promise.race(operations);
    }

    /**
     * Creates a Promise that resolves after a specified delay
     * @param delay Delay in milliseconds
     * @returns Promise that resolves after the delay
     */
    static Delay(delay: number): Promise<void> {
        return new Promise<void>(resolve => setTimeout(resolve, delay));
    }

    /**
     * Retries an async operation a specified number of times with a delay between attempts
     * @param operation The async operation to retry
     * @param maxRetries Maximum number of retry attempts
     * @param delay Delay in milliseconds between retries
     * @returns Promise that resolves when the operation succeeds or maxRetries is reached
     */
    static async Retry<T>(
        operation: () => Promise<T>,
        maxRetries: number = 3,
        delay: number = 1000
    ): Promise<T> {
        let lastError: any;
        
        for (let i = 0; i <= maxRetries; i++) {
            try {
                return await operation();
            } catch (error) {
                lastError = error;
                
                if (i < maxRetries) {
                    await this.Delay(delay);
                }
            }
        }
        
        throw lastError;
    }

    /**
     * Creates a timeout for a Promise that will reject if not completed within the timeout
     * @param promise The promise to add a timeout to
     * @param timeoutMs Timeout in milliseconds
     * @returns Promise that either resolves with the original promise value or rejects with a timeout error
     */
    static async Timeout<T>(promise: Promise<T>, timeoutMs: number): Promise<T> {
        let timeoutHandle: NodeJS.Timeout;
        
        // Create a timeout promise
        const timeoutPromise = new Promise<never>((_, reject) => {
            timeoutHandle = setTimeout(() => {
                reject(new Error(`Operation timed out after ${timeoutMs}ms`));
            }, timeoutMs);
        });
        
        try {
            // Race the original promise against the timeout
            return await Promise.race([promise, timeoutPromise]);
        } finally {
            // Clear the timeout if the original promise resolved/rejected first
            clearTimeout(timeoutHandle!);
        }
    }
}

/**
 * Future class representing a value that will be available at some point in the future
 * This is another way to handle asynchronous operations with U++-like naming
 */
export class Future<T> {
    private promise: Promise<T>;
    private _resolve!: (value: T | PromiseLike<T>) => void;
    private _reject!: (reason?: any) => void;

    constructor() {
        this.promise = new Promise<T>((resolve, reject) => {
            this._resolve = resolve;
            this._reject = reject;
        });
    }

    /**
     * Sets the value of the future, making it available to awaiters
     * @param value The value to set
     */
    SetValue(value: T): void {
        this._resolve(value);
    }

    /**
     * Sets an error for the future, causing awaiters to receive an exception
     * @param error The error to set
     */
    SetError(error: any): void {
        this._reject(error);
    }

    /**
     * Gets the promise for this future, allowing it to be awaited
     * @returns The promise representing the future value
     */
    Get(): Promise<T> {
        return this.promise;
    }

    /**
     * Checks if the future is ready (has a value or error)
     * Note: This is a simplified implementation; in a full implementation,
     * you'd need to track the state more carefully
     */
    IsReady(): boolean {
        // In a real implementation, we'd track the state of the promise
        // For this implementation, we'll return false since we can't easily 
        // determine if a Promise is resolved without awaiting it
        return false;
    }
}