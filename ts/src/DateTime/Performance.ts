/**
 * Performance timing utilities for U++-like performance measurement
 * Provides performance timing functions similar to U++ performance timing functionality
 * 
 * @example
 * ```typescript
 * const startTime = GetPerformanceCounter();
 * // ... perform some operation ...
 * const endTime = GetPerformanceCounter();
 * const duration = endTime - startTime;
 * const durationMs = CounterToMilliseconds(duration);
 * console.log(`Operation took ${durationMs}ms`);
 * 
 * const timer = new PerformanceTimer();
 * timer.Start();
 * // ... perform some operation ...
 * const elapsedMs = timer.ElapsedMilliseconds();
 * console.log(`Operation took ${elapsedMs}ms`);
 * ```
 */

// GetTickCount was already implemented in the Time class
// Here we'll add more performance timing utilities

/**
 * Represents a high-resolution timestamp for performance measurements
 */
export class PerformanceCounter {
    private readonly timestamp: number;

    constructor(timestamp?: number) {
        this.timestamp = timestamp !== undefined ? timestamp : this.getHighResolutionTimestamp();
    }

    /**
     * Gets the high-resolution timestamp
     * @returns The high-resolution timestamp
     */
    GetTimestamp(): number {
        return this.timestamp;
    }

    /**
     * Gets the high-resolution timestamp as milliseconds
     * @returns The timestamp in milliseconds
     */
    ToMilliseconds(): number {
        return this.timestamp;
    }

    /**
     * Gets the high-resolution timestamp as seconds
     * @returns The timestamp in seconds
     */
    ToSeconds(): number {
        return this.timestamp / 1000;
    }

    /**
     * Gets a high-resolution timestamp from the system
     * @returns The high-resolution timestamp
     */
    private getHighResolutionTimestamp(): number {
        // In Node.js, we can use process.hrtime for high-resolution timing
        // but for simplicity and browser compatibility, we'll use performance.now() if available
        // or fallback to Date.now()
        if (typeof performance !== 'undefined' && performance.now) {
            return performance.now();
        } else {
            return Date.now();
        }
    }
}

/**
 * A performance timer for measuring elapsed time
 */
export class PerformanceTimer {
    private startTime: PerformanceCounter | null = null;
    private endTime: PerformanceCounter | null = null;

    /**
     * Starts the performance timer
     * @returns This performance timer instance for chaining
     */
    Start(): PerformanceTimer {
        this.startTime = new PerformanceCounter();
        this.endTime = null;
        return this;
    }

    /**
     * Stops the performance timer
     * @returns This performance timer instance for chaining
     */
    Stop(): PerformanceTimer {
        this.endTime = new PerformanceCounter();
        return this;
    }

    /**
     * Gets the elapsed time in milliseconds
     * @returns The elapsed time in milliseconds
     */
    ElapsedMilliseconds(): number {
        if (!this.startTime) {
            return 0;
        }

        const end = this.endTime || new PerformanceCounter();
        return end.GetTimestamp() - this.startTime.GetTimestamp();
    }

    /**
     * Gets the elapsed time in seconds
     * @returns The elapsed time in seconds
     */
    ElapsedSeconds(): number {
        return this.ElapsedMilliseconds() / 1000;
    }

    /**
     * Checks if the timer is currently running
     * @returns true if the timer is running, false otherwise
     */
    IsRunning(): boolean {
        return this.startTime !== null && this.endTime === null;
    }

    /**
     * Restarts the timer
     * @returns This performance timer instance for chaining
     */
    Restart(): PerformanceTimer {
        this.Start();
        return this;
    }
}

/**
 * Gets the current performance counter value
 * @returns A PerformanceCounter instance for the current time
 */
export function GetPerformanceCounter(): PerformanceCounter {
    return new PerformanceCounter();
}

/**
 * Converts a performance counter difference to milliseconds
 * @param counterDiff The difference between two performance counter values
 * @returns The time in milliseconds
 */
export function CounterToMilliseconds(counterDiff: number): number {
    // In this implementation, the counter is already in milliseconds
    return counterDiff;
}

/**
 * Converts a performance counter difference to seconds
 * @param counterDiff The difference between two performance counter values
 * @returns The time in seconds
 */
export function CounterToSeconds(counterDiff: number): number {
    return counterDiff / 1000;
}

/**
 * Measures the execution time of a function
 * @param fn The function to measure
 * @returns The execution time in milliseconds
 */
export function MeasureTime<T>(fn: () => T): { result: T, timeMs: number } {
    const start = GetPerformanceCounter();
    const result = fn();
    const end = GetPerformanceCounter();
    
    const timeMs = CounterToMilliseconds(end.GetTimestamp() - start.GetTimestamp());
    return { result, timeMs };
}

/**
 * Measures the execution time of an async function
 * @param fn The async function to measure
 * @returns A promise that resolves to the result and execution time
 */
export async function MeasureTimeAsync<T>(fn: () => Promise<T>): Promise<{ result: T, timeMs: number }> {
    const start = GetPerformanceCounter();
    const result = await fn();
    const end = GetPerformanceCounter();
    
    const timeMs = CounterToMilliseconds(end.GetTimestamp() - start.GetTimestamp());
    return { result, timeMs };
}

/**
 * A stopwatch for measuring elapsed time
 */
export class Stopwatch {
    private startTime: number | null = null;
    private elapsedTime: number = 0;
    private isRunning: boolean = false;

    /**
     * Starts the stopwatch
     */
    Start(): void {
        if (!this.isRunning) {
            this.startTime = Date.now();
            this.isRunning = true;
        }
    }

    /**
     * Stops the stopwatch
     */
    Stop(): void {
        if (this.isRunning) {
            this.elapsedTime += Date.now() - this.startTime!;
            this.isRunning = false;
        }
    }

    /**
     * Resets the stopwatch
     */
    Reset(): void {
        this.startTime = null;
        this.elapsedTime = 0;
        this.isRunning = false;
    }

    /**
     * Gets the elapsed time in milliseconds
     * @returns The elapsed time in milliseconds
     */
    ElapsedMilliseconds(): number {
        if (this.isRunning && this.startTime !== null) {
            return this.elapsedTime + (Date.now() - this.startTime);
        }
        return this.elapsedTime;
    }

    /**
     * Gets the elapsed time in seconds
     * @returns The elapsed time in seconds
     */
    ElapsedSeconds(): number {
        return this.ElapsedMilliseconds() / 1000;
    }

    /**
     * Checks if the stopwatch is running
     * @returns true if the stopwatch is running, false otherwise
     */
    IsRunning(): boolean {
        return this.isRunning;
    }

    /**
     * Gets the elapsed time as a Duration object
     * @returns A Duration object representing the elapsed time
     */
    Elapsed(): any { // Using any to avoid circular dependency
        // In a real implementation, we'd need to handle this differently to avoid circular imports
        return { milliseconds: this.ElapsedMilliseconds() };
    }
}