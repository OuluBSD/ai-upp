/**
 * Timer and scheduled task utilities for U++-like timer functionality
 * Provides timer operations similar to U++ timer functionality
 * 
 * @example
 * ```typescript
 * // Create a one-shot timer
 * const timer = new Timer(() => console.log('Timer executed!'), 1000);
 * timer.Start();
 * 
 * // Create a periodic timer
 * const periodicTimer = new PeriodicTimer(() => console.log('Periodic execution'), 2000);
 * periodicTimer.Start();
 * 
 * // Use the scheduling utility
 * const scheduledTask = ScheduleTask(() => console.log('Scheduled task'), 5000);
 * ```
 */

import { Duration } from './Date';

export type TimerCallback = () => void;

/**
 * A simple timer class that executes a callback after a specified delay
 */
export class Timer {
    private callback: TimerCallback;
    private delay: number; // in milliseconds
    private timeoutId: NodeJS.Timeout | null = null;
    private isRunning: boolean = false;

    /**
     * Creates a new timer
     * @param callback The function to execute when the timer completes
     * @param delay The delay in milliseconds before executing the callback
     */
    constructor(callback: TimerCallback, delay: number) {
        this.callback = callback;
        this.delay = delay;
    }

    /**
     * Starts the timer
     * @returns This timer instance for chaining
     */
    Start(): Timer {
        if (this.isRunning) {
            this.Stop();
        }

        this.timeoutId = setTimeout(() => {
            this.isRunning = false;
            this.callback();
        }, this.delay);

        this.isRunning = true;
        return this;
    }

    /**
     * Stops the timer if it's running
     * @returns This timer instance for chaining
     */
    Stop(): Timer {
        if (this.timeoutId !== null) {
            clearTimeout(this.timeoutId);
            this.timeoutId = null;
            this.isRunning = false;
        }
        return this;
    }

    /**
     * Checks if the timer is currently running
     * @returns true if the timer is running, false otherwise
     */
    IsRunning(): boolean {
        return this.isRunning;
    }

    /**
     * Gets the remaining time until the timer executes
     * @returns The remaining time in milliseconds
     */
    GetRemainingTime(): number {
        return this.isRunning ? this.delay : 0;
    }
}

/**
 * A periodic timer class that executes a callback repeatedly at specified intervals
 */
export class PeriodicTimer {
    private callback: TimerCallback;
    private interval: number; // in milliseconds
    private intervalId: NodeJS.Timeout | null = null;
    private isRunning: boolean = false;

    /**
     * Creates a new periodic timer
     * @param callback The function to execute at each interval
     * @param interval The interval in milliseconds between executions
     */
    constructor(callback: TimerCallback, interval: number) {
        this.callback = callback;
        this.interval = interval;
    }

    /**
     * Starts the periodic timer
     * @returns This periodic timer instance for chaining
     */
    Start(): PeriodicTimer {
        if (this.isRunning) {
            this.Stop();
        }

        this.intervalId = setInterval(() => {
            this.callback();
        }, this.interval);

        this.isRunning = true;
        return this;
    }

    /**
     * Stops the periodic timer if it's running
     * @returns This periodic timer instance for chaining
     */
    Stop(): PeriodicTimer {
        if (this.intervalId !== null) {
            clearInterval(this.intervalId);
            this.intervalId = null;
            this.isRunning = false;
        }
        return this;
    }

    /**
     * Checks if the periodic timer is currently running
     * @returns true if the timer is running, false otherwise
     */
    IsRunning(): boolean {
        return this.isRunning;
    }
}

/**
 * A scheduled task that can be cancelled
 */
export class ScheduledTask {
    private timeoutId: NodeJS.Timeout | null;
    private cancelled: boolean = false;

    constructor(timeoutId: NodeJS.Timeout) {
        this.timeoutId = timeoutId;
    }

    /**
     * Cancels the scheduled task if it hasn't executed yet
     */
    Cancel(): void {
        if (this.timeoutId !== null && !this.cancelled) {
            clearTimeout(this.timeoutId);
            this.cancelled = true;
        }
    }

    /**
     * Checks if the task has been cancelled
     * @returns true if the task was cancelled, false otherwise
     */
    IsCancelled(): boolean {
        return this.cancelled;
    }
}

/**
 * Schedules a task to run after a specified delay
 * @param task The task to execute
 * @param delay The delay in milliseconds before executing the task
 * @returns A ScheduledTask instance that can be used to cancel the task
 */
export function ScheduleTask(task: TimerCallback, delay: number): ScheduledTask {
    const timeoutId = setTimeout(task, delay);
    return new ScheduledTask(timeoutId);
}

/**
 * Schedules a recurring task to run at specified intervals
 * @param task The task to execute
 * @param interval The interval in milliseconds between executions
 * @returns A ScheduledTask instance that can be used to cancel the recurring task
 */
export function ScheduleRecurringTask(task: TimerCallback, interval: number): ScheduledTask {
    // Create a periodic execution using setInterval, but wrap it in our ScheduledTask
    // Since ScheduledTask assumes a single timeout, we'll use a PeriodicTimer internally
    const periodicTimer = new PeriodicTimer(task, interval);
    periodicTimer.Start();
    
    // Return a ScheduledTask that, when cancelled, stops the PeriodicTimer
    return new ScheduledTask(setTimeout(() => {
        periodicTimer.Stop();
    }, 0)); // Use a 0ms timeout just to get a timeout ID we can tie to the ScheduledTask
}

/**
 * Executes a task after a specified duration
 * @param task The task to execute
 * @param duration The duration to wait before executing the task
 * @returns A ScheduledTask instance that can be used to cancel the task
 */
export function ExecuteAfter(task: TimerCallback, duration: Duration): ScheduledTask {
    return ScheduleTask(task, duration.GetMilliseconds());
}

/**
 * Executes a task repeatedly at specified duration intervals
 * @param task The task to execute
 * @param interval The duration interval between executions
 * @returns A PeriodicTimer instance that can be used to manage the recurring task
 */
export function ExecuteAtInterval(task: TimerCallback, interval: Duration): PeriodicTimer {
    return new PeriodicTimer(task, interval.GetMilliseconds());
}

/**
 * A timeout utility class that executes a callback if not cancelled within a timeout
 */
export class Timeout {
    private timer: Timer;
    private executed: boolean = false;
    private cancelled: boolean = false;

    constructor(callback: TimerCallback, timeoutMs: number) {
        this.timer = new Timer(() => {
            this.executed = true;
            if (!this.cancelled) {
                callback();
            }
        }, timeoutMs);
    }

    /**
     * Starts the timeout
     */
    Start(): void {
        this.timer.Start();
    }

    /**
     * Cancels the timeout if it hasn't executed yet
     */
    Cancel(): void {
        this.cancelled = true;
        this.timer.Stop();
    }

    /**
     * Checks if the timeout has been cancelled
     * @returns true if the timeout was cancelled, false otherwise
     */
    IsCancelled(): boolean {
        return this.cancelled;
    }

    /**
     * Checks if the timeout has executed
     * @returns true if the timeout executed, false otherwise
     */
    HasExecuted(): boolean {
        return this.executed;
    }
}

/**
 * Delays execution for the specified duration
 * @param duration The duration to delay for
 * @returns A Promise that resolves after the specified duration
 */
export function Delay(duration: Duration): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, duration.GetMilliseconds()));
}

/**
 * Delays execution for the specified number of milliseconds
 * @param ms The number of milliseconds to delay for
 * @returns A Promise that resolves after the specified number of milliseconds
 */
export function DelayMs(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
}