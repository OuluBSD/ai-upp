/**
 * A Throttle class similar to U++ Throttle, controlling the rate at which a function can be called.
 * It ensures that a function is called at most once per specified time interval.
 * 
 * @example
 * ```typescript
 * const throttle = new Throttle(1000); // 1 second throttle
 * 
 * function executeAction() {
 *     console.log("Action executed");
 * }
 * 
 * // This will execute immediately
 * throttle.Execute(executeAction);
 * 
 * // These calls will be ignored until the throttle period expires
 * throttle.Execute(executeAction);
 * throttle.Execute(executeAction);
 * 
 * // After 1 second, this will execute again
 * setTimeout(() => {
 *     throttle.Execute(executeAction);
 * }, 1100);
 * ```
 */
export class Throttle {
    private delay: number;
    private lastExecTime: number;
    private timerId: any | null;

    /**
     * Creates a new Throttle instance.
     * @param delay The minimum time interval (in milliseconds) between function executions
     */
    constructor(delay: number) {
        this.delay = delay;
        this.lastExecTime = -delay; // Initialize to allow first execution
        this.timerId = null;
    }

    /**
     * Executes the provided function if the throttle period has passed.
     * @param func The function to execute
     * @returns True if the function was executed, false if it was throttled
     */
    Execute<T extends (...args: any[]) => any>(func: T, ...args: Parameters<T>): boolean {
        const now = Date.now();
        
        // If enough time has passed since the last execution
        if (now - this.lastExecTime >= this.delay) {
            func(...args);
            this.lastExecTime = now;
            return true;
        }
        
        return false; // Throttled
    }

    /**
     * Resets the throttle, allowing the next Execute call to succeed regardless of time.
     */
    Reset(): void {
        this.lastExecTime = 0;
        if (this.timerId !== null) {
            clearTimeout(this.timerId);
            this.timerId = null;
        }
    }

    /**
     * Gets the remaining time (in milliseconds) until the next function call is allowed.
     * @returns The remaining time in milliseconds
     */
    GetRemainingTime(): number {
        const now = Date.now();
        const elapsed = now - this.lastExecTime;
        return Math.max(0, this.delay - elapsed);
    }

    /**
     * Gets the throttle delay in milliseconds.
     * @returns The throttle delay
     */
    GetDelay(): number {
        return this.delay;
    }
}

/**
 * A more advanced Throttle class that queues function calls instead of dropping them.
 * This version will execute the last queued call after the throttle period expires.
 *
 * @example
 * ```typescript
 * const queuedThrottle = new QueuedThrottle(1000); // 1 second throttle
 *
 * function executeAction(value: number) {
 *     console.log("Action executed with value:", value);
 * }
 *
 * queuedThrottle.Execute(executeAction, 1); // Executes immediately
 * queuedThrottle.Execute(executeAction, 2); // Queued
 * queuedThrottle.Execute(executeAction, 3); // Queued - this will be executed next
 *
 * // After 1 second, executeAction(3) will be called
 * ```
 */
export class QueuedThrottle {
    private delay: number;
    private lastExecTime: number;
    private timerId: any | null;
    private queuedCall: { func: Function; args: any[] } | null;

    /**
     * Creates a new QueuedThrottle instance.
     * @param delay The minimum time interval (in milliseconds) between function executions
     */
    constructor(delay: number) {
        this.delay = delay;
        this.lastExecTime = -delay; // Initialize to allow first execution
        this.timerId = null;
        this.queuedCall = null;
    }

    /**
     * Executes the provided function if the throttle period has passed,
     * or queues it for later execution if throttled.
     * @param func The function to execute
     * @param args The arguments to pass to the function
     */
    Execute<T extends (...args: any[]) => any>(func: T, ...args: Parameters<T>): void {
        const now = Date.now();

        // If enough time has passed since the last execution
        if (now - this.lastExecTime >= this.delay) {
            func(...args);
            this.lastExecTime = now;

            // Process any previously queued call
            if (this.queuedCall) {
                const call = this.queuedCall;
                this.queuedCall = null;

                // Execute the queued function after the delay
                this.timerId = setTimeout(() => {
                    // Execute the queued function with its arguments directly
                    (call.func as Function)(...call.args);
                    // Update the last execution time after executing the queued call
                    this.lastExecTime = Date.now();
                }, this.delay);
            }
        } else {
            // If we're creating a new timeout, clear the previous one first
            if (this.timerId) {
                clearTimeout(this.timerId);
            }

            // Queue this call to execute after the delay
            this.queuedCall = { func, args };

            // Schedule the queued call to execute when the current throttle period ends
            // The call will execute at: lastExecTime + delay (when throttling period ends)
            const executeAt = this.lastExecTime + this.delay;
            const timeUntilExecution = Math.max(0, executeAt - Date.now());

            this.timerId = setTimeout(() => {
                if (this.queuedCall) {
                    const call = this.queuedCall;
                    this.queuedCall = null;
                    (call.func as Function)(...call.args);
                    this.lastExecTime = Date.now();
                }
            }, timeUntilExecution);
        }
    }

    /**
     * Resets the throttle, clearing any queued calls.
     */
    Reset(): void {
        this.lastExecTime = 0;
        if (this.timerId !== null) {
            clearTimeout(this.timerId);
            this.timerId = null;
        }
        this.queuedCall = null;
    }

    /**
     * Gets the remaining time (in milliseconds) until the next function call is allowed.
     * @returns The remaining time in milliseconds
     */
    GetRemainingTime(): number {
        const now = Date.now();
        const elapsed = now - this.lastExecTime;
        return Math.max(0, this.delay - elapsed);
    }
}